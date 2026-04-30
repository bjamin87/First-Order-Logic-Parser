/**
 * @file BaselineProver.cpp
 */

#include "BaselineProver.hpp"
#include "Sequent.hpp"
#include <chrono>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <iostream>

namespace {

// =================================================
// Config
// =================================================

struct RuleConfig {
    std::string mode = "heuristic"; // manual, heuristic, disabled
    int priority = 100;
    int weight = 0;
};

struct BaselineConfig {
    std::string profileName = "default";
    int stepLimit = 5000;

    int close_bonus = 100;
    int non_branching_bonus = 50;
    int branching_penalty = 20;
    int used_term_bonus = 20;
    int fresh_term_penalty = 10;
    int complexity_penalty = 1;
    int quantifier_bonus = 10;
    int implication_bonus = 15;
    int conjunction_bonus = 12;
    int disjunction_penalty = 8;

    std::map<std::string, RuleConfig> rules;
};

BaselineConfig loadBaselineConfig(const std::string& filename) {
    BaselineConfig config;

    std::ifstream in(filename);
    if (!in.is_open()) {
        return config;
    }

    auto trim = [](std::string& s) {
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) {
            s.pop_back();
        }

        std::size_t start = 0;
        while (start < s.size() && (s[start] == ' ' || s[start] == '\t')) {
            ++start;
        }

        if (start > 0) {
            s.erase(0, start);
        }
    };

    std::string line;
    while (std::getline(in, line)) {
        trim(line);

        if (line.empty()) {
            continue;
        }

        std::size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        trim(key);
        trim(value);

        if (key == "profile_name") config.profileName = value;
        else if (key == "step_limit") config.stepLimit = std::stoi(value);
        else if (key == "close_bonus") config.close_bonus = std::stoi(value);
        else if (key == "non_branching_bonus") config.non_branching_bonus = std::stoi(value);
        else if (key == "branching_penalty") config.branching_penalty = std::stoi(value);
        else if (key == "used_term_bonus") config.used_term_bonus = std::stoi(value);
        else if (key == "fresh_term_penalty") config.fresh_term_penalty = std::stoi(value);
        else if (key == "complexity_penalty") config.complexity_penalty = std::stoi(value);
        else if (key == "quantifier_bonus") config.quantifier_bonus = std::stoi(value);
        else if (key == "implication_bonus") config.implication_bonus = std::stoi(value);
        else if (key == "conjunction_bonus") config.conjunction_bonus = std::stoi(value);
        else if (key == "disjunction_penalty") config.disjunction_penalty = std::stoi(value);
        else {
            std::size_t dot = key.find('.');
            if (dot != std::string::npos) {
                std::string ruleName = key.substr(0, dot);
                std::string field = key.substr(dot + 1);

                trim(ruleName);
                trim(field);

                RuleConfig& rc = config.rules[ruleName];

                if (field == "mode") rc.mode = value;
                else if (field == "priority") rc.priority = std::stoi(value);
                else if (field == "weight") rc.weight = std::stoi(value);
            }
        }
    }

    return config;
}

RuleConfig getRuleConfig(const BaselineConfig& cfg, const std::string& ruleName) {
    auto it = cfg.rules.find(ruleName);
    if (it != cfg.rules.end()) {
        return it->second;
    }
    return RuleConfig{};
}

bool isManualRule(const BaselineConfig& cfg, const std::string& ruleName) {
    return getRuleConfig(cfg, ruleName).mode == "manual";
}

bool isHeuristicRule(const BaselineConfig& cfg, const std::string& ruleName) {
    return getRuleConfig(cfg, ruleName).mode == "heuristic";
}

bool isDisabledRule(const BaselineConfig& cfg, const std::string& ruleName) {
    return getRuleConfig(cfg, ruleName).mode == "disabled";
}

// =================================================
// Helpers
// =================================================

std::string freshConstantBaseline() {
    static int counter = 0;
    ++counter;
    return "ic" + std::to_string(counter);
}

std::string makeInstantiationKeyBaseline(
    const Formula* formula,
    const std::string& term,
    const std::string& side
) {
    return side + "::" + formula->toString() + "::" + term;
}

int formulaComplexity(const Formula& formula) {
    switch (formula.getType()) {
        case FormulaType::Atom:
        case FormulaType::True:
        case FormulaType::False:
            return 1;
        case FormulaType::Not:
            return 1 + formulaComplexity(*formula.getOperand());
        case FormulaType::And:
        case FormulaType::Or:
        case FormulaType::Implies:
            return 1
                + formulaComplexity(*formula.getLeft())
                + formulaComplexity(*formula.getRight());
        case FormulaType::Forall:
        case FormulaType::Exists:
            return 1 + formulaComplexity(*formula.getBody());
    }

    return 1;
}

int sequentComplexity(const Sequent& sequent) {
    int total = 0;

    for (const Formula* f : sequent.getLeft()) {
        if (f != nullptr) {
            total += formulaComplexity(*f);
        }
    }

    for (const Formula* f : sequent.getRight()) {
        if (f != nullptr) {
            total += formulaComplexity(*f);
        }
    }

    return total;
}

// =================================================
// Rule option
// =================================================

struct RuleOption {
    std::string ruleName;
    bool branching = false;
    int priority = 100;

    std::vector<Sequent> nextSequents;
    std::set<std::string> nextUsedInstantiations;

    int score = 0;
};

int scoreRuleOption(const RuleOption& option, const BaselineConfig& cfg) {
    int score = 0;

    RuleConfig rc = getRuleConfig(cfg, option.ruleName);
    score += rc.weight;

    if (!option.branching) {
        score += cfg.non_branching_bonus;
    } else {
        score -= cfg.branching_penalty;
    }

    if (option.ruleName.find("used") != std::string::npos) {
        score += cfg.used_term_bonus;
    }

    if (option.ruleName.find("fresh") != std::string::npos) {
        score -= cfg.fresh_term_penalty;
    }

    if (option.ruleName == "impR" || option.ruleName == "impL") {
        score += cfg.implication_bonus;
    }

    if (option.ruleName == "andL" || option.ruleName == "andR") {
        score += cfg.conjunction_bonus;
    }

    if (option.ruleName == "orL" ||
        option.ruleName == "orR_left" ||
        option.ruleName == "orR_right") {
        score -= cfg.disjunction_penalty;
    }

    if (option.ruleName == "forallR" || option.ruleName == "existsL" ||
        option.ruleName == "forallL_used" || option.ruleName == "forallL_fresh" ||
        option.ruleName == "existsR_used" || option.ruleName == "existsR_fresh") {
        score += cfg.quantifier_bonus;
    }

    int futureComplexity = 0;
    for (const Sequent& s : option.nextSequents) {
        futureComplexity += sequentComplexity(s);
    }

    score -= futureComplexity * cfg.complexity_penalty;

    return score;
}

// =================================================
// Collect ALL rule options
// =================================================

std::vector<RuleOption> collectAllOptions(

    const Sequent& sequent,
    const std::set<std::string>& usedInstantiations,
    const BaselineConfig& cfg
) {
    // std::cout << "collectAllOptions on: " << sequent.toString() << "\n";
    std::vector<RuleOption> options;

    // impR
    if (!isDisabledRule(cfg, "impR")) {
        for (std::size_t i = 0; i < sequent.getRight().size(); ++i) {
            const Formula* formula = sequent.getRight()[i];
            if (formula && formula->getType() == FormulaType::Implies) {
                const Formula* A = formula->getLeft();
                const Formula* B = formula->getRight();

                auto newLeft = sequent.getLeft();
                auto newRight = sequent.getRight();

                newLeft.push_back(A);
                newRight.erase(newRight.begin() + static_cast<long long>(i));
                newRight.push_back(B);

                Sequent next = sequent;
                next.setLeft(newLeft);
                next.setRight(newRight);

                RuleOption o;
                o.ruleName = "impR";
                o.branching = false;
                o.priority = getRuleConfig(cfg, "impR").priority;
                o.nextSequents.push_back(next);
                o.nextUsedInstantiations = usedInstantiations;
                o.score = scoreRuleOption(o, cfg);
                //std::cout << "Generated impR option\n";   // Debuging
                options.push_back(o);
            }
        }
    }

    // andL
    if (!isDisabledRule(cfg, "andL")) {
        for (std::size_t i = 0; i < sequent.getLeft().size(); ++i) {
            const Formula* formula = sequent.getLeft()[i];
            if (formula && formula->getType() == FormulaType::And) {
                const Formula* A = formula->getLeft();
                const Formula* B = formula->getRight();

                auto newLeft = sequent.getLeft();
                auto newRight = sequent.getRight();

                newLeft.erase(newLeft.begin() + static_cast<long long>(i));
                newLeft.push_back(A);
                newLeft.push_back(B);

                Sequent next = sequent;
                next.setLeft(newLeft);
                next.setRight(newRight);

                RuleOption o;
                o.ruleName = "andL";
                o.branching = false;
                o.priority = getRuleConfig(cfg, "andL").priority;
                o.nextSequents.push_back(next);
                o.nextUsedInstantiations = usedInstantiations;
                o.score = scoreRuleOption(o, cfg);
                options.push_back(o);
            }
        }
    }

    // orR_left / orR_right
    if (!isDisabledRule(cfg, "orR_left") || !isDisabledRule(cfg, "orR_right")) {
        for (std::size_t i = 0; i < sequent.getRight().size(); ++i) {
            const Formula* formula = sequent.getRight()[i];
            if (formula != nullptr && formula->getType() == FormulaType::Or) {
                const Formula* A = formula->getLeft();
                const Formula* B = formula->getRight();

                if (!isDisabledRule(cfg, "orR_left")) {
                    auto newLeft = sequent.getLeft();
                    auto newRight = sequent.getRight();
                    newRight.erase(newRight.begin() + static_cast<long long>(i));
                    newRight.push_back(A);

                    Sequent next = sequent;
                    next.setLeft(newLeft);
                    next.setRight(newRight);

                    RuleOption o;
                    o.ruleName = "orR_left";
                    o.branching = false;
                    o.priority = getRuleConfig(cfg, "orR_left").priority;
                    o.nextSequents.push_back(next);
                    o.nextUsedInstantiations = usedInstantiations;
                    o.score = scoreRuleOption(o, cfg);
                    options.push_back(o);
                }

                if (!isDisabledRule(cfg, "orR_right")) {
                    auto newLeft = sequent.getLeft();
                    auto newRight = sequent.getRight();
                    newRight.erase(newRight.begin() + static_cast<long long>(i));
                    newRight.push_back(B);

                    Sequent next = sequent;
                    next.setLeft(newLeft);
                    next.setRight(newRight);

                    RuleOption o;
                    o.ruleName = "orR_right";
                    o.branching = false;
                    o.priority = getRuleConfig(cfg, "orR_right").priority;
                    o.nextSequents.push_back(next);
                    o.nextUsedInstantiations = usedInstantiations;
                    o.score = scoreRuleOption(o, cfg);
                    options.push_back(o);
                }
            }
        }
    }

    // notL
    if (!isDisabledRule(cfg, "notL")) {
        for (std::size_t i = 0; i < sequent.getLeft().size(); ++i) {
            const Formula* formula = sequent.getLeft()[i];
            if (formula && formula->getType() == FormulaType::Not) {
                const Formula* A = formula->getOperand();

                auto newLeft = sequent.getLeft();
                auto newRight = sequent.getRight();

                newLeft.erase(newLeft.begin() + static_cast<long long>(i));
                newRight.push_back(A);

                Sequent next = sequent;
                next.setLeft(newLeft);
                next.setRight(newRight);

                RuleOption o;
                o.ruleName = "notL";
                o.branching = false;
                o.priority = getRuleConfig(cfg, "notL").priority;
                o.nextSequents.push_back(next);
                o.nextUsedInstantiations = usedInstantiations;
                o.score = scoreRuleOption(o, cfg);
                options.push_back(o);
            }
        }
    }

    // notR
    if (!isDisabledRule(cfg, "notR")) {
        for (std::size_t i = 0; i < sequent.getRight().size(); ++i) {
            const Formula* formula = sequent.getRight()[i];
            if (formula && formula->getType() == FormulaType::Not) {
                const Formula* A = formula->getOperand();

                auto newLeft = sequent.getLeft();
                auto newRight = sequent.getRight();

                newRight.erase(newRight.begin() + static_cast<long long>(i));
                newLeft.push_back(A);

                Sequent next = sequent;
                next.setLeft(newLeft);
                next.setRight(newRight);

                RuleOption o;
                o.ruleName = "notR";
                o.branching = false;
                o.priority = getRuleConfig(cfg, "notR").priority;
                o.nextSequents.push_back(next);
                o.nextUsedInstantiations = usedInstantiations;
                o.score = scoreRuleOption(o, cfg);
                options.push_back(o);
            }
        }
    }

    // forallR
    if (!isDisabledRule(cfg, "forallR")) {
        for (std::size_t i = 0; i < sequent.getRight().size(); ++i) {
            const Formula* formula = sequent.getRight()[i];
            if (formula && formula->getType() == FormulaType::Forall) {
                const std::string* var = formula->getVariable();
                const Formula* body = formula->getBody();

                if (var != nullptr && body != nullptr) {
                    std::string c = freshConstantBaseline();

                    auto newLeft = sequent.getLeft();
                    auto newRight = sequent.getRight();
                    newRight.erase(newRight.begin() + static_cast<long long>(i));

                    Sequent next = sequent;
                    next.setLeft(newLeft);
                    next.setRight(newRight);
                    next.addOwnedRight(body->substitute(*var, c));

                    RuleOption o;
                    o.ruleName = "forallR";
                    o.branching = false;
                    o.priority = getRuleConfig(cfg, "forallR").priority;
                    o.nextSequents.push_back(next);
                    o.nextUsedInstantiations = usedInstantiations;
                    o.score = scoreRuleOption(o, cfg);
                    options.push_back(o);
                }
            }
        }
    }

    // existsL
    if (!isDisabledRule(cfg, "existsL")) {
        for (std::size_t i = 0; i < sequent.getLeft().size(); ++i) {
            const Formula* formula = sequent.getLeft()[i];
            if (formula && formula->getType() == FormulaType::Exists) {
                const std::string* var = formula->getVariable();
                const Formula* body = formula->getBody();

                if (var != nullptr && body != nullptr) {
                    std::string c = freshConstantBaseline();

                    auto newLeft = sequent.getLeft();
                    auto newRight = sequent.getRight();
                    newLeft.erase(newLeft.begin() + static_cast<long long>(i));

                    Sequent next = sequent;
                    next.setLeft(newLeft);
                    next.setRight(newRight);
                    next.addOwnedLeft(body->substitute(*var, c));

                    RuleOption o;
                    o.ruleName = "existsL";
                    o.branching = false;
                    o.priority = getRuleConfig(cfg, "existsL").priority;
                    o.nextSequents.push_back(next);
                    o.nextUsedInstantiations = usedInstantiations;
                    o.score = scoreRuleOption(o, cfg);
                    options.push_back(o);
                }
            }
        }
    }

    // andR
    if (!isDisabledRule(cfg, "andR")) {
        for (std::size_t i = 0; i < sequent.getRight().size(); ++i) {
            const Formula* formula = sequent.getRight()[i];
            if (formula != nullptr && formula->getType() == FormulaType::And) {
                const Formula* A = formula->getLeft();
                const Formula* B = formula->getRight();

                auto left1 = sequent.getLeft();
                auto right1 = sequent.getRight();
                right1.erase(right1.begin() + static_cast<long long>(i));
                right1.push_back(A);

                auto left2 = sequent.getLeft();
                auto right2 = sequent.getRight();
                right2.erase(right2.begin() + static_cast<long long>(i));
                right2.push_back(B);

                Sequent branch1 = sequent;
                branch1.setLeft(left1);
                branch1.setRight(right1);

                Sequent branch2 = sequent;
                branch2.setLeft(left2);
                branch2.setRight(right2);

                RuleOption o;
                o.ruleName = "andR";
                o.branching = true;
                o.priority = getRuleConfig(cfg, "andR").priority;
                o.nextSequents.push_back(branch1);
                o.nextSequents.push_back(branch2);
                o.nextUsedInstantiations = usedInstantiations;
                o.score = scoreRuleOption(o, cfg);
                options.push_back(o);
            }
        }
    }

    // orL
    if (!isDisabledRule(cfg, "orL")) {
        for (std::size_t i = 0; i < sequent.getLeft().size(); ++i) {
            const Formula* formula = sequent.getLeft()[i];
            if (formula != nullptr && formula->getType() == FormulaType::Or) {
                const Formula* A = formula->getLeft();
                const Formula* B = formula->getRight();

                auto left1 = sequent.getLeft();
                auto right1 = sequent.getRight();
                left1.erase(left1.begin() + static_cast<long long>(i));
                left1.push_back(A);

                auto left2 = sequent.getLeft();
                auto right2 = sequent.getRight();
                left2.erase(left2.begin() + static_cast<long long>(i));
                left2.push_back(B);

                Sequent branch1 = sequent;
                branch1.setLeft(left1);
                branch1.setRight(right1);

                Sequent branch2 = sequent;
                branch2.setLeft(left2);
                branch2.setRight(right2);

                RuleOption o;
                o.ruleName = "orL";
                o.branching = true;
                o.priority = getRuleConfig(cfg, "orL").priority;
                o.nextSequents.push_back(branch1);
                o.nextSequents.push_back(branch2);
                o.nextUsedInstantiations = usedInstantiations;
                o.score = scoreRuleOption(o, cfg);
                options.push_back(o);
            }
        }
    }

    // impL
    if (!isDisabledRule(cfg, "impL")) {
        for (std::size_t i = 0; i < sequent.getLeft().size(); ++i) {
            const Formula* formula = sequent.getLeft()[i];
            if (formula != nullptr && formula->getType() == FormulaType::Implies) {
                const Formula* A = formula->getLeft();
                const Formula* B = formula->getRight();

                auto left1 = sequent.getLeft();
                auto right1 = sequent.getRight();
                left1.erase(left1.begin() + static_cast<long long>(i));
                right1.push_back(A);

                auto left2 = sequent.getLeft();
                auto right2 = sequent.getRight();
                left2.erase(left2.begin() + static_cast<long long>(i));
                left2.push_back(B);

                Sequent branch1 = sequent;
                branch1.setLeft(left1);
                branch1.setRight(right1);

                Sequent branch2 = sequent;
                branch2.setLeft(left2);
                branch2.setRight(right2);

                RuleOption o;
                o.ruleName = "impL";
                o.branching = true;
                o.priority = getRuleConfig(cfg, "impL").priority;
                o.nextSequents.push_back(branch1);
                o.nextSequents.push_back(branch2);
                o.nextUsedInstantiations = usedInstantiations;
                o.score = scoreRuleOption(o, cfg);
                options.push_back(o);
            }
        }
    }

    {
        std::vector<std::string> terms = sequent.getAllTerms();

        // forallL_used
        if (!isDisabledRule(cfg, "forallL_used")) {
            for (std::size_t i = 0; i < sequent.getLeft().size(); ++i) {
                const Formula* formula = sequent.getLeft()[i];
                if (formula != nullptr && formula->getType() == FormulaType::Forall) {
                    const std::string* var = formula->getVariable();
                    const Formula* body = formula->getBody();

                    if (var != nullptr && body != nullptr) {
                        for (const std::string& term : terms) {
                            std::string key = makeInstantiationKeyBaseline(formula, term, "forallL");
                            if (usedInstantiations.find(key) != usedInstantiations.end()) {
                                continue;
                            }

                            auto newLeft = sequent.getLeft();
                            auto newRight = sequent.getRight();
                            newLeft.erase(newLeft.begin() + static_cast<long long>(i));

                            Sequent next = sequent;
                            next.setLeft(newLeft);
                            next.setRight(newRight);
                            next.addOwnedLeft(body->substitute(*var, term));

                            auto nextInst = usedInstantiations;
                            nextInst.insert(key);

                            RuleOption o;
                            o.ruleName = "forallL_used";
                            o.branching = false;
                            o.priority = getRuleConfig(cfg, "forallL_used").priority;
                            o.nextSequents.push_back(next);
                            o.nextUsedInstantiations = nextInst;
                            o.score = scoreRuleOption(o, cfg);
                            options.push_back(o);
                        }
                    }
                }
            }
        }

        // existsR_used
        if (!isDisabledRule(cfg, "existsR_used")) {
            for (std::size_t i = 0; i < sequent.getRight().size(); ++i) {
                const Formula* formula = sequent.getRight()[i];
                if (formula != nullptr && formula->getType() == FormulaType::Exists) {
                    const std::string* var = formula->getVariable();
                    const Formula* body = formula->getBody();

                    if (var != nullptr && body != nullptr) {
                        for (const std::string& term : terms) {
                            std::string key = makeInstantiationKeyBaseline(formula, term, "existsR");
                            if (usedInstantiations.find(key) != usedInstantiations.end()) {
                                continue;
                            }

                            auto newLeft = sequent.getLeft();
                            auto newRight = sequent.getRight();
                            newRight.erase(newRight.begin() + static_cast<long long>(i));

                            Sequent next = sequent;
                            next.setLeft(newLeft);
                            next.setRight(newRight);
                            next.addOwnedRight(body->substitute(*var, term));

                            auto nextInst = usedInstantiations;
                            nextInst.insert(key);

                            RuleOption o;
                            o.ruleName = "existsR_used";
                            o.branching = false;
                            o.priority = getRuleConfig(cfg, "existsR_used").priority;
                            o.nextSequents.push_back(next);
                            o.nextUsedInstantiations = nextInst;
                            o.score = scoreRuleOption(o, cfg);
                            options.push_back(o);
                        }
                    }
                }
            }
        }
    }

    // forallL_fresh
    if (!isDisabledRule(cfg, "forallL_fresh")) {
        for (std::size_t i = 0; i < sequent.getLeft().size(); ++i) {
            const Formula* formula = sequent.getLeft()[i];
            if (formula != nullptr && formula->getType() == FormulaType::Forall) {
                const std::string* var = formula->getVariable();
                const Formula* body = formula->getBody();

                if (var != nullptr && body != nullptr) {
                    std::string freshKey = makeInstantiationKeyBaseline(formula, "__fresh__", "forallL");
                    if (usedInstantiations.find(freshKey) != usedInstantiations.end()) {
                        continue;
                    }

                    std::string c = freshConstantBaseline();

                    auto newLeft = sequent.getLeft();
                    auto newRight = sequent.getRight();
                    newLeft.erase(newLeft.begin() + static_cast<long long>(i));

                    Sequent next = sequent;
                    next.setLeft(newLeft);
                    next.setRight(newRight);
                    next.addOwnedLeft(body->substitute(*var, c));

                    auto nextInst = usedInstantiations;
                    nextInst.insert(freshKey);

                    RuleOption o;
                    o.ruleName = "forallL_fresh";
                    o.branching = false;
                    o.priority = getRuleConfig(cfg, "forallL_fresh").priority;
                    o.nextSequents.push_back(next);
                    o.nextUsedInstantiations = nextInst;
                    o.score = scoreRuleOption(o, cfg);
                    options.push_back(o);
                }
            }
        }
    }

    // existsR_fresh
    if (!isDisabledRule(cfg, "existsR_fresh")) {
        for (std::size_t i = 0; i < sequent.getRight().size(); ++i) {
            const Formula* formula = sequent.getRight()[i];
            if (formula != nullptr && formula->getType() == FormulaType::Exists) {
                const std::string* var = formula->getVariable();
                const Formula* body = formula->getBody();

                if (var != nullptr && body != nullptr) {
                    std::string freshKey = makeInstantiationKeyBaseline(formula, "__fresh__", "existsR");
                    if (usedInstantiations.find(freshKey) != usedInstantiations.end()) {
                        continue;
                    }

                    std::string c = freshConstantBaseline();

                    auto newLeft = sequent.getLeft();
                    auto newRight = sequent.getRight();
                    newRight.erase(newRight.begin() + static_cast<long long>(i));

                    Sequent next = sequent;
                    next.setLeft(newLeft);
                    next.setRight(newRight);
                    next.addOwnedRight(body->substitute(*var, c));

                    auto nextInst = usedInstantiations;
                    nextInst.insert(freshKey);

                    RuleOption o;
                    o.ruleName = "existsR_fresh";
                    o.branching = false;
                    o.priority = getRuleConfig(cfg, "existsR_fresh").priority;
                    o.nextSequents.push_back(next);
                    o.nextUsedInstantiations = nextInst;
                    o.score = scoreRuleOption(o, cfg);
                    options.push_back(o);
                }
            }
        }
    }

    return options;
}

std::vector<RuleOption> collectManualOptions(
    const Sequent& sequent,
    const std::set<std::string>& usedInstantiations,
    const BaselineConfig& cfg
) {
    std::vector<RuleOption> all = collectAllOptions(sequent, usedInstantiations, cfg);
    std::vector<RuleOption> manual;

    for (const RuleOption& option : all) {
        if (isManualRule(cfg, option.ruleName)) {
            manual.push_back(option);
        }
    }

    std::sort(manual.begin(), manual.end(),
        [](const RuleOption& a, const RuleOption& b) {
            if (a.priority == b.priority) {
                return a.score > b.score;
            }
            return a.priority < b.priority;
        });

    return manual;
}

std::vector<RuleOption> collectHeuristicOptions(
    const Sequent& sequent,
    const std::set<std::string>& usedInstantiations,
    const BaselineConfig& cfg
) {
    std::vector<RuleOption> all = collectAllOptions(sequent, usedInstantiations, cfg);
    std::vector<RuleOption> heuristic;

    for (const RuleOption& option : all) {
        if (isHeuristicRule(cfg, option.ruleName)) {
            heuristic.push_back(option);
        }
    }

    std::sort(heuristic.begin(), heuristic.end(),
        [](const RuleOption& a, const RuleOption& b) {
            if (a.score == b.score) {
                return a.priority < b.priority;
            }
            return a.score > b.score;
        });

    return heuristic;
}

// =================================================
// Baseline search
// =================================================

bool proveSequentBaseline(
    const Sequent& sequent,
    long long& steps,
    long long& branches,
    std::set<std::string> usedInstantiations,
    std::set<std::string>& visited,
    std::vector<std::string>& trace,
    const BaselineConfig& cfg
) {
    ++steps;

    if (steps > cfg.stepLimit) {
        trace.push_back("Abort: step limit reached");
        return false;
    }

    std::string stateKey = sequent.toString();
    if (!visited.insert(stateKey).second) {
        trace.push_back("Loop detected");
        return false;
    }

    if (sequent.isClosedById()) {
        trace.push_back("Apply id: Branch Closes");
        return true;
    }

    for (const Formula* f : sequent.getRight()) {
        if (f != nullptr && f->getType() == FormulaType::True) {
            trace.push_back("Apply ⊤R: Branch Closes");
            return true;
        }
    }

    for (const Formula* f : sequent.getLeft()) {
        if (f != nullptr && f->getType() == FormulaType::False) {
            trace.push_back("Apply ⊥L: Branch Closes");
            return true;
        }
    }

    // std::cout << "Sequent: " << sequent.toString() << "\n"; /debuging

    std::vector<RuleOption> manualOptions =
        collectManualOptions(sequent, usedInstantiations, cfg);

//      std::cout << "manualOptions.size() = " << manualOptions.size() << "\n";
//      for (const auto& opt : manualOptions) {
//          std::cout << "  manual: " << opt.ruleName << "\n";
// }

for (const RuleOption& option : manualOptions) {
    trace.push_back("Manual rule: " + option.ruleName);

    if (!option.branching) {
        std::set<std::string> nextVisited = visited;

        if (proveSequentBaseline(
                option.nextSequents[0],
                steps,
                branches,
                option.nextUsedInstantiations,
                nextVisited,
                trace,
                cfg)) {
            return true;
        }
    } else {
    branches += static_cast<long long>(option.nextSequents.size());

    trace.push_back("Apply branching rule: " + option.ruleName);

    bool allSucceeded = true;
    int branchIndex = 1;

    for (const Sequent& branch : option.nextSequents) {
        trace.push_back("  Enter branch " + std::to_string(branchIndex));

        std::set<std::string> nextVisited = visited;

        if (!proveSequentBaseline(
                branch,
                steps,
                branches,
                option.nextUsedInstantiations,
                nextVisited,
                trace,
                cfg)) {

            trace.push_back("  Branch " + std::to_string(branchIndex) + " FAILED");

            allSucceeded = false;
            break;
        }

        trace.push_back("  Branch " + std::to_string(branchIndex) + " CLOSED");
        branchIndex++;
    }

    if (allSucceeded) {
        trace.push_back("All branches closed for: " + option.ruleName);
        return true;
    }
}
}

    std::vector<RuleOption> options =
        collectHeuristicOptions(sequent, usedInstantiations, cfg);

    // std::cout << "heuristicOptions.size() = " << options.size() << "\n";
    // for (const auto& opt : options) {
    //     std::cout << "  heuristic: " << opt.ruleName << "\n";


    for (const RuleOption& option : options) {
        trace.push_back("Try rule: " + option.ruleName);

        if (!option.branching) {
            std::set<std::string> nextVisited = visited;

            if (proveSequentBaseline(
                option.nextSequents[0],
                steps,
                branches,
                option.nextUsedInstantiations,
                nextVisited,
                trace,
                cfg)) {
            return true;
    }

        } else {
    branches += static_cast<long long>(option.nextSequents.size());

    bool allSucceeded = true;
    for (const Sequent& branch : option.nextSequents) {
        std::set<std::string> nextVisited = visited;

        if (!proveSequentBaseline(
                branch,
                steps,
                branches,
                option.nextUsedInstantiations,
                nextVisited,
                trace,
                cfg)) {
            allSucceeded = false;
            break;
        }
    }

    if (allSucceeded) {
        return true;
    }
}
    }

    return false;
}

} // namespace

// =================================================
// Public API
// =================================================

std::string BaselineProver::getName() const {
    return "BaselineProver";
}

ProofResult BaselineProver::prove(ProofState& state) {
    auto start = std::chrono::high_resolution_clock::now();

    BaselineConfig cfg = loadBaselineConfig("baseline_heuristics.txt");

    // std::cout << "Profile: [" << cfg.profileName << "]\n";
    // std::cout << "impR mode: [" << getRuleConfig(cfg, "impR").mode << "]\n";
    // std::cout << "andL mode: [" << getRuleConfig(cfg, "andL").mode << "]\n";
    // std::cout << "orR_left mode: [" << getRuleConfig(cfg, "orR_left").mode << "]\n";

    const Formula& formula = state.getFormula();
    Sequent sequent({}, { &formula });

    long long steps = 0;
    long long branches = 1;
    std::set<std::string> usedInstantiations;
    std::set<std::string> visited;
    std::vector<std::string> trace;

    bool proved = proveSequentBaseline(
        sequent,
        steps,
        branches,
        usedInstantiations,
        visited,
        trace,
        cfg
    );

    auto end = std::chrono::high_resolution_clock::now();
    double runtimeMs =
        std::chrono::duration<double, std::milli>(end - start).count();

    std::string message = proved
        ? "Baseline (" + cfg.profileName + "): hybrid search succeeded"
        : "Baseline (" + cfg.profileName + "): hybrid search failed";

    state.incrementSteps(steps);
    state.incrementBranches(branches);

    return ProofResult(proved, steps, branches, runtimeMs, message, trace);
}
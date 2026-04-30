/**
 * @file Formula.cpp
 */

#include "Formula.hpp"
#include <utility>

// AtomFormula
AtomFormula::AtomFormula(const std::string& name)
    : name(name) {}

AtomFormula::AtomFormula(const std::string& name, const std::vector<std::string>& arguments)
    : name(name), arguments(arguments) {}

std::string AtomFormula::toString() const {
    if (arguments.empty()) {
        return name;
    }

    std::string result = name + "(";

    for (std::size_t i = 0; i < arguments.size(); ++i) {
        result += arguments[i];
        if (i + 1 < arguments.size()) {
            result += ",";
        }
    }

    result += ")";
    return result;
}

FormulaType AtomFormula::getType() const {
    return FormulaType::Atom;
}

bool AtomFormula::equals(const Formula& other) const {
    if (other.getType() != FormulaType::Atom) {
        return false;
    }

    const AtomFormula& rhs = static_cast<const AtomFormula&>(other);
    return name == rhs.name && arguments == rhs.arguments;
}

std::unique_ptr<Formula> AtomFormula::clone() const {
    return std::make_unique<AtomFormula>(name, arguments);
}

std::unique_ptr<Formula> AtomFormula::substitute(
    const std::string& variable,
    const std::string& replacement
) const {
    std::vector<std::string> newArgs = arguments;

    for (std::string& arg : newArgs) {
        if (arg == variable) {
            arg = replacement;
        }
    }

    return std::make_unique<AtomFormula>(name, newArgs);
}

void AtomFormula::collectTerms(
    std::set<std::string>& terms,
    std::set<std::string>& boundVariables
) const {
    for (const std::string& arg : arguments) {
        if (boundVariables.find(arg) == boundVariables.end()) {
            terms.insert(arg);
        }
    }
}

// NotFormula
NotFormula::NotFormula(std::unique_ptr<Formula> operand)
    : operand(std::move(operand)) {}

std::string NotFormula::toString() const {
    return "not (" + operand->toString() + ")";
}

FormulaType NotFormula::getType() const {
    return FormulaType::Not;
}

bool NotFormula::equals(const Formula& other) const {
    if (other.getType() != FormulaType::Not) {
        return false;
    }

    const NotFormula& rhs = static_cast<const NotFormula&>(other);
    return operand->equals(*rhs.operand);
}

std::unique_ptr<Formula> NotFormula::clone() const {
    return std::make_unique<NotFormula>(operand->clone());
}

std::unique_ptr<Formula> NotFormula::substitute(
    const std::string& variable,
    const std::string& replacement
) const {
    return std::make_unique<NotFormula>(
        operand->substitute(variable, replacement)
    );
}

void NotFormula::collectTerms(
    std::set<std::string>& terms,
    std::set<std::string>& boundVariables
) const {
    operand->collectTerms(terms, boundVariables);
}

const Formula* NotFormula::getOperand() const {
    return operand.get();
}

// AndFormula
AndFormula::AndFormula(std::unique_ptr<Formula> left,
                       std::unique_ptr<Formula> right)
    : left(std::move(left)), right(std::move(right)) {}

std::string AndFormula::toString() const {
    return "(" + left->toString() + " and " + right->toString() + ")";
}

FormulaType AndFormula::getType() const {
    return FormulaType::And;
}

bool AndFormula::equals(const Formula& other) const {
    if (other.getType() != FormulaType::And) {
        return false;
    }

    const AndFormula& rhs = static_cast<const AndFormula&>(other);
    return left->equals(*rhs.left) && right->equals(*rhs.right);
}

std::unique_ptr<Formula> AndFormula::clone() const {
    return std::make_unique<AndFormula>(left->clone(), right->clone());
}

std::unique_ptr<Formula> AndFormula::substitute(
    const std::string& variable,
    const std::string& replacement
) const {
    return std::make_unique<AndFormula>(
        left->substitute(variable, replacement),
        right->substitute(variable, replacement)
    );
}

void AndFormula::collectTerms(
    std::set<std::string>& terms,
    std::set<std::string>& boundVariables
) const {
    left->collectTerms(terms, boundVariables);
    right->collectTerms(terms, boundVariables);
}

const Formula* AndFormula::getLeft() const {
    return left.get();
}

const Formula* AndFormula::getRight() const {
    return right.get();
}

// OrFormula
OrFormula::OrFormula(std::unique_ptr<Formula> left,
                     std::unique_ptr<Formula> right)
    : left(std::move(left)), right(std::move(right)) {}

std::string OrFormula::toString() const {
    return "(" + left->toString() + " or " + right->toString() + ")";
}

FormulaType OrFormula::getType() const {
    return FormulaType::Or;
}

bool OrFormula::equals(const Formula& other) const {
    if (other.getType() != FormulaType::Or) {
        return false;
    }

    const OrFormula& rhs = static_cast<const OrFormula&>(other);
    return left->equals(*rhs.left) && right->equals(*rhs.right);
}

std::unique_ptr<Formula> OrFormula::clone() const {
    return std::make_unique<OrFormula>(left->clone(), right->clone());
}

std::unique_ptr<Formula> OrFormula::substitute(
    const std::string& variable,
    const std::string& replacement
) const {
    return std::make_unique<OrFormula>(
        left->substitute(variable, replacement),
        right->substitute(variable, replacement)
    );
}

void OrFormula::collectTerms(
    std::set<std::string>& terms,
    std::set<std::string>& boundVariables
) const {
    left->collectTerms(terms, boundVariables);
    right->collectTerms(terms, boundVariables);
}

const Formula* OrFormula::getLeft() const {
    return left.get();
}

const Formula* OrFormula::getRight() const {
    return right.get();
}

// ImpliesFormula
ImpliesFormula::ImpliesFormula(std::unique_ptr<Formula> left,
                               std::unique_ptr<Formula> right)
    : left(std::move(left)), right(std::move(right)) {}

std::string ImpliesFormula::toString() const {
    return "(" + left->toString() + " -> " + right->toString() + ")";
}

FormulaType ImpliesFormula::getType() const {
    return FormulaType::Implies;
}

bool ImpliesFormula::equals(const Formula& other) const {
    if (other.getType() != FormulaType::Implies) {
        return false;
    }

    const ImpliesFormula& rhs = static_cast<const ImpliesFormula&>(other);
    return left->equals(*rhs.left) && right->equals(*rhs.right);
}

std::unique_ptr<Formula> ImpliesFormula::clone() const {
    return std::make_unique<ImpliesFormula>(left->clone(), right->clone());
}

std::unique_ptr<Formula> ImpliesFormula::substitute(
    const std::string& variable,
    const std::string& replacement
) const {
    return std::make_unique<ImpliesFormula>(
        left->substitute(variable, replacement),
        right->substitute(variable, replacement)
    );
}

void ImpliesFormula::collectTerms(
    std::set<std::string>& terms,
    std::set<std::string>& boundVariables
) const {
    left->collectTerms(terms, boundVariables);
    right->collectTerms(terms, boundVariables);
}

const Formula* ImpliesFormula::getLeft() const {
    return left.get();
}

const Formula* ImpliesFormula::getRight() const {
    return right.get();
}

// ForallFormula
ForallFormula::ForallFormula(const std::string& variable,
                             std::unique_ptr<Formula> body)
    : variable(variable), body(std::move(body)) {}

std::string ForallFormula::toString() const {
    return "forall " + variable + " (" + body->toString() + ")";
}

FormulaType ForallFormula::getType() const {
    return FormulaType::Forall;
}

bool ForallFormula::equals(const Formula& other) const {
    if (other.getType() != FormulaType::Forall) {
        return false;
    }

    const ForallFormula& rhs = static_cast<const ForallFormula&>(other);
    return variable == rhs.variable && body->equals(*rhs.body);
}

std::unique_ptr<Formula> ForallFormula::clone() const {
    return std::make_unique<ForallFormula>(variable, body->clone());
}

std::unique_ptr<Formula> ForallFormula::substitute(
    const std::string& variableToReplace,
    const std::string& replacement
) const {
    if (variable == variableToReplace) {
        return clone();
    }

    return std::make_unique<ForallFormula>(
        variable,
        body->substitute(variableToReplace, replacement)
    );
}

void ForallFormula::collectTerms(
    std::set<std::string>& terms,
    std::set<std::string>& boundVariables
) const {
    boundVariables.insert(variable);
    body->collectTerms(terms, boundVariables);
    boundVariables.erase(variable);
}

const Formula* ForallFormula::getBody() const {
    return body.get();
}

const std::string* ForallFormula::getVariable() const {
    return &variable;
}

// ExistsFormula
ExistsFormula::ExistsFormula(const std::string& variable,
                             std::unique_ptr<Formula> body)
    : variable(variable), body(std::move(body)) {}

std::string ExistsFormula::toString() const {
    return "exists " + variable + " (" + body->toString() + ")";
}

FormulaType ExistsFormula::getType() const {
    return FormulaType::Exists;
}

bool ExistsFormula::equals(const Formula& other) const {
    if (other.getType() != FormulaType::Exists) {
        return false;
    }

    const ExistsFormula& rhs = static_cast<const ExistsFormula&>(other);
    return variable == rhs.variable && body->equals(*rhs.body);
}

std::unique_ptr<Formula> ExistsFormula::clone() const {
    return std::make_unique<ExistsFormula>(variable, body->clone());
}

std::unique_ptr<Formula> ExistsFormula::substitute(
    const std::string& variableToReplace,
    const std::string& replacement
) const {
    if (variable == variableToReplace) {
        return clone();
    }

    return std::make_unique<ExistsFormula>(
        variable,
        body->substitute(variableToReplace, replacement)
    );
}

void ExistsFormula::collectTerms(
    std::set<std::string>& terms,
    std::set<std::string>& boundVariables
) const {
    boundVariables.insert(variable);
    body->collectTerms(terms, boundVariables);
    boundVariables.erase(variable);
}

const Formula* ExistsFormula::getBody() const {
    return body.get();
}

const std::string* ExistsFormula::getVariable() const {
    return &variable;
}
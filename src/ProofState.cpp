/**
 * @file ProofState.cpp
 */

#include "ProofState.hpp"
#include "Tokenizer.hpp"
#include "Parser.hpp"
#include <stdexcept>

ProofState::ProofState(std::unique_ptr<Formula> formula)
    : formula(std::move(formula)), steps(0), branches(0) {
    if (!this->formula) {
        throw std::runtime_error("ProofState error: formula cannot be null");
    }
}

ProofState::ProofState(const Formula& formula)
    : steps(0), branches(0) {
    auto tokens = Tokenizer::tokenize(formula.toString());
    Parser parser(tokens);
    this->formula = parser.parse();
}

const Formula& ProofState::getFormula() const {
    return *formula;
}

std::string ProofState::toString() const {
    return formula->toString();
}

long long ProofState::getSteps() const {
    return steps;
}

long long ProofState::getBranches() const {
    return branches;
}

void ProofState::incrementSteps(long long amount) {
    steps += amount;
}

void ProofState::incrementBranches(long long amount) {
    branches += amount;
}

void ProofState::resetCounters() {
    steps = 0;
    branches = 0;
}
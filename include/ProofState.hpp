/**
 * @file ProofState.hpp
 */

#pragma once

#include <string>
#include <memory>
#include "Formula.hpp"

class ProofState {
private:
    std::unique_ptr<Formula> formula;
    long long steps;
    long long branches;

public:
    ProofState(std::unique_ptr<Formula> formula);
    ProofState(const Formula& formula);

    const Formula& getFormula() const;
    std::string toString() const;

    long long getSteps() const;
    long long getBranches() const;

    void incrementSteps(long long amount = 1);
    void incrementBranches(long long amount = 1);

    void resetCounters();
};
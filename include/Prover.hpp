/**
 * @file Prover.hpp
 */

#pragma once

#include <string>
#include "ProofResult.hpp"
#include "ProofState.hpp"

class Prover {
public:
    virtual ~Prover() = default;
    virtual std::string getName() const = 0;
    virtual ProofResult prove(ProofState& state) = 0;
};
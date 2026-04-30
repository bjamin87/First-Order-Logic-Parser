/**
 * @file BaselineProver.hpp
 */

#pragma once

#include "Prover.hpp"

class BaselineProver : public Prover {
public:
    std::string getName() const override;
    ProofResult prove(ProofState& state) override;
};
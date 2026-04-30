/**
 * @file ImprovedProver.hpp
 */

#pragma once

#include "Prover.hpp"

class ImprovedProver : public Prover {
public:
    std::string getName() const override;
    ProofResult prove(ProofState& state) override;
};
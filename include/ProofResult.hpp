/**
 * @file ProofResult.hpp
 */

#pragma once
#include <string>
#include <vector>

class ProofResult {
public:
    bool proved;
    long long steps;
    long long branches;
    double runtimeMs;
    std::string message;
    std::vector<std::string> trace;
    ProofResult(

        bool proved = false,
        long long steps = 0,
        long long branches = 0,
        double runtimeMs = 0.0,
        const std::string& message = "",
        const std::vector<std::string>& trace = {}
    )
        : proved(proved),
          steps(steps),
          branches(branches),
          runtimeMs(runtimeMs),
          message(message),
          trace(trace) {}

};
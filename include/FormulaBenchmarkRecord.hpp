/**
 * @file FormulaBenchmarkRecord.hpp
 */

#pragma once

#include <string>
#include <vector>

class FormulaBenchmarkRecord {
public:
    int formulaIndex = 0;
    std::string formulaText;
    bool proved = false;
    long long steps = 0;
    long long branches = 0;
    double runtimeMs = 0.0;
    std::string message;
    std::vector<std::string> trace;
};
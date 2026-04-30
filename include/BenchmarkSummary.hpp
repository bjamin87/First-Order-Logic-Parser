/**
 * @file BenchmarkSummary.hpp
 */

#pragma once

#include <string>

class BenchmarkSummary {
public:
    std::string algorithmName;
    int totalFormulas = 0;
    int solvedCount = 0;
    int failedCount = 0;
    long long totalSteps = 0;
    long long totalBranches = 0;
    double totalRuntimeMs = 0.0;

    double averageSteps() const {
        return totalFormulas == 0 ? 0.0
            : static_cast<double>(totalSteps) / totalFormulas;
    }

    double averageRuntimeMs() const {
        return totalFormulas == 0 ? 0.0
            : totalRuntimeMs / totalFormulas;
    }
};
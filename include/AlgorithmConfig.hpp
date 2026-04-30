/**
 * @file AlgorithmConfig.hpp
 */

#pragma once

#include<string>

class AlgorithmConfig {
    public:
        bool runBaseline;
        bool runImproved;
        bool writeResults;
        int maxSteps;
        int timeoutMs;

        AlgorithmConfig();

        bool loadFromFile(const std::string& filename);
};
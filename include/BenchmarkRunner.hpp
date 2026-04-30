/**
 * @file BenchmarkRunner.hpp
 */

#pragma once

#include <string>
#include "AppConfig.hpp"
#include "AlgorithmConfig.hpp"

class BenchmarkRunner {
    private:
        AppConfig& appConfig;
        AlgorithmConfig algorithmConfig;

    public:
        BenchmarkRunner(AppConfig& appConfig);

        bool loadAlgorithmConfig( const std::string& filename);
        void run();
};
/**
 * @file ResultWriter.hpp
 */

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "BenchmarkSummary.hpp"
#include "FormulaBenchmarkRecord.hpp"

class ResultWriter {
public: 
    static bool writeTextReport(
        const std::string& filename,
        const std::string& inputFile,
        int parseSuccessCount,
        int parseFailureCount,
        const BenchmarkSummary& summary,
        const std::vector<FormulaBenchmarkRecord>& records,
        const std::string& configFilename
    );

    static bool writeCsvReport(
        const std::string& filename,
        const std::string& algorithmName,
        const std::vector<FormulaBenchmarkRecord>& records
        );
};
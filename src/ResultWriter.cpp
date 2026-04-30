/**
 * @file ResultWriter.cpp
 */

#include "ResultWriter.hpp"

bool ResultWriter::writeTextReport(
    const std::string& filename,
    const std::string& inputFile,
    int parseSuccessCount,
    int parseFailureCount,
    const BenchmarkSummary& summary,
    const std::vector<FormulaBenchmarkRecord>& records,
    const std::string& configFilename
) {
    std::ofstream out(filename);

    if (!out) {
        return false;
    }

    out << "========================================\n";
    out << "3806ICT Logic Prover Benchmark Report\n";
    out << "========================================\n";
    out << "Input file: " << inputFile << "\n";
    out << "Parsed successfully: " << parseSuccessCount << "\n";
    out << "Parse failures: " << parseFailureCount << "\n";
    out << "Total formulas benchmarked: " << summary.totalFormulas << "\n\n";

    out << "----------------------------------------\n";
    out << "Algorithm: " << summary.algorithmName << "\n";
    out << "----------------------------------------\n";
    out << "Solved: " << summary.solvedCount << " / " << summary.totalFormulas << "\n";
    out << "Failed: " << summary.failedCount << " / " << summary.totalFormulas << "\n";
    out << "Total steps: " << summary.totalSteps << "\n";
    out << "Average steps: " << summary.averageSteps() << "\n";
    out << "Total branches: " << summary.totalBranches << "\n";
    out << "Total runtime (ms): " << summary.totalRuntimeMs << "\n";
    out << "Average runtime (ms): " << summary.averageRuntimeMs() << "\n\n";

    out << "----------------------------------------\n";
    out << "Per-formula Results\n";
    out << "----------------------------------------\n";

    for (const auto& record : records) {
        out << "Formula " << record.formulaIndex << ": " << record.formulaText << "\n";
        out << "  Proved: " << (record.proved ? "true" : "false") << "\n";
        out << "  Steps: " << record.steps << "\n";
        out << "  Branches: " << record.branches << "\n";
        out << "  Runtime (ms): " << record.runtimeMs << "\n";
        out << "  Message: " << record.message << "\n\n";

        out << "  Trace:\n";
        for (const auto& line : record.trace) {
            out << "    " << line << "\n";
        }
        out << "\n";
    }

    out << "----------------------------------------\n";
    out << "Heuristic Configuration Used\n";
    out << "----------------------------------------\n";
    out << "Config file: " << configFilename << "\n\n";

    std::ifstream configFile(configFilename);
    
    if (!configFile) {
        out << "Could not open config file.\n";
    } else {
        std::string line;
        while (std::getline(configFile, line)) {
            out << line << "\n";
        }
    }

    return true;
}

bool ResultWriter::writeCsvReport(
    const std::string& filename,
    const std::string& algorithmName,
    const std::vector<FormulaBenchmarkRecord>& records
) {
    std::ofstream out(filename);

    if (!out) {
        return false;
    }

    out << "algorithm,formula_index,formula,proved,steps,branches,runtime_ms,message\n";

    for (const auto& record : records) {
        out << algorithmName << ","
            << record.formulaIndex << ","
            << "\"" << record.formulaText << "\","
            << (record.proved ? "true" : "false") << ","
            << record.steps << ","
            << record.branches << ","
            << record.runtimeMs << ","
            << "\"" << record.message << "\"\n";
    }

    return true;
}
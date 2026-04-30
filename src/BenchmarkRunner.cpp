/**
 * @file BenchmarkRunner.cpp
 */

#include "BenchmarkRunner.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <exception>

#include "Tokenizer.hpp"
#include "Parser.hpp"
#include "Formula.hpp"
#include "ProofState.hpp"
#include "BaselineProver.hpp"
#include "ImprovedProver.hpp"
#include "BenchmarkSummary.hpp"
#include "FormulaBenchmarkRecord.hpp"
#include "ResultWriter.hpp"

BenchmarkRunner::BenchmarkRunner(AppConfig& appConfig)
    : appConfig(appConfig) {}

bool BenchmarkRunner::loadAlgorithmConfig(const std::string& filename) {
    return algorithmConfig.loadFromFile(filename);
}

void BenchmarkRunner::run() {
    std::ifstream inputFile(appConfig.inputFile);

    if (!inputFile) {
        std::cout << "Error: Could not open input file: " << appConfig.inputFile << "\n";
        return;
    }

    std::vector<std::unique_ptr<Formula>> parsedFormulas;
    std::string line;
    int lineNumber = 1;
    int successCount = 0;
    int failureCount = 0;

    std::cout << "\n=== Loading formulas ===\n";
    std::cout << "Input file: " << appConfig.inputFile << "\n\n";

    while (std::getline(inputFile, line)) {
        if (line.empty()) {
            ++lineNumber;
            continue;
        }

        try {
            auto tokens = Tokenizer::tokenize(line);
            Parser parser(tokens);
            auto parsedFormula = parser.parse();

            parsedFormulas.push_back(std::move(parsedFormula));
            ++successCount;
        }
        catch (const std::exception& ex) {
            std::cout << "Parse error on line " << lineNumber << ": " << ex.what() << "\n";
            ++failureCount;
        }

        ++lineNumber;
    }

    std::cout << "Successfully parsed: " << successCount << "\n";
    std::cout << "Failed to parse:     " << failureCount << "\n";
    std::cout << "Stored formulas:     " << parsedFormulas.size() << "\n\n";

    std::vector<std::unique_ptr<Prover>> provers;

    if (algorithmConfig.runBaseline) {
        provers.push_back(std::make_unique<BaselineProver>());
    }

    if (algorithmConfig.runImproved) {
        provers.push_back(std::make_unique<ImprovedProver>());
    }

    for (const auto& prover : provers) {
        BenchmarkSummary summary;
        summary.algorithmName = prover->getName();

        std::vector<FormulaBenchmarkRecord> records;

        std::cout << "=== Running algorithm: " << prover->getName() << " ===\n";

        for (std::size_t i = 0; i < parsedFormulas.size(); ++i) {
            const Formula& formula = *parsedFormulas[i];
            ProofState state(formula);

            ProofResult result = prover->prove(state);

            FormulaBenchmarkRecord record;
            record.formulaIndex = static_cast<int>(i + 1);
            record.formulaText = formula.toString();
            record.proved = result.proved;
            record.steps = result.steps;
            record.branches = result.branches;
            record.runtimeMs = result.runtimeMs;
            record.message = result.message;
            record.trace = result.trace;

            records.push_back(record);

            summary.totalFormulas++;
            if (result.proved) {
                summary.solvedCount++;
            } else {
                summary.failedCount++;
            }
            summary.totalSteps += result.steps;
            summary.totalBranches += result.branches;
            summary.totalRuntimeMs += result.runtimeMs;
        }

        std::cout << "--- Summary for " << summary.algorithmName << " ---\n";
        std::cout << "Solved:             " << summary.solvedCount << " / " << summary.totalFormulas << "\n";
        std::cout << "Failed:             " << summary.failedCount << " / " << summary.totalFormulas << "\n";
        std::cout << "Total steps:        " << summary.totalSteps << "\n";
        std::cout << "Average steps:      " << summary.averageSteps() << "\n";
        std::cout << "Total branches:     " << summary.totalBranches << "\n";
        std::cout << "Total runtime (ms): " << summary.totalRuntimeMs << "\n";
        std::cout << "Average runtime:    " << summary.averageRuntimeMs() << "\n\n";

        if (algorithmConfig.writeResults) {
            std::string textFilename = summary.algorithmName + "_" + appConfig.outputFile;
            std::string csvFilename = summary.algorithmName + "_results.csv";

            std::string configFilename;

            if (summary.algorithmName == "BaselineProver") {
                configFilename = "baseline_heuristics.txt";
            } else if (summary.algorithmName == "ImprovedProver") {
                configFilename = "heuristics.txt";
    }

            if (!ResultWriter::writeTextReport(
                    textFilename,
                    appConfig.inputFile,
                    successCount,
                    failureCount,
                    summary,
                    records,
                    configFilename)) {
                std::cout << "Warning: Failed to write text report: " << textFilename << "\n";
            }

            if (!ResultWriter::writeCsvReport(
                    csvFilename,
                    summary.algorithmName,
                    records)) {
                std::cout << "Warning: Failed to write CSV report: " << csvFilename << "\n";
            } else {
                std::cout << "CSV report written to: " << csvFilename << "\n";
            }
        }
    }

    std::cout << "Benchmark run complete.\n";
}
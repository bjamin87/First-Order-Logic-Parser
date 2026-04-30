/**
 * @file AlgorithmConfig.cpp
 */

#include "AlgorithmConfig.hpp"
#include <fstream>
#include <sstream>

AlgorithmConfig::AlgorithmConfig() : 
    runBaseline(true), 
    runImproved(true), 
    writeResults(true), 
    maxSteps(1000000), 
    timeoutMs(1000000) {}

bool AlgorithmConfig::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);

    if (!file) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (key == "runBaseline") {
            runBaseline = (value == "true");
        }
        else if (key == "runImproved") {
            runImproved = (value == "true");
        }
        else if (key == "writeResults") {
            writeResults = (value == "true");
        }
        else if (key == "maxSteps") {
            maxSteps = std::stoi(value);
        }
        else if (key == "timeoutMs") {
            timeoutMs = std::stoi(value);
        }
    }
    return true;
}

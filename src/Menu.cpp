/**
 * @file Menu.cpp
 */

#include "Menu.hpp"
#include "Tokenizer.hpp"
#include "Parser.hpp"
#include "Formula.hpp"
#include "BenchmarkRunner.hpp"
#include <iostream>
#include <limits>
#include <fstream>
#include <vector>
#include <memory>
#include <exception>

Menu::Menu(AppConfig& cfg) : config(cfg) {}

void Menu::run() {
    bool running = true;

    while (running) 
    {
        printMenu();
        int choice = getUserChoice();

        switch (choice) {
            case 1:
                setInputFile();
                break;
            case 2:
                setOutputFile();
                break;
            case 3:
                selectAlgorithm();
                break;
            case 4:
                showConfig();
                break;
            case 5:
                runBenchmark();
                break;
            case 6:
                running = false;
                break;
            default:
                std::cout << "Invalide Selection\n";
        }
    }
}

void Menu::printMenu() {
    std::cout << "\n===  MAIN MENU.  ===\n";
    std::cout << " 1. Set input file (current: " << config.inputFile << ")\n";
    std::cout << " 2. Set output file (current: " << config.outputFile << ")\n";
    std::cout << " 3. Select algorithm\n";
    std::cout << " 4. Show current config\n";
    std::cout << " 5. Run Benchmark\n";
    std::cout << " 6. Exit\n";
}

int Menu::getUserChoice() {
    int choice;

    std::cout << "Enter choice: ";
    std::cin >> choice;

    //Handle invalid Input (eg., leters instead of numbers)
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }
    return choice;
}

void Menu::setInputFile() {
    std::cout << "Enter input file path: ";
    std::cin >> config.inputFile;
}

void Menu::setOutputFile() {
    std::cout << "Enter output file path: ";
    std::cin >> config.outputFile;
}

void Menu::selectAlgorithm() {
    std::cout << "\nSelect algorithm:\n";
    std::cout << "1. Baseline (Algorithm 2)\n";
    std::cout << "2. Improved V1\n";
    std::cout << "3. Improved V2\n";

    int choice;
    std::cout << "Enter Choice: ";
    std::cin >> choice;

    switch (choice) {
        case 1:
            config.algorithm = "baseline";
            break;
        case 2:
            config.algorithm = "improved_v1";
            break;
        case 3:
            config.algorithm = "improved_v2";
            break;
        default:
            std::cout << "Invalid selection. Keeping previous. \n";
    }
}

void Menu::showConfig() {
    std::cout << "\n=== Current Configuration ===\n";
    std::cout << "Input file: " << config.inputFile << "\n";
    std::cout << "Output file: " << config.outputFile << "\n";
    std::cout << "Algorithm: " << config.algorithm << "\n";
}

void Menu::runBenchmark() {
    std::string filename;
    std::cout << "Enter output filename (or press Enter to use current: "
              << config.outputFile << "): ";
    std::getline(std::cin >> std::ws, filename);

    if (filename.empty()) {
    config.outputFile = "output_" + std::to_string(time(nullptr)) + ".txt";
    } 
    else {
        config.outputFile = filename;
    }

    std::ifstream checkFile(config.outputFile);
    if (checkFile.good()) {
        std::cout << "Warning: File '" << config.outputFile << "' already exists.\n";
        std::cout << "Overwrite? (y/n): ";
        char choice;
        std::cin >> choice;
        if (choice != 'y' && choice != 'Y') {
            std::cout << "Benchmark cancelled.\n";
            return;
        }
    }
    BenchmarkRunner runner(config);
    if (!runner.loadAlgorithmConfig("algorithms.txt")) {
        std::cout << "Warning: Could not load algorithms.txt. Using default settings.\n";
    }
    runner.run();
}
/**
 * @file Menu.hpp
 */

#pragma once
#include "AppConfig.hpp"

class Menu
{
    private:
        AppConfig& config;

        void printMenu();
        int getUserChoice();

        void setInputFile();
        void setOutputFile();
        void selectAlgorithm();
        void runBenchmark();
        void showConfig();

    public:
        Menu(AppConfig& cfg);
        void run();
};
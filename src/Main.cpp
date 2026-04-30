/**
 * @file Main.cpp
 */


#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include "AppConfig.hpp"
#include "Menu.hpp"
#include "Formula.hpp"
#include "Tokenizer.hpp"
#include "Parser.hpp"

int main() 
{
    std::cout << "==================================================\n";
    std::cout << "===           3806ICT - Logic Prover.          ===\n";
    std::cout << "==================================================\n\n";

    
    // Main app
    AppConfig config;
    Menu menu(config);

    menu.run();

    std::cout << "Exiting Program. \n";
}
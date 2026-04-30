/**
 * @file AppConfig.hpp
 */

#pragma once
#include <string> 

class AppConfig 
{
    public:
        std::string inputFile = "input.txt";
        std::string outputFile = "output.txt";
        std::string algorithm = "baseline";
};

/**
 * @file Tokenizer.hpp
 */

#pragma once

#include<string>
#include<vector>
#include "Token.hpp"

class Tokenizer {
    public:
        static std::vector<Token> tokenize(const std::string& input);
};


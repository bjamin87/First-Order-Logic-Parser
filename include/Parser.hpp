/**
 * @file Parser.hpp
 */

#pragma once

#include <memory>
#include <vector>
#include "Token.hpp"
#include "Formula.hpp"

class Parser 
{
    private:
        std::vector<Token> tokens;
        std::size_t current;

        const Token& peek() const;
        const Token& advance();
        bool match(TokenType type);

        std::vector<std::string> parseArguments();

        std::unique_ptr<Formula> parseImplication();
        std::unique_ptr<Formula> parseOr();
        std::unique_ptr<Formula> parseAnd();
        std::unique_ptr<Formula> parseUnary();
        std::unique_ptr<Formula> parsePrimary();

    public:
        Parser(const std::vector<Token>& tokens);
        std::unique_ptr<Formula> parse();
};
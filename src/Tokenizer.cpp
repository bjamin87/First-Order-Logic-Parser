/**
 * @file Tokenizer.cpp
 */

#include "Tokenizer.hpp"
#include <cctype>

std::vector<Token> Tokenizer::tokenize(const std::string& input) {
    std::vector<Token> tokens;

    std::size_t i = 0;
    while (i < input.size()) {
        char ch = input[i];

        if (std::isspace(static_cast<unsigned char>(ch))) {
            ++i;
            continue;
        }

        if (ch == '(') {
            tokens.emplace_back(TokenType::LParen, "(");
            ++i;
            continue;
        }

        if (ch == ')') {
            tokens.emplace_back(TokenType::RParen, ")");
            ++i;
            continue;
        
        }

        if (ch == ',') {
            tokens.emplace_back(TokenType::Comma, ",");
            ++i;
            continue;
        }

        if (ch == '-' && i + 1 <input.size() && input[i + 1] == '>') {
            tokens.emplace_back(TokenType::Implies, "->");
            i += 2;
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(ch))) {
            std::string word;

            while (i < input.size() && std::isalnum(static_cast<unsigned char>(input[i]))) {
                word += input[i];
                ++i;
            }

            if (word == "not") {
                tokens.emplace_back(TokenType::Not, word);}
            else if (word == "and") {
                tokens.emplace_back(TokenType::And, word);}
            else if (word == "or") {
                tokens.emplace_back(TokenType::Or, word);}
            else if (word == "forall") {
                tokens.emplace_back(TokenType::Forall, word);}
            else if (word == "exists") {
                tokens.emplace_back(TokenType::Exists, word);}
            else if (word == "true") {
                tokens.emplace_back(TokenType::True, word);}
            else if (word == "false") {
                tokens.emplace_back(TokenType::False, word);}
            else {
                tokens.emplace_back(TokenType::Identifier, word);
            }

            continue;
        }

        // skip unknown characters for now
        ++i;
    }
    tokens.emplace_back(TokenType::End, "");
    return tokens;
}
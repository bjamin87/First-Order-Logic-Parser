/**
 * @file Token.hpp
 */

#pragma once

#include<string>

enum class TokenType {
    Identifier,
    Not,
    And,
    Or,
    Implies,
    Forall,
    Exists,
    LParen,
    RParen,
    Comma,
    End,
    True,
    False
};

class Token {
    private:
        TokenType type;
        std::string value;

    public:
        Token(TokenType type, const std::string& value);

        TokenType getType() const;
        const std:: string& getValue() const;
};
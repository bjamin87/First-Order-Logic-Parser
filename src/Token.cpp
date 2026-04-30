/**
 * @file Token.cpp
 */

#include "Token.hpp"

Token::Token(TokenType type, const std::string& value) : type(type), value(value) {}

TokenType Token::getType() const {
    return type;
}

const std::string& Token::getValue() const {
    return value;
}
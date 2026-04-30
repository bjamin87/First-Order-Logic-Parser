/**
 * @file Parser.cpp
 */

#include "Parser.hpp"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::advance() {
    if (current < tokens.size()) {
        ++current;
    }
    return tokens[current - 1];
}
bool Parser::match(TokenType type) {
    if (peek().getType() == type) {
        advance();
        return true;
    }
    return false;
}

std::unique_ptr<Formula> Parser::parse() {
    auto result = parseImplication();

    if (peek().getType() != TokenType::End) {
        throw std::runtime_error("Parser error: unexpected extra input");
    }
    return result;
}

std::vector<std::string> Parser::parseArguments() {
    std::vector<std::string> arguments;

    if (peek().getType() != TokenType::Identifier) {
        throw std::runtime_error("Parser error: expected argument");
    }

    arguments.push_back(advance().getValue());

    while (match(TokenType::Comma)) {
        if (peek().getType() != TokenType::Identifier) {
            throw std::runtime_error("Parser error: expected argument after comma");
        }
        arguments.push_back(advance().getValue());
    }
    return arguments;
}

std::unique_ptr<Formula> Parser::parseImplication() {
    auto left = parseOr();

    if (match(TokenType::Implies)) {
        auto right = parseImplication();
        return std::make_unique<ImpliesFormula>(std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Formula> Parser::parseOr() {
    auto left = parseAnd();

    while (match(TokenType::Or)) {
        auto right = parseAnd();
        left = std::make_unique<OrFormula>(std::move(left), std::move(right));
    }
    return left; 
}

std::unique_ptr<Formula> Parser::parseAnd() {
    auto left = parseUnary();

    while (match(TokenType::And)) {
        auto right = parseUnary();
        left = std::make_unique<AndFormula>(std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Formula> Parser::parseUnary() {
    if (match(TokenType::Not)) {
        auto operand = parseUnary();
        return std::make_unique<NotFormula>(std::move(operand));
    }

    if (match(TokenType::Forall)) {
        if (peek().getType() != TokenType::Identifier) {
            throw std::runtime_error("Parser error: expected variable after forall");
        }
        std::string var = advance().getValue();
        auto body = parseUnary();
        return std::make_unique<ForallFormula>(var, std::move(body));
    }

    if (match(TokenType::Exists)) {
        if (peek().getType() != TokenType::Identifier) {
            throw std::runtime_error("Parser error: expected variable after exists");
        }
        std::string var = advance().getValue();
        auto body = parseUnary();
        return std::make_unique<ExistsFormula>(var, std::move(body));
    }
    return parsePrimary();
}

std::unique_ptr<Formula> Parser::parsePrimary() {
    if (match(TokenType::True)) {
        return std::make_unique<TrueFormula>();
    }

    if (match(TokenType::False)) {
        return std::make_unique<FalseFormula>();
    }

    if (peek().getType() == TokenType::Identifier) {
        std::string name = advance().getValue();
        if (match(TokenType::LParen)) {
            auto arguments = parseArguments();
            if (!match(TokenType::RParen)) {
                throw std::runtime_error("Parser error: expected ')'");
            }

            return std::make_unique<AtomFormula>(name, arguments);
        }
        return std::make_unique<AtomFormula>(name);
    }

    if (match(TokenType::LParen)) {
        auto expr = parseImplication();
        if (!match(TokenType::RParen)) {
            throw std::runtime_error("Parser error: expected ')'");
        }
        return expr;
    }

    throw std::runtime_error("Parser error: unexpected token");
}
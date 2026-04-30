/**
 * @file Sequent.hpp
 */

#pragma once

#include <vector>
#include <string>
#include <memory>
#include "Formula.hpp"

class Sequent {
private:
    std::vector<const Formula*> left;
    std::vector<const Formula*> right;
    std::vector<std::shared_ptr<Formula>> ownedFormulas;

public:
    Sequent() = default;
    Sequent(const std::vector<const Formula*>& left,
            const std::vector<const Formula*>& right);

    // Important: preserve ownership when copying
    Sequent(const Sequent& other) = default;
    Sequent& operator=(const Sequent& other) = default;

    const std::vector<const Formula*>& getLeft() const;
    const std::vector<const Formula*>& getRight() const;

    void setLeft(const std::vector<const Formula*>& newLeft);
    void setRight(const std::vector<const Formula*>& newRight);

    void addLeft(const Formula* formula);
    void addRight(const Formula* formula);

    void addOwnedLeft(std::unique_ptr<Formula> formula);
    void addOwnedRight(std::unique_ptr<Formula> formula);

    std::vector<std::string> getAllTerms() const;

    bool isClosedById() const;
    std::string toString() const;
};
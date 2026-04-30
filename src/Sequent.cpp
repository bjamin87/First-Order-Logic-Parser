/**
 * @file Sequent.cpp
 */

#include "Sequent.hpp"
#include <sstream>
#include <set>

Sequent::Sequent(const std::vector<const Formula*>& left,
                 const std::vector<const Formula*>& right)
    : left(left), right(right) {}

const std::vector<const Formula*>& Sequent::getLeft() const {
    return left;
}

const std::vector<const Formula*>& Sequent::getRight() const {
    return right;
}

void Sequent::setLeft(const std::vector<const Formula*>& newLeft) {
    left = newLeft;
}

void Sequent::setRight(const std::vector<const Formula*>& newRight) {
    right = newRight;
}

void Sequent::addLeft(const Formula* formula) {
    if (formula != nullptr) {
        left.push_back(formula);
    }
}

void Sequent::addRight(const Formula* formula) {
    if (formula != nullptr) {
        right.push_back(formula);
    }
}

void Sequent::addOwnedLeft(std::unique_ptr<Formula> formula) {
    if (formula != nullptr) {
        std::shared_ptr<Formula> shared = std::move(formula);
        left.push_back(shared.get());
        ownedFormulas.push_back(shared);
    }
}

void Sequent::addOwnedRight(std::unique_ptr<Formula> formula) {
    if (formula != nullptr) {
        std::shared_ptr<Formula> shared = std::move(formula);
        right.push_back(shared.get());
        ownedFormulas.push_back(shared);
    }
}

std::vector<std::string> Sequent::getAllTerms() const {
    std::set<std::string> terms;
    std::set<std::string> boundVariables;

    for (const Formula* f : left) {
        if (f != nullptr) {
            f->collectTerms(terms, boundVariables);
        }
    }

    for (const Formula* f : right) {
        if (f != nullptr) {
            f->collectTerms(terms, boundVariables);
        }
    }

    return std::vector<std::string>(terms.begin(), terms.end());
}

bool Sequent::isClosedById() const {
    for (const Formula* l : left) {
        for (const Formula* r : right) {
            if (l != nullptr && r != nullptr && l->equals(*r)) {
                return true;
            }
        }
    }
    return false;
}

std::string Sequent::toString() const {
    std::ostringstream out;

    for (std::size_t i = 0; i < left.size(); ++i) {
        if (left[i] != nullptr) {
            out << left[i]->toString();
        }
        if (i + 1 < left.size()) {
            out << ", ";
        }
    }

    out << " |- ";

    for (std::size_t i = 0; i < right.size(); ++i) {
        if (right[i] != nullptr) {
            out << right[i]->toString();
        }
        if (i + 1 < right.size()) {
            out << ", ";
        }
    }

    return out.str();
}
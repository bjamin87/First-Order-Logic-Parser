/**
 * @file Formula.hpp
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <set>

enum class FormulaType {
    Atom,
    Not,
    And,
    Or,
    Implies,
    Forall,
    Exists,
    True,
    False
};

class Formula {
public:
    virtual ~Formula() = default;
    virtual std::string toString() const = 0;
    virtual FormulaType getType() const = 0;
    virtual bool equals(const Formula& other) const = 0;

    virtual std::unique_ptr<Formula> clone() const = 0;

    virtual std::unique_ptr<Formula> substitute(
        const std::string& variable,
        const std::string& replacement
    ) const = 0;

    virtual void collectTerms(
        std::set<std::string>& terms,
        std::set<std::string>& boundVariables
    ) const = 0;

    virtual const Formula* getOperand() const { return nullptr; }
    virtual const Formula* getLeft() const { return nullptr; }
    virtual const Formula* getRight() const { return nullptr; }
    virtual const Formula* getBody() const { return nullptr; }
    virtual const std::string* getVariable() const { return nullptr; }
};

class AtomFormula : public Formula {
private:
    std::string name;
    std::vector<std::string> arguments;

public:
    AtomFormula(const std::string& name);
    AtomFormula(const std::string& name, const std::vector<std::string>& arguments);

    std::string toString() const override;
    FormulaType getType() const override;
    bool equals(const Formula& other) const override;
    std::unique_ptr<Formula> clone() const override;
    std::unique_ptr<Formula> substitute(
        const std::string& variable,
        const std::string& replacement
    ) const override;
    void collectTerms(
        std::set<std::string>& terms,
        std::set<std::string>& boundVariables
    ) const override;
};

class NotFormula : public Formula {
private:
    std::unique_ptr<Formula> operand;

public:
    NotFormula(std::unique_ptr<Formula> operand);

    std::string toString() const override;
    FormulaType getType() const override;
    bool equals(const Formula& other) const override;
    std::unique_ptr<Formula> clone() const override;
    std::unique_ptr<Formula> substitute(
        const std::string& variable,
        const std::string& replacement
    ) const override;
    void collectTerms(
        std::set<std::string>& terms,
        std::set<std::string>& boundVariables
    ) const override;
    const Formula* getOperand() const override;
};

class AndFormula : public Formula {
private:
    std::unique_ptr<Formula> left;
    std::unique_ptr<Formula> right;

public:
    AndFormula(std::unique_ptr<Formula> left,
               std::unique_ptr<Formula> right);

    std::string toString() const override;
    FormulaType getType() const override;
    bool equals(const Formula& other) const override;
    std::unique_ptr<Formula> clone() const override;
    std::unique_ptr<Formula> substitute(
        const std::string& variable,
        const std::string& replacement
    ) const override;
    void collectTerms(
        std::set<std::string>& terms,
        std::set<std::string>& boundVariables
    ) const override;
    const Formula* getLeft() const override;
    const Formula* getRight() const override;
};

class OrFormula : public Formula {
private:
    std::unique_ptr<Formula> left;
    std::unique_ptr<Formula> right;

public:
    OrFormula(std::unique_ptr<Formula> left,
              std::unique_ptr<Formula> right);

    std::string toString() const override;
    FormulaType getType() const override;
    bool equals(const Formula& other) const override;
    std::unique_ptr<Formula> clone() const override;
    std::unique_ptr<Formula> substitute(
        const std::string& variable,
        const std::string& replacement
    ) const override;
    void collectTerms(
        std::set<std::string>& terms,
        std::set<std::string>& boundVariables
    ) const override;
    const Formula* getLeft() const override;
    const Formula* getRight() const override;
};

class ImpliesFormula : public Formula {
private:
    std::unique_ptr<Formula> left;
    std::unique_ptr<Formula> right;

public:
    ImpliesFormula(std::unique_ptr<Formula> left,
                   std::unique_ptr<Formula> right);

    std::string toString() const override;
    FormulaType getType() const override;
    bool equals(const Formula& other) const override;
    std::unique_ptr<Formula> clone() const override;
    std::unique_ptr<Formula> substitute(
        const std::string& variable,
        const std::string& replacement
    ) const override;
    void collectTerms(
        std::set<std::string>& terms,
        std::set<std::string>& boundVariables
    ) const override;
    const Formula* getLeft() const override;
    const Formula* getRight() const override;
};

class ForallFormula : public Formula {
private:
    std::string variable;
    std::unique_ptr<Formula> body;

public:
    ForallFormula(const std::string& variable,
                  std::unique_ptr<Formula> body);

    std::string toString() const override;
    FormulaType getType() const override;
    bool equals(const Formula& other) const override;
    std::unique_ptr<Formula> clone() const override;
    std::unique_ptr<Formula> substitute(
        const std::string& variable,
        const std::string& replacement
    ) const override;
    void collectTerms(
        std::set<std::string>& terms,
        std::set<std::string>& boundVariables
    ) const override;
    const Formula* getBody() const override;
    const std::string* getVariable() const override;
};

class ExistsFormula : public Formula {
private:
    std::string variable;
    std::unique_ptr<Formula> body;

public:
    ExistsFormula(const std::string& variable,
                  std::unique_ptr<Formula> body);

    std::string toString() const override;
    FormulaType getType() const override;
    bool equals(const Formula& other) const override;
    std::unique_ptr<Formula> clone() const override;
    std::unique_ptr<Formula> substitute(
        const std::string& variable,
        const std::string& replacement
    ) const override;
    void collectTerms(
        std::set<std::string>& terms,
        std::set<std::string>& boundVariables
    ) const override;
    const Formula* getBody() const override;
    const std::string* getVariable() const override;
};

class TrueFormula : public Formula {
public:
    std::string toString() const override { return "true"; }
    FormulaType getType() const override { return FormulaType::True; }

    bool equals(const Formula& other) const override {
        return other.getType() == FormulaType::True;
    }

    std::unique_ptr<Formula> clone() const override {
        return std::make_unique<TrueFormula>();
    }

    std::unique_ptr<Formula> substitute(
        const std::string&, const std::string&
    ) const override {
        return clone();
    }

    void collectTerms(
        std::set<std::string>&,
        std::set<std::string>&
    ) const override {}
};

class FalseFormula : public Formula {
public:
    std::string toString() const override { return "false"; }
    FormulaType getType() const override { return FormulaType::False; }

    bool equals(const Formula& other) const override {
        return other.getType() == FormulaType::False;
    }

    std::unique_ptr<Formula> clone() const override {
        return std::make_unique<FalseFormula>();
    }

    std::unique_ptr<Formula> substitute(
        const std::string&, const std::string&
    ) const override {
        return clone();
    }

    void collectTerms(
        std::set<std::string>&,
        std::set<std::string>&
    ) const override {}
};
/**
 * @file
 * @brief Definition of tokens that can be parsed and a tokenizer functions
 */
#ifndef AST_BUILDER_TOKENIZER_H
#define AST_BUILDER_TOKENIZER_H

#include <cctype>
#include <climits>
#include <cstring>
#include <map>
#include <memory>
#include <vector>

enum TokenType {
    CONSTANT_VALUE,
    PARENTHESIS,
    OPERATOR,
    COMPARISON_OPERATOR,
    VARIABLE,
    FUNCTION,
    SEMICOLON,
    IF,
    ELSE,
    WHILE,
};

static const char* const TokenTypeStrings[] = {
    "CONSTANT_VALUE",
    "PARENTHESIS",
    "OPERATOR",
    "COMPARISON_OPERATOR",
    "VARIABLE",
    "FUNCTION",
    "SEMICOLON",
    "IF",
    "ELSE",
    "WHILE",
};

class Token {

private:
    const TokenType type;
    const size_t originPos;

public:
    Token(TokenType type_, size_t originPos_) : type(type_), originPos(originPos_) { }
    virtual ~Token() = default;

    TokenType getType() const {
        return type;
    }

    size_t getOriginPos() const {
        return originPos;
    }

    virtual void print() const;
};

class ConstantValueToken : public Token {

private:
    const double value;

public:
    ConstantValueToken(size_t originPos_, double value_) : Token(CONSTANT_VALUE, originPos_), value(value_) { }

    double getValue() const {
        return value;
    }

    void print() const override;
};

enum ParenthesisType {
    ROUND,
    CURLY,
};

class ParenthesisToken : public Token {

private:
    const bool open;
    const ParenthesisType parenthesisType;

public:
    ParenthesisToken(size_t originPos_, bool open_, ParenthesisType parenthesisType_) :
        Token(PARENTHESIS, originPos_), open(open_), parenthesisType(parenthesisType_) { }

    bool isOpen() const {
        return open;
    }

    bool isClose() const {
        return !open;
    }

    ParenthesisType getParenthesisType() const {
        return parenthesisType;
    }

    void print() const override;
};

enum OperatorType {
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    ARITHMETIC_NEGATION,
    UNARY_ADDITION,
    POWER,
    ASSIGNMENT,
};

static const char* const OperatorTypeStrings[] = {
    "ADDITION",
    "SUBTRACTION",
    "MULTIPLICATION",
    "DIVISION",
    "ARITHMETIC_NEGATION",
    "UNARY_ADDITION",
    "POWER",
    "ASSIGNMENT",
};

class OperatorToken : public Token {

private:
    const size_t arity;
    const size_t precedence;
    const bool leftAssociative;
    const OperatorType operatorType;

public:
    OperatorToken(size_t originPos_, size_t arity_, size_t precedence_, bool leftAssociative_, OperatorType operatorType_) :
        Token(OPERATOR, originPos_), arity(arity_), precedence(precedence_), leftAssociative(leftAssociative_), operatorType(operatorType_) { }

    size_t getArity() const {
        return arity;
    }

    size_t getPrecedence() const {
        return precedence;
    }

    OperatorType getOperatorType() const {
        return operatorType;
    }

    bool isLeftAssociative() const {
        return leftAssociative;
    }

    bool isRightAssociative() const {
        return !leftAssociative;
    }

    virtual const char* getSymbol() const = 0;

    void print() const override;

    virtual double calculate(size_t argc, ...) const = 0;
};

class AdditionOperator : public OperatorToken {

public:
    explicit AdditionOperator(size_t originPos_) : OperatorToken(originPos_, 2, 1, true, ADDITION) { }

    const char* getSymbol() const override {
        return "+";
    }

    double calculate(size_t argc, ...) const override;
};

class SubtractionOperator : public OperatorToken {

public:
    explicit SubtractionOperator(size_t originPos_) : OperatorToken(originPos_, 2, 1, true, SUBTRACTION) { }

    const char* getSymbol() const override {
        return "-";
    }

    double calculate(size_t argc, ...) const override;
};

class MultiplicationOperator : public OperatorToken {

public:
    explicit MultiplicationOperator(size_t originPos_) : OperatorToken(originPos_, 2, 2, true, MULTIPLICATION) { }

    const char* getSymbol() const override {
        return "*";
    }

    double calculate(size_t argc, ...) const override;
};

class DivisionOperator : public OperatorToken {

public:
    explicit DivisionOperator(size_t originPos_) : OperatorToken(originPos_, 2, 2, true, DIVISION) { }

    const char* getSymbol() const override {
        return "/";
    }

    double calculate(size_t argc, ...) const override;
};

class ArithmeticNegationOperator : public OperatorToken {

public:
    explicit ArithmeticNegationOperator(size_t originPos_) : OperatorToken(originPos_, 1, 1000, false, ARITHMETIC_NEGATION) { }

    const char* getSymbol() const override {
        return "-";
    }

    double calculate(size_t argc, ...) const override;
};

class UnaryAdditionOperator : public OperatorToken {

public:
    explicit UnaryAdditionOperator(size_t originPos_) : OperatorToken(originPos_, 1, 1000, false, UNARY_ADDITION) { }

    const char* getSymbol() const override {
        return "+";
    }

    double calculate(size_t argc, ...) const override;
};

class PowerOperator : public OperatorToken {

public:
    explicit PowerOperator(size_t originPos_) : OperatorToken(originPos_, 2, 3, false, POWER) { } // Power is right-associative like in math

    const char* getSymbol() const override {
        return "^";
    }

    double calculate(size_t argc, ...) const override;
};

class AssignmentOperator : public OperatorToken {

public:
    explicit AssignmentOperator(size_t originPos_) : OperatorToken(originPos_, 2, 0, false, ASSIGNMENT) { }

    const char* getSymbol() const override {
        return "=";
    }

    double calculate(size_t argc __attribute__((unused)), ...) const override;
};

enum ComparisonOperatorType {
    LESS,
    LESS_OR_EQUAL,
    GREATER,
    GREATER_OR_EQUAL,
    EQUAL,
};

static const char* const ComparisonOperatorTypeStrings[] = {
    "LESS",
    "LESS_OR_EQUAL",
    "GREATER",
    "GREATER_OR_EQUAL",
    "EQUAL",
};

class ComparisonOperatorToken : public Token {

private:
    const ComparisonOperatorType operatorType;

public:
    ComparisonOperatorToken(size_t originPos_, ComparisonOperatorType operatorType_) :
        Token(COMPARISON_OPERATOR, originPos_), operatorType(operatorType_) { }

    ComparisonOperatorType getOperatorType() const {
        return operatorType;
    }

    virtual const char* getSymbol() const = 0;

    void print() const override;
};

class LessComparisonOperator : public ComparisonOperatorToken {

public:
    explicit LessComparisonOperator(size_t originPos_) : ComparisonOperatorToken(originPos_, LESS) { }

    const char * getSymbol() const override {
        return "<";
    }
};

class LessOrEqualComparisonOperator : public ComparisonOperatorToken {

public:
    explicit LessOrEqualComparisonOperator(size_t originPos_) : ComparisonOperatorToken(originPos_, LESS_OR_EQUAL) { }

    const char * getSymbol() const override {
        return "<=";
    }
};

class GreaterComparisonOperator : public ComparisonOperatorToken {

public:
    explicit GreaterComparisonOperator(size_t originPos_) : ComparisonOperatorToken(originPos_, GREATER) { }

    const char * getSymbol() const override {
        return ">";
    }
};

class GreaterOrEqualComparisonOperator : public ComparisonOperatorToken {

public:
    explicit GreaterOrEqualComparisonOperator(size_t originPos_) : ComparisonOperatorToken(originPos_, GREATER_OR_EQUAL) { }

    const char * getSymbol() const override {
        return ">=";
    }
};

class EqualComparisonOperator : public ComparisonOperatorToken {

public:
    explicit EqualComparisonOperator(size_t originPos_) : ComparisonOperatorToken(originPos_, EQUAL) { }

    const char * getSymbol() const override {
        return "==";
    }
};

class VariableToken : public Token {

private:
    char* name;

public:
    static constexpr size_t MAX_NAME_LENGTH = 256u;

    VariableToken(size_t originPos_, const char* name_) : Token(VARIABLE, originPos_) {
        name = (char*)calloc(MAX_NAME_LENGTH, sizeof(char));
        for (unsigned int i = 0; i < MAX_NAME_LENGTH; ++i) {
            name[i] = name_[i];
            if (name[i] == '\0') break;
        }
    }

    ~VariableToken() override {
        free(name);
    }

    char* getName() const {
        return name;
    }

    void print() const override;
};

class FunctionToken : public Token {

private:
    const size_t arity;

public:
    FunctionToken(size_t originPos_, size_t arity_) : Token(FUNCTION, originPos_), arity(arity_) { }

    size_t getArity() const {
        return arity;
    }

    virtual const char* getName() const = 0;

    void print() const override;
};

class SemicolonToken : public Token {
public:
    explicit SemicolonToken(size_t originPos_) : Token(SEMICOLON, originPos_) { }

    void print() const override;
};

class IfToken : public Token {
public:
    explicit IfToken(size_t originPos_) : Token(IF, originPos_) { }
};

class ElseToken : public Token {
public:
    explicit ElseToken(size_t originPos_) : Token(ELSE, originPos_) { }
};

class WhileToken : public Token {
public:
    explicit WhileToken(size_t originPos_) : Token(WHILE, originPos_) { }
};

/**
 * Splits the expression into Token objects.
 * @param expression expression to tokenize
 * @return vector of parsed tokens.
 * @throws std::invalid_argument if invalid symbol met.
 */
std::vector<std::shared_ptr<Token>> tokenize(char* expression);

#endif // AST_BUILDER_TOKENIZER_H

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
    VARIABLE,
    FUNCTION,
};

static const char* const TokenTypeStrings[] = {
    "CONSTANT_VALUE",
    "PARENTHESIS",
    "OPERATOR",
    "VARIABLE",
    "FUNCTION",
};

class Token {

private:
    const TokenType type;

public:
    explicit Token(TokenType type_) : type(type_) { }
    virtual ~Token() = default;

    TokenType getType() const {
        return type;
    }

    virtual void print() const;

    virtual double calculate(size_t argc, ...) const = 0;
};

class ConstantValueToken : public Token {

private:
    const double value;

public:
    explicit ConstantValueToken(double value_) : Token(CONSTANT_VALUE), value(value_) { }

    double getValue() const {
        return value;
    }

    void print() const override;

    double calculate(size_t argc, ...) const override;
};

class ParenthesisToken : public Token {

private:
    const bool open;

public:
    explicit ParenthesisToken(bool open_) : Token(PARENTHESIS), open(open_) { }

    bool isOpen() const {
        return open;
    }

    bool isClose() const {
        return !open;
    }

    void print() const override;

    double calculate(size_t argc __attribute__((unused)), ...) const override;
};

enum OperatorType {
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    ARITHMETIC_NEGATION,
    UNARY_ADDITION,
    POWER,
};

static const char* const OperatorTypeStrings[] = {
    "ADDITION",
    "SUBTRACTION",
    "MULTIPLICATION",
    "DIVISION",
    "ARITHMETIC_NEGATION",
    "UNARY_ADDITION",
    "POWER",
};

class OperatorToken : public Token {

private:
    const size_t arity;
    const size_t precedence;
    const bool leftAssociative;
    const OperatorType operatorType;

public:
    OperatorToken(size_t arity_, size_t precedence_, bool leftAssociative_, OperatorType operatorType_) :
        Token(OPERATOR), arity(arity_), precedence(precedence_), leftAssociative(leftAssociative_), operatorType(operatorType_) { }

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
};

class AdditionOperator : public OperatorToken {

public:
    AdditionOperator() : OperatorToken(2, 1, true, ADDITION) { }

    const char* getSymbol() const override {
        return "+";
    }

    double calculate(size_t argc, ...) const override;
};

class SubtractionOperator : public OperatorToken {

public:
    SubtractionOperator() : OperatorToken(2, 1, true, SUBTRACTION) { }

    const char* getSymbol() const override {
        return "-";
    }

    double calculate(size_t argc, ...) const override;
};

class MultiplicationOperator : public OperatorToken {

public:
    MultiplicationOperator() : OperatorToken(2, 2, true, MULTIPLICATION) { }

    const char* getSymbol() const override {
        return "*";
    }

    double calculate(size_t argc, ...) const override;
};

class DivisionOperator : public OperatorToken {

public:
    DivisionOperator() : OperatorToken(2, 2, true, DIVISION) { }

    const char* getSymbol() const override {
        return "/";
    }

    double calculate(size_t argc, ...) const override;
};

class ArithmeticNegationOperator : public OperatorToken {

public:
    ArithmeticNegationOperator() : OperatorToken(1, 1000, false, ARITHMETIC_NEGATION) { }

    const char* getSymbol() const override {
        return "-";
    }

    double calculate(size_t argc, ...) const override;
};

class UnaryAdditionOperator : public OperatorToken {

public:
    UnaryAdditionOperator() : OperatorToken(1, 1000, false, UNARY_ADDITION) { }

    const char* getSymbol() const override {
        return "+";
    }

    double calculate(size_t argc, ...) const override;
};

class PowerOperator : public OperatorToken {

public:
    PowerOperator() : OperatorToken(2, 3, false, POWER) { } // Power is right-associative like in math

    const char* getSymbol() const override {
        return "^";
    }

    double calculate(size_t argc, ...) const override;
};

class VariableToken : public Token {

private:
    struct keyCompare {
        bool operator()(char* a, char* b) const {
            return strcmp(a, b) < 0;
        }
    };

    static std::map<char*, std::shared_ptr<VariableToken>, keyCompare> symbolTable;
    char* name;

    explicit VariableToken(const char* name_) : Token(VARIABLE) {
        name = (char*)calloc(MAX_NAME_LENGTH, sizeof(char));
        for (unsigned int i = 0; i < MAX_NAME_LENGTH; ++i) {
            name[i] = name_[i];
            if (name[i] == '\0') break;
        }
    }

public:
    ~VariableToken() override {
        free(name);
    }

    static constexpr size_t MAX_NAME_LENGTH = 256u;

    static std::shared_ptr<VariableToken> getVariableByName(char* name);

    void print() const override;

    double calculate(size_t argc __attribute__((unused)), ...) const override;

    char* getName() const {
        return name;
    }
};

enum FunctionType {
    SIN,
    COS,
    TG,
    CTG,
    LN,
};

static const char* const FunctionTypeStrings[] = {
    "SIN",
    "COS",
    "TG",
    "CTG",
    "LN",
};

class FunctionToken : public Token {

private:
    const size_t arity;
    const FunctionType functionType;

public:
    FunctionToken(size_t arity_, FunctionType functionType_) :
        Token(FUNCTION), arity(arity_), functionType(functionType_) { }

    size_t getArity() const {
        return arity;
    }

    FunctionType getFunctionType() const {
        return functionType;
    }

    virtual const char* getName() const = 0;

    void print() const override;
};

class SinFunction : public FunctionToken {

public:
    SinFunction() : FunctionToken(1, SIN) { }

    const char * getName() const override {
        return "sin";
    }

    double calculate(size_t argc, ...) const override;
};

class CosFunction : public FunctionToken {

public:
    CosFunction() : FunctionToken(1, COS) { }

    const char * getName() const override {
        return "cos";
    }

    double calculate(size_t argc, ...) const override;
};

class TgFunction : public FunctionToken {

public:
    TgFunction() : FunctionToken(1, TG) { }

    const char * getName() const override {
        return "tg";
    }

    double calculate(size_t argc, ...) const override;
};

class CtgFunction : public FunctionToken {

public:
    CtgFunction() : FunctionToken(1, CTG) { }

    const char * getName() const override {
        return "ctg";
    }

    double calculate(size_t argc, ...) const override;
};

class LnFunction : public FunctionToken {

public:
    LnFunction() : FunctionToken(1, LN) { }

    const char * getName() const override {
        return "ln";
    }

    double calculate(size_t argc, ...) const override;
};

/**
 * Splits the expression into Token objects.
 * @param expression expression to tokenize
 * @return vector of parsed tokens.
 * @throws std::invalid_argument if invalid symbol met.
 */
std::vector<std::shared_ptr<Token>> tokenize(char* expression);

#endif // AST_BUILDER_TOKENIZER_H

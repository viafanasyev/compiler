/**
 * @file
 * @brief Definition of tokens that can be parsed and tokenizer functions
 */
#ifndef COMPILER_TOKENIZER_H
#define COMPILER_TOKENIZER_H

#include <cctype>
#include <climits>
#include <cstddef>
#include <cstring>
#include <map>
#include <memory>
#include <vector>
#include "../util/constants.h"
#include "../util/TokenOrigin.h"

enum TokenType {
    CONSTANT_VALUE,
    PARENTHESIS,
    OPERATOR,
    ASSIGNMENT_OPERATOR,
    COMPARISON_OPERATOR,
    ID,
    SEMICOLON,
    IF,
    ELSE,
    WHILE,
    FUNC,
    VAR,
    VAL,
    COMMA,
    RETURN,
};

static const char* const TokenTypeStrings[] = {
    "CONSTANT_VALUE",
    "PARENTHESIS",
    "OPERATOR",
    "ASSIGNMENT_OPERATOR",
    "COMPARISON_OPERATOR",
    "ID",
    "FUNCTION",
    "SEMICOLON",
    "IF",
    "ELSE",
    "WHILE",
    "FUNC",
    "VAR",
    "VAL",
    "COMMA",
    "RETURN",
};

class Token {

private:
    const TokenType type;
    const TokenOrigin originPos;

public:
    Token(TokenType type_, TokenOrigin originPos_) : type(type_), originPos(originPos_) { }
    virtual ~Token() = default;

    TokenType getType() const {
        return type;
    }

    TokenOrigin getOriginPos() const {
        return originPos;
    }

    virtual void print() const;
};

class ConstantValueToken : public Token {

private:
    const double value;

public:
    ConstantValueToken(TokenOrigin originPos_, double value_) : Token(CONSTANT_VALUE, originPos_), value(value_) { }

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
    ParenthesisToken(TokenOrigin originPos_, bool open_, ParenthesisType parenthesisType_) :
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
};

static const char* const OperatorTypeStrings[] = {
    "ADDITION",
    "SUBTRACTION",
    "MULTIPLICATION",
    "DIVISION",
    "ARITHMETIC_NEGATION",
    "UNARY_ADDITION",
};

class OperatorToken : public Token {

private:
    const size_t arity;
    const size_t precedence;
    const bool leftAssociative;
    const OperatorType operatorType;

public:
    OperatorToken(TokenOrigin originPos_, size_t arity_, size_t precedence_, bool leftAssociative_, OperatorType operatorType_) :
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
    explicit AdditionOperator(TokenOrigin originPos_) : OperatorToken(originPos_, 2, 1, true, ADDITION) { }

    const char* getSymbol() const override {
        return "+";
    }

    double calculate(size_t argc, ...) const override;
};

class SubtractionOperator : public OperatorToken {

public:
    explicit SubtractionOperator(TokenOrigin originPos_) : OperatorToken(originPos_, 2, 1, true, SUBTRACTION) { }

    const char* getSymbol() const override {
        return "-";
    }

    double calculate(size_t argc, ...) const override;
};

class MultiplicationOperator : public OperatorToken {

public:
    explicit MultiplicationOperator(TokenOrigin originPos_) : OperatorToken(originPos_, 2, 2, true, MULTIPLICATION) { }

    const char* getSymbol() const override {
        return "*";
    }

    double calculate(size_t argc, ...) const override;
};

class DivisionOperator : public OperatorToken {

public:
    explicit DivisionOperator(TokenOrigin originPos_) : OperatorToken(originPos_, 2, 2, true, DIVISION) { }

    const char* getSymbol() const override {
        return "/";
    }

    double calculate(size_t argc, ...) const override;
};

class ArithmeticNegationOperator : public OperatorToken {

public:
    explicit ArithmeticNegationOperator(TokenOrigin originPos_) : OperatorToken(originPos_, 1, 1000, false, ARITHMETIC_NEGATION) { }

    const char* getSymbol() const override {
        return "-";
    }

    double calculate(size_t argc, ...) const override;
};

class UnaryAdditionOperator : public OperatorToken {

public:
    explicit UnaryAdditionOperator(TokenOrigin originPos_) : OperatorToken(originPos_, 1, 1000, false, UNARY_ADDITION) { }

    const char* getSymbol() const override {
        return "+";
    }

    double calculate(size_t argc, ...) const override;
};

class AssignmentOperatorToken : public Token {
public:
    explicit AssignmentOperatorToken(TokenOrigin originPos_) : Token(ASSIGNMENT_OPERATOR, originPos_) { }
};

enum ComparisonOperatorType {
    LESS,
    LESS_OR_EQUAL,
    GREATER,
    GREATER_OR_EQUAL,
    EQUAL,
    NOT_EQUAL,
};

static const char* const ComparisonOperatorTypeStrings[] = {
    "LESS",
    "LESS_OR_EQUAL",
    "GREATER",
    "GREATER_OR_EQUAL",
    "EQUAL",
    "NOT_EQUAL",
};

class ComparisonOperatorToken : public Token {

private:
    const ComparisonOperatorType operatorType;

public:
    ComparisonOperatorToken(TokenOrigin originPos_, ComparisonOperatorType operatorType_) :
        Token(COMPARISON_OPERATOR, originPos_), operatorType(operatorType_) { }

    ComparisonOperatorType getOperatorType() const {
        return operatorType;
    }

    virtual const char* getSymbol() const = 0;

    void print() const override;
};

class LessComparisonOperator : public ComparisonOperatorToken {

public:
    explicit LessComparisonOperator(TokenOrigin originPos_) : ComparisonOperatorToken(originPos_, LESS) { }

    const char * getSymbol() const override {
        return "<";
    }
};

class LessOrEqualComparisonOperator : public ComparisonOperatorToken {

public:
    explicit LessOrEqualComparisonOperator(TokenOrigin originPos_) : ComparisonOperatorToken(originPos_, LESS_OR_EQUAL) { }

    const char * getSymbol() const override {
        return "<=";
    }
};

class GreaterComparisonOperator : public ComparisonOperatorToken {

public:
    explicit GreaterComparisonOperator(TokenOrigin originPos_) : ComparisonOperatorToken(originPos_, GREATER) { }

    const char * getSymbol() const override {
        return ">";
    }
};

class GreaterOrEqualComparisonOperator : public ComparisonOperatorToken {

public:
    explicit GreaterOrEqualComparisonOperator(TokenOrigin originPos_) : ComparisonOperatorToken(originPos_, GREATER_OR_EQUAL) { }

    const char * getSymbol() const override {
        return ">=";
    }
};

class EqualComparisonOperator : public ComparisonOperatorToken {

public:
    explicit EqualComparisonOperator(TokenOrigin originPos_) : ComparisonOperatorToken(originPos_, EQUAL) { }

    const char * getSymbol() const override {
        return "==";
    }
};

class NotEqualComparisonOperator : public ComparisonOperatorToken {

public:
    explicit NotEqualComparisonOperator(TokenOrigin originPos_) : ComparisonOperatorToken(originPos_, NOT_EQUAL) { }

    const char * getSymbol() const override {
        return "!=";
    }
};

class IdToken : public Token {

private:
    char* name;

public:
    IdToken(TokenOrigin originPos_, const char* name_) : Token(ID, originPos_) {
        name = (char*)calloc(MAX_ID_LENGTH + 1, sizeof(char)); // +1 is for '\0'
        for (unsigned short i = 0; i < MAX_ID_LENGTH; ++i) {
            name[i] = name_[i];
            if (name[i] == '\0') break;
        }
    }

    ~IdToken() override {
        free(name);
    }

    inline char* getName() const {
        return name;
    }

    void print() const override;
};

class SemicolonToken : public Token {
public:
    explicit SemicolonToken(TokenOrigin originPos_) : Token(SEMICOLON, originPos_) { }

    void print() const override;
};

class IfToken : public Token {
public:
    explicit IfToken(TokenOrigin originPos_) : Token(IF, originPos_) { }
};

class ElseToken : public Token {
public:
    explicit ElseToken(TokenOrigin originPos_) : Token(ELSE, originPos_) { }
};

class WhileToken : public Token {
public:
    explicit WhileToken(TokenOrigin originPos_) : Token(WHILE, originPos_) { }
};

class FuncToken : public Token {
public:
    explicit FuncToken(TokenOrigin originPos_) : Token(FUNC, originPos_) { }
};

class VarToken : public Token {
public:
    explicit VarToken(TokenOrigin originPos_) : Token(VAR, originPos_) { }
};

class ValToken : public Token {
public:
    explicit ValToken(TokenOrigin originPos_) : Token(VAL, originPos_) { }
};

class CommaToken : public Token {
public:
    explicit CommaToken(TokenOrigin originPos_) : Token(COMMA, originPos_) { }
};

class ReturnToken : public Token {
public:
    explicit ReturnToken(TokenOrigin originPos_) : Token(RETURN, originPos_) { }
};

bool isOpenCurlyParenthesisToken(const std::shared_ptr<Token>& token);
bool isCloseCurlyParenthesisToken(const std::shared_ptr<Token>& token);
bool isOpenRoundParenthesisToken(const std::shared_ptr<Token>& token);
bool isCloseRoundParenthesisToken(const std::shared_ptr<Token>& token);
bool isExpressionOperator(const std::shared_ptr<Token>& token);
bool isTermOperator(const std::shared_ptr<Token>& token);

/**
 * Splits the expression into Token objects.
 * @param expression expression to tokenize
 * @return vector of parsed tokens.
 * @throws SyntaxError if invalid symbol met.
 */
std::vector<std::shared_ptr<Token>> tokenize(char* expression);

#endif // COMPILER_TOKENIZER_H

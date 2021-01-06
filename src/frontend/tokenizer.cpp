/**
 * @file
 * @brief Implementation of tokenizer functions
 */
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <cmath>
#include "tokenizer.h"

void Token::print() const {
    printf("%s", TokenTypeStrings[type]);
}

void ConstantValueToken::print() const {
    Token::print();
    printf(" VALUE=%lf", value);
}

double ConstantValueToken::calculate(size_t argc, ...) const {
    assert(argc == 0);
    return value;
}

void ParenthesisToken::print() const {
    Token::print();
    printf(" %s", open ? "OPEN" : "CLOSE");
}

double ParenthesisToken::calculate(size_t argc __attribute__((unused)), ...) const {
    throw std::logic_error("Parenthesis can't be calculated");
}

void OperatorToken::print() const {
    Token::print();
    printf(" ARITY=%zu, PRECEDENCE=%zu, TYPE=%s", arity, precedence, OperatorTypeStrings[operatorType]);
}

double AdditionOperator::calculate(size_t argc, ...) const {
    assert(argc == 2);
    va_list operands;
    va_start(operands, argc);
    double leftOperand = va_arg(operands, double);
    double rightOperand = va_arg(operands, double);
    va_end(operands);
    return leftOperand + rightOperand;
}

double SubtractionOperator::calculate(size_t argc, ...) const {
    assert(argc == 2);
    va_list operands;
    va_start(operands, argc);
    double leftOperand = va_arg(operands, double);
    double rightOperand = va_arg(operands, double);
    va_end(operands);
    return leftOperand - rightOperand;
}

double MultiplicationOperator::calculate(size_t argc, ...) const {
    assert(argc == 2);
    va_list operands;
    va_start(operands, argc);
    double leftOperand = va_arg(operands, double);
    double rightOperand = va_arg(operands, double);
    va_end(operands);
    return leftOperand * rightOperand;
}

double DivisionOperator::calculate(size_t argc, ...) const {
    assert(argc == 2);
    va_list operands;
    va_start(operands, argc);
    double leftOperand = va_arg(operands, double);
    double rightOperand = va_arg(operands, double);
    va_end(operands);
    return leftOperand / rightOperand;
}

double ArithmeticNegationOperator::calculate(size_t argc, ...) const {
    assert(argc == 1);
    va_list operands;
    va_start(operands, argc);
    double operand = va_arg(operands, double);
    va_end(operands);
    return -operand;
}

double UnaryAdditionOperator::calculate(size_t argc, ...) const {
    assert(argc == 1);
    va_list operands;
    va_start(operands, argc);
    double operand = va_arg(operands, double);
    va_end(operands);
    return operand;
}

double PowerOperator::calculate(size_t argc, ...) const {
    assert(argc == 2);
    va_list operands;
    va_start(operands, argc);
    double leftOperand = va_arg(operands, double);
    double rightOperand = va_arg(operands, double);
    va_end(operands);
    return pow(leftOperand, rightOperand);
}

void VariableToken::print() const {
    Token::print();
    printf(" NAME=%s", name);
}

double VariableToken::calculate(size_t argc __attribute__((unused)), ...) const {
    throw std::logic_error("Variable can't be calculated");
}

void FunctionToken::print() const {
    Token::print();
    printf(" ARITY=%zu, NAME=%s", arity, getName());
}

void SemicolonToken::print() const {
    Token::print();
    printf(" SEMICOLON");
}

double SemicolonToken::calculate(size_t argc __attribute__((unused)), ...) const {
    throw std::logic_error("Semicolon can't be calculated");
}

static bool addNextToken(char*& expression, const char* expressionStart, std::vector<std::shared_ptr<Token>>& tokens);

/**
 * Splits the expression into Token objects.
 * @param expression expression to tokenize
 * @return vector of parsed tokens.
 * @throws std::invalid_argument if invalid symbol met.
 */
std::vector<std::shared_ptr<Token>> tokenize(char* expression) {
    assert(expression != nullptr);

    const char* expressionStart = expression;
    std::vector<std::shared_ptr<Token>> tokens;
    while (addNextToken(expression, expressionStart, tokens))
        ;
    return tokens;
}

static bool addNextToken(char*& expression, const char* expressionStart, std::vector<std::shared_ptr<Token>>& tokens) {
    assert(expression != nullptr);

    while (std::isspace(*expression)) {
        ++expression;
    }
    if (*expression == '\0') return false;

    if (*expression == ';') {
        tokens.emplace_back(new SemicolonToken(expression - expressionStart));
        ++expression;
    } else if (*expression == '(') {
        tokens.emplace_back(new ParenthesisToken(expression - expressionStart, true, ParenthesisType::ROUND));
        ++expression;
    } else if (*expression == ')') {
        tokens.emplace_back(new ParenthesisToken(expression - expressionStart, false, ParenthesisType::ROUND));
        ++expression;
    } else if (*expression == '{') {
        tokens.emplace_back(new ParenthesisToken(expression - expressionStart, true, ParenthesisType::CURLY));
        ++expression;
    } else if (*expression == '}') {
        tokens.emplace_back(new ParenthesisToken(expression - expressionStart, false, ParenthesisType::CURLY));
        ++expression;
    } else if (*expression == '*') {
        tokens.emplace_back(new MultiplicationOperator(expression - expressionStart));
        ++expression;
    } else if (*expression == '/') {
        tokens.emplace_back(new DivisionOperator(expression - expressionStart));
        ++expression;
    } else if ((*expression == '+') || (*expression == '-')) {
        Token* previousToken = nullptr;
        if (!tokens.empty()) {
            previousToken = tokens[tokens.size() - 1].get();
        }

        bool isBinary = (previousToken != nullptr) && (
            (previousToken->getType() == CONSTANT_VALUE) ||
            (previousToken->getType() == VARIABLE) || (
                (previousToken->getType() == PARENTHESIS) &&
                (dynamic_cast<ParenthesisToken*>(previousToken)->isClose())
            )
        );

        if (isBinary) {
            if (*expression == '+') {
                tokens.emplace_back(new AdditionOperator(expression - expressionStart));
            } else {
                tokens.emplace_back(new SubtractionOperator(expression - expressionStart));
            }
        } else {
            if (*expression == '+') {
                tokens.emplace_back(new UnaryAdditionOperator(expression - expressionStart));
            } else {
                tokens.emplace_back(new ArithmeticNegationOperator(expression - expressionStart));
            }
        }

        ++expression;
    } else if (*expression == '^') {
        tokens.emplace_back(new PowerOperator(expression - expressionStart));
        ++expression;
    } else if (isdigit(*expression)) {
        double tokenValue = strtod(expression, &expression);
        tokens.emplace_back(new ConstantValueToken(expression - expressionStart, tokenValue));
    } else if (isalpha(*expression)) { // Variable name starts with letter
        char* name = (char*)calloc(VariableToken::MAX_NAME_LENGTH, sizeof(char));
        unsigned int i = 0;
        do {
            name[i++] = *expression++;
        } while (i < VariableToken::MAX_NAME_LENGTH && (isalpha(*expression) || isdigit(*expression))); // Other symbols in the name can be letters or digits
        tokens.emplace_back(new VariableToken(expression - expressionStart - i, name));
        free(name);
    } else {
        char message[26];
        snprintf(message, sizeof(message), "Invalid symbol found: '%c'", *expression);
        throw std::invalid_argument(message);
    }

    return true;
}

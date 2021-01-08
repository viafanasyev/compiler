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

void ParenthesisToken::print() const {
    Token::print();
    printf(" %s", open ? "OPEN" : "CLOSE");
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

double AssignmentOperator::calculate(size_t argc __attribute__((unused)), ...) const {
    throw std::logic_error("Assignment can't be calculated");
}

void ComparisonOperatorToken::print() const {
    Token::print();
    printf(" TYPE=%s", ComparisonOperatorTypeStrings[operatorType]);
}

void VariableToken::print() const {
    Token::print();
    printf(" NAME=%s", name);
}

void FunctionToken::print() const {
    Token::print();
    printf(" ARITY=%zu, NAME=%s", arity, getName());
}

void SemicolonToken::print() const {
    Token::print();
    printf(" SEMICOLON");
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

    size_t currentTokenOrigin = expression - expressionStart;
    if (*expression == ';') {
        tokens.emplace_back(new SemicolonToken(currentTokenOrigin));
        ++expression;
    } else if (*expression == '(') {
        tokens.emplace_back(new ParenthesisToken(currentTokenOrigin, true, ParenthesisType::ROUND));
        ++expression;
    } else if (*expression == ')') {
        tokens.emplace_back(new ParenthesisToken(currentTokenOrigin, false, ParenthesisType::ROUND));
        ++expression;
    } else if (*expression == '{') {
        tokens.emplace_back(new ParenthesisToken(currentTokenOrigin, true, ParenthesisType::CURLY));
        ++expression;
    } else if (*expression == '}') {
        tokens.emplace_back(new ParenthesisToken(currentTokenOrigin, false, ParenthesisType::CURLY));
        ++expression;
    } else if (*expression == '*') {
        tokens.emplace_back(new MultiplicationOperator(currentTokenOrigin));
        ++expression;
    } else if (*expression == '/') {
        tokens.emplace_back(new DivisionOperator(currentTokenOrigin));
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
                tokens.emplace_back(new AdditionOperator(currentTokenOrigin));
            } else {
                tokens.emplace_back(new SubtractionOperator(currentTokenOrigin));
            }
        } else {
            if (*expression == '+') {
                tokens.emplace_back(new UnaryAdditionOperator(currentTokenOrigin));
            } else {
                tokens.emplace_back(new ArithmeticNegationOperator(currentTokenOrigin));
            }
        }

        ++expression;
    } else if (*expression == '^') {
        tokens.emplace_back(new PowerOperator(currentTokenOrigin));
        ++expression;
    } else if (*expression == '<') {
        ++expression;
        if (*expression == '=') {
            tokens.emplace_back(new LessOrEqualComparisonOperator(currentTokenOrigin));
            ++expression;
        } else {
            tokens.emplace_back(new LessComparisonOperator(currentTokenOrigin));
        }
    } else if (*expression == '>') {
        ++expression;
        if (*expression == '=') {
            tokens.emplace_back(new GreaterOrEqualComparisonOperator(currentTokenOrigin));
            ++expression;
        } else {
            tokens.emplace_back(new GreaterComparisonOperator(currentTokenOrigin));
        }
    } else if (*expression == '=') {
        ++expression;
        if (*expression == '=') {
            tokens.emplace_back(new EqualComparisonOperator(currentTokenOrigin));
            ++expression;
        } else {
            tokens.emplace_back(new AssignmentOperator(currentTokenOrigin));
        }
    } else if (isdigit(*expression)) {
        double tokenValue = strtod(expression, &expression);
        tokens.emplace_back(new ConstantValueToken(currentTokenOrigin, tokenValue));
    } else if (isalpha(*expression)) { // Variable name starts with letter
        char* name = (char*)calloc(VariableToken::MAX_NAME_LENGTH, sizeof(char));
        unsigned int i = 0;
        do {
            name[i++] = *expression++;
        } while (i < VariableToken::MAX_NAME_LENGTH && (isalpha(*expression) || isdigit(*expression))); // Other symbols in the name can be letters or digits
        if (strcmp(name, "if") == 0) {
            tokens.emplace_back(new IfToken(currentTokenOrigin));
        } else if (strcmp(name, "else") == 0) {
            tokens.emplace_back(new ElseToken(currentTokenOrigin));
        } else if (strcmp(name, "while") == 0) {
            tokens.emplace_back(new WhileToken(currentTokenOrigin));
        } else {
            tokens.emplace_back(new VariableToken(currentTokenOrigin, name));
        }
        free(name);
    } else {
        char message[26];
        snprintf(message, sizeof(message), "Invalid symbol found: '%c'", *expression);
        throw std::invalid_argument(message);
    }

    return true;
}

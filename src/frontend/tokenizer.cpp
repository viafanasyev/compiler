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
#include "../util/constants.h"
#include "../util/SyntaxError.h"
#include "../util/TokenOrigin.h"

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

void IdToken::print() const {
    Token::print();
    printf(" NAME=%s", name);
}

void SemicolonToken::print() const {
    Token::print();
    printf(" SEMICOLON");
}

bool isOpenCurlyParenthesisToken(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::PARENTHESIS) return false;
    auto parenthesisToken = dynamic_cast<ParenthesisToken*>(token.get());
    return parenthesisToken->isOpen() && (parenthesisToken->getParenthesisType() == ParenthesisType::CURLY);
}

bool isCloseCurlyParenthesisToken(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::PARENTHESIS) return false;
    auto parenthesisToken = dynamic_cast<ParenthesisToken*>(token.get());
    return parenthesisToken->isClose() && (parenthesisToken->getParenthesisType() == ParenthesisType::CURLY);
}

bool isOpenRoundParenthesisToken(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::PARENTHESIS) return false;
    auto parenthesisToken = dynamic_cast<ParenthesisToken*>(token.get());
    return parenthesisToken->isOpen() && (parenthesisToken->getParenthesisType() == ParenthesisType::ROUND);
}

bool isCloseRoundParenthesisToken(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::PARENTHESIS) return false;
    auto parenthesisToken = dynamic_cast<ParenthesisToken*>(token.get());
    return parenthesisToken->isClose() && (parenthesisToken->getParenthesisType() == ParenthesisType::ROUND);
}

bool isExpressionOperator(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::OPERATOR) return false;
    auto operatorToken = std::dynamic_pointer_cast<OperatorToken>(token);
    return operatorToken->getOperatorType() == OperatorType::ADDITION || operatorToken->getOperatorType() == OperatorType::SUBTRACTION;
}

bool isTermOperator(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::OPERATOR) return false;
    auto operatorToken = std::dynamic_pointer_cast<OperatorToken>(token);
    return operatorToken->getOperatorType() == OperatorType::MULTIPLICATION || operatorToken->getOperatorType() == OperatorType::DIVISION;
}

bool isFactorOperator(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::OPERATOR) return false;
    auto operatorToken = std::dynamic_pointer_cast<OperatorToken>(token);
    return operatorToken->getOperatorType() == OperatorType::POWER;
}

static bool addNextToken(char*& expression, TokenOrigin& currentTokenOrigin, std::vector<std::shared_ptr<Token>>& tokens);

/**
 * Splits the expression into Token objects.
 * @param expression expression to tokenize
 * @return vector of parsed tokens.
 * @throws SyntaxError if invalid symbol met.
 */
std::vector<std::shared_ptr<Token>> tokenize(char* expression) {
    assert(expression != nullptr);

    TokenOrigin currentTokenOrigin = {1, 1};
    std::vector<std::shared_ptr<Token>> tokens;
    while (addNextToken(expression, currentTokenOrigin, tokens))
        ;
    return tokens;
}

static bool addNextToken(char*& expression, TokenOrigin& currentTokenOrigin, std::vector<std::shared_ptr<Token>>& tokens) {
    assert(expression != nullptr);

    while (std::isspace(*expression)) {
        if (*expression == '\n') {
            currentTokenOrigin.column = 1;
            ++currentTokenOrigin.line;
        } else {
            ++currentTokenOrigin.column;
        }
        ++expression;
    }
    if (*expression == '\0') return false;

    const char* currentTokenStart = expression;
    if (*expression == ';') {
        tokens.emplace_back(new SemicolonToken(currentTokenOrigin));
        ++expression;
    } else if (*expression == ',') {
        tokens.emplace_back(new CommaToken(currentTokenOrigin));
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
        auto previousToken = !tokens.empty() ? tokens[tokens.size() - 1] : nullptr;

        bool isBinary = (previousToken != nullptr) && (
            (previousToken->getType() == CONSTANT_VALUE) ||
            (previousToken->getType() == ID) ||
            (isCloseRoundParenthesisToken(previousToken))
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
    } else if (strncmp(expression, "!=", 2) == 0) {
        tokens.emplace_back(new NotEqualComparisonOperator(currentTokenOrigin));
        expression += 2;
    } else if (isdigit(*expression)) {
        double tokenValue = strtod(expression, &expression);
        tokens.emplace_back(new ConstantValueToken(currentTokenOrigin, tokenValue));
    } else if (isalpha(*expression)) { // Name starts with letter
        char* name = (char*)calloc(MAX_ID_LENGTH + 1, sizeof(char)); // +1 is for '\0'
        unsigned short i = 0;
        do {
            name[i++] = *expression++;
        } while (i < MAX_ID_LENGTH && (isalpha(*expression) || isdigit(*expression))); // Other symbols in the name can be letters or digits
        if (strcmp(name, "if") == 0) {
            tokens.emplace_back(new IfToken(currentTokenOrigin));
        } else if (strcmp(name, "else") == 0) {
            tokens.emplace_back(new ElseToken(currentTokenOrigin));
        } else if (strcmp(name, "while") == 0) {
            tokens.emplace_back(new WhileToken(currentTokenOrigin));
        } else if (strcmp(name, "func") == 0) {
            tokens.emplace_back(new FuncToken(currentTokenOrigin));
        } else if (strcmp(name, "var") == 0) {
            tokens.emplace_back(new VarToken(currentTokenOrigin));
        } else if (strcmp(name, "return") == 0) {
            tokens.emplace_back(new ReturnToken(currentTokenOrigin));
        } else {
            tokens.emplace_back(new IdToken(currentTokenOrigin, name));
        }
        free(name);
    } else {
        char message[26];
        snprintf(message, sizeof(message), "Invalid symbol '%c' found", *expression);
        throw SyntaxError(currentTokenOrigin, message);
    }

    currentTokenOrigin.column += expression - currentTokenStart;
    return true;
}

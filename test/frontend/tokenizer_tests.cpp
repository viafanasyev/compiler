/**
 * @file
 * @brief Tests for tokenizer functions
 */
#include <cstring>
#include <stdexcept>
#include <vector>
#include "../testlib.h"
#include "../../src/frontend/tokenizer.h"

#define ASSERT_CONSTANT_VALUE_TOKEN(token, value) do {                                                                 \
    ASSERT_NOT_NULL(token);                                                                                            \
    ASSERT_EQUALS(token->getType(), CONSTANT_VALUE);                                                                   \
    auto constantValueToken = dynamic_cast<ConstantValueToken*>(token.get());                                          \
    ASSERT_NOT_NULL(constantValueToken);                                                                               \
    ASSERT_DOUBLE_EQUALS(constantValueToken->getValue(), value);                                                       \
} while(0)

#define ASSERT_PARENTHESIS_TOKEN(token, open, type) do {                                                               \
    ASSERT_NOT_NULL(token);                                                                                            \
    ASSERT_EQUALS(token->getType(), PARENTHESIS);                                                                      \
    auto parenthesisToken = dynamic_cast<ParenthesisToken*>(token.get());                                              \
    ASSERT_NOT_NULL(parenthesisToken);                                                                                 \
    ASSERT_EQUALS(parenthesisToken->isOpen(), open);                                                                   \
    ASSERT_EQUALS(parenthesisToken->getParenthesisType(), type);                                                                  \
} while(0)

#define ASSERT_OPERATOR_TOKEN(token, arity, precedence, operatorType) do {                                             \
    ASSERT_NOT_NULL(token);                                                                                            \
    ASSERT_EQUALS(token->getType(), OPERATOR);                                                                         \
    auto operatorToken = dynamic_cast<OperatorToken*>(token.get());                                                    \
    ASSERT_NOT_NULL(operatorToken);                                                                                    \
    ASSERT_EQUALS(operatorToken->getArity(), arity);                                                                   \
    ASSERT_EQUALS(operatorToken->getPrecedence(), precedence);                                                         \
    ASSERT_EQUALS(operatorToken->getOperatorType(), operatorType);                                                     \
} while (0)

#define ASSERT_COMPARISON_OPERATOR_TOKEN(token, operatorType) do {                                                     \
    ASSERT_NOT_NULL(token);                                                                                            \
    ASSERT_EQUALS(token->getType(), COMPARISON_OPERATOR);                                                              \
    auto operatorToken = dynamic_cast<ComparisonOperatorToken*>(token.get());                                          \
    ASSERT_NOT_NULL(operatorToken);                                                                                    \
    ASSERT_EQUALS(operatorToken->getOperatorType(), operatorType);                                                     \
} while (0)

#define ASSERT_VARIABLE_TOKEN(token, name) do {                                                                        \
    ASSERT_NOT_NULL(token);                                                                                            \
    ASSERT_EQUALS(token->getType(), VARIABLE);                                                                         \
    auto variableToken = dynamic_cast<VariableToken*>(token.get());                                                    \
    ASSERT_NOT_NULL(variableToken);                                                                                    \
    ASSERT_TRUE(strcmp(variableToken->getName(), name) == 0);                                                          \
} while(0)

#define ASSERT_IF_TOKEN(token) do {                                                                                    \
    ASSERT_NOT_NULL(token);                                                                                            \
    ASSERT_EQUALS(token->getType(), IF);                                                                               \
    auto operatorToken = dynamic_cast<IfToken*>(token.get());                                                          \
    ASSERT_NOT_NULL(operatorToken);                                                                                    \
} while (0)

#define ASSERT_ELSE_TOKEN(token) do {                                                                                    \
    ASSERT_NOT_NULL(token);                                                                                            \
    ASSERT_EQUALS(token->getType(), ELSE);                                                                               \
    auto operatorToken = dynamic_cast<ElseToken*>(token.get());                                                          \
    ASSERT_NOT_NULL(operatorToken);                                                                                    \
} while (0)

#define ASSERT_WHILE_TOKEN(token) do {                                                                                    \
    ASSERT_NOT_NULL(token);                                                                                            \
    ASSERT_EQUALS(token->getType(), WHILE);                                                                               \
    auto operatorToken = dynamic_cast<WhileToken*>(token.get());                                                          \
    ASSERT_NOT_NULL(operatorToken);                                                                                    \
} while (0)

TEST(tokenize, simpleExpression) {
    char* expression = (char*)"1*(2+3)";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 7);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[0], 1);
    ASSERT_OPERATOR_TOKEN(tokens[1], 2, 2, MULTIPLICATION);
    ASSERT_PARENTHESIS_TOKEN(tokens[2], true, ROUND);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[3], 2);
    ASSERT_OPERATOR_TOKEN(tokens[4], 2, 1, ADDITION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[5], 3);
    ASSERT_PARENTHESIS_TOKEN(tokens[6], false, ROUND);
}

TEST(tokenize, simpleExpressionWithSpaces) {
    char* expression = (char*)"    1* ( 2  +        3  )    ";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 7);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[0], 1);
    ASSERT_OPERATOR_TOKEN(tokens[1], 2, 2, MULTIPLICATION);
    ASSERT_PARENTHESIS_TOKEN(tokens[2], true, ROUND);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[3], 2);
    ASSERT_OPERATOR_TOKEN(tokens[4], 2, 1, ADDITION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[5], 3);
    ASSERT_PARENTHESIS_TOKEN(tokens[6], false, ROUND);
}

TEST(tokenize, multipleArithmeticNegationOperators) {
    char* expression = (char*)"-1 * -2 / --(4 --5)";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 14);
    ASSERT_OPERATOR_TOKEN(tokens[0], 1, 1000, ARITHMETIC_NEGATION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[1], 1);
    ASSERT_OPERATOR_TOKEN(tokens[2], 2, 2, MULTIPLICATION);
    ASSERT_OPERATOR_TOKEN(tokens[3], 1, 1000, ARITHMETIC_NEGATION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 2);
    ASSERT_OPERATOR_TOKEN(tokens[5], 2, 2, DIVISION);
    ASSERT_OPERATOR_TOKEN(tokens[6], 1, 1000, ARITHMETIC_NEGATION);
    ASSERT_OPERATOR_TOKEN(tokens[7], 1, 1000, ARITHMETIC_NEGATION);
    ASSERT_PARENTHESIS_TOKEN(tokens[8], true, ROUND);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[9], 4);
    ASSERT_OPERATOR_TOKEN(tokens[10], 2, 1, SUBTRACTION);
    ASSERT_OPERATOR_TOKEN(tokens[11], 1, 1000, ARITHMETIC_NEGATION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[12], 5);
    ASSERT_PARENTHESIS_TOKEN(tokens[13], false, ROUND);
}

TEST(tokenize, multiplePlusAndMinusSignsBeforeValues) {
    char* expression = (char*)"-+-+-5";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 6);
    ASSERT_OPERATOR_TOKEN(tokens[0], 1, 1000, ARITHMETIC_NEGATION);
    ASSERT_OPERATOR_TOKEN(tokens[1], 1, 1000, UNARY_ADDITION);
    ASSERT_OPERATOR_TOKEN(tokens[2], 1, 1000, ARITHMETIC_NEGATION);
    ASSERT_OPERATOR_TOKEN(tokens[3], 1, 1000, UNARY_ADDITION);
    ASSERT_OPERATOR_TOKEN(tokens[4], 1, 1000, ARITHMETIC_NEGATION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[5], 5);
}

TEST(tokenize, realConstant) {
    char* expression = (char*)"-5.25";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 2);
    ASSERT_OPERATOR_TOKEN(tokens[0], 1, 1000, ARITHMETIC_NEGATION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[1], 5.25);
}

TEST(tokenize, realConstantInExponentionalForm) {
    char* expression = (char*)"-1e9";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 2);
    ASSERT_OPERATOR_TOKEN(tokens[0], 1, 1000, ARITHMETIC_NEGATION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[1], 1e9);
}

TEST(tokenize, invalidToken) {
    char* expression = (char*)"1/_";
    try {
        tokenize(expression);
        ASSERT_TRUE(false);
    } catch (std::invalid_argument& ex) {
        ASSERT_TRUE(strcmp(ex.what(), "Invalid symbol found: '_'") == 0);
    }
}

TEST(tokenize, invalidConstant) {
    char* expression = (char*)"1.5.5";
    try {
        tokenize(expression);
        ASSERT_TRUE(false);
    } catch (std::invalid_argument& ex) {
        ASSERT_TRUE(strcmp(ex.what(), "Invalid symbol found: '.'") == 0);
    }
}

TEST(tokenize, simpleExpressionWithVariables) {
    char* expression = (char*)"x+5*const-tmp";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 7);
    ASSERT_VARIABLE_TOKEN(tokens[0], "x");
    ASSERT_OPERATOR_TOKEN(tokens[1], 2, 1, ADDITION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[2], 5);
    ASSERT_OPERATOR_TOKEN(tokens[3], 2, 2, MULTIPLICATION);
    ASSERT_VARIABLE_TOKEN(tokens[4], "const");
    ASSERT_OPERATOR_TOKEN(tokens[5], 2, 1, SUBTRACTION);
    ASSERT_VARIABLE_TOKEN(tokens[6], "tmp");
}

TEST(tokenize, simpleExpressionWithMultiplePowerOperations) {
    char* expression = (char*)"x + x^2 + x^y^z";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 11);
    ASSERT_VARIABLE_TOKEN(tokens[0], "x");
    ASSERT_OPERATOR_TOKEN(tokens[1], 2, 1, ADDITION);
    ASSERT_VARIABLE_TOKEN(tokens[2], "x");
    ASSERT_OPERATOR_TOKEN(tokens[3], 2, 3, POWER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 2);
    ASSERT_OPERATOR_TOKEN(tokens[5], 2, 1, ADDITION);
    ASSERT_VARIABLE_TOKEN(tokens[6], "x");
    ASSERT_OPERATOR_TOKEN(tokens[7], 2, 3, POWER);
    ASSERT_VARIABLE_TOKEN(tokens[8], "y");
    ASSERT_OPERATOR_TOKEN(tokens[9], 2, 3, POWER);
    ASSERT_VARIABLE_TOKEN(tokens[10], "z");
}

TEST(tokenize, simpleExpressionWithLessComparisonOperator) {
    char* expression = (char*)"x + x^2 < y + y^2";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 11);
    ASSERT_VARIABLE_TOKEN(tokens[0], "x");
    ASSERT_OPERATOR_TOKEN(tokens[1], 2, 1, ADDITION);
    ASSERT_VARIABLE_TOKEN(tokens[2], "x");
    ASSERT_OPERATOR_TOKEN(tokens[3], 2, 3, POWER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 2);
    ASSERT_COMPARISON_OPERATOR_TOKEN(tokens[5], LESS);
    ASSERT_VARIABLE_TOKEN(tokens[6], "y");
    ASSERT_OPERATOR_TOKEN(tokens[7], 2, 1, ADDITION);
    ASSERT_VARIABLE_TOKEN(tokens[8], "y");
    ASSERT_OPERATOR_TOKEN(tokens[9], 2, 3, POWER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[10], 2);
}

TEST(tokenize, simpleExpressionWithLessOrEqualComparisonOperator) {
    char* expression = (char*)"x + x^2 <= y + y^2";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 11);
    ASSERT_VARIABLE_TOKEN(tokens[0], "x");
    ASSERT_OPERATOR_TOKEN(tokens[1], 2, 1, ADDITION);
    ASSERT_VARIABLE_TOKEN(tokens[2], "x");
    ASSERT_OPERATOR_TOKEN(tokens[3], 2, 3, POWER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 2);
    ASSERT_COMPARISON_OPERATOR_TOKEN(tokens[5], LESS_OR_EQUAL);
    ASSERT_VARIABLE_TOKEN(tokens[6], "y");
    ASSERT_OPERATOR_TOKEN(tokens[7], 2, 1, ADDITION);
    ASSERT_VARIABLE_TOKEN(tokens[8], "y");
    ASSERT_OPERATOR_TOKEN(tokens[9], 2, 3, POWER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[10], 2);
}

TEST(tokenize, simpleExpressionWithGreaterComparisonOperator) {
    char* expression = (char*)"x + x^2 > y + y^2";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 11);
    ASSERT_VARIABLE_TOKEN(tokens[0], "x");
    ASSERT_OPERATOR_TOKEN(tokens[1], 2, 1, ADDITION);
    ASSERT_VARIABLE_TOKEN(tokens[2], "x");
    ASSERT_OPERATOR_TOKEN(tokens[3], 2, 3, POWER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 2);
    ASSERT_COMPARISON_OPERATOR_TOKEN(tokens[5], GREATER);
    ASSERT_VARIABLE_TOKEN(tokens[6], "y");
    ASSERT_OPERATOR_TOKEN(tokens[7], 2, 1, ADDITION);
    ASSERT_VARIABLE_TOKEN(tokens[8], "y");
    ASSERT_OPERATOR_TOKEN(tokens[9], 2, 3, POWER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[10], 2);
}

TEST(tokenize, simpleExpressionWithGreaterOrEqualComparisonOperator) {
    char* expression = (char*)"x + x^2 >= y + y^2";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 11);
    ASSERT_VARIABLE_TOKEN(tokens[0], "x");
    ASSERT_OPERATOR_TOKEN(tokens[1], 2, 1, ADDITION);
    ASSERT_VARIABLE_TOKEN(tokens[2], "x");
    ASSERT_OPERATOR_TOKEN(tokens[3], 2, 3, POWER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 2);
    ASSERT_COMPARISON_OPERATOR_TOKEN(tokens[5], GREATER_OR_EQUAL);
    ASSERT_VARIABLE_TOKEN(tokens[6], "y");
    ASSERT_OPERATOR_TOKEN(tokens[7], 2, 1, ADDITION);
    ASSERT_VARIABLE_TOKEN(tokens[8], "y");
    ASSERT_OPERATOR_TOKEN(tokens[9], 2, 3, POWER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[10], 2);
}

TEST(tokenize, simpleExpressionWithEqualComparisonOperator) {
    char* expression = (char*)"x + x^2 == y + y^2";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 11);
    ASSERT_VARIABLE_TOKEN(tokens[0], "x");
    ASSERT_OPERATOR_TOKEN(tokens[1], 2, 1, ADDITION);
    ASSERT_VARIABLE_TOKEN(tokens[2], "x");
    ASSERT_OPERATOR_TOKEN(tokens[3], 2, 3, POWER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 2);
    ASSERT_COMPARISON_OPERATOR_TOKEN(tokens[5], EQUAL);
    ASSERT_VARIABLE_TOKEN(tokens[6], "y");
    ASSERT_OPERATOR_TOKEN(tokens[7], 2, 1, ADDITION);
    ASSERT_VARIABLE_TOKEN(tokens[8], "y");
    ASSERT_OPERATOR_TOKEN(tokens[9], 2, 3, POWER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[10], 2);
}

TEST(tokenize, simpleIfStatement) {
    char* expression = (char*)"if (x > 0) { x + 1 }";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 11);
    ASSERT_IF_TOKEN(tokens[0]);
    ASSERT_PARENTHESIS_TOKEN(tokens[1], true, ROUND);
    ASSERT_VARIABLE_TOKEN(tokens[2], "x");
    ASSERT_COMPARISON_OPERATOR_TOKEN(tokens[3], GREATER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 0);
    ASSERT_PARENTHESIS_TOKEN(tokens[5], false, ROUND);
    ASSERT_PARENTHESIS_TOKEN(tokens[6], true, CURLY);
    ASSERT_VARIABLE_TOKEN(tokens[7], "x");
    ASSERT_OPERATOR_TOKEN(tokens[8], 2, 1, ADDITION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[9], 1);
    ASSERT_PARENTHESIS_TOKEN(tokens[10], false, CURLY);
}

TEST(tokenize, simpleIfElseStatement) {
    char* expression = (char*)"if (x > 0) { x + 1 } else { x - 1 }";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 17);
    ASSERT_IF_TOKEN(tokens[0]);
    ASSERT_PARENTHESIS_TOKEN(tokens[1], true, ROUND);
    ASSERT_VARIABLE_TOKEN(tokens[2], "x");
    ASSERT_COMPARISON_OPERATOR_TOKEN(tokens[3], GREATER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 0);
    ASSERT_PARENTHESIS_TOKEN(tokens[5], false, ROUND);
    ASSERT_PARENTHESIS_TOKEN(tokens[6], true, CURLY);
    ASSERT_VARIABLE_TOKEN(tokens[7], "x");
    ASSERT_OPERATOR_TOKEN(tokens[8], 2, 1, ADDITION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[9], 1);
    ASSERT_PARENTHESIS_TOKEN(tokens[10], false, CURLY);
    ASSERT_ELSE_TOKEN(tokens[11]);
    ASSERT_PARENTHESIS_TOKEN(tokens[12], true, CURLY);
    ASSERT_VARIABLE_TOKEN(tokens[13], "x");
    ASSERT_OPERATOR_TOKEN(tokens[14], 2, 1, SUBTRACTION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[15], 1);
    ASSERT_PARENTHESIS_TOKEN(tokens[16], false, CURLY);
}

TEST(tokenize, variableNameStartsWithIf) {
    char* expression = (char*)"ifconfig (x > 0) { x + 1 }";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 11);
    ASSERT_VARIABLE_TOKEN(tokens[0], "ifconfig");
    ASSERT_PARENTHESIS_TOKEN(tokens[1], true, ROUND);
    ASSERT_VARIABLE_TOKEN(tokens[2], "x");
    ASSERT_COMPARISON_OPERATOR_TOKEN(tokens[3], GREATER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 0);
    ASSERT_PARENTHESIS_TOKEN(tokens[5], false, ROUND);
    ASSERT_PARENTHESIS_TOKEN(tokens[6], true, CURLY);
    ASSERT_VARIABLE_TOKEN(tokens[7], "x");
    ASSERT_OPERATOR_TOKEN(tokens[8], 2, 1, ADDITION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[9], 1);
    ASSERT_PARENTHESIS_TOKEN(tokens[10], false, CURLY);
}

TEST(tokenize, variableNameStartsWithElse) {
    char* expression = (char*)"if (x > 0) { x + 1 } elseif { x - 1 }";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 17);
    ASSERT_IF_TOKEN(tokens[0]);
    ASSERT_PARENTHESIS_TOKEN(tokens[1], true, ROUND);
    ASSERT_VARIABLE_TOKEN(tokens[2], "x");
    ASSERT_COMPARISON_OPERATOR_TOKEN(tokens[3], GREATER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 0);
    ASSERT_PARENTHESIS_TOKEN(tokens[5], false, ROUND);
    ASSERT_PARENTHESIS_TOKEN(tokens[6], true, CURLY);
    ASSERT_VARIABLE_TOKEN(tokens[7], "x");
    ASSERT_OPERATOR_TOKEN(tokens[8], 2, 1, ADDITION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[9], 1);
    ASSERT_PARENTHESIS_TOKEN(tokens[10], false, CURLY);
    ASSERT_VARIABLE_TOKEN(tokens[11], "elseif");
    ASSERT_PARENTHESIS_TOKEN(tokens[12], true, CURLY);
    ASSERT_VARIABLE_TOKEN(tokens[13], "x");
    ASSERT_OPERATOR_TOKEN(tokens[14], 2, 1, SUBTRACTION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[15], 1);
    ASSERT_PARENTHESIS_TOKEN(tokens[16], false, CURLY);
}

TEST(tokenize, simpleWhileStatement) {
    char* expression = (char*)"while (x > 0) { x - 1 }";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 11);
    ASSERT_WHILE_TOKEN(tokens[0]);
    ASSERT_PARENTHESIS_TOKEN(tokens[1], true, ROUND);
    ASSERT_VARIABLE_TOKEN(tokens[2], "x");
    ASSERT_COMPARISON_OPERATOR_TOKEN(tokens[3], GREATER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 0);
    ASSERT_PARENTHESIS_TOKEN(tokens[5], false, ROUND);
    ASSERT_PARENTHESIS_TOKEN(tokens[6], true, CURLY);
    ASSERT_VARIABLE_TOKEN(tokens[7], "x");
    ASSERT_OPERATOR_TOKEN(tokens[8], 2, 1, SUBTRACTION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[9], 1);
    ASSERT_PARENTHESIS_TOKEN(tokens[10], false, CURLY);
}

TEST(tokenize, variableNameStartsWithWhile) {
    char* expression = (char*)"whiled (x > 0) { x - 1 }";

    std::vector<std::shared_ptr<Token>> tokens = tokenize(expression);

    ASSERT_EQUALS(tokens.size(), 11);
    ASSERT_VARIABLE_TOKEN(tokens[0], "whiled");
    ASSERT_PARENTHESIS_TOKEN(tokens[1], true, ROUND);
    ASSERT_VARIABLE_TOKEN(tokens[2], "x");
    ASSERT_COMPARISON_OPERATOR_TOKEN(tokens[3], GREATER);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[4], 0);
    ASSERT_PARENTHESIS_TOKEN(tokens[5], false, ROUND);
    ASSERT_PARENTHESIS_TOKEN(tokens[6], true, CURLY);
    ASSERT_VARIABLE_TOKEN(tokens[7], "x");
    ASSERT_OPERATOR_TOKEN(tokens[8], 2, 1, SUBTRACTION);
    ASSERT_CONSTANT_VALUE_TOKEN(tokens[9], 1);
    ASSERT_PARENTHESIS_TOKEN(tokens[10], false, CURLY);
}

// TODO: Add AST and TeX tests for expressions like (a1^a2)^a3, a1^a2^a3 and (x - y) ^ -(x + y)

/**
 * @file
 * @brief Implementation of recursive parser
 *
 * It parses mathematical expressions using next grammar:
 *
 *     G = E '\0'
 *     E = T ([+|-] T)*
 *     T = F ([*|/] F)*
 *     F = P (^ P)*
 *     P = '(' E ')' | N | ID | ID '(' E ')'
 *     N = [0-9]+
 *     ID = [a-zA-Z]+
 *
 * This is an AST building copy of https://github.com/viafanasyev/recursive-parser
 */
#include <cstdlib>
#include <cctype>
#include <vector>
#include "recursive_parser.h"
#include "SyntaxError.h"

SymbolTable::SymbolTable() {
    addFunction("sin", std::make_shared<SinFunction>());
    addFunction("cos", std::make_shared<CosFunction>());
    addFunction("tg" , std::make_shared<TgFunction >());
    addFunction("ctg", std::make_shared<CtgFunction>());
    addFunction("ln" , std::make_shared<LnFunction >());
}

void SymbolTable::addFunction(const char* name, const std::shared_ptr<FunctionToken>& functionToken) noexcept {
    symbols[name] = functionToken;
}

void SymbolTable::addVariable(char* name) noexcept {
    symbols[name] = VariableToken::getVariableByName(name);
}

std::shared_ptr<Token> SymbolTable::getSymbolByName(char* name) noexcept {
    if (symbols.count(name) == 0) {
        addVariable(name);
    }
    return symbols.at(name);
}

SymbolTable symbolTable;

std::shared_ptr<ASTNode> getExpression(const char* expression, int& pos);

std::shared_ptr<ASTNode> getTerm(const char* expression, int& pos);

std::shared_ptr<ASTNode> getFactor(const char* expression, int& pos);

std::shared_ptr<ASTNode> getParenthesised(const char* expression, int& pos);

std::shared_ptr<ASTNode> getNumber(const char* expression, int& pos);

std::shared_ptr<Token> getId(const char* expression, int& pos);

void skipSpaces(const char* expression, int& pos);

std::shared_ptr<ASTNode> buildASTRecursively(const char* expression) {
    int pos = 0;
    skipSpaces(expression, pos);
    std::shared_ptr<ASTNode> root = getExpression(expression, pos);
    if (expression[pos] != '\0') {
        throw SyntaxError(pos, "Invalid symbol");
    }
    ++pos;
    return root;
}

std::shared_ptr<ASTNode> getExpression(const char* expression, int& pos) {
    std::shared_ptr<ASTNode> result = getTerm(expression, pos);
    skipSpaces(expression, pos);
    std::shared_ptr<ASTNode> term = nullptr;
    std::shared_ptr<OperatorToken> token;
    while (expression[pos] == '+' || expression[pos] == '-') {
        if (expression[pos] == '+') {
            token = std::make_shared<AdditionOperator>();
        } else {
            token = std::make_shared<SubtractionOperator>();
        }
        ++pos;
        skipSpaces(expression, pos);

        term = getTerm(expression, pos);
        skipSpaces(expression, pos);

        result = std::make_shared<ASTNode>(token, result, term);
    }
    return result;
}

std::shared_ptr<ASTNode> getTerm(const char* expression, int& pos) {
    std::shared_ptr<ASTNode> result = getFactor(expression, pos);
    skipSpaces(expression, pos);
    std::shared_ptr<ASTNode> factor = nullptr;
    std::shared_ptr<OperatorToken> token;
    while (expression[pos] == '*' || expression[pos] == '/') {
        if (expression[pos] == '*') {
            token = std::make_shared<MultiplicationOperator>();
        } else {
            token = std::make_shared<DivisionOperator>();
        }
        ++pos;
        skipSpaces(expression, pos);

        factor = getFactor(expression, pos);
        skipSpaces(expression, pos);

        result = std::make_shared<ASTNode>(token, result, factor);
    }
    return result;
}

std::shared_ptr<ASTNode> getFactor(const char* expression, int& pos) {
    std::shared_ptr<ASTNode> result = getParenthesised(expression, pos);
    skipSpaces(expression, pos);
    std::shared_ptr<ASTNode> operand = nullptr;
    std::vector<std::shared_ptr<ASTNode>> operands;
    while (expression[pos] == '^') {
        ++pos;
        skipSpaces(expression, pos);

        operand = getParenthesised(expression, pos);
        skipSpaces(expression, pos);

        operands.push_back(operand);
    }
    if (!operands.empty()) { // Calculating right-to-left because '^' is right-associative
        size_t i = operands.size() - 1;
        while (i > 0) {
            operands[i - 1] = std::make_shared<ASTNode>(std::make_shared<PowerOperator>(), operands[i - 1], operands[i]);
            --i;
        }
        result = std::make_shared<ASTNode>(std::make_shared<PowerOperator>(), result, operands[0]);
    }
    return result;
}

std::shared_ptr<ASTNode> getParenthesised(const char* expression, int& pos) {
    std::shared_ptr<Token> idToken = nullptr;
    if (expression[pos] != '(') {
        if (isdigit(expression[pos])) {
            return getNumber(expression, pos);
        } else if (isalpha(expression[pos])) {
            idToken = getId(expression, pos);
            skipSpaces(expression, pos);
            if (expression[pos] != '(') {
                if (idToken->getType() == TokenType::FUNCTION) {
                    throw SyntaxError(pos, "Expected open parenthesis");
                } else {
                    return std::make_shared<ASTNode>(idToken);
                }
            }
        } else {
            throw SyntaxError(pos, "Invalid symbol");
        }
    }
    assert(expression[pos] == '(');
    ++pos;
    skipSpaces(expression, pos);

    std::shared_ptr<ASTNode> result = getExpression(expression, pos);

    if (expression[pos] != ')') {
        throw SyntaxError(pos, "Expected closing parenthesis");
    }
    ++pos;

    if (idToken != nullptr) {
        assert(idToken->getType() == TokenType::FUNCTION);
        result = std::make_shared<ASTNode>(idToken, result);
    }
    return result;
}

std::shared_ptr<ASTNode> getNumber(const char* expression, int &pos) {
    int result = 0;
    const int startPos = pos;
    while (isdigit(expression[pos])) {
        result = result * 10 + (expression[pos++] - '0');
    }
    if (pos == startPos) {
        throw SyntaxError(pos, "Expected number");
    }
    return std::make_shared<ASTNode>(std::make_shared<ConstantValueToken>(result));
}

std::shared_ptr<Token> getId(const char* expression, int& pos) {
    int startPos = pos;
    while (isalpha(expression[pos])) {
        ++pos;
    }
    if (pos == startPos) {
        throw SyntaxError(pos, "Expected id");
    }

    char* name = (char*)calloc(pos - startPos + 1, sizeof(char));
    for (int i = startPos; i < pos; ++i) {
        name[i - startPos] = expression[i];
    }
    std::shared_ptr<Token> id = symbolTable.getSymbolByName(name);
    free(name);

    return id;
}

void skipSpaces(const char* expression, int& pos) {
    while (std::isspace(expression[pos])) ++pos;
}

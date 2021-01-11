/**
 * @file
 * @brief Implementation of recursive parser
 *
 * It parses programs using next grammar:
 *
 *     G = Statements '\0'
 *     Statements = (Statement)*
 *     Statement = Expression ';' | Block | IfStatement | WhileStatement
 *     Block = '{' Statements '}'
 *     IfStatement = 'if' '(' ComparisonExpression ')' Statement ('else' Statement)?
 *     WhileStatement = 'while' '(' ComparisonExpression ')' Statement
 *     ComparisonExpression = Expression [< > == <= >=] Expression
 *     Expression = Term ([+ -] Term)* | Assignment
 *     Term = Factor ([* /] Factor)*
 *     Factor = Parenthesised (^ Parenthesised)*
 *     Parenthesised = '(' Expression ')' | Number | ID
 *     Assignment = ID '=' Expression
 *     Number = [0-9]+
 *     ID = [a-z A-Z]+
 */
#include <vector>
#include "recursive_parser.h"
#include "SyntaxError.h"

std::shared_ptr<StatementsNode> getStatements(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ASTNode> getStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<BlockNode> getBlock(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ASTNode> getIfStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<WhileNode> getWhileStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ComparisonOperatorNode> getComparisonExpression(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ASTNode> getExpression(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ASTNode> getTerm(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ASTNode> getFactor(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ASTNode> getParenthesised(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<OperatorNode> getAssignment(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ConstantValueNode> getNumber(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<VariableNode> getId(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);

static inline bool isOpenCurlyParenthesisToken(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::PARENTHESIS) return false;
    auto parenthesisToken = dynamic_cast<ParenthesisToken*>(token.get());
    return parenthesisToken->isOpen() && (parenthesisToken->getParenthesisType() == ParenthesisType::CURLY);
}

static inline bool isCloseCurlyParenthesisToken(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::PARENTHESIS) return false;
    auto parenthesisToken = dynamic_cast<ParenthesisToken*>(token.get());
    return parenthesisToken->isClose() && (parenthesisToken->getParenthesisType() == ParenthesisType::CURLY);
}

static inline bool isOpenRoundParenthesisToken(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::PARENTHESIS) return false;
    auto parenthesisToken = dynamic_cast<ParenthesisToken*>(token.get());
    return parenthesisToken->isOpen() && (parenthesisToken->getParenthesisType() == ParenthesisType::ROUND);
}

static inline bool isCloseRoundParenthesisToken(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::PARENTHESIS) return false;
    auto parenthesisToken = dynamic_cast<ParenthesisToken*>(token.get());
    return parenthesisToken->isClose() && (parenthesisToken->getParenthesisType() == ParenthesisType::ROUND);
}

static inline bool isExpressionOperator(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::OPERATOR) return false;
    auto operatorToken = std::dynamic_pointer_cast<OperatorToken>(token);
    return operatorToken->getOperatorType() == OperatorType::ADDITION || operatorToken->getOperatorType() == OperatorType::SUBTRACTION;
}

static inline bool isTermOperator(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::OPERATOR) return false;
    auto operatorToken = std::dynamic_pointer_cast<OperatorToken>(token);
    return operatorToken->getOperatorType() == OperatorType::MULTIPLICATION || operatorToken->getOperatorType() == OperatorType::DIVISION;
}

static inline bool isFactorOperator(const std::shared_ptr<Token>& token) {
    if (token->getType() != TokenType::OPERATOR) return false;
    auto operatorToken = std::dynamic_pointer_cast<OperatorToken>(token);
    return operatorToken->getOperatorType() == OperatorType::POWER;
}

std::shared_ptr<StatementsNode> buildASTRecursively(char* expression) {
    auto tokens = tokenize(expression);
    size_t pos = 0;

    std::shared_ptr<StatementsNode> root = getStatements(tokens, pos);
    if (pos < tokens.size()) {
        throw SyntaxError(tokens[pos]->getOriginPos(), "Invalid symbol");
    }
    return root;
}

std::shared_ptr<StatementsNode> getStatements(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    std::vector<std::shared_ptr<ASTNode>> statements;
    while (pos < tokens.size() && !isCloseCurlyParenthesisToken(tokens[pos])) {
        statements.push_back(getStatement(tokens, pos));
    }
    return std::make_shared<StatementsNode>(statements);
}

std::shared_ptr<ASTNode> getStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    std::shared_ptr<ASTNode> statement = nullptr;
    if (pos >= tokens.size()) throw SyntaxError("Expected statement, but got EOF");
    if (isOpenCurlyParenthesisToken(tokens[pos])) {
        statement = getBlock(tokens, pos);
    } else if (tokens[pos]->getType() == TokenType::IF) {
        statement = getIfStatement(tokens, pos);
    } else if (tokens[pos]->getType() == TokenType::WHILE) {
        statement = getWhileStatement(tokens, pos);
    } else {
        statement = getExpression(tokens, pos);
        if (pos >= tokens.size()) throw SyntaxError("Expected ';', but got EOF");
        if (tokens[pos]->getType() != TokenType::SEMICOLON) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected ';'");
        ++pos;
    }
    return statement;
}

std::shared_ptr<BlockNode> getBlock(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected '{', but got EOF");
    if (!isOpenCurlyParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected '{'");
    ++pos;

    auto statements = getStatements(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected '}', but got EOF");
    if (!isCloseCurlyParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected '}'");
    ++pos;

    return std::make_shared<BlockNode>(statements);
}

std::shared_ptr<ASTNode> getIfStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected 'if', but got EOF");
    if (tokens[pos]->getType() != TokenType::IF) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected 'if'");
    ++pos;

    if (pos >= tokens.size()) throw SyntaxError("Expected '(', but got EOF");
    if (!isOpenRoundParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected '('");
    ++pos;

    auto condition = getComparisonExpression(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected ')', but got EOF");
    if (!isCloseRoundParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected ')'");
    ++pos;

    auto body = getStatement(tokens, pos);

    if (pos < tokens.size() && tokens[pos]->getType() == TokenType::ELSE) {
        ++pos;
        auto elseBody = getStatement(tokens, pos);
        return std::make_shared<IfElseNode>(condition, body, elseBody);
    }
    return std::make_shared<IfNode>(condition, body);
}

std::shared_ptr<WhileNode> getWhileStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected 'while', but got EOF");
    if (tokens[pos]->getType() != TokenType::WHILE) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected 'while'");
    ++pos;

    if (pos >= tokens.size()) throw SyntaxError("Expected '(', but got EOF");
    if (!isOpenRoundParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected '('");
    ++pos;

    auto condition = getComparisonExpression(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected ')', but got EOF");
    if (!isCloseRoundParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected ')'");
    ++pos;

    auto body = getStatement(tokens, pos);

    return std::make_shared<WhileNode>(condition, body);
}

std::shared_ptr<ComparisonOperatorNode> getComparisonExpression(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    std::shared_ptr<ASTNode> lhs = getExpression(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected comparison operator, but got EOF");
    if (tokens[pos]->getType() != COMPARISON_OPERATOR) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected comparison operator");
    auto operatorToken = std::dynamic_pointer_cast<ComparisonOperatorToken>(tokens[pos]);
    ++pos;

    std::shared_ptr<ASTNode> rhs = getExpression(tokens, pos);

    return std::make_shared<ComparisonOperatorNode>(operatorToken, lhs, rhs);
}

std::shared_ptr<ASTNode> getExpression(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected expression, but got EOF");

    if (
        tokens[pos]->getType() == TokenType::VARIABLE &&
        pos + 1 < tokens.size() &&
        tokens[pos + 1]->getType() == TokenType::OPERATOR &&
        std::dynamic_pointer_cast<OperatorToken>(tokens[pos + 1])->getOperatorType() == OperatorType::ASSIGNMENT
    ) {
        return getAssignment(tokens, pos);
    }

    std::shared_ptr<ASTNode> result = getTerm(tokens, pos);
    std::shared_ptr<ASTNode> term = nullptr;
    std::shared_ptr<OperatorToken> token = nullptr;
    while (pos < tokens.size() && isExpressionOperator(tokens[pos])) {
        assert(tokens[pos]->getType() == TokenType::OPERATOR);
        token = std::dynamic_pointer_cast<OperatorToken>(tokens[pos]);
        ++pos;

        term = getTerm(tokens, pos);

        result = std::make_shared<OperatorNode>(token, result, term);
    }
    return result;
}

std::shared_ptr<ASTNode> getTerm(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    std::shared_ptr<ASTNode> result = getFactor(tokens, pos);
    std::shared_ptr<ASTNode> factor = nullptr;
    std::shared_ptr<OperatorToken> token = nullptr;
    while (pos < tokens.size() && isTermOperator(tokens[pos])) {
        assert(tokens[pos]->getType() == TokenType::OPERATOR);
        token = std::dynamic_pointer_cast<OperatorToken>(tokens[pos]);
        ++pos;

        factor = getFactor(tokens, pos);

        result = std::make_shared<OperatorNode>(token, result, factor);
    }
    return result;
}

std::shared_ptr<ASTNode> getFactor(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    std::shared_ptr<ASTNode> result = getParenthesised(tokens, pos);
    std::shared_ptr<ASTNode> operand = nullptr;
    std::vector<std::shared_ptr<ASTNode>> operands;
    std::vector<std::shared_ptr<OperatorToken>> operators;
    while (pos < tokens.size() && isFactorOperator(tokens[pos])) {
        assert(tokens[pos]->getType() == TokenType::OPERATOR);
        operators.push_back(std::dynamic_pointer_cast<OperatorToken>(tokens[pos]));
        ++pos;

        operand = getParenthesised(tokens, pos);

        operands.push_back(operand);
    }
    if (!operands.empty()) { // Calculating right-to-left because '^' is right-associative
        size_t i = operands.size() - 1;
        while (i > 0) {
            operands[i - 1] = std::make_shared<OperatorNode>(operators[i], operands[i - 1], operands[i]);
            --i;
        }
        result = std::make_shared<OperatorNode>(operators[0], result, operands[0]);
    }
    return result;
}

std::shared_ptr<ASTNode> getParenthesised(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected number, identifier or '(', but got EOF");

    if (!isOpenRoundParenthesisToken(tokens[pos])) {
        if (tokens[pos]->getType() == TokenType::CONSTANT_VALUE) return getNumber(tokens, pos);
        if (tokens[pos]->getType() == TokenType::VARIABLE) return getId(tokens, pos);
        throw SyntaxError(tokens[pos]->getOriginPos(), "Expected number, identifier or '('");
    }
    assert(isOpenRoundParenthesisToken(tokens[pos]));
    ++pos;

    std::shared_ptr<ASTNode> result = getExpression(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected ')', but got EOF");
    if (!isCloseRoundParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected ')'");
    ++pos;

    return result;
}

std::shared_ptr<OperatorNode> getAssignment(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected assignment, but got EOF");
    if (tokens[pos]->getType() != TokenType::VARIABLE) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected identifier, but got EOF");
    auto id = getId(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected '=', but got EOF");
    std::shared_ptr<OperatorToken> operatorToken = nullptr;
    if (
        tokens[pos]->getType() != TokenType::OPERATOR ||
        (operatorToken = std::dynamic_pointer_cast<OperatorToken>(tokens[pos]))->getOperatorType() != OperatorType::ASSIGNMENT
    ) {
        throw SyntaxError(tokens[pos]->getOriginPos(), "Expected '='");
    }
    ++pos;

    auto assignedExpression = getExpression(tokens, pos);

    return std::make_shared<OperatorNode>(operatorToken, id, assignedExpression);
}

std::shared_ptr<ConstantValueNode> getNumber(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected number, but got EOF");
    if (tokens[pos]->getType() != TokenType::CONSTANT_VALUE) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected number");
    const double value = dynamic_cast<ConstantValueToken*>(tokens[pos].get())->getValue();
    ++pos;
    return std::make_shared<ConstantValueNode>(value);
}

std::shared_ptr<VariableNode> getId(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected id, but got EOF");
    if (tokens[pos]->getType() != TokenType::VARIABLE) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected id");
    auto variableToken = dynamic_cast<VariableToken*>(tokens[pos].get());
    ++pos;
    return std::make_shared<VariableNode>(variableToken->getName(), variableToken->getOriginPos());
}

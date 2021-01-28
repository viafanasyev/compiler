/**
 * @file
 * @brief Implementation of recursive parser
 *
 * It parses programs using next grammar:
 *
 *     G = OuterScopeStatements '\0'
 *     OuterScopeStatements = (OuterScopeStatement)*
 *     FunctionScopeStatements = (FunctionScopeStatement)*
 *     OuterScopeStatement = FunctionDefinition
 *     FunctionScopeStatement = Expression ';' | Assignment ';' | VariableDeclaration | Block | IfStatement | WhileStatement | ReturnStatement
 *     Block = '{' FunctionScopeStatements '}'
 *     IfStatement = IfStatementHeader FunctionScopeStatement ('else' FunctionScopeStatement)?
 *     IfStatementHeader = 'if' '(' ComparisonExpression ')'
 *     WhileStatement = WhileStatementHeader FunctionScopeStatement
 *     WhileStatementHeader = 'while' '(' ComparisonExpression ')'
 *     ComparisonExpression = Expression [< > == <= >=] Expression
 *     FunctionDefinition = 'func' ID '(' ParametersList ')' Block
 *     ParametersList = ( Variable (',' Variable)* )?
 *     ReturnStatement = 'return' Expression ';'
 *     VariableDeclaration = 'var' Variable ('=' Expression)? ';'
 *     Expression = Term ([+ -] Term)*
 *     Term = Factor ([* /] Factor)*
 *     Factor = ('+' | '-') Factor | '(' Expression ')' | Number | Variable | FunctionCall
 *     Assignment = Variable '=' Expression
 *     FunctionCall = ID '(' ArgumentsList ')'
 *     ArgumentsList = ( Expression (',' Expression)* )?
 *     Variable = ID
 *     Number = [0-9]+
 *     ID = [a-z A-Z] [a-z A-Z 0-9]*
 */
#include <vector>
#include "recursive_parser.h"
#include "../util/SyntaxError.h"

std::shared_ptr<StatementsNode> getOuterScopeStatements(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<StatementsNode> getFunctionScopeStatements(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ASTNode> getOuterScopeStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ASTNode> getFunctionScopeStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<BlockNode> getBlock(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ASTNode> getIfStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ComparisonOperatorNode> getIfStatementHeader(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<WhileNode> getWhileStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ComparisonOperatorNode> getWhileStatementHeader(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ComparisonOperatorNode> getComparisonExpression(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<FunctionDefinitionNode> getFunctionDefinition(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ParametersListNode> getParametersList(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ReturnStatementNode> getReturnStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<VariableDeclarationNode> getVariableDeclaration(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ASTNode> getExpression(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ASTNode> getTerm(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ASTNode> getFactor(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<AssignmentOperatorNode> getAssignment(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<FunctionCallNode> getFunctionCall(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ArgumentsListNode> getArgumentsList(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<VariableNode> getVariable(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<ConstantValueNode> getNumber(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);
std::shared_ptr<IdToken> getId(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos);

static inline std::shared_ptr<ASTNode> wrapIntoBlockIfNeeded(const std::shared_ptr<ASTNode>& node) {
    if (node->getType() == BLOCK_NODE) return node;
    return std::make_shared<BlockNode>(std::make_shared<StatementsNode>(node));
}

static inline bool isAssignment(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    return  pos + 1 < tokens.size() &&
            tokens[pos]->getType() == TokenType::ID &&
            tokens[pos + 1]->getType() == TokenType::ASSIGNMENT_OPERATOR;
}

std::shared_ptr<StatementsNode> buildASTRecursively(char* expression) {
    auto tokens = tokenize(expression);
    size_t pos = 0;

    std::shared_ptr<StatementsNode> root = getOuterScopeStatements(tokens, pos);
    if (pos < tokens.size()) {
        throw SyntaxError(tokens[pos]->getOriginPos(), "Invalid symbol");
    }
    return root;
}

std::shared_ptr<StatementsNode> getOuterScopeStatements(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    std::vector<std::shared_ptr<ASTNode>> statements;
    while (pos < tokens.size() && !isCloseCurlyParenthesisToken(tokens[pos])) {
        statements.push_back(getOuterScopeStatement(tokens, pos));
    }
    return std::make_shared<StatementsNode>(statements);
}

std::shared_ptr<StatementsNode> getFunctionScopeStatements(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    std::vector<std::shared_ptr<ASTNode>> statements;
    while (pos < tokens.size() && !isCloseCurlyParenthesisToken(tokens[pos])) {
        statements.push_back(getFunctionScopeStatement(tokens, pos));
    }
    return std::make_shared<StatementsNode>(statements);
}

std::shared_ptr<ASTNode> getOuterScopeStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    std::shared_ptr<ASTNode> statement = nullptr;
    if (pos >= tokens.size()) throw SyntaxError("Expected outer scope statement, but got EOF");
    if (tokens[pos]->getType() == TokenType::FUNC) {
        statement = getFunctionDefinition(tokens, pos);
    } else {
        throw SyntaxError(tokens[pos]->getOriginPos(), "Expected function definition");
    }
    return statement;
}

std::shared_ptr<ASTNode> getFunctionScopeStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    std::shared_ptr<ASTNode> statement = nullptr;
    if (pos >= tokens.size()) throw SyntaxError("Expected function scope statement, but got EOF");
    if (isOpenCurlyParenthesisToken(tokens[pos])) {
        statement = getBlock(tokens, pos);
    } else if (tokens[pos]->getType() == TokenType::IF) {
        statement = getIfStatement(tokens, pos);
    } else if (tokens[pos]->getType() == TokenType::WHILE) {
        statement = getWhileStatement(tokens, pos);
    } else if (tokens[pos]->getType() == TokenType::VAR) {
        statement = getVariableDeclaration(tokens, pos);
    } else if (tokens[pos]->getType() == TokenType::RETURN) {
        statement = getReturnStatement(tokens, pos);
    } else {
        if (isAssignment(tokens, pos)) {
            statement = getAssignment(tokens, pos);
        } else {
            statement = getExpression(tokens, pos);
        }

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

    auto statements = getFunctionScopeStatements(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected '}', but got EOF");
    if (!isCloseCurlyParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected '}'");
    ++pos;

    return std::make_shared<BlockNode>(statements);
}

std::shared_ptr<ASTNode> getIfStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected function scope if statement, but got EOF");

    auto condition = getIfStatementHeader(tokens, pos);
    auto body = wrapIntoBlockIfNeeded(getFunctionScopeStatement(tokens, pos)); // Single-statement if wrapped into block for proper variable scopes
    if (pos < tokens.size() && tokens[pos]->getType() == TokenType::ELSE) {
        ++pos;
        auto elseBody = wrapIntoBlockIfNeeded(getFunctionScopeStatement(tokens, pos)); // Single-statement else wrapped into block for proper variable scopes
        return std::make_shared<IfElseNode>(condition, body, elseBody);
    }
    return std::make_shared<IfNode>(condition, body);
}

std::shared_ptr<ComparisonOperatorNode> getIfStatementHeader(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
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

    return condition;
}

std::shared_ptr<WhileNode> getWhileStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected function scope while statement, but got EOF");

    auto condition = getWhileStatementHeader(tokens, pos);
    auto body = wrapIntoBlockIfNeeded(getFunctionScopeStatement(tokens, pos)); // Single-statement while wrapped into block for proper variable scopes
    return std::make_shared<WhileNode>(condition, body);
}

std::shared_ptr<ComparisonOperatorNode> getWhileStatementHeader(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
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

    return condition;
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

std::shared_ptr<FunctionDefinitionNode> getFunctionDefinition(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected function definition, but got EOF");
    if (tokens[pos]->getType() != TokenType::FUNC) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected 'func'");
    ++pos;

    auto functionName = getId(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected '(', but got EOF");
    if (!isOpenRoundParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected '('");
    ++pos;

    auto parameters = getParametersList(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected ')', but got EOF");
    if (!isCloseRoundParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected ')'");
    ++pos;

    auto definition = getBlock(tokens, pos);

    return std::make_shared<FunctionDefinitionNode>(functionName, parameters, definition);
}

std::shared_ptr<ParametersListNode> getParametersList(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected parameters list, but got EOF");
    if (isCloseRoundParenthesisToken(tokens[pos])) { // Check if this is an empty list
        return std::make_shared<ParametersListNode>();
    }

    std::vector<std::shared_ptr<ASTNode>> arguments = { getVariable(tokens, pos) };
    while (pos < tokens.size() && tokens[pos]->getType() == TokenType::COMMA) {
        ++pos;
        arguments.push_back(getVariable(tokens, pos));
    }
    return std::make_shared<ParametersListNode>(arguments);
}

std::shared_ptr<ReturnStatementNode> getReturnStatement(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected return statement, but got EOF");
    if (tokens[pos]->getType() != TokenType::RETURN) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected return");
    ++pos;

    auto returnedExpression = getExpression(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected ';', but got EOF");
    if (tokens[pos]->getType() != TokenType::SEMICOLON) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected ';'");
    ++pos;

    return std::make_shared<ReturnStatementNode>(returnedExpression);
}

std::shared_ptr<VariableDeclarationNode> getVariableDeclaration(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected variable declaration, but got EOF");
    if (tokens[pos]->getType() != TokenType::VAR) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected variable declaration");
    ++pos;

    auto variable = getVariable(tokens, pos);

    std::shared_ptr<ASTNode> initialValue = nullptr;
    if (pos >= tokens.size()) throw SyntaxError("Expected '=' or ';', but got EOF");
    if (tokens[pos]->getType() == TokenType::ASSIGNMENT_OPERATOR) {
        ++pos;
        initialValue = getExpression(tokens, pos);
    }

    if (pos >= tokens.size()) throw SyntaxError("Expected ';', but got EOF");
    if (tokens[pos]->getType() != TokenType::SEMICOLON) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected ';'");
    ++pos;

    return initialValue == nullptr
        ? std::make_shared<VariableDeclarationNode>(variable)
        : std::make_shared<VariableDeclarationNode>(variable, initialValue);
}

std::shared_ptr<ASTNode> getExpression(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected expression, but got EOF");

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
    if (pos >= tokens.size()) throw SyntaxError("Expected number, identifier, '(' or unary operator, but got EOF");

    if (tokens[pos]->getType() == TokenType::OPERATOR) {
        auto operatorToken = std::dynamic_pointer_cast<OperatorToken>(tokens[pos]);
        if (operatorToken->getOperatorType() == OperatorType::ARITHMETIC_NEGATION ||
            operatorToken->getOperatorType() == OperatorType::UNARY_ADDITION
        ) {
            ++pos;
            return std::make_shared<OperatorNode>(operatorToken, getFactor(tokens, pos));
        }
    }
    if (tokens[pos]->getType() == TokenType::CONSTANT_VALUE) return getNumber(tokens, pos);
    if (tokens[pos]->getType() == TokenType::ID) {
        if (pos + 1 < tokens.size() && isOpenRoundParenthesisToken(tokens[pos + 1])) return getFunctionCall(tokens, pos);
        return getVariable(tokens, pos);
    }

    if (!isOpenRoundParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected number, identifier,  '(' or unary operator");
    ++pos;

    std::shared_ptr<ASTNode> result = getExpression(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected ')', but got EOF");
    if (!isCloseRoundParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected ')'");
    ++pos;

    return result;
}

std::shared_ptr<AssignmentOperatorNode> getAssignment(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected assignment, but got EOF");
    if (tokens[pos]->getType() != TokenType::ID) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected identifier, but got EOF");
    auto id = getVariable(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected '=', but got EOF");
    std::shared_ptr<AssignmentOperatorToken> assignmentToken = nullptr;
    if (tokens[pos]->getType() != TokenType::ASSIGNMENT_OPERATOR) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected '='");
    ++pos;

    auto assignedExpression = getExpression(tokens, pos);

    return std::make_shared<AssignmentOperatorNode>(assignmentToken, id, assignedExpression);
}


std::shared_ptr<FunctionCallNode> getFunctionCall(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected function call, but got EOF");
    auto functionName = getId(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected '(', but got EOF");
    if (!isOpenRoundParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected '('");
    ++pos;

    auto arguments = getArgumentsList(tokens, pos);

    if (pos >= tokens.size()) throw SyntaxError("Expected ')', but got EOF");
    if (!isCloseRoundParenthesisToken(tokens[pos])) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected ')'");
    ++pos;

    return std::make_shared<FunctionCallNode>(functionName, arguments);
}

std::shared_ptr<ArgumentsListNode> getArgumentsList(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected arguments list, but got EOF");
    if (isCloseRoundParenthesisToken(tokens[pos])) { // Check if this is an empty list
        return std::make_shared<ArgumentsListNode>();
    }

    std::vector<std::shared_ptr<ASTNode>> arguments = { getExpression(tokens, pos) };
    while (pos < tokens.size() && tokens[pos]->getType() == TokenType::COMMA) {
        ++pos;
        arguments.push_back(getExpression(tokens, pos));
    }
    return std::make_shared<ArgumentsListNode>(arguments);
}

std::shared_ptr<VariableNode> getVariable(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    auto idToken = getId(tokens, pos);
    return std::make_shared<VariableNode>(idToken->getName(), idToken->getOriginPos());
}

std::shared_ptr<ConstantValueNode> getNumber(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected number, but got EOF");
    if (tokens[pos]->getType() != TokenType::CONSTANT_VALUE) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected number");
    const double value = dynamic_cast<ConstantValueToken*>(tokens[pos].get())->getValue();
    ++pos;
    return std::make_shared<ConstantValueNode>(value);
}

std::shared_ptr<IdToken> getId(const std::vector<std::shared_ptr<Token>>& tokens, size_t& pos) {
    if (pos >= tokens.size()) throw SyntaxError("Expected id, but got EOF");
    if (tokens[pos]->getType() != TokenType::ID) throw SyntaxError(tokens[pos]->getOriginPos(), "Expected id");
    auto idToken = std::dynamic_pointer_cast<IdToken>(tokens[pos]);
    ++pos;
    return idToken;
}

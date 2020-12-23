/**
 * @file
 * @brief Implementation of AST building functions
 */
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stack>
#include <stdexcept>
#include <vector>
#include "ast.h"
#include "tokenizer.h"

void ASTNode::print(int depth) const {
    for (int i = 0; i < depth; ++i) {
        printf("\t");
    }
    token->print();
    printf("\n");

    for (size_t i = 0; i < childrenNumber; ++i) {
        children[i]->print(depth + 1);
    }
}

void ASTNode::visualize(const char* fileName) const {
    assert(fileName != nullptr);

    char dotFileName[100];
    strcpy(dotFileName, fileName);
    strcat(dotFileName, ".dot");

    FILE* graphvizTextFile = fopen(dotFileName, "w");
    fprintf(graphvizTextFile, "digraph AST {\n");
    int nodeId = 0;
    dotPrint(graphvizTextFile, nodeId);
    fprintf(graphvizTextFile, "}\n");
    fclose(graphvizTextFile);

    char command[1000];
    sprintf(command, "dot -Tpng -o%s.png %s.dot && xdg-open %s.png", fileName, fileName, fileName);
    system(command);
}

void ASTNode::texify(const char* fileName) const {
    assert(fileName != nullptr);

    char texFileName[100];
    strcpy(texFileName, fileName);
    strcat(texFileName, ".tex");

    FILE* texFile = fopen(texFileName, "w");
    fprintf(texFile, "\\documentclass{article}\n\\begin{document}\n\\begin{center}\n\t$ ");
    texPrint(texFile);
    fprintf(texFile, " $\n\\end{center}\n\\end{document}\n");
    fclose(texFile);

    char command[1000];
    sprintf(command, "pdflatex %s.tex > .tmp_%s && rm .tmp_%s && rm %s.log && rm %s.aux && xdg-open %s.pdf",
            fileName, fileName, fileName, fileName, fileName, fileName);
    system(command);
}

double ASTNode::calculate() const {
    switch (childrenNumber) {
        case 0:
            return token->calculate(0);
        case 1:
            return token->calculate(1, children[0]->calculate());
        case 2:
            return token->calculate(2, children[0]->calculate(), children[1]->calculate());
        default:
            throw std::logic_error("Unsupported arity of operator. Only unary and binary are supported yet");
    }
}

void ASTNode::dotPrint(FILE* dotFile, int& nodeId) const {
    if (token->getType() == TokenType::CONSTANT_VALUE) {
        auto constantValueToken = dynamic_cast<ConstantValueToken*>(token.get());
        fprintf(dotFile, "%d [label=\"const\nvalue: %lg\", shape=box, style=filled, color=\"grey\", fillcolor=\"#FFFEC9\"];\n", nodeId, constantValueToken->getValue());
        ++nodeId;
    } else if (token->getType() == TokenType::VARIABLE) {
        auto variableToken = dynamic_cast<VariableToken*>(token.get());
        fprintf(dotFile, "%d [label=\"var\nname: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#99FF9D\"];\n", nodeId, variableToken->getName());
        ++nodeId;
    } else if (token->getType() == TokenType::OPERATOR) {
        auto operatorToken = dynamic_cast<OperatorToken*>(token.get());
        const char* operatorSymbol = operatorToken->getSymbol();
        if (operatorToken->getArity() == 1) {
            fprintf(dotFile, "%d [label=\"unary op\nop: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#C9E7FF\"];\n",
                    nodeId, operatorSymbol);
        } else if (operatorToken->getArity() == 2) {
            fprintf(dotFile, "%d [label=\"binary op\nop: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#C9E7FF\"];\n",
                    nodeId, operatorSymbol);
        } else {
            throw std::logic_error("Unsupported arity of operator. Only unary and binary are supported yet");
        }
        int childrenNodeId = nodeId + 1;
        for (size_t i = 0; i < childrenNumber; ++i) {
            fprintf(dotFile, "%d->%d\n", nodeId, childrenNodeId);
            children[i]->dotPrint(dotFile, childrenNodeId);
        }
        nodeId = childrenNodeId;
    } else if (token->getType() == TokenType::FUNCTION) {
        auto functionToken = dynamic_cast<FunctionToken*>(token.get());
        if (functionToken->getArity() == 1) {
            fprintf(dotFile, "%d [label=\"unary func\nfunc: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#C9E7FF\"];\n",
                    nodeId, functionToken->getName());
        } else {
            throw std::logic_error("Unsupported arity of function. Only unary are supported yet");
        }
        int childrenNodeId = nodeId + 1;
        for (size_t i = 0; i < childrenNumber; ++i) {
            fprintf(dotFile, "%d->%d\n", nodeId, childrenNodeId);
            children[i]->dotPrint(dotFile, childrenNodeId);
        }
        nodeId = childrenNodeId;
    } else if (token->getType() == TokenType::STATEMENTS) {
        fprintf(dotFile, "%d [label=\"statements\", shape=box, style=filled, color=\"grey\", fillcolor=\"grey\"];\n", nodeId);
        int childrenNodeId = nodeId + 1;
        for (size_t i = 0; i < childrenNumber; ++i) {
            fprintf(dotFile, "%d->%d\n", nodeId, childrenNodeId);
            children[i]->dotPrint(dotFile, childrenNodeId);
        }
        nodeId = childrenNodeId;
    } else if (token->getType() == TokenType::BLOCK) {
        fprintf(dotFile, "%d [label=\"block\", shape=box, style=filled, color=\"grey\", fillcolor=\"grey\"];\n", nodeId);
        int childrenNodeId = nodeId + 1;
        for (size_t i = 0; i < childrenNumber; ++i) {
            fprintf(dotFile, "%d->%d\n", nodeId, childrenNodeId);
            children[i]->dotPrint(dotFile, childrenNodeId);
        }
        nodeId = childrenNodeId;
    } else {
        throw std::logic_error("Unsupported token type");
    }
}

void ASTNode::texPrint(FILE* texFile, TexBraceType braceType) const {
    if (token->getType() == TokenType::CONSTANT_VALUE) {
        auto constantValueToken = dynamic_cast<ConstantValueToken*>(token.get());
        fprintf(texFile, "{%lg}", constantValueToken->getValue());
    } else if (token->getType() == TokenType::VARIABLE) {
        auto variableToken = dynamic_cast<VariableToken*>(token.get());
        fprintf(texFile, "{%s}", variableToken->getName());
    } else if (token->getType() == TokenType::OPERATOR) {
        auto operatorToken = dynamic_cast<OperatorToken*>(token.get());
        const char* operatorSymbol = operatorToken->getSymbol();
        if (operatorToken->getArity() == 1) {
            if (braceType != NONE) fprintf(texFile, braceType == ROUND ? "(" : "{");
            fprintf(texFile, "%s", operatorSymbol);
            children[0]->texPrint(texFile, ROUND);
            if (braceType != NONE) fprintf(texFile, braceType == ROUND ? ")" : "}");
        } else if (operatorToken->getArity() == 2) {
            if (operatorToken->getOperatorType() == OperatorType::DIVISION) {
                // No braces needed, because `\frac` can be safely used with `^` as `\frac{...}{...} ^ \frac{...}{...}`
                fprintf(texFile, "\\frac{");
                children[0]->texPrint(texFile, NONE);
                fprintf(texFile, "}{");
                children[1]->texPrint(texFile, NONE);
                fprintf(texFile, "}");
            } else {
                if (braceType != NONE) fprintf(texFile, braceType == ROUND ? "(" : "{");
                auto leftChild = children[0];
                TexBraceType leftChildBrace = getChildBraceType(operatorToken, leftChild->getToken().get(), false);
                leftChild->texPrint(texFile, leftChildBrace);

                fprintf(texFile, " %s ", operatorSymbol);

                auto rightChild = children[1];
                TexBraceType rightChildBrace = getChildBraceType(operatorToken, rightChild->getToken().get(), true);
                rightChild->texPrint(texFile, rightChildBrace);
                if (braceType != NONE) fprintf(texFile, braceType == ROUND ? ")" : "}");
            }
        } else {
            throw std::logic_error("Unsupported arity of operator. Only unary and binary are supported yet");
        }
    } else if (token->getType() == TokenType::FUNCTION) {
        auto functionToken = dynamic_cast<FunctionToken*>(token.get());
        if (functionToken->getArity() == 1) {
            fprintf(texFile, "{%s}", functionToken->getName());
            children[0]->texPrint(texFile, ROUND);
        } else {
            throw std::logic_error("Unsupported arity of function. Only unary are supported yet");
        }
    } else {
        throw std::logic_error("Unsupported token type");
    }
}

ASTNode::TexBraceType ASTNode::getChildBraceType(const OperatorToken* parentOperator, const Token* child, bool isRightChild) {
    if (child->getType() != TokenType::OPERATOR)
        return NONE;

    if (parentOperator->getOperatorType() == OperatorType::POWER)
        return isRightChild ? CURLY : ROUND;

    if (dynamic_cast<const OperatorToken*>(child)->getPrecedence() < parentOperator->getPrecedence())
        return ROUND;

    if (isRightChild && (parentOperator->getOperatorType() == OperatorType::SUBTRACTION) &&
        (dynamic_cast<const OperatorToken*>(child)->getPrecedence() == parentOperator->getPrecedence()))
        return ROUND;

    return NONE;
}

static inline void connectWithOperands(std::stack<std::shared_ptr<ASTNode> >& astNodes, const std::shared_ptr<Token>& parentNodeToken);

std::shared_ptr<ASTNode> buildAST(char* expression) {
    return buildAST(tokenize(expression));
}

std::shared_ptr<ASTNode> buildAST(const std::vector<std::shared_ptr<Token> >& infixNotationTokens) {
    std::stack<std::shared_ptr<Token> > stack;
    std::stack<std::shared_ptr<ASTNode> > astNodes;

    for (auto& token : infixNotationTokens) {
        if ((token->getType() == TokenType::CONSTANT_VALUE) || (token->getType() == TokenType::VARIABLE)) {
            astNodes.push(std::make_shared<ASTNode>(token));
        } else if (token->getType() == TokenType::PARENTHESIS) {
            auto parenthesisToken = dynamic_cast<ParenthesisToken*>(token.get());
            if (parenthesisToken->isOpen()) {
                stack.push(token);
            } else {
                while (!stack.empty() && (stack.top()->getType() != TokenType::PARENTHESIS)) {
                    auto& topToken = stack.top();
                    stack.pop();
                    connectWithOperands(astNodes, topToken);
                }
                if (stack.empty()) {
                    throw std::invalid_argument("Missing open parenthesis");
                }
                assert(dynamic_cast<ParenthesisToken*>(stack.top().get())->isOpen()); // There should be only open parentheses on the stack
                stack.pop();
            }
        } else if (token->getType() == TokenType::OPERATOR) {
            auto currentToken = dynamic_cast<OperatorToken*>(token.get());
            int currentTokenPrecedence = currentToken->getPrecedence();
            while (!stack.empty() && stack.top()->getType() == TokenType::OPERATOR) {
                int topTokenPrecedence = dynamic_cast<OperatorToken*>(stack.top().get())->getPrecedence();
                if ((topTokenPrecedence > currentTokenPrecedence) ||
                   ((topTokenPrecedence == currentTokenPrecedence) && currentToken->isLeftAssociative())
                ) {
                    connectWithOperands(astNodes, stack.top());
                    stack.pop();
                } else {
                    break;
                }
            }
            stack.push(token);
        } else {
            throw std::logic_error("Unsupported token type");
        }
    }

    while (!stack.empty()) {
        auto& token = stack.top();
        stack.pop();
        if (token->getType() == TokenType::PARENTHESIS) {
            throw std::invalid_argument("Unclosed parenthesis");
        }
        connectWithOperands(astNodes, token);
    }
    if (astNodes.size() != 1) {
        throw std::invalid_argument("Too much operands");
    }
    return astNodes.top();
}

static inline void connectWithOperands(std::stack<std::shared_ptr<ASTNode> >& astNodes, const std::shared_ptr<Token>& parentNodeToken) {
    assert(parentNodeToken->getType() == TokenType::OPERATOR);

    size_t parentNodeArity = dynamic_cast<OperatorToken*>(parentNodeToken.get())->getArity();
    if (astNodes.size() < parentNodeArity) {
        throw std::invalid_argument("Too little operands"); // TODO: More verbose error context (put stack size and parentNode operatorType?)
    }

    if (parentNodeArity == 1) {
        auto child = astNodes.top();
        astNodes.pop();
        astNodes.push(std::make_shared<ASTNode>(parentNodeToken, child));
    } else if (parentNodeArity == 2) {
        auto rightChild = astNodes.top();
        astNodes.pop();
        auto leftChild = astNodes.top();
        astNodes.pop();
        astNodes.push(std::make_shared<ASTNode>(parentNodeToken, leftChild, rightChild));
    } else {
        throw std::logic_error("Unsupported arity of operator. Only unary and binary are supported yet");
    }
}

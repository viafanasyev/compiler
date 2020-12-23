/**
 * @file
 * @brief Implementation of AST optimizers
 */
#include <cmath>
#include <memory>
#include "../frontend/ast.h"
#include "ast-optimizers.h"
#include "../frontend/tokenizer.h"

static constexpr double COMPARE_EPS = 1e-9;


std::shared_ptr<ASTNode>& Optimizer::optimize(std::shared_ptr<ASTNode>& node) const {
    if (optimizeChildrenFirst) {
        return optimizeCurrent(optimizeChildren(node));
    } else {
        return optimizeChildren(optimizeCurrent(node));
    }
}

std::shared_ptr<ASTNode>& Optimizer::optimizeChildren(std::shared_ptr<ASTNode>& node) const {
    const auto children = node->getChildren();
    const size_t childrenNumber = node->getChildrenNumber();
    for (size_t i = 0; i < childrenNumber; ++i) {
        children[i] = optimize(children[i]);
    }
    return node;
}

std::shared_ptr<ASTNode>& UnaryAdditionOptimizer::optimizeCurrent(std::shared_ptr<ASTNode>& node) const {
    bool hasChanges = false;
    do {
        hasChanges = false;
        auto token = node->getToken().get();
        if (token->getType() == TokenType::OPERATOR) {
            auto operatorToken = dynamic_cast<OperatorToken*>(token);
            if (operatorToken->getOperatorType() == OperatorType::UNARY_ADDITION) {
                assert(node->getChildrenNumber() == 1);
                node = node->getChildren()[0];
                hasChanges = true;
            }
        }
    } while (hasChanges);
    return node;
}

std::shared_ptr<ASTNode>& ArithmeticNegationOptimizer::optimizeCurrent(std::shared_ptr<ASTNode>& node) const {
    bool hasChanges = false;
    do {
        hasChanges = false;
        auto token = node->getToken().get();
        if (token->getType() == TokenType::OPERATOR) {
            auto operatorToken = dynamic_cast<OperatorToken*>(token);
            if (operatorToken->getOperatorType() == OperatorType::ARITHMETIC_NEGATION) {
                assert(node->getChildrenNumber() == 1);

                auto child = node->getChildren()[0];
                auto childToken = child->getToken().get();
                if (childToken->getType() == TokenType::OPERATOR) {
                    auto childOperatorToken = dynamic_cast<OperatorToken*>(childToken);
                    if (childOperatorToken->getOperatorType() == OperatorType::ARITHMETIC_NEGATION) {
                        assert(child->getChildrenNumber() == 1);
                        node = child->getChildren()[0];
                        hasChanges = true;
                    }
                }

            }
        }
    } while (hasChanges);
    return node;
}

static inline bool isZeroConstant(const std::shared_ptr<Token>& token) {
    return (token->getType() == TokenType::CONSTANT_VALUE) && (fabs(dynamic_cast<ConstantValueToken*>(token.get())->getValue()) < COMPARE_EPS);
}

static inline bool isOneConstant(const std::shared_ptr<Token>& token) {
    return (token->getType() == TokenType::CONSTANT_VALUE) && (fabs(dynamic_cast<ConstantValueToken*>(token.get())->getValue() - 1) < COMPARE_EPS);
}

std::shared_ptr<ASTNode>& TrivialAdditionOptimizer::optimizeCurrent(std::shared_ptr<ASTNode>& node) const {
    bool hasChanges = false;
    do {
        hasChanges = false;
        const auto token = node->getToken().get();
        if (token->getType() == TokenType::OPERATOR) {
            const auto operatorToken = dynamic_cast<OperatorToken*>(token);
            if (operatorToken->getOperatorType() == OperatorType::ADDITION) {
                assert(node->getChildrenNumber() == 2);

                const auto leftChild = node->getChildren()[0];
                const auto rightChild = node->getChildren()[1];
                const bool isLeftZero = isZeroConstant(leftChild->getToken());
                const bool isRightZero = isZeroConstant(rightChild->getToken());
                if (isLeftZero) {
                    node = rightChild;
                    hasChanges = true;
                } else if (isRightZero) {
                    node = leftChild;
                    hasChanges = true;
                }
            }
        }
    } while (hasChanges);
    return node;
}

std::shared_ptr<ASTNode>& TrivialMultiplicationOptimizer::optimizeCurrent(std::shared_ptr<ASTNode>& node) const {
    bool hasChanges = false;
    do {
        hasChanges = false;
        const auto token = node->getToken().get();
        if (token->getType() == TokenType::OPERATOR) {
            const auto operatorToken = dynamic_cast<OperatorToken*>(token);
            if (operatorToken->getOperatorType() == OperatorType::MULTIPLICATION) {
                assert(node->getChildrenNumber() == 2);

                const auto leftChild = node->getChildren()[0];
                const auto rightChild = node->getChildren()[1];
                const bool isLeftZero = isZeroConstant(leftChild->getToken());
                const bool isRightZero = isZeroConstant(rightChild->getToken());
                const bool isLeftOne = isOneConstant(leftChild->getToken());
                const bool isRightOne = isOneConstant(rightChild->getToken());
                if (isLeftZero || isRightOne) { // (0 * x) = 0, (x * 1) = x
                    node = leftChild;
                    hasChanges = true;
                } else if (isRightZero || isLeftOne) { // (x * 0) = 0, (1 * x) = x
                    node = rightChild;
                    hasChanges = true;
                }
            }
        }
    } while (hasChanges);
    return node;
}

std::shared_ptr<ASTNode>& ConstantCompressor::optimizeCurrent(std::shared_ptr<ASTNode>& node) const {
    if (node->getToken()->getType() == TokenType::STATEMENTS || node->getToken()->getType() == TokenType::BLOCK) {
        return node;
    }

    const auto children = node->getChildren();
    const size_t childrenNumber = node->getChildrenNumber();
    if (childrenNumber == 0) {
        return node;
    } else if (childrenNumber == 1) {
        const auto childToken = children[0]->getToken().get();
        if (childToken->getType() == TokenType::CONSTANT_VALUE) {
            const double result = node->getToken()->calculate(1, dynamic_cast<ConstantValueToken*>(childToken)->getValue());
            node = std::make_shared<ASTNode>(std::make_shared<ConstantValueToken>(result));
        }
        return node;
    } else if (childrenNumber == 2) {
        const auto leftChild = children[0]->getToken().get();
        const auto rightChild = children[1]->getToken().get();
        if ((leftChild->getType() == TokenType::CONSTANT_VALUE) && (rightChild->getType() == TokenType::CONSTANT_VALUE)) {
            double result = node->getToken()->calculate(2, dynamic_cast<ConstantValueToken*>(leftChild)->getValue(), dynamic_cast<ConstantValueToken*>(rightChild)->getValue());
            node = std::make_shared<ASTNode>(std::make_shared<ConstantValueToken>(result));
        }
        return node;
    } else {
        throw std::logic_error("Unsupported arity of operator. Only unary and binary are supported yet");
    }
}

std::shared_ptr<ASTNode>& TrivialOperationsOptimizer::optimize(std::shared_ptr<ASTNode>& node) const {
    const auto children = node->getChildren();
    const size_t childrenNumber = node->getChildrenNumber();
    for (size_t i = 0; i < childrenNumber; ++i) {
        children[i] = optimize(children[i]);
    }
    return CompositeOptimizer::optimizeCurrent(node);
}

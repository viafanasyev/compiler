/**
 * @file
 * @brief Implementation of AST optimizers
 */
#include <cmath>
#include <memory>
#include "../frontend/ast.h"
#include "ast-optimizers.h"

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
        if (node->getType() == NodeType::OPERATOR_NODE) {
            const auto operatorNode = dynamic_cast<OperatorNode*>(node.get());
            if (operatorNode->getToken()->getOperatorType() == OperatorType::UNARY_ADDITION) {
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
        if (node->getType() == NodeType::OPERATOR_NODE) {
            const auto operatorNode = dynamic_cast<OperatorNode*>(node.get());
            if (operatorNode->getToken()->getOperatorType() == OperatorType::ARITHMETIC_NEGATION) {
                assert(node->getChildrenNumber() == 1);

                auto child = node->getChildren()[0];
                if (child->getType() == NodeType::OPERATOR_NODE) {
                    const auto childOperatorNode = dynamic_cast<OperatorNode*>(child.get());
                    if (childOperatorNode->getToken()->getOperatorType() == OperatorType::ARITHMETIC_NEGATION) {
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

static inline bool isZeroConstant(const std::shared_ptr<ASTNode>& node) {
    return (node->getType() == NodeType::CONSTANT_VALUE_NODE) && (fabs(dynamic_cast<ConstantValueNode*>(node.get())->getValue()) < COMPARE_EPS);
}

static inline bool isOneConstant(const std::shared_ptr<ASTNode>& node) {
    return (node->getType() == NodeType::CONSTANT_VALUE_NODE) && (fabs(dynamic_cast<ConstantValueNode*>(node.get())->getValue() - 1) < COMPARE_EPS);
}

std::shared_ptr<ASTNode>& TrivialAdditionOptimizer::optimizeCurrent(std::shared_ptr<ASTNode>& node) const {
    bool hasChanges = false;
    do {
        hasChanges = false;
        if (node->getType() == NodeType::OPERATOR_NODE) {
            const auto operatorNode = dynamic_cast<OperatorNode*>(node.get());
            if (operatorNode->getToken()->getOperatorType() == OperatorType::ADDITION) {
                assert(node->getChildrenNumber() == 2);

                const auto leftChild = node->getChildren()[0];
                const auto rightChild = node->getChildren()[1];
                if (isZeroConstant(leftChild)) {
                    node = rightChild;
                    hasChanges = true;
                } else if (isZeroConstant(rightChild)) {
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
        if (node->getType() == NodeType::OPERATOR_NODE) {
            const auto operatorNode = dynamic_cast<OperatorNode*>(node.get());
            if (operatorNode->getToken()->getOperatorType() == OperatorType::MULTIPLICATION) {
                assert(node->getChildrenNumber() == 2);

                const auto leftChild = node->getChildren()[0];
                const auto rightChild = node->getChildren()[1];
                if (isZeroConstant(leftChild)|| isOneConstant(rightChild)) { // (0 * x) = 0, (x * 1) = x
                    node = leftChild;
                    hasChanges = true;
                } else if (isZeroConstant(rightChild) || isOneConstant(leftChild)) { // (x * 0) = 0, (1 * x) = x
                    node = rightChild;
                    hasChanges = true;
                }
            }
        }
    } while (hasChanges);
    return node;
}

std::shared_ptr<ASTNode>& ConstantCompressor::optimizeCurrent(std::shared_ptr<ASTNode>& node) const {
    if (node->getType() != NodeType::OPERATOR_NODE) {
        return node;
    }

    const auto operatorNode = dynamic_cast<OperatorNode*>(node.get());
    const auto children = node->getChildren();
    const size_t childrenNumber = node->getChildrenNumber();
    if (childrenNumber == 0) {
        return node;
    } else if (childrenNumber == 1) {
        const auto child = children[0];
        if (child->getType() == NodeType::CONSTANT_VALUE_NODE) {
            const double result = operatorNode->getToken()->calculate(1, dynamic_cast<ConstantValueNode*>(child.get())->getValue());
            node = std::make_shared<ConstantValueNode>(result);
        }
        return node;
    } else if (childrenNumber == 2) {
        const auto leftChild = children[0];
        const auto rightChild = children[1];
        if ((leftChild->getType() == NodeType::CONSTANT_VALUE_NODE) && (rightChild->getType() == NodeType::CONSTANT_VALUE_NODE)) {
            double result = operatorNode->getToken()->calculate(
                2,
                dynamic_cast<ConstantValueNode*>(leftChild.get())->getValue(),
                dynamic_cast<ConstantValueNode*>(rightChild.get())->getValue()
            );
            node = std::make_shared<ConstantValueNode>(result);
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

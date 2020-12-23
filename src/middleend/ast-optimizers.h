/**
 * @file
 * @brief Definition of AST optimizers
 */
#ifndef AST_BUILDER_AST_OPTIMIZERS_H
#define AST_BUILDER_AST_OPTIMIZERS_H

#include <memory>
#include <vector>
#include "../frontend/ast.h"

class Optimizer {

private:
    const bool optimizeChildrenFirst;
public:
    explicit Optimizer(bool optimizeChildrenFirst_) : optimizeChildrenFirst(optimizeChildrenFirst_) { }

    virtual std::shared_ptr<ASTNode>& optimize(std::shared_ptr<ASTNode>& node) const;
    virtual std::shared_ptr<ASTNode>& optimizeCurrent(std::shared_ptr<ASTNode>& node) const = 0;
    virtual std::shared_ptr<ASTNode>& optimizeChildren(std::shared_ptr<ASTNode>& node) const;
};

class CompositeOptimizer : public Optimizer {

protected:
    std::vector<std::shared_ptr<Optimizer> > optimizers;

public:
    explicit CompositeOptimizer() : Optimizer(false) { }

    void addOptimizer(const std::shared_ptr<Optimizer>& optimizer) {
        optimizers.push_back(optimizer);
    }

    std::shared_ptr<ASTNode>& optimize(std::shared_ptr<ASTNode>& node) const override {
        for (const auto& optimizer : optimizers) {
            node = optimizer->optimize(node);
        }
        return node;
    }

    std::shared_ptr<ASTNode>& optimizeChildren(std::shared_ptr<ASTNode>& node) const override {
        for (const auto& optimizer : optimizers) {
            node = optimizer->optimizeChildren(node);
        }
        return node;
    }

    std::shared_ptr<ASTNode>& optimizeCurrent(std::shared_ptr<ASTNode>& node) const override {
        for (const auto& optimizer : optimizers) {
            node = optimizer->optimizeCurrent(node);
        }
        return node;
    }
};

/**
 * Optimizer for unary addition. Removes nodes with unary addition because they are useless.
 */
class UnaryAdditionOptimizer : public Optimizer {

public:
    UnaryAdditionOptimizer() : Optimizer(false) { }
    std::shared_ptr<ASTNode>& optimizeCurrent(std::shared_ptr<ASTNode>& node) const override;
};

/**
 * Optimizer for double arithmetic negations. All double negations are removed.
 */
class ArithmeticNegationOptimizer : public Optimizer {

public:
    ArithmeticNegationOptimizer() : Optimizer(false) { }
    std::shared_ptr<ASTNode>& optimizeCurrent(std::shared_ptr<ASTNode>& node) const override;
};

/**
 * Optimizer for expressions like (0 + ...), (... + 0)
 */
class TrivialAdditionOptimizer : public Optimizer {

public:
    TrivialAdditionOptimizer() : Optimizer(true) { }
    std::shared_ptr<ASTNode>& optimizeCurrent(std::shared_ptr<ASTNode>& node) const override;
};

/**
 * Optimizer for expressions like (1 * ...), (... * 1), (0 * ...), (... * 0)
 */
class TrivialMultiplicationOptimizer : public Optimizer {

public:
    TrivialMultiplicationOptimizer() : Optimizer(true) { }
    std::shared_ptr<ASTNode>& optimizeCurrent(std::shared_ptr<ASTNode>& node) const override;
};

/**
 * Compresses all expressions where all operands are constants
 */
class ConstantCompressor : public Optimizer {

public:
    ConstantCompressor() : Optimizer(true) { }
    std::shared_ptr<ASTNode>& optimizeCurrent(std::shared_ptr<ASTNode>& node) const override;
};

// TODO: TrivialPowerOptimizer (x^0 = 1, x^1 = x, 1^x = 1, maybe x^-y = 1/x^y)

/**
 * Composite optimizer for trivial operations (see TrivialMultiplicationOptimizer, TrivialAdditionOptimizer, ConstantCompressor)
 */
class TrivialOperationsOptimizer : public CompositeOptimizer {

public:
    TrivialOperationsOptimizer() : CompositeOptimizer() {
        addOptimizer(std::make_shared<TrivialMultiplicationOptimizer>());
        addOptimizer(std::make_shared<TrivialAdditionOptimizer>());
        addOptimizer(std::make_shared<ConstantCompressor>());
    }

    std::shared_ptr<ASTNode>& optimize(std::shared_ptr<ASTNode>& node) const override;
};

// TODO: 0 - x -> -x
// TODO: Push negation operators down to constants and variables. (to eliminate x - -4*x)

// TODO: x + -y -> x - y ; x - -y -> x + y

// TODO: -1 * x -> -x ; x * -1 -> -x (NOTE: Use before ArithmeticNegationOptimizer: -(-1 * X) -> --X -> X; otherwise --X wil be produced) (But what if --1 * --X? It will be optimized to 1 * X, not X)
/*
 *      ...                ...
 *     /                  /
 *    *        --->      -
 *   / \                 |
 *  /   \                |
 * -    X                X
 * |
 * |
 * 1
 */
// TODO: -1 * -x -> x ; -x * -1 -> x (Solution of the problem above?)

#endif // AST_BUILDER_AST_OPTIMIZERS_H

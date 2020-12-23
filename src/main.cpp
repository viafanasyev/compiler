/**
 * @file
 */
#include <memory>
#include "frontend/ast.h"
#include "frontend/recursive_parser.h"
#include "frontend/SyntaxError.h"
#include "middleend/ast-optimizers.h"

void outputAST(const std::shared_ptr<ASTNode>& root, const char* fileName) {
    root->visualize(fileName);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Invalid arguments number (argc = %d). Expected expression", argc);
        return -1;
    }

    auto optimizer = std::make_shared<CompositeOptimizer>();
    optimizer->addOptimizer(std::make_shared<UnaryAdditionOptimizer>());
    optimizer->addOptimizer(std::make_shared<ArithmeticNegationOptimizer>());
    optimizer->addOptimizer(std::make_shared<TrivialOperationsOptimizer>());

    try {
        auto ASTRoot = buildASTRecursively(argv[1]);
        ASTRoot = optimizer->optimize(ASTRoot);
        outputAST(ASTRoot, "expression");
    } catch (const std::invalid_argument& ex) {
        fprintf(stderr, "Invalid expression: %s", ex.what());
        return -1;
    } catch (const std::logic_error& ex) {
        fprintf(stderr, "Invalid expression: %s", ex.what());
        return -1;
    } catch (const SyntaxError& ex) {
        fprintf(stderr, "Syntax error: %s", ex.what());
        return -1;
    }
    return 0;
}

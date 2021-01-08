/**
 * @file
 */
#include <memory>
#include "frontend/ast.h"
#include "frontend/recursive_parser.h"
#include "frontend/SyntaxError.h"
#include "MappedFile.h"
#include "middleend/ast-optimizers.h"

void outputAST(const std::shared_ptr<ASTNode>& root, const char* fileName) {
    root->visualize(fileName);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Invalid arguments number (argc = %d). Expected filename", argc);
        return -1;
    }
    MappedFile file(argv[1]);

    auto optimizer = std::make_shared<CompositeOptimizer>();
    optimizer->addOptimizer(std::make_shared<UnaryAdditionOptimizer>());
    optimizer->addOptimizer(std::make_shared<ArithmeticNegationOptimizer>());
    optimizer->addOptimizer(std::make_shared<TrivialOperationsOptimizer>());

    try {
        std::shared_ptr<ASTNode> ASTRoot = buildASTRecursively(file.getTextPtr());
        ASTRoot = optimizer->optimize(ASTRoot);
        outputAST(ASTRoot, argv[1]);
    } catch (const std::logic_error& ex) {
        fprintf(stderr, "Invalid expression: %s", ex.what());
        return -1;
    } catch (const SyntaxError& ex) {
        fprintf(stderr, "Syntax error: %s", ex.what());
        return -1;
    }
    return 0;
}

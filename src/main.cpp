/**
 * @file
 */
#include <memory>
#include "backend/codegen.h"
#include "frontend/ast.h"
#include "frontend/recursive_parser.h"
#include "frontend/SyntaxError.h"
#include "MappedFile.h"
#include "middleend/ast-optimizers.h"
#include "stack-machine/src/arg-parser.h"
#include "stack-machine/src/stack-machine.h"

const char* const irFileExtension = ".ir";

void outputAST(const std::shared_ptr<ASTNode>& root, const char* fileName) {
    root->visualize(fileName);
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Invalid arguments number (argc = %d). Expected filename or filename and mode", argc);
        return -1;
    }
    const char* codeFileName = argv[1];
    MappedFile file(codeFileName);
    bool runAfterCompilation = (argc >= 3) && (strcmp(argv[2], "run") == 0);

    auto optimizer = std::make_shared<CompositeOptimizer>();
    optimizer->addOptimizer(std::make_shared<UnaryAdditionOptimizer>());
    optimizer->addOptimizer(std::make_shared<ArithmeticNegationOptimizer>());
    optimizer->addOptimizer(std::make_shared<TrivialOperationsOptimizer>());

    int exitCode = 0;
    try {
        std::shared_ptr<ASTNode> ASTRoot = buildASTRecursively(file.getTextPtr());
        ASTRoot = optimizer->optimize(ASTRoot);
        outputAST(ASTRoot, codeFileName);

        char irFileName[maxFileNameLength];
        replaceExtension(irFileName, codeFileName, irFileExtension);
        codegen(ASTRoot, irFileName);

        if (runAfterCompilation) {
            char assemblyFileName[maxFileNameLength];
            replaceExtension(assemblyFileName, codeFileName, assemblyFileExtension);

            exitCode = assemble(irFileName, assemblyFileName);
            if (exitCode == 0) exitCode = run(assemblyFileName);

            if (exitCode != 0) printErrorMessageForExitCode(exitCode);
        }
    } catch (const std::logic_error& ex) {
        fprintf(stderr, "Invalid expression: %s", ex.what());
        exitCode = -1;
    } catch (const SyntaxError& ex) {
        fprintf(stderr, "Syntax error: %s", ex.what());
        exitCode = -1;
    }
    return exitCode;
}

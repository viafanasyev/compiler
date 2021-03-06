/**
 * @file
 */
#include <memory>
#include "backend/codegen.h"
#include "frontend/ast.h"
#include "frontend/recursive_parser.h"
#include "util/SyntaxError.h"
#include "util/RedefinitionError.h"
#include "util/CoercionError.h"
#include "util/ValueReassignmentError.h"
#include "MappedFile.h"
#include "middleend/ast-optimizers.h"
#include "stack-machine/src/arg-parser.h"
#include "stack-machine/src/stack-machine.h"

const char* const irFileExtension = ".ir";

enum CompilerRunningMode {
    PRINT_AST,
    COMPILE,
    COMPILE_AND_RUN,
};

CompilerRunningMode parseCompilerRunningMode(const char* mode) {
    if (strcmp(mode, "ast") == 0) {
        return PRINT_AST;
    } else if (strcmp(mode, "run") == 0) {
        return COMPILE_AND_RUN;
    } else {
        fprintf(stderr, "Unknown running mode. Just compiling\n");
        return COMPILE;
    }
}

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
    CompilerRunningMode mode = (argc >= 3) ? parseCompilerRunningMode(argv[2]) : COMPILE;

    auto optimizer = std::make_shared<CompositeOptimizer>();
    optimizer->addOptimizer(std::make_shared<UnaryAdditionOptimizer>());
    optimizer->addOptimizer(std::make_shared<ArithmeticNegationOptimizer>());
    optimizer->addOptimizer(std::make_shared<TrivialOperationsOptimizer>());

    int exitCode = 0;
    try {
        std::shared_ptr<ASTNode> ASTRoot = buildASTRecursively(file.getTextPtr());
        ASTRoot = optimizer->optimize(ASTRoot);

        if (mode == PRINT_AST) {
            outputAST(ASTRoot, codeFileName);
        } else if (mode == COMPILE || mode == COMPILE_AND_RUN) {
            char irFileName[maxFileNameLength];
            replaceExtension(irFileName, codeFileName, irFileExtension);
            codegen(ASTRoot, irFileName);

            char assemblyFileName[maxFileNameLength];
            replaceExtension(assemblyFileName, codeFileName, assemblyFileExtension);
            exitCode = assemble(irFileName, assemblyFileName);

            if (mode == COMPILE_AND_RUN && exitCode == 0) {
                exitCode = run(assemblyFileName);
            }
        } else {
            assert(!"Running mode not implemented");
        }

        if (exitCode != 0) printErrorMessageForExitCode(exitCode);
    } catch (const std::logic_error& ex) {
        fprintf(stderr, "Invalid expression: %s", ex.what());
        exitCode = -1;
    } catch (const SyntaxError& ex) {
        fprintf(stderr, "Syntax error: %s", ex.what());
        exitCode = -1;
    } catch (const RedefinitionError& ex) {
        fprintf(stderr, "Redefinition error: %s", ex.what());
        exitCode = -1;
    } catch (const CoercionError& ex) {
        fprintf(stderr, "Coercion error: %s", ex.what());
        exitCode = -1;
    } catch (const ValueReassignmentError& ex) {
        fprintf(stderr, "Value reassignment error: %s", ex.what());
        exitCode = -1;
    }
    return exitCode;
}

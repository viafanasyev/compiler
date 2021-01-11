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

static constexpr size_t maxFileNameLength = 256;
static const char* assemblyFileExtension = ".asm";

static void stripExtension(char* fileName) {
    assert(fileName != nullptr);

    char* end = fileName + strlen(fileName);

    while ((end > fileName) && (*end != '.') && (*end != '\\') && (*end != '/')) {
        --end;
    }

    if ((end > fileName) && (*end == '.') && (*(end - 1) != '\\') && (*(end - 1) != '/')) {
        *end = '\0';
    }
}

static void replaceExtension(char* destination, const char* originalFileName, const char* newExtension) {
    assert(originalFileName != nullptr);
    assert(newExtension != nullptr);

    char tmp[maxFileNameLength];
    strcpy(tmp, originalFileName);
    stripExtension(tmp);
    strcpy(destination, strcat(tmp, newExtension));
}

void outputAST(const std::shared_ptr<ASTNode>& root, const char* fileName) {
    root->visualize(fileName);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Invalid arguments number (argc = %d). Expected filename", argc);
        return -1;
    }
    const char* codeFileName = argv[1];
    MappedFile file(codeFileName);

    auto optimizer = std::make_shared<CompositeOptimizer>();
    optimizer->addOptimizer(std::make_shared<UnaryAdditionOptimizer>());
    optimizer->addOptimizer(std::make_shared<ArithmeticNegationOptimizer>());
    optimizer->addOptimizer(std::make_shared<TrivialOperationsOptimizer>());

    try {
        std::shared_ptr<ASTNode> ASTRoot = buildASTRecursively(file.getTextPtr());
        ASTRoot = optimizer->optimize(ASTRoot);
        outputAST(ASTRoot, codeFileName);

        char assemblyFileName[maxFileNameLength];
        replaceExtension(assemblyFileName, codeFileName, assemblyFileExtension);
        codegen(ASTRoot, assemblyFileName);
    } catch (const std::logic_error& ex) {
        fprintf(stderr, "Invalid expression: %s", ex.what());
        return -1;
    } catch (const SyntaxError& ex) {
        fprintf(stderr, "Syntax error: %s", ex.what());
        return -1;
    }
    return 0;
}

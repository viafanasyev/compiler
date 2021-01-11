/**
 * @file
 */
#ifndef COMPILER_CODEGEN_H
#define COMPILER_CODEGEN_H

#include "../frontend/ast.h"
#include "SymbolTable.h"

class Label {

private:
    static size_t nextId;

public:
    const size_t id;
    Label() : id(nextId++) { }
};

class CodegenVisitor {

private:
    FILE* assemblyFile = nullptr;
    SymbolTable variables;

public:
    explicit CodegenVisitor(FILE* assemblyFile_) : assemblyFile(assemblyFile_) {
        assert(assemblyFile_ != nullptr);
    }

    void visitConstantValueNode(const ConstantValueNode* node);
    void visitVariableNode(const VariableNode* node);
    void visitOperatorNode(const OperatorNode* node);
    void visitComparisonOperatorNode(const ComparisonOperatorNode* node);
    void visitFunctionCallNode(const FunctionCallNode* node);
    void visitStatementsNode(const StatementsNode* node);
    void visitBlockNode(const BlockNode* node);
    void visitIfNode(const IfNode* node);
    void visitIfElseNode(const IfElseNode* node);
    void visitWhileNode(const WhileNode* node);
    void visitLabel(const Label* label);

    void push(double value);
    void pushRam(size_t address);
    void pop();
    void popRam(size_t address);
    void condJump(ComparisonOperatorType compOp, const Label* label, bool isNegated);
    void uncondJump(const Label* label);
    void arithmeticOperation(OperatorType op);
    void halt();

private:
    static ComparisonOperatorType negateCompOp(ComparisonOperatorType compOp);
};

void codegen(const std::shared_ptr<ASTNode>& root, const char* assemblyFileName);

#endif // COMPILER_CODEGEN_H

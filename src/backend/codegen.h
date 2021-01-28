/**
 * @file
 * @brief Definition of IR code generation functions
 */
#ifndef COMPILER_CODEGEN_H
#define COMPILER_CODEGEN_H

#include "../frontend/ast.h"
#include "Label.h"
#include "SymbolTable.h"

/**
 * Notes about function calling/arguments passing/etc in IR code:
 *   - Each program should contain no-arg 'main' function. Program starts from it.
 *   - Arguments for functions are passed through the stack in the reverse order.
 *   - Each function preserves it's stack frame using 'AX', 'BX' and 'CX' register:
 *        -# Register 'AX' points to the address of the next empty byte in RAM.
 *        -# If there is some local variable at the LOCAL address 'X', then it's RAM address is equal to ('AX'  - (nextLocalVarAddress - varLocalAddress)). So to get or set a value of a variable, this value should be calculated. ('BX' register is used as helper in this calculations).
 *        -# On function enter, current 'AX' is pushed onto the stack (if there is some parameters, then 'CX' is used as a helper, to temporarily save 'AX' value while parameters popped from the stack).
 *        -# When function is left, old 'AX' is popped from the stack and the current 'AX' is assigned to that value.
 */
class CodegenVisitor {

private:
    FILE* assemblyFile = nullptr;
    SymbolTable symbolTable;

public:
    explicit CodegenVisitor(FILE* assemblyFile_) : assemblyFile(assemblyFile_) {
        assert(assemblyFile_ != nullptr);
    }

    void codegen(const std::shared_ptr<ASTNode>& root);

    void visitConstantValueNode(const ConstantValueNode* node);
    void visitVariableNode(const VariableNode* node);
    void visitOperatorNode(const OperatorNode* node);
    void visitAssignmentOperatorNode(const AssignmentOperatorNode* node);
    void visitComparisonOperatorNode(const ComparisonOperatorNode* node);
    void visitStatementsNode(const StatementsNode* node);
    void visitBlockNode(const BlockNode* node);
    void visitIfNode(const IfNode* node);
    void visitIfElseNode(const IfElseNode* node);
    void visitWhileNode(const WhileNode* node);
    void visitParametersListNode(const ParametersListNode* node);
    void visitArgumentsListNode(const ArgumentsListNode* node);
    void visitFunctionDefinitionNode(const FunctionDefinitionNode* node);
    void visitFunctionCallNode(const FunctionCallNode* node);
    void visitVariableDeclarationNode(const VariableDeclarationNode* node);
    void visitReturnStatementNode(const ReturnStatementNode* node);
    void visitLabel(const Label* label);

    void functionProlog();
    void functionEpilog();

    void push(double value);
    void pushRam(size_t address);
    void pushRamByReg(const char* regName);
    void pushReg(const char* regName);
    void pop();
    void popRam(size_t address);
    void popRamByReg(const char* regName);
    void popReg(const char* regName);
    void condJump(ComparisonOperatorType compOp, const Label* label, bool isNegated);
    void uncondJump(const Label* label);
    void arithmeticOperation(OperatorType op);
    void ret();
    void call(const std::shared_ptr<FunctionSymbol>& functionSymbol);
    void halt();

    void getVarByAddress(unsigned int address);
    void setVarByAddress(unsigned int address);

private:
    static ComparisonOperatorType negateCompOp(ComparisonOperatorType compOp);

    std::shared_ptr<VariableSymbol> addVariable(char* name, const TokenOrigin& originPos);

    void pushDefaultValueForType(Type type);

    void coerceTo(std::shared_ptr<ASTNode>& node, Type to);
};

void codegen(const std::shared_ptr<ASTNode>& root, const char* assemblyFileName);

#endif // COMPILER_CODEGEN_H

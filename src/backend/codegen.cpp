/**
 * @file
 * @brief Implementation of IR code generation functions
 */
#include <cstdio>
#include "codegen.h"
#include "Label.h"
#include "SymbolTable.h"
#include "../util/constants.h"
#include "../util/SyntaxError.h"

static inline bool returnsNonVoid(const std::shared_ptr<ASTNode>& node, const SymbolTable& symbolTable) {
    const NodeType nodeType = node->getType();
    return (
        nodeType == NodeType::CONSTANT_VALUE_NODE
    ) || (
        nodeType == NodeType::VARIABLE_NODE
    ) || (
        nodeType == NodeType::OPERATOR_NODE &&
        dynamic_cast<OperatorNode*>(node.get())->getToken()->getOperatorType() != OperatorType::ASSIGNMENT
    ) || (
        nodeType == NodeType::FUNCTION_CALL_NODE &&
        !symbolTable.getFunctionByName(dynamic_cast<FunctionCallNode*>(node.get())->getFunctionName()->getName())->isVoid()
    );
}

void CodegenVisitor::codegen(const std::shared_ptr<ASTNode>& root) {
    const TokenOrigin fakeOrigin = { INT64_MAX, INT64_MAX };
    auto mainFunction = std::make_shared<FunctionSymbol>("main", Type::VOID, 0, fakeOrigin);
    push(0);
    popReg("AX");
    call(mainFunction);
    halt();

    root->accept(this);

    if (!symbolTable.hasFunction(mainFunction->getName()) ||
        symbolTable.getFunctionByName(mainFunction->getName())->argumentsNumber != 0
    ) {
        throw SyntaxError("Expected no-arg 'main' function declaration");
    }
}

void CodegenVisitor::visitConstantValueNode(const ConstantValueNode* node) {
    push(node->getValue());
}

void CodegenVisitor::visitVariableNode(const VariableNode* node) {
    char* variableName = node->getName();
    if (!symbolTable.hasVariable(variableName)) throw SyntaxError(node->getOriginPos(), "Undeclared variable");

    getVarByAddress(symbolTable.getVariableByName(variableName)->address);
}

void CodegenVisitor::visitOperatorNode(const OperatorNode* node) {
    size_t arity = node->getChildrenNumber();
    if (arity == 1) {
        node->getChildren()[0]->accept(this);
    } else if (arity == 2) {
        auto children = node->getChildren();
        if (node->getToken()->getOperatorType() == OperatorType::ASSIGNMENT) {
            auto variableNode = dynamic_cast<VariableNode*>(children[0].get());
            char* variableName = variableNode->getName();
            if (!symbolTable.hasVariable(variableName)) {
                addVariable(variableName, variableNode->getOriginPos());
            }

            children[1]->accept(this);

            setVarByAddress(symbolTable.getVariableByName(variableName)->address);
        } else {
            children[0]->accept(this);
            children[1]->accept(this);
            arithmeticOperation(node->getToken()->getOperatorType());
        }
    } else {
        throw std::logic_error("Unsupported arity of operator. Only unary and binary are supported yet");
    }
}

void CodegenVisitor::visitComparisonOperatorNode(const ComparisonOperatorNode* node) {
    size_t arity = node->getChildrenNumber();
    if (arity == 1) {
        node->getChildren()[0]->accept(this);
    } else if (arity == 2) {
        auto children = node->getChildren();
        children[0]->accept(this);
        children[1]->accept(this);
    } else {
        throw std::logic_error("Unsupported arity of operator. Only unary and binary are supported yet");
    }
}

void CodegenVisitor::visitStatementsNode(const StatementsNode* node) {
    size_t childrenNumber = node->getChildrenNumber();
    auto children = node->getChildren();
    for (size_t i = 0; i < childrenNumber; ++i) {
        children[i]->accept(this);
        if (returnsNonVoid(children[i], symbolTable)) {
            pop(); // If variable is left on stack, it should be removed
        }
    }
}

void CodegenVisitor::visitBlockNode(const BlockNode* node) {
    symbolTable.enterBlock();

    assert(node->getChildrenNumber() == 1);
    node->getChildren()[0]->accept(this);

    symbolTable.leaveBlock();
}

void CodegenVisitor::visitIfNode(const IfNode* node) {
    auto children = node->getChildren();
    assert(node->getChildrenNumber() == 2);
    auto condition = dynamic_cast<ComparisonOperatorNode*>(children[0].get());
    auto body = children[1];

    Label elseLabel;
    condition->accept(this);
    condJump(condition->getToken()->getOperatorType(), &elseLabel, true);

    body->accept(this);
    visitLabel(&elseLabel);
}

void CodegenVisitor::visitIfElseNode(const IfElseNode* node) {
    auto children = node->getChildren();
    assert(node->getChildrenNumber() == 3);
    auto condition = dynamic_cast<ComparisonOperatorNode*>(children[0].get());
    auto ifBody = children[1];
    auto elseBody = children[2];

    Label elseLabel;
    condition->accept(this);
    condJump(condition->getToken()->getOperatorType(), &elseLabel, true);

    Label endLabel;
    ifBody->accept(this);
    uncondJump(&endLabel);
    visitLabel(&elseLabel);

    elseBody->accept(this);
    visitLabel(&endLabel);
}

void CodegenVisitor::visitWhileNode(const WhileNode* node) {
    auto children = node->getChildren();
    assert(node->getChildrenNumber() == 2);
    auto condition = dynamic_cast<ComparisonOperatorNode*>(children[0].get());
    auto body = children[1];

    Label loopStartLabel;
    Label loopEndLabel;
    visitLabel(&loopStartLabel);
    condition->accept(this);
    condJump(condition->getToken()->getOperatorType(), &loopEndLabel, true);

    body->accept(this);
    uncondJump(&loopStartLabel);

    visitLabel(&loopEndLabel);
}

void CodegenVisitor::visitParametersListNode(const ParametersListNode* node) {
    size_t childrenNumber = node->getChildrenNumber();
    if (childrenNumber == 0) return;

    popReg("CX"); // Temporarily save AX to CX

    auto children = node->getChildren();
    for (size_t i = 0; i < childrenNumber; ++i) {
        assert(children[i]->getType() == NodeType::VARIABLE_NODE);

        // Parameter value put in RAM before adding new variable to SymbolTable, because it's more optimal
        setVarByAddress(symbolTable.getNextLocalVariableAddress());

        auto variableNode = dynamic_cast<VariableNode*>(children[i].get());
        addVariable(variableNode->getName(), variableNode->getOriginPos());
    }

    pushReg("CX"); // Put saved AX value on stack
}

void CodegenVisitor::visitArgumentsListNode(const ArgumentsListNode* node) {
    auto children = node->getChildren();
    for (size_t i = node->getChildrenNumber(); i --> 0 ;) { // from (childrenNumber - 1) to 0
        children[i]->accept(this);
    }
}

void CodegenVisitor::visitFunctionDefinitionNode(const FunctionDefinitionNode* node) {
    assert(node->getChildrenNumber() == 2);
    auto children = node->getChildren();
    auto parameters = children[0];
    auto body       = children[1];
    auto functionName = node->getFunctionName();

    auto functionSymbol = symbolTable.addFunction(functionName->getName(), Type::DOUBLE, parameters->getChildrenNumber(), functionName->getOriginPos());
    visitLabel(functionSymbol->label.get());

    functionProlog();

    symbolTable.enterFunction();

    parameters->accept(this);

    // Block node is visited manually, because only one wrapping block should be created for parameters and body blocks
    assert(body->getType() == NodeType::BLOCK_NODE);
    assert(body->getChildrenNumber() == 1);
    body->getChildren()[0]->accept(this);

    symbolTable.leaveFunction();

    functionEpilog();

    if (!functionSymbol->isVoid()) {
        // Implicit 'return 0' to be sure function is terminated in each case
        push(0);
        ret();
    }
}

void CodegenVisitor::visitFunctionCallNode(const FunctionCallNode* node) {
    assert(node->getChildrenNumber() == 1);
    auto children = node->getChildren();
    auto arguments = children[0];
    auto functionName = node->getFunctionName();

    if (!symbolTable.hasFunction(functionName->getName())) throw SyntaxError(functionName->getOriginPos(), "Undeclared function");

    auto symbol = symbolTable.getFunctionByName(functionName->getName());

    if (arguments->getChildrenNumber() != symbol->argumentsNumber) throw SyntaxError(functionName->getOriginPos(), "Invalid arguments number");

    arguments->accept(this);

    call(symbol);
}

void CodegenVisitor::visitReturnStatementNode(const ReturnStatementNode* node) {
    assert(node->getChildrenNumber() == 1);

    bool nonVoidReturn = returnsNonVoid(node->getChildren()[0], symbolTable);
    node->getChildren()[0]->accept(this);
    if (nonVoidReturn) popReg("BX"); // Save returned value temporarily to BX
    functionEpilog();
    if (nonVoidReturn) pushReg("BX");
    ret();
}

void CodegenVisitor::visitLabel(const Label* label) {
    fprintf(assemblyFile, "%s:\n", label->getName());
}

void CodegenVisitor::functionProlog() {
    pushReg("AX");
}

void CodegenVisitor::functionEpilog() {
    popReg("AX");
}

void CodegenVisitor::push(double value) {
    fprintf(assemblyFile, "PUSH %lg\n", value);
}

void CodegenVisitor::pushRam(size_t address) {
    fprintf(assemblyFile, "PUSH [%zu]\n", address);
}

void CodegenVisitor::pushRamByReg(const char* regName) {
    assert(strcmp(regName, "AX") == 0 || strcmp(regName, "BX") == 0 || strcmp(regName, "CX") == 0 || strcmp(regName, "DX") == 0);

    fprintf(assemblyFile, "PUSH [%s]\n", regName);
}

void CodegenVisitor::pushReg(const char* regName) {
    assert(strcmp(regName, "AX") == 0 || strcmp(regName, "BX") == 0 || strcmp(regName, "CX") == 0 || strcmp(regName, "DX") == 0);

    fprintf(assemblyFile, "PUSH %s\n", regName);
}

void CodegenVisitor::pop() {
    fprintf(assemblyFile, "POP\n");
}

void CodegenVisitor::popRam(size_t address) {
    fprintf(assemblyFile, "POP [%zu]\n", address);
}

void CodegenVisitor::popRamByReg(const char* regName) {
    assert(strcmp(regName, "AX") == 0 || strcmp(regName, "BX") == 0 || strcmp(regName, "CX") == 0 || strcmp(regName, "DX") == 0);

    fprintf(assemblyFile, "POP [%s]\n", regName);
}

void CodegenVisitor::popReg(const char* regName) {
    assert(strcmp(regName, "AX") == 0 || strcmp(regName, "BX") == 0 || strcmp(regName, "CX") == 0 || strcmp(regName, "DX") == 0);

    fprintf(assemblyFile, "POP %s\n", regName);
}

void CodegenVisitor::condJump(ComparisonOperatorType compOp, const Label* label, bool isNegated) {
    if (isNegated) compOp = negateCompOp(compOp);

    switch (compOp) {
        case LESS:             fprintf(assemblyFile, "JMPL %s\n",  label->getName()); return;
        case LESS_OR_EQUAL:    fprintf(assemblyFile, "JMPLE %s\n", label->getName()); return;
        case GREATER:          fprintf(assemblyFile, "JMPG %s\n",  label->getName()); return;
        case GREATER_OR_EQUAL: fprintf(assemblyFile, "JMPGE %s\n", label->getName()); return;
        case EQUAL:            fprintf(assemblyFile, "JMPE %s\n",  label->getName()); return;
        case NOT_EQUAL:        fprintf(assemblyFile, "JMPNE %s\n", label->getName()); return;
        default:               throw std::logic_error("Unsupported comparison operator type");
    }
}

void CodegenVisitor::uncondJump(const Label* label) {
    fprintf(assemblyFile, "JMP %s\n", label->getName());
}

void CodegenVisitor::arithmeticOperation(OperatorType op) {
    switch (op) {
        case ADDITION:            fprintf(assemblyFile, "ADD\n"); return;
        case SUBTRACTION:         fprintf(assemblyFile, "SUB\n"); return;
        case ARITHMETIC_NEGATION: push(-1); /* Fall through */
        case MULTIPLICATION:      fprintf(assemblyFile, "MUL\n"); return;
        case DIVISION:            fprintf(assemblyFile, "DIV\n"); return;
        case UNARY_ADDITION:      /* Do nothing */ return;
        case POWER:               fprintf(assemblyFile, "POW\n"); return;
        case ASSIGNMENT:          throw std::logic_error("Assignment is not an assignment operation");
        default:                  throw std::logic_error("Unsupported arithmetic operation");
    }
}

void CodegenVisitor::ret() {
    fprintf(assemblyFile, "RET\n");
}

void CodegenVisitor::call(const std::shared_ptr<FunctionSymbol>& functionSymbol) {
    if (functionSymbol->isInternal()) {
        fprintf(assemblyFile, "%s\n", functionSymbol->internalName);
    } else {
        fprintf(assemblyFile, "CALL %s\n", functionSymbol->label->getName());
    }
}

void CodegenVisitor::halt() {
    fprintf(assemblyFile, "HLT\n");
}

ComparisonOperatorType CodegenVisitor::negateCompOp(ComparisonOperatorType compOp) {
    switch (compOp) {
        case LESS:             return GREATER_OR_EQUAL;
        case LESS_OR_EQUAL:    return GREATER;
        case GREATER:          return LESS_OR_EQUAL;
        case GREATER_OR_EQUAL: return LESS;
        case EQUAL:            return NOT_EQUAL;
        case NOT_EQUAL:        return EQUAL;
        default:               throw std::logic_error("Unsupported comparison operator type");
    }
}

void CodegenVisitor::getVarByAddress(size_t address) {
    address = symbolTable.getNextLocalVariableAddress() - address;

    // varRamAddress = AX - (nextVarLocalAddress - varLocalAddress)
    if (address == 0) {
        pushRamByReg("AX");
    } else {
        pushReg("AX");
        push(address);
        arithmeticOperation(SUBTRACTION);
        popReg("BX"); // Now BX = varRamAddress

        pushRamByReg("BX"); // To put variable value on stack
    }
}

void CodegenVisitor::setVarByAddress(size_t address) {
    address = symbolTable.getNextLocalVariableAddress() - address;

    // varRamAddress = AX - (nextVarLocalAddress - varLocalAddress)
    if (address == 0) {
        popRamByReg("AX");
    } else {
        pushReg("AX");
        push(address);
        arithmeticOperation(SUBTRACTION);
        popReg("BX"); // Now BX = varRamAddress

        popRamByReg("BX"); // To put variable value to RAM
    }
}

std::shared_ptr<VariableSymbol> CodegenVisitor::addVariable(char* name, const TokenOrigin& originPos) {
    // Increase AX by variable size
    pushReg("AX");
    push(VARIABLE_SIZE_IN_BYTES);
    fprintf(assemblyFile, "ADD\n");
    popReg("AX");

    return symbolTable.addVariable(name, originPos);
}

void codegen(const std::shared_ptr<ASTNode>& root, const char* assemblyFileName) {
    assert(assemblyFileName != nullptr);
    FILE* assemblyFile = fopen(assemblyFileName, "wb");
    if (assemblyFile == nullptr) return;

    CodegenVisitor visitor(assemblyFile);
    visitor.codegen(root);

    fclose(assemblyFile);
}

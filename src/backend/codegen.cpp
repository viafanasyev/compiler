/**
 * @file
 */
#include <cstdio>
#include "codegen.h"
#include "SymbolTable.h"
#include "../frontend/SyntaxError.h"

size_t Label::nextId = 0;

void CodegenVisitor::visitConstantValueNode(const ConstantValueNode* node) {
    push(node->getValue());
}

void CodegenVisitor::visitVariableNode(const VariableNode* node) {
    char* variableName = node->getName();
    if (!variables.hasVariable(variableName)) throw SyntaxError(node->getOriginPos(), "Undeclared variable");
    pushRam(variables.getVariableByName(variableName).address);
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
            if (!variables.hasVariable(variableName)) {
                variables.addVariable(variableName);
            }

            children[1]->accept(this);
            popRam(variables.getVariableByName(variableName).address);
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

void CodegenVisitor::visitFunctionCallNode(const FunctionCallNode* node __attribute__((unused))) {
    // TODO
    throw std::logic_error("Functions are not supported yet");
}

void CodegenVisitor::visitStatementsNode(const StatementsNode* node) {
    size_t childrenNumber = node->getChildrenNumber();
    auto children = node->getChildren();
    for (size_t i = 0; i < childrenNumber; ++i) {
        children[i]->accept(this);
    }
}

void CodegenVisitor::visitBlockNode(const BlockNode* node) {
    variables.enterBlock();

    assert(node->getChildrenNumber() == 1);
    node->getChildren()[0]->accept(this);

    variables.leaveBlock();
}

void CodegenVisitor::visitIfNode(const IfNode* node) {
    auto children = node->getChildren();
    assert(node->getChildrenNumber() == 2);
    auto condition = dynamic_cast<ComparisonOperatorNode*>(children[0].get());
    auto body = children[1];

    auto elseLabel = new Label();
    condition->accept(this);
    condJump(condition->getToken()->getOperatorType(), elseLabel, true);

    if (body->getType() != NodeType::BLOCK_NODE) variables.enterBlock();
    body->accept(this);
    if (body->getType() != NodeType::BLOCK_NODE) variables.leaveBlock();
    visitLabel(elseLabel);

    delete elseLabel;
}

void CodegenVisitor::visitIfElseNode(const IfElseNode* node) {
    auto children = node->getChildren();
    assert(node->getChildrenNumber() == 3);
    auto condition = dynamic_cast<ComparisonOperatorNode*>(children[0].get());
    auto ifBody = children[1];
    auto elseBody = children[2];

    auto elseLabel = new Label();
    condition->accept(this);
    condJump(condition->getToken()->getOperatorType(), elseLabel, true);

    auto endLabel = new Label();
    if (ifBody->getType() != NodeType::BLOCK_NODE) variables.enterBlock();
    ifBody->accept(this);
    if (ifBody->getType() != NodeType::BLOCK_NODE) variables.leaveBlock();
    uncondJump(endLabel);
    visitLabel(elseLabel);

    if (elseBody->getType() != NodeType::BLOCK_NODE) variables.enterBlock();
    elseBody->accept(this);
    if (elseBody->getType() != NodeType::BLOCK_NODE) variables.leaveBlock();
    visitLabel(endLabel);

    delete elseLabel;
    delete endLabel;
}

void CodegenVisitor::visitWhileNode(const WhileNode* node) {
    auto children = node->getChildren();
    assert(node->getChildrenNumber() == 2);
    auto condition = dynamic_cast<ComparisonOperatorNode*>(children[0].get());
    auto body = children[1];

    auto loopStartLabel = new Label();
    auto loopEndLabel = new Label();
    visitLabel(loopStartLabel);
    condition->accept(this);
    condJump(condition->getToken()->getOperatorType(), loopEndLabel, true);

    if (body->getType() != NodeType::BLOCK_NODE) variables.enterBlock();
    body->accept(this);
    if (body->getType() != NodeType::BLOCK_NODE) variables.leaveBlock();
    uncondJump(loopStartLabel);

    visitLabel(loopEndLabel);

    delete loopStartLabel;
    delete loopEndLabel;
}

void CodegenVisitor::visitLabel(const Label* label) {
    fprintf(assemblyFile, "LABEL%zu:\n", label->id); // TODO: Mark label.
}

void CodegenVisitor::push(double value) {
    fprintf(assemblyFile, "PUSH %lg\n", value); // TODO: Push value.
}

void CodegenVisitor::pushRam(size_t address) {
    fprintf(assemblyFile, "PUSH [%zu]\n", address); // TODO: Push value from RAM.
}

void CodegenVisitor::pop() {
    fprintf(assemblyFile, "POP\n"); // TODO: Pop.
}

void CodegenVisitor::popRam(size_t address) {
    fprintf(assemblyFile, "POP [%zu]\n", address); // TODO: Pop to RAM.
}

void CodegenVisitor::condJump(ComparisonOperatorType compOp, const Label* label, bool isNegated) {
    if (isNegated) compOp = negateCompOp(compOp);

    switch (compOp) {
        case LESS:             fprintf(assemblyFile, "JMPL LABEL%zu\n",  label->id); return; // TODO: Add cond.jumps to label.
        case LESS_OR_EQUAL:    fprintf(assemblyFile, "JMPLE LABEL%zu\n", label->id); return;
        case GREATER:          fprintf(assemblyFile, "JMPG LABEL%zu\n",  label->id); return;
        case GREATER_OR_EQUAL: fprintf(assemblyFile, "JMPGE LABEL%zu\n", label->id); return;
        case EQUAL:            fprintf(assemblyFile, "JMPE LABEL%zu\n",  label->id); return;
        case NOT_EQUAL:        fprintf(assemblyFile, "JMPNE LABEL%zu\n", label->id); return;
        default:               throw std::logic_error("Unsupported comparison operator type");
    }
}

void CodegenVisitor::uncondJump(const Label* label) {
    fprintf(assemblyFile, "JMP LABEL%zu\n", label->id); // TODO: Add uncond.jump.
}

void CodegenVisitor::arithmeticOperation(OperatorType op) {
    switch (op) {
        case ADDITION:            fprintf(assemblyFile, "ADD\n"); return; // TODO: Process arithm.op.
        case SUBTRACTION:         fprintf(assemblyFile, "SUB\n"); return;
        case MULTIPLICATION:      fprintf(assemblyFile, "MUL\n"); return;
        case DIVISION:            fprintf(assemblyFile, "DIV\n"); return;
        case ARITHMETIC_NEGATION: fprintf(assemblyFile, "PUSH -1\nMUL\n"); return;
        case UNARY_ADDITION:      /* Do nothing */ return;
        case POWER:               throw std::logic_error("Power operation is not supported yet"); // TODO: printf("POW\n");
        case ASSIGNMENT:          throw std::logic_error("Assignment is not an assignment operation");
        default:                  throw std::logic_error("Unsupported arithmetic operation");
    }
}

void CodegenVisitor::halt() {
    fprintf(assemblyFile, "HLT\n"); // TODO: Halt.
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

void codegen(const std::shared_ptr<ASTNode>& root, const char* assemblyFileName) {
    assert(assemblyFileName != nullptr);
    FILE* assemblyFile = fopen(assemblyFileName, "wb");
    if (assemblyFile == nullptr) return;

    CodegenVisitor visitor(assemblyFile);
    root->accept(&visitor);
    visitor.halt();

    fclose(assemblyFile);
}

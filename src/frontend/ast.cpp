/**
 * @file
 * @brief Implementation of AST building functions
 */
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include "ast.h"
#include "../backend/codegen.h"

size_t ASTNode::nextNodeId = 0;

void ASTNode::visualize(const char* fileName) const {
    assert(fileName != nullptr);

    char dotFileName[100];
    strcpy(dotFileName, fileName);
    strcat(dotFileName, ".dot");

    FILE* graphvizTextFile = fopen(dotFileName, "w");
    fprintf(graphvizTextFile, "digraph AST {\n");
    dotPrint(graphvizTextFile);
    fprintf(graphvizTextFile, "}\n");
    fclose(graphvizTextFile);

    char command[1000];
    sprintf(command, "dot -Tpng -o%s.png %s.dot && xdg-open %s.png", fileName, fileName, fileName);
    system(command);
}

void ASTNode::dotPrintChildren(const ASTNode* node, FILE* dotFile) {
    assert(node && dotFile);

    size_t arity = node->getChildrenNumber();
    auto children = node->getChildren();
    for (size_t i = 0; i < arity; ++i) {
        fprintf(dotFile, "%zu->%zu\n", node->nodeId, children[i]->nodeId);
        children[i]->dotPrint(dotFile);
    }
}

void ASTNode::dotPrintCurrent(const ASTNode* node, FILE* dotFile, const char* label, const char* fillColor) {
    assert(node && dotFile && label && fillColor);

    fprintf(dotFile, "%zu [label=\"%s\", shape=box, style=filled, color=\"grey\", fillcolor=\"%s\"];\n", node->nodeId, label, fillColor);
}

void ConstantValueNode::accept(CodegenVisitor* visitor) const {
    visitor->visitConstantValueNode(this);
}

void ConstantValueNode::dotPrint(FILE* dotFile) const {
    constexpr unsigned char maxLabelLen = 64; // (strlen("const\nvalue: ") = 13) + (50 symbols for double value) + ('\0')
    char label[maxLabelLen];
    snprintf(label, maxLabelLen, "const\nvalue: %lg", value);

    ASTNode::dotPrintCurrent(this, dotFile, label, "#FFFEC9");
    assert(getChildrenNumber() == 0);
}

void VariableNode::accept(CodegenVisitor* visitor) const {
    visitor->visitVariableNode(this);
}

void VariableNode::dotPrint(FILE* dotFile) const {
    constexpr size_t maxLabelLen = 11 + MAX_NAME_LENGTH; // (strlen("var\nname: ") = 10) + MAX_NAME_LENGTH + ('\0')
    char label[maxLabelLen];
    snprintf(label, maxLabelLen, "var\nname: %s", name);

    ASTNode::dotPrintCurrent(this, dotFile, label, "#99FF9D");
    assert(getChildrenNumber() == 0);
}

void OperatorNode::accept(CodegenVisitor* visitor) const {
    visitor->visitOperatorNode(this);
}

void OperatorNode::dotPrint(FILE* dotFile) const {
    constexpr unsigned char maxLabelLen = 17; // (strlen("binary op\nop: ") = 14) + (strlen(symbol) <= 2) + ('\0')
    char label[maxLabelLen];
    switch (getChildrenNumber()) {
        case 1:
            snprintf(label, maxLabelLen, "unary op\nop: %s", token->getSymbol());
            break;
        case 2:
            snprintf(label, maxLabelLen, "binary op\nop: %s", token->getSymbol());
            break;
        default:
            throw std::logic_error("Unsupported arity of operator. Only unary and binary are supported yet");
    }

    ASTNode::dotPrintCurrent(this, dotFile, label, "#C9E7FF");
    ASTNode::dotPrintChildren(this, dotFile);
}

void ComparisonOperatorNode::accept(CodegenVisitor* visitor) const {
    visitor->visitComparisonOperatorNode(this);
}

void ComparisonOperatorNode::dotPrint(FILE* dotFile) const {
    constexpr unsigned char maxLabelLen = 15; // (strlen("comp op\nop: ") = 12) + (strlen(symbol) <= 2) + ('\0')
    char label[maxLabelLen];
    snprintf(label, maxLabelLen, "comp op\nop: %s", token->getSymbol());

    ASTNode::dotPrintCurrent(this, dotFile, label, "#C9E7FF");
    assert(getChildrenNumber() == 2);
    ASTNode::dotPrintChildren(this, dotFile);
}

void StatementsNode::accept(CodegenVisitor* visitor) const {
    visitor->visitStatementsNode(this);
}

void StatementsNode::dotPrint(FILE* dotFile) const {
    ASTNode::dotPrintCurrent(this, dotFile, "statements", "grey");
    ASTNode::dotPrintChildren(this, dotFile);
}

void BlockNode::accept(CodegenVisitor* visitor) const {
    visitor->visitBlockNode(this);
}

void BlockNode::dotPrint(FILE* dotFile) const {
    ASTNode::dotPrintCurrent(this, dotFile, "block", "grey");
    ASTNode::dotPrintChildren(this, dotFile);
}

void IfNode::accept(CodegenVisitor* visitor) const {
    visitor->visitIfNode(this);
}

void IfNode::dotPrint(FILE* dotFile) const {
    ASTNode::dotPrintCurrent(this, dotFile, "if", "grey");
    assert(getChildrenNumber() == 2);
    ASTNode::dotPrintChildren(this, dotFile);
}

void IfElseNode::accept(CodegenVisitor* visitor) const {
    visitor->visitIfElseNode(this);
}

void IfElseNode::dotPrint(FILE* dotFile) const {
    ASTNode::dotPrintCurrent(this, dotFile, "if-else", "grey");
    assert(getChildrenNumber() == 3);
    ASTNode::dotPrintChildren(this, dotFile);
}

void WhileNode::accept(CodegenVisitor* visitor) const {
    visitor->visitWhileNode(this);
}

void WhileNode::dotPrint(FILE* dotFile) const {
    ASTNode::dotPrintCurrent(this, dotFile, "while", "grey");
    assert(getChildrenNumber() == 2);
    ASTNode::dotPrintChildren(this, dotFile);
}

void ParametersListNode::accept(CodegenVisitor* visitor) const {
    visitor->visitParametersListNode(this);
}

void ParametersListNode::dotPrint(FILE* dotFile) const {
    ASTNode::dotPrintCurrent(this, dotFile, (getChildrenNumber() == 0) ? "no params" : "params", "grey");
    ASTNode::dotPrintChildren(this, dotFile);
}

void ArgumentsListNode::accept(CodegenVisitor* visitor) const {
    visitor->visitArgumentsListNode(this);
}

void ArgumentsListNode::dotPrint(FILE* dotFile) const {
    ASTNode::dotPrintCurrent(this, dotFile, (getChildrenNumber() == 0) ? "no args" : "args", "grey");
    ASTNode::dotPrintChildren(this, dotFile);
}

void FunctionDefinitionNode::accept(CodegenVisitor* visitor) const {
    visitor->visitFunctionDefinitionNode(this);
}

void FunctionDefinitionNode::dotPrint(FILE* dotFile) const {
    constexpr size_t maxLabelLen = 16 + IdToken::MAX_NAME_LENGTH; // (strlen("func def\nname: ") = 15) + MAX_NAME_LENGTH + ('\0')
    char label[maxLabelLen];
    snprintf(label, maxLabelLen, "func def\nname: %s", functionName->getName());

    ASTNode::dotPrintCurrent(this, dotFile, label, "#F9C7FF");
    assert(getChildrenNumber() == 2);
    ASTNode::dotPrintChildren(this, dotFile);
}

void FunctionCallNode::accept(CodegenVisitor* visitor) const {
    visitor->visitFunctionCallNode(this);
}

void FunctionCallNode::dotPrint(FILE* dotFile) const {
    constexpr size_t maxLabelLen = 17 + IdToken::MAX_NAME_LENGTH; // (strlen("func call\nname: ") = 16) + MAX_NAME_LENGTH + ('\0')
    char label[maxLabelLen];
    snprintf(label, maxLabelLen, "func call\nname: %s", functionName->getName());

    ASTNode::dotPrintCurrent(this, dotFile, label, "#F9C7FF");
    assert(getChildrenNumber() == 1);
    ASTNode::dotPrintChildren(this, dotFile);
}

void ReturnStatementNode::accept(CodegenVisitor* visitor) const {
    visitor->visitReturnStatementNode(this);
}

void ReturnStatementNode::dotPrint(FILE* dotFile) const {
    ASTNode::dotPrintCurrent(this, dotFile, "return", "grey");
    assert(getChildrenNumber() == 1);
    ASTNode::dotPrintChildren(this, dotFile);
}

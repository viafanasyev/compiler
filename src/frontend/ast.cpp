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

void ConstantValueNode::accept(CodegenVisitor* visitor) const {
    visitor->visitConstantValueNode(this);
}

void ConstantValueNode::dotPrint(FILE* dotFile) const {
    fprintf(dotFile, "%zu [label=\"const\nvalue: %lg\", shape=box, style=filled, color=\"grey\", fillcolor=\"#FFFEC9\"];\n", nodeId, value);
}

void VariableNode::accept(CodegenVisitor* visitor) const {
    visitor->visitVariableNode(this);
}

void VariableNode::dotPrint(FILE* dotFile) const {
    fprintf(dotFile, "%zu [label=\"var\nname: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#99FF9D\"];\n", nodeId, name);
}

void OperatorNode::accept(CodegenVisitor* visitor) const {
    visitor->visitOperatorNode(this);
}

void OperatorNode::dotPrint(FILE* dotFile) const {
    size_t arity = getChildrenNumber();
    if (arity == 1) {
        fprintf(dotFile, "%zu [label=\"unary op\nop: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#C9E7FF\"];\n", nodeId, token->getSymbol());
    } else if (arity == 2) {
        fprintf(dotFile, "%zu [label=\"binary op\nop: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#C9E7FF\"];\n", nodeId, token->getSymbol());
    } else {
        throw std::logic_error("Unsupported arity of operator. Only unary and binary are supported yet");
    }
    auto children = getChildren();
    for (size_t i = 0; i < arity; ++i) {
        fprintf(dotFile, "%zu->%zu\n", nodeId, children[i]->nodeId);
        ASTNode::dotPrint(children[i], dotFile);
    }
}

void ComparisonOperatorNode::accept(CodegenVisitor* visitor) const {
    visitor->visitComparisonOperatorNode(this);
}

void ComparisonOperatorNode::dotPrint(FILE* dotFile) const {
    fprintf(dotFile, "%zu [label=\"comp op\nop: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#C9E7FF\"];\n", nodeId, token->getSymbol());
    auto children = getChildren();
    assert(getChildrenNumber() == 2);
    for (size_t i = 0; i < 2; ++i) {
        fprintf(dotFile, "%zu->%zu\n", nodeId, children[i]->nodeId);
        ASTNode::dotPrint(children[i], dotFile);
    }
}

void FunctionCallNode::accept(CodegenVisitor* visitor) const {
    visitor->visitFunctionCallNode(this);
}

void FunctionCallNode::dotPrint(FILE* dotFile) const {
    size_t arity = getChildrenNumber();
    if (arity == 1) {
        fprintf(dotFile, "%zu [label=\"unary func\nfunc: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#C9E7FF\"];\n", nodeId, name);
    } else {
        throw std::logic_error("Unsupported arity of function. Only unary are supported yet");
    }
    auto children = getChildren();
    for (size_t i = 0; i < arity; ++i) {
        fprintf(dotFile, "%zu->%zu\n", nodeId, children[i]->nodeId);
        ASTNode::dotPrint(children[i], dotFile);
    }
}

void StatementsNode::accept(CodegenVisitor* visitor) const {
    visitor->visitStatementsNode(this);
}

void StatementsNode::dotPrint(FILE* dotFile) const {
    fprintf(dotFile, "%zu [label=\"statements\", shape=box, style=filled, color=\"grey\", fillcolor=\"grey\"];\n", nodeId);
    auto children = getChildren();
    for (size_t i = 0; i < getChildrenNumber(); ++i) {
        fprintf(dotFile, "%zu->%zu\n", nodeId, children[i]->nodeId);
        ASTNode::dotPrint(children[i], dotFile);
    }
}

void BlockNode::accept(CodegenVisitor* visitor) const {
    visitor->visitBlockNode(this);
}

void BlockNode::dotPrint(FILE* dotFile) const {
    fprintf(dotFile, "%zu [label=\"block\", shape=box, style=filled, color=\"grey\", fillcolor=\"grey\"];\n", nodeId);
    auto children = getChildren();
    for (size_t i = 0; i < getChildrenNumber(); ++i) {
        fprintf(dotFile, "%zu->%zu\n", nodeId, children[i]->nodeId);
        ASTNode::dotPrint(children[i], dotFile);
    }
}

void IfNode::accept(CodegenVisitor* visitor) const {
    visitor->visitIfNode(this);
}

void IfNode::dotPrint(FILE* dotFile) const {
    fprintf(dotFile, "%zu [label=\"if\", shape=box, style=filled, color=\"grey\", fillcolor=\"grey\"];\n", nodeId);
    auto children = getChildren();
    assert(getChildrenNumber() == 2);
    for (size_t i = 0; i < 2; ++i) {
        fprintf(dotFile, "%zu->%zu\n", nodeId, children[i]->nodeId);
        ASTNode::dotPrint(children[i], dotFile);
    }
}

void IfElseNode::accept(CodegenVisitor* visitor) const {
    visitor->visitIfElseNode(this);
}

void IfElseNode::dotPrint(FILE* dotFile) const {
    fprintf(dotFile, "%zu [label=\"if-else\", shape=box, style=filled, color=\"grey\", fillcolor=\"grey\"];\n", nodeId);
    auto children = getChildren();
    assert(getChildrenNumber() == 3);
    for (size_t i = 0; i < 3; ++i) {
        fprintf(dotFile, "%zu->%zu\n", nodeId, children[i]->nodeId);
        ASTNode::dotPrint(children[i], dotFile);
    }
}

void WhileNode::accept(CodegenVisitor* visitor) const {
    visitor->visitWhileNode(this);
}

void WhileNode::dotPrint(FILE* dotFile) const {
    fprintf(dotFile, "%zu [label=\"while\", shape=box, style=filled, color=\"grey\", fillcolor=\"grey\"];\n", nodeId);
    auto children = getChildren();
    assert(getChildrenNumber() == 2);
    for (size_t i = 0; i < 2; ++i) {
        fprintf(dotFile, "%zu->%zu\n", nodeId, children[i]->nodeId);
        ASTNode::dotPrint(children[i], dotFile);
    }
}

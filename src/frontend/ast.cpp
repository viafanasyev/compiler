/**
 * @file
 * @brief Implementation of AST building functions
 */
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include "ast.h"

void ASTNode::visualize(const char* fileName) const {
    assert(fileName != nullptr);

    char dotFileName[100];
    strcpy(dotFileName, fileName);
    strcat(dotFileName, ".dot");

    FILE* graphvizTextFile = fopen(dotFileName, "w");
    fprintf(graphvizTextFile, "digraph AST {\n");
    int nodeId = 0;
    dotPrint(graphvizTextFile, nodeId);
    fprintf(graphvizTextFile, "}\n");
    fclose(graphvizTextFile);

    char command[1000];
    sprintf(command, "dot -Tpng -o%s.png %s.dot && xdg-open %s.png", fileName, fileName, fileName);
    system(command);
}

void ConstantValueNode::dotPrint(FILE* dotFile, int& nodeId) const {
    fprintf(dotFile, "%d [label=\"const\nvalue: %lg\", shape=box, style=filled, color=\"grey\", fillcolor=\"#FFFEC9\"];\n", nodeId, value);
    ++nodeId;
}

void VariableNode::dotPrint(FILE* dotFile, int& nodeId) const {
    fprintf(dotFile, "%d [label=\"var\nname: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#99FF9D\"];\n", nodeId, name);
    ++nodeId;
}

void OperatorNode::dotPrint(FILE* dotFile, int& nodeId) const {
    size_t arity = getChildrenNumber();
    if (arity == 1) {
        fprintf(dotFile, "%d [label=\"unary op\nop: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#C9E7FF\"];\n", nodeId, token->getSymbol());
    } else if (arity == 2) {
        fprintf(dotFile, "%d [label=\"binary op\nop: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#C9E7FF\"];\n", nodeId, token->getSymbol());
    } else {
        throw std::logic_error("Unsupported arity of operator. Only unary and binary are supported yet");
    }
    int childrenNodeId = nodeId + 1;
    auto children = getChildren();
    for (size_t i = 0; i < arity; ++i) {
        fprintf(dotFile, "%d->%d\n", nodeId, childrenNodeId);
        ASTNode::dotPrint(children[i], dotFile, childrenNodeId);
    }
    nodeId = childrenNodeId;
}

void ComparisonOperatorNode::dotPrint(FILE* dotFile, int& nodeId) const {
    fprintf(dotFile, "%d [label=\"comp op\nop: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#C9E7FF\"];\n", nodeId, token->getSymbol());
    int childrenNodeId = nodeId + 1;
    auto children = getChildren();
    assert(getChildrenNumber() == 2);
    for (size_t i = 0; i < 2; ++i) {
        fprintf(dotFile, "%d->%d\n", nodeId, childrenNodeId);
        ASTNode::dotPrint(children[i], dotFile, childrenNodeId);
    }
    nodeId = childrenNodeId;
}

void FunctionCallNode::dotPrint(FILE* dotFile, int& nodeId) const {
    size_t arity = getChildrenNumber();
    if (arity == 1) {
        fprintf(dotFile, "%d [label=\"unary func\nfunc: %s\", shape=box, style=filled, color=\"grey\", fillcolor=\"#C9E7FF\"];\n", nodeId, name);
    } else {
        throw std::logic_error("Unsupported arity of function. Only unary are supported yet");
    }
    int childrenNodeId = nodeId + 1;
    auto children = getChildren();
    for (size_t i = 0; i < arity; ++i) {
        fprintf(dotFile, "%d->%d\n", nodeId, childrenNodeId);
        ASTNode::dotPrint(children[i], dotFile, childrenNodeId);
    }
    nodeId = childrenNodeId;
}

void StatementsNode::dotPrint(FILE* dotFile, int& nodeId) const {
    fprintf(dotFile, "%d [label=\"statements\", shape=box, style=filled, color=\"grey\", fillcolor=\"grey\"];\n", nodeId);
    int childrenNodeId = nodeId + 1;
    auto children = getChildren();
    for (size_t i = 0; i < getChildrenNumber(); ++i) {
        fprintf(dotFile, "%d->%d\n", nodeId, childrenNodeId);
        ASTNode::dotPrint(children[i], dotFile, childrenNodeId);
    }
    nodeId = childrenNodeId;
}

void BlockNode::dotPrint(FILE* dotFile, int& nodeId) const {
    fprintf(dotFile, "%d [label=\"block\", shape=box, style=filled, color=\"grey\", fillcolor=\"grey\"];\n", nodeId);
    int childrenNodeId = nodeId + 1;
    auto children = getChildren();
    for (size_t i = 0; i < getChildrenNumber(); ++i) {
        fprintf(dotFile, "%d->%d\n", nodeId, childrenNodeId);
        ASTNode::dotPrint(children[i], dotFile, childrenNodeId);
    }
    nodeId = childrenNodeId;
}

void IfNode::dotPrint(FILE* dotFile, int& nodeId) const {
    fprintf(dotFile, "%d [label=\"if\", shape=box, style=filled, color=\"grey\", fillcolor=\"grey\"];\n", nodeId);
    int childrenNodeId = nodeId + 1;
    auto children = getChildren();
    assert(getChildrenNumber() == 2);
    for (size_t i = 0; i < 2; ++i) {
        fprintf(dotFile, "%d->%d\n", nodeId, childrenNodeId);
        ASTNode::dotPrint(children[i], dotFile, childrenNodeId);
    }
    nodeId = childrenNodeId;
}

void IfElseNode::dotPrint(FILE* dotFile, int& nodeId) const {
    fprintf(dotFile, "%d [label=\"if-else\", shape=box, style=filled, color=\"grey\", fillcolor=\"grey\"];\n", nodeId);
    int childrenNodeId = nodeId + 1;
    auto children = getChildren();
    assert(getChildrenNumber() == 3);
    for (size_t i = 0; i < 3; ++i) {
        fprintf(dotFile, "%d->%d\n", nodeId, childrenNodeId);
        ASTNode::dotPrint(children[i], dotFile, childrenNodeId);
    }
    nodeId = childrenNodeId;
}

void WhileNode::dotPrint(FILE* dotFile, int& nodeId) const {
    fprintf(dotFile, "%d [label=\"while\", shape=box, style=filled, color=\"grey\", fillcolor=\"grey\"];\n", nodeId);
    int childrenNodeId = nodeId + 1;
    auto children = getChildren();
    assert(getChildrenNumber() == 2);
    for (size_t i = 0; i < 2; ++i) {
        fprintf(dotFile, "%d->%d\n", nodeId, childrenNodeId);
        ASTNode::dotPrint(children[i], dotFile, childrenNodeId);
    }
    nodeId = childrenNodeId;
}

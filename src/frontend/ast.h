/**
 * @file
 * @brief Definition of AST node and AST building functions
 */
#ifndef AST_BUILDER_AST_H
#define AST_BUILDER_AST_H

#include <cassert>
#include <cstdarg>
#include <memory>
#include <vector>
#include "tokenizer.h"

class CodegenVisitor;

enum NodeType {
    CONSTANT_VALUE_NODE,
    VARIABLE_NODE,
    OPERATOR_NODE,
    COMPARISON_OPERATOR_NODE,
    FUNCTION_CALL_NODE,
    STATEMENTS_NODE,
    BLOCK_NODE,
    IF_NODE,
    IF_ELSE_NODE,
    WHILE_NODE,
};

class ASTNode {

private:
    std::shared_ptr<ASTNode>* children = nullptr;
    size_t childrenNumber = 0;
    NodeType type;

    static size_t nextNodeId;

public:
    const size_t nodeId;

    explicit ASTNode(NodeType type_) : type(type_), nodeId(nextNodeId++) {
        childrenNumber = 0;
        children = nullptr;
    }

    ASTNode(NodeType type_, const std::vector<std::shared_ptr<ASTNode>>& children_) : type(type_), nodeId(nextNodeId++) {
        childrenNumber = children_.size();
        children = new std::shared_ptr<ASTNode>[childrenNumber];
        for (size_t i = 0; i < childrenNumber; ++i) {
            children[i] = children_[i];
        }
    }

    ASTNode(NodeType type_, const std::shared_ptr<ASTNode>& child) : type(type_), nodeId(nextNodeId++) {
        childrenNumber = 1;
        children = new std::shared_ptr<ASTNode>[1];
        children[0] = child;
    }

    ASTNode(NodeType type_, const std::shared_ptr<ASTNode>& leftChild, const std::shared_ptr<ASTNode>& rightChild) : type(type_), nodeId(nextNodeId++) {
        childrenNumber = 2;
        children = new std::shared_ptr<ASTNode>[2];
        children[0] = leftChild;
        children[1] = rightChild;
    }

    ~ASTNode() {
        delete[] children;
    }

    std::shared_ptr<ASTNode>* getChildren() const {
        return children;
    }

    size_t getChildrenNumber() const {
        return childrenNumber;
    }

    NodeType getType() const {
        return type;
    }

    void visualize(const char* fileName) const;

    virtual void accept(CodegenVisitor* visitor) const = 0;

protected:
    virtual void dotPrint(FILE* dotFile) const = 0;

    /**
     * This method is created for code like below to be possible
     * (because C++ `protected` modifier doesn't allow this, unlike in other OOP languages)
     *
     *     class Parent {
     *     protected:
     *         virtual void foo() = 0;
     *     };
     *
     *     class Child {
     *     protected:
     *         void foo() override {
     *             Parent *p;
     *             ...
     *             p->foo();
     *         }
     *     };
     *
     *  Use `dotPrint(child, dotFile)` instead of `child->dotPrint(dotFile)`.
     */
    static void dotPrint(const std::shared_ptr<ASTNode>& node, FILE* dotFile) {
        node->dotPrint(dotFile);
    }
};

class ConstantValueNode : public ASTNode {

private:
    double value;

public:
    explicit ConstantValueNode(double value_) : ASTNode(CONSTANT_VALUE_NODE), value(value_) { }

    double getValue() const {
        return value;
    }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class VariableNode : public ASTNode {

private:
    char* name;
    TokenOrigin originPos;

public:
    static constexpr size_t MAX_NAME_LENGTH = 256u;

    explicit VariableNode(const char* name_, TokenOrigin originPos_) : ASTNode(VARIABLE_NODE), originPos(originPos_) {
        name = (char*)calloc(MAX_NAME_LENGTH, sizeof(char));
        for (unsigned int i = 0; i < MAX_NAME_LENGTH; ++i) {
            name[i] = name_[i];
            if (name[i] == '\0') break;
        }
    }

    ~VariableNode() {
        free(name);
    }

    char* getName() const {
        return name;
    }

    TokenOrigin getOriginPos() const {
        return originPos;
    }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class OperatorNode : public ASTNode {

private:
    std::shared_ptr<OperatorToken> token;

public:
    OperatorNode(const std::shared_ptr<OperatorToken>& token_, const std::vector<std::shared_ptr<ASTNode>>& children_) :
        ASTNode(OPERATOR_NODE, children_), token(token_) {
        assert(token_->getType() == OPERATOR && dynamic_cast<OperatorToken*>(token_.get())->getArity() == children_.size());
    }

    OperatorNode(const std::shared_ptr<OperatorToken>& token_, const std::shared_ptr<ASTNode>& child) :
        ASTNode(OPERATOR_NODE, child), token(token_) {
        assert(token_->getType() == OPERATOR && dynamic_cast<OperatorToken*>(token_.get())->getArity() == 1);
    }

    OperatorNode(const std::shared_ptr<OperatorToken>& token_, const std::shared_ptr<ASTNode>& leftChild, const std::shared_ptr<ASTNode>& rightChild) :
        ASTNode(OPERATOR_NODE, leftChild, rightChild), token(token_) {
        assert(token_->getType() == OPERATOR && dynamic_cast<OperatorToken*>(token_.get())->getArity() == 2);
    }

    const std::shared_ptr<OperatorToken>& getToken() const {
        return token;
    }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class ComparisonOperatorNode : public ASTNode {

private:
    std::shared_ptr<ComparisonOperatorToken> token;

public:
    ComparisonOperatorNode(const std::shared_ptr<ComparisonOperatorToken>& token_, const std::shared_ptr<ASTNode>& leftChild, const std::shared_ptr<ASTNode>& rightChild) :
        ASTNode(COMPARISON_OPERATOR_NODE, leftChild, rightChild), token(token_) {
        assert(token_->getType() == COMPARISON_OPERATOR);
    }

    const std::shared_ptr<ComparisonOperatorToken>& getToken() const {
        return token;
    }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class FunctionCallNode : public ASTNode {

private:
    const char* name;

public:
    FunctionCallNode(const char* name_, const std::vector<std::shared_ptr<ASTNode>>& children_) :
        ASTNode(FUNCTION_CALL_NODE, children_), name(name_) { }

    const char* getName() const {
        return name;
    }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class StatementsNode : public ASTNode {

public:
    explicit StatementsNode(const std::vector<std::shared_ptr<ASTNode>>& children_) : ASTNode(STATEMENTS_NODE, children_) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class BlockNode : public ASTNode {

public:
    explicit BlockNode(const std::shared_ptr<StatementsNode>& nestedStatements) : ASTNode(BLOCK_NODE, nestedStatements) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class IfNode : public ASTNode {

public:
    IfNode(const std::shared_ptr<ComparisonOperatorNode>& condition, const std::shared_ptr<ASTNode>& body) :
        ASTNode(IF_NODE, condition, body) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class IfElseNode : public ASTNode {

public:
    IfElseNode(const std::shared_ptr<ComparisonOperatorNode>& condition, const std::shared_ptr<ASTNode>& ifBody, const std::shared_ptr<ASTNode>& elseBody) :
        ASTNode(IF_ELSE_NODE, {condition, ifBody, elseBody}) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class WhileNode : public ASTNode {

public:
    WhileNode(const std::shared_ptr<ComparisonOperatorNode>& condition, const std::shared_ptr<ASTNode>& body) :
        ASTNode(WHILE_NODE, condition, body) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

#endif // AST_BUILDER_AST_H

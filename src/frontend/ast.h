/**
 * @file
 * @brief Definition of AST nodes and AST building functions
 */
#ifndef COMPILER_AST_H
#define COMPILER_AST_H

#include <cassert>
#include <cstdarg>
#include <memory>
#include <utility>
#include <vector>
#include "tokenizer.h"
#include "../util/constants.h"

class CodegenVisitor;

enum NodeType {
    CONSTANT_VALUE_NODE,
    VARIABLE_NODE,
    OPERATOR_NODE,
    COMPARISON_OPERATOR_NODE,
    STATEMENTS_NODE,
    BLOCK_NODE,
    IF_NODE,
    IF_ELSE_NODE,
    WHILE_NODE,
    PARAMETERS_LIST_NODE,
    ARGUMENTS_LIST_NODE,
    FUNCTION_DEFINITION_NODE,
    FUNCTION_CALL_NODE,
    RETURN_STATEMENT_NODE,
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
    static void dotPrintChildren(const ASTNode* node, FILE* dotFile);
    static void dotPrintCurrent(const ASTNode* node, FILE* dotFile, const char* label, const char* fillColor);
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
    explicit VariableNode(const char* name_, TokenOrigin originPos_) : ASTNode(VARIABLE_NODE), originPos(originPos_) {
        name = (char*)calloc(MAX_ID_LENGTH + 1, sizeof(char)); // +1 is for '\0'
        for (unsigned short i = 0; i < MAX_ID_LENGTH; ++i) {
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

class StatementsNode : public ASTNode {

public:
    explicit StatementsNode(const std::shared_ptr<ASTNode>& singleStatement) : ASTNode(STATEMENTS_NODE, singleStatement) { }
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

class ParametersListNode : public ASTNode {

public:
    explicit ParametersListNode(const std::vector<std::shared_ptr<ASTNode>>& parameters) :
        ASTNode(PARAMETERS_LIST_NODE, parameters) {
        for (const auto& parameter : parameters) assert(parameter->getType() == VARIABLE_NODE);
    }

    ParametersListNode() : ASTNode(PARAMETERS_LIST_NODE) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class ArgumentsListNode : public ASTNode {

public:
    explicit ArgumentsListNode(const std::vector<std::shared_ptr<ASTNode>>& arguments) :
        ASTNode(ARGUMENTS_LIST_NODE, arguments) { }

    ArgumentsListNode() : ASTNode(ARGUMENTS_LIST_NODE) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class FunctionDefinitionNode : public ASTNode {

private:
    const std::shared_ptr<IdToken> functionName;

public:
    FunctionDefinitionNode(
        const std::shared_ptr<IdToken>& functionName_,
        const std::shared_ptr<ParametersListNode>& parameters,
        const std::shared_ptr<BlockNode>& definition
    ) : ASTNode(FUNCTION_DEFINITION_NODE, {parameters, definition}), functionName(functionName_) { }

    void accept(CodegenVisitor* visitor) const override;

    inline std::shared_ptr<IdToken> getFunctionName() const {
        return functionName;
    }

protected:
    void dotPrint(FILE* dotFile) const override;
};

class FunctionCallNode : public ASTNode {

private:
    const std::shared_ptr<IdToken> functionName;

public:
    FunctionCallNode(
        const std::shared_ptr<IdToken>& functionName_,
        const std::shared_ptr<ArgumentsListNode>& arguments
    ) : ASTNode(FUNCTION_CALL_NODE, arguments), functionName(functionName_) { }

    void accept(CodegenVisitor* visitor) const override;

    inline std::shared_ptr<IdToken> getFunctionName() const {
        return functionName;
    }

protected:
    void dotPrint(FILE* dotFile) const override;
};

class ReturnStatementNode : public ASTNode {

public:
    explicit ReturnStatementNode(const std::shared_ptr<ASTNode>& returnedExpression) :
        ASTNode(RETURN_STATEMENT_NODE, returnedExpression) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

#endif // COMPILER_AST_H

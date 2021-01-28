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
    ASSIGNMENT_OPERATOR_NODE,
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
    VARIABLE_DECLARATION_NODE,
    RETURN_STATEMENT_NODE,
};

class ASTNode {

private:
    std::shared_ptr<ASTNode>* children = nullptr;
    size_t childrenNumber = 0;
    NodeType type;
    TokenOrigin originPos;

    static size_t nextNodeId;

public:
    const size_t nodeId;

    ASTNode(NodeType type_, TokenOrigin originPos_) : type(type_), originPos(originPos_), nodeId(nextNodeId++) {
        childrenNumber = 0;
        children = nullptr;
    }

    ASTNode(NodeType type_, TokenOrigin originPos_, const std::vector<std::shared_ptr<ASTNode>>& children_) : ASTNode(type_, originPos_) {
        childrenNumber = children_.size();
        children = new std::shared_ptr<ASTNode>[childrenNumber];
        for (size_t i = 0; i < childrenNumber; ++i) {
            children[i] = children_[i];
        }
    }

    ASTNode(NodeType type_, TokenOrigin originPos_, const std::shared_ptr<ASTNode>& child) : ASTNode(type_, originPos_) {
        childrenNumber = 1;
        children = new std::shared_ptr<ASTNode>[1];
        children[0] = child;
    }

    ASTNode(NodeType type_, TokenOrigin originPos_, const std::shared_ptr<ASTNode>& leftChild, const std::shared_ptr<ASTNode>& rightChild) : ASTNode(type_, originPos_) {
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

    TokenOrigin getOriginPos() const {
        return originPos;
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
    explicit ConstantValueNode(TokenOrigin originPos_, double value_) : ASTNode(CONSTANT_VALUE_NODE, originPos_), value(value_) { }

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

public:
    explicit VariableNode(TokenOrigin originPos_, const char* name_) : ASTNode(VARIABLE_NODE, originPos_) {
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

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class OperatorNode : public ASTNode {

private:
    std::shared_ptr<OperatorToken> token;

public:
    OperatorNode(const std::shared_ptr<OperatorToken>& token_, const std::vector<std::shared_ptr<ASTNode>>& children_) :
        ASTNode(OPERATOR_NODE, token_->getOriginPos(), children_), token(token_) {
        assert(token_->getType() == OPERATOR && dynamic_cast<OperatorToken*>(token_.get())->getArity() == children_.size());
    }

    OperatorNode(const std::shared_ptr<OperatorToken>& token_, const std::shared_ptr<ASTNode>& child) :
        ASTNode(OPERATOR_NODE, token_->getOriginPos(), child), token(token_) {
        assert(token_->getType() == OPERATOR && dynamic_cast<OperatorToken*>(token_.get())->getArity() == 1);
    }

    OperatorNode(const std::shared_ptr<OperatorToken>& token_, const std::shared_ptr<ASTNode>& leftChild, const std::shared_ptr<ASTNode>& rightChild) :
        ASTNode(OPERATOR_NODE, token_->getOriginPos(), leftChild, rightChild), token(token_) {
        assert(token_->getType() == OPERATOR && dynamic_cast<OperatorToken*>(token_.get())->getArity() == 2);
    }

    const std::shared_ptr<OperatorToken>& getToken() const {
        return token;
    }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class AssignmentOperatorNode : public ASTNode {

public:
    AssignmentOperatorNode(
        const std::shared_ptr<AssignmentOperatorToken>& token_,
        const std::shared_ptr<VariableNode>& variable,
        const std::shared_ptr<ASTNode>& value
    ) : ASTNode(ASSIGNMENT_OPERATOR_NODE, token_->getOriginPos(), variable, value) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class ComparisonOperatorNode : public ASTNode {

private:
    std::shared_ptr<ComparisonOperatorToken> token;

public:
    ComparisonOperatorNode(
        const std::shared_ptr<ComparisonOperatorToken>& token_,
        const std::shared_ptr<ASTNode>& leftChild,
        const std::shared_ptr<ASTNode>& rightChild
    ) : ASTNode(COMPARISON_OPERATOR_NODE, token_->getOriginPos(), leftChild, rightChild), token(token_) {
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
    StatementsNode(TokenOrigin originPos_, const std::shared_ptr<ASTNode>& singleStatement) : ASTNode(STATEMENTS_NODE, originPos_, singleStatement) { }
    StatementsNode(TokenOrigin originPos_, const std::vector<std::shared_ptr<ASTNode>>& children_) : ASTNode(STATEMENTS_NODE, originPos_, children_) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class BlockNode : public ASTNode {

public:
    BlockNode(TokenOrigin originPos_, const std::shared_ptr<StatementsNode>& nestedStatements) : ASTNode(BLOCK_NODE, originPos_, nestedStatements) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class IfNode : public ASTNode {

public:
    IfNode(TokenOrigin originPos_, const std::shared_ptr<ComparisonOperatorNode>& condition, const std::shared_ptr<ASTNode>& body) :
        ASTNode(IF_NODE, originPos_, condition, body) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class IfElseNode : public ASTNode {

public:
    IfElseNode(TokenOrigin originPos_, const std::shared_ptr<ComparisonOperatorNode>& condition, const std::shared_ptr<ASTNode>& ifBody, const std::shared_ptr<ASTNode>& elseBody) :
        ASTNode(IF_ELSE_NODE, originPos_, {condition, ifBody, elseBody}) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class WhileNode : public ASTNode {

public:
    WhileNode(TokenOrigin originPos_, const std::shared_ptr<ComparisonOperatorNode>& condition, const std::shared_ptr<ASTNode>& body) :
        ASTNode(WHILE_NODE, originPos_, condition, body) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class ParametersListNode : public ASTNode {

public:
    ParametersListNode(TokenOrigin originPos_, const std::vector<std::shared_ptr<ASTNode>>& parameters) :
        ASTNode(PARAMETERS_LIST_NODE, originPos_, parameters) {
        for (const auto& parameter : parameters) assert(parameter->getType() == VARIABLE_NODE);
    }

    explicit ParametersListNode(TokenOrigin originPos_) : ASTNode(PARAMETERS_LIST_NODE, originPos_) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class ArgumentsListNode : public ASTNode {

public:
    ArgumentsListNode(TokenOrigin originPos_, const std::vector<std::shared_ptr<ASTNode>>& arguments) :
        ASTNode(ARGUMENTS_LIST_NODE, originPos_, arguments) { }

    explicit ArgumentsListNode(TokenOrigin originPos_) : ASTNode(ARGUMENTS_LIST_NODE, originPos_) { }

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
    ) : ASTNode(FUNCTION_DEFINITION_NODE, functionName_->getOriginPos(), {parameters, definition}), functionName(functionName_) { }

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
    ) : ASTNode(FUNCTION_CALL_NODE, functionName_->getOriginPos(), arguments), functionName(functionName_) { }

    void accept(CodegenVisitor* visitor) const override;

    inline std::shared_ptr<IdToken> getFunctionName() const {
        return functionName;
    }

protected:
    void dotPrint(FILE* dotFile) const override;
};

class VariableDeclarationNode : public ASTNode {

public:
    VariableDeclarationNode(TokenOrigin originPos_, const std::shared_ptr<VariableNode>& variable) :
            ASTNode(VARIABLE_DECLARATION_NODE, originPos_, variable) { }
    VariableDeclarationNode(TokenOrigin originPos_, const std::shared_ptr<VariableNode>& variable, const std::shared_ptr<ASTNode>& initialValue) :
            ASTNode(VARIABLE_DECLARATION_NODE, originPos_, variable, initialValue) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

class ReturnStatementNode : public ASTNode {

public:
    ReturnStatementNode(TokenOrigin originPos_, const std::shared_ptr<ASTNode>& returnedExpression) :
        ASTNode(RETURN_STATEMENT_NODE, originPos_, returnedExpression) { }

    void accept(CodegenVisitor* visitor) const override;

protected:
    void dotPrint(FILE* dotFile) const override;
};

#endif // COMPILER_AST_H

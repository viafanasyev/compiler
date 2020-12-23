/**
 * @file
 * @brief Definition of AST node and AST building functions
 */
#ifndef AST_BUILDER_AST_H
#define AST_BUILDER_AST_H

#include <cassert>
#include <cstdarg>
#include <memory>
#include "tokenizer.h"

class ASTNode {

private:
    std::shared_ptr<ASTNode>* children = nullptr;
    size_t childrenNumber = 0;
    std::shared_ptr<Token> token;

public:
    explicit ASTNode(const std::shared_ptr<Token>& token_) {
        assert((token_->getType() == TokenType::CONSTANT_VALUE) || (token_->getType() == TokenType::VARIABLE));

        token = token_;
        childrenNumber = 0;
    }

    ASTNode(const std::shared_ptr<Token>& token_, const std::shared_ptr<ASTNode>& child) {
        assert(((token_->getType() == TokenType::OPERATOR) && (dynamic_cast<OperatorToken*>(token_.get())->getArity() == 1)) ||
               ((token_->getType() == TokenType::FUNCTION) && (dynamic_cast<FunctionToken*>(token_.get())->getArity() == 1)));

        token = token_;
        childrenNumber = 1;
        children = new std::shared_ptr<ASTNode>[1];
        children[0] = child;
    }

    ASTNode(const std::shared_ptr<Token>& token_, const std::shared_ptr<ASTNode>& leftChild, const std::shared_ptr<ASTNode>& rightChild) {
        assert(token_->getType() == TokenType::OPERATOR);
        assert(dynamic_cast<OperatorToken*>(token_.get())->getArity() == 2);

        token = token_;
        childrenNumber = 2;
        children = new std::shared_ptr<ASTNode>[2];
        children[0] = leftChild;
        children[1] = rightChild;
    }

    ASTNode(ASTNode&& astNode) noexcept {
        token = astNode.token;
        childrenNumber = astNode.childrenNumber;
        children = new std::shared_ptr<ASTNode>[childrenNumber];
        for (size_t i = 0; i < childrenNumber; ++i) {
            children[i] = astNode.children[i];
        }
    }

    void swap(ASTNode& astNode) {
        std::swap(token, astNode.token);
        std::swap(childrenNumber, astNode.childrenNumber);
        std::swap(children, astNode.children);
    }

    ASTNode& operator=(ASTNode astNode) {
        swap(astNode);
        return *this;
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

    std::shared_ptr<Token> getToken() const {
        return token;
    }

    void print(int depth = 0) const;

    void visualize(const char* fileName) const;
    void texify(const char* fileName) const;

    double calculate() const;

private:
    enum TexBraceType { NONE, ROUND, CURLY };

    void dotPrint(FILE* dotFile, int& nodeId) const;
    void texPrint(FILE* texFile, TexBraceType braceType = NONE) const;

    static TexBraceType getChildBraceType(const OperatorToken* parentOperator, const Token* child, bool isRightChild);
};

std::shared_ptr<ASTNode> buildAST(char* expression);

std::shared_ptr<ASTNode> buildAST(const std::vector<std::shared_ptr<Token> >& infixNotationTokens);

#endif // AST_BUILDER_AST_H

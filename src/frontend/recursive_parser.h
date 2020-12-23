/**
 * @file
 * @brief Definition of calculator that uses recursive parser
 *
 * This is a AST building copy of https://github.com/viafanasyev/recursive-parser
 */
#ifndef RECURSIVE_PARSER_CALCULATOR_H
#define RECURSIVE_PARSER_CALCULATOR_H

#include <cstring>
#include <map>
#include <memory>
#include "ast.h"
#include "tokenizer.h"

class SymbolTable {
private:
    struct keyCompare {
        bool operator()(const char* a, const char* b) const {
            return strcmp(a, b) < 0;
        }
    };

    std::map<const char*, std::shared_ptr<Token>, keyCompare> symbols;

public:
    SymbolTable();

    void addFunction(const char* name, const std::shared_ptr<FunctionToken>& functionToken) noexcept;
    void addVariable(char* name) noexcept;

    std::shared_ptr<Token> getSymbolByName(char* name) noexcept;
};

std::shared_ptr<ASTNode> buildASTRecursively(const char* expression);

#endif // RECURSIVE_PARSER_CALCULATOR_H

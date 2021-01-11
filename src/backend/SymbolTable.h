/**
 * @file
 */
#ifndef COMPILER_SYMBOLTABLE_H
#define COMPILER_SYMBOLTABLE_H

#include <cstddef>
#include <cstring>
#include <map>
#include <memory>
#include <stack>

struct VariableSymbol {
    size_t address;

    VariableSymbol() : address(-1) { }
    explicit VariableSymbol(size_t address_) : address(address_) { }
};

class SymbolTable {

private:
    struct keyCompare {
        bool operator()(char* a, char* b) const {
            return strcmp(a, b) < 0;
        }
    };

    std::map<char*, VariableSymbol, keyCompare> symbols;
    std::stack<std::pair<char**, size_t>> savedBlocks;

public:
    static constexpr size_t MAX_NAME_LENGTH = 256u;
    static constexpr unsigned char VARIABLE_SIZE_IN_BYTES = 8;

    ~SymbolTable();

    VariableSymbol addVariable(char* name);
    bool hasVariable(char* name) const;
    VariableSymbol getVariableByName(char* name) const;

    void enterBlock();
    void leaveBlock();
};


#endif // COMPILER_SYMBOLTABLE_H

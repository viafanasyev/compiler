/**
 * @file
 * @brief Definition of symbol table and symbols for variables and functions.
 *        Used to save symbols, their positions in memory and specific information (like labels for functions)
 */
#ifndef COMPILER_SYMBOLTABLE_H
#define COMPILER_SYMBOLTABLE_H

#include <cstddef>
#include <cstring>
#include <forward_list>
#include <map>
#include <memory>
#include <stack>
#include "Label.h"
#include "../util/TokenOrigin.h"

enum Type {
    VOID,
    DOUBLE,
};

static const char* const TypeStrings[] = {
    "void",
    "double",
};

struct VariableSymbol {
    const unsigned int address;
    const TokenOrigin originPos;

    VariableSymbol(unsigned int address_, const TokenOrigin& originPos_);
};

struct FunctionSymbol {
    const std::shared_ptr<Label> label = nullptr;
    char* const internalName = nullptr;
    const Type returnType;
    const unsigned char argumentsNumber;
    const TokenOrigin originPos;

    /** Constructor for non-internal functions */
    FunctionSymbol(const char* functionName, Type returnType_, unsigned char argumentsNumber_, const TokenOrigin& originPos_);
    /** Constructor for internal functions */
    FunctionSymbol(const char* functionName, Type returnType_, unsigned char argumentsNumber_);

    char* getName() const;

    bool isInternal() const;
    bool isVoid() const;
};

/**
 * Symbol table contains variables and functions.
 *
 * Functions can be defined only in outer scope, so they just stored in std::map by their names.
 *
 * Variables can be defined everywhere except outer scope.
 * Also, any variable can be redefined in any block, that is a child of the block this variable defined in.
 * For example, code below works (nested variable shadows previous defined variable):
 *
 *     {
 *         ...define x...
 *         {
 *             ...define x...
 *         }
 *     }
 *
 * But the next code - doesn't:
 *     {
 *         ...define x...
 *         ...define x...
 *     }
 *
 * Next algorithm is used for variable storing:
 *   - Each scope is an std::map inside of the std::forward_list. Front of the list is the current scope.
 *   - On block enter: new empty std::map pushed into the list.
 *   - On block leave: node on the front of the list is removed and all the variables in this scope are removed too.
 *   - On variable create: variable added into the current scope (node on the front of the list).
 *   - Variables are searched from front to back. So the variable in the nested scope shadows the variable in the parent scope, if they both have similar names.
 *   - Each variable has a size of 8 bytes, because only type 'double' is supported yet.
 *   - All variables are saved linearly in the RAM - if variable 'y' is declared just after variable 'x', then 'y'.address = 'x'.address + 8.
 */
class SymbolTable {

private:
    struct keyCompare {
        bool operator()(const char* a, const char* b) const {
            return strcmp(a, b) < 0;
        }
    };

    template <typename S>
    using SymbolsMap = std::map<char*, std::shared_ptr<S>, keyCompare>;

    std::forward_list<SymbolsMap<VariableSymbol>> variables;
    unsigned int nextLocalVariableAddress = 0;

    SymbolsMap<FunctionSymbol> functions;

public:
    SymbolTable();
    ~SymbolTable();

    std::shared_ptr<VariableSymbol> addVariable(char* name, const TokenOrigin& originPos);
    bool hasVariable(char* name) const;
    std::shared_ptr<VariableSymbol> getVariableByName(char* name) const;
    unsigned int getNextLocalVariableAddress() const;

    std::shared_ptr<FunctionSymbol> addFunction(char* name, Type returnType, unsigned char argumentsNumber, const TokenOrigin& originPos);
    bool hasFunction(char* name) const;
    std::shared_ptr<FunctionSymbol> getFunctionByName(char* name) const;

    void enterFunction();
    void leaveFunction();
    void enterBlock();
    void leaveBlock();
};

#endif // COMPILER_SYMBOLTABLE_H

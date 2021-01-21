/**
 * @file
 * @brief Implementation of symbol table and symbols for variables and functions
 */
#include <cassert>
#include <cstdlib>
#include <vector>
#include "SymbolTable.h"
#include "Label.h"
#include "../util/constants.h"
#include "../util/RedefinitionError.h"

static inline char* copyName(const char* name);

VariableSymbol::VariableSymbol(unsigned int address_, const TokenOrigin& originPos_) :
    address(address_), originPos(originPos_)
{ }

/** Constructor for non-internal functions */
FunctionSymbol::FunctionSymbol(const char* functionName, Type returnType_, unsigned char argumentsNumber_, const TokenOrigin& originPos_) :
        label(std::make_shared<Label>(functionName)),
        internalName(nullptr),
        returnType(returnType_),
        argumentsNumber(argumentsNumber_),
        originPos(originPos_)
{ }

/** Constructor for internal functions */
FunctionSymbol::FunctionSymbol(const char* functionName, Type returnType_, unsigned char argumentsNumber_) :
        label(nullptr),
        internalName(copyName(functionName)),
        returnType(returnType_),
        argumentsNumber(argumentsNumber_),
        originPos({ INT64_MAX, INT64_MAX })
{ }

bool FunctionSymbol::isInternal() const {
    return internalName != nullptr;
}

bool FunctionSymbol::isVoid() const {
    return returnType == VOID;
}

char* FunctionSymbol::getName() const {
    return isInternal() ? internalName : label->getName();
}

SymbolTable::SymbolTable() {
    variables.push_front(SymbolsMap<VariableSymbol>());

    functions[copyName("read")]  = std::make_shared<FunctionSymbol>("IN",   Type::DOUBLE, 0);
    functions[copyName("print")] = std::make_shared<FunctionSymbol>("OUT",  Type::VOID,   1);
    functions[copyName("sqrt")]  = std::make_shared<FunctionSymbol>("SQRT", Type::DOUBLE, 1);
}

SymbolTable::~SymbolTable() {
    for (const auto& block : variables) {
        for (const auto& symbol : block) {
            free(symbol.first);
        }
    }

    for (const auto& symbol : functions) {
        free(symbol.first);
    }
}

std::shared_ptr<VariableSymbol> SymbolTable::addVariable(char* name, const TokenOrigin& originPos) {
    assert(name != nullptr);

    if (variables.front().count(name) != 0) throw RedefinitionError(name, originPos, getVariableByName(name)->originPos);

    auto symbol = std::make_shared<VariableSymbol>(nextLocalVariableAddress, originPos);
    nextLocalVariableAddress += VARIABLE_SIZE_IN_BYTES;
    variables.front()[copyName(name)] = symbol;
    return symbol;
}

bool SymbolTable::hasVariable(char* name) const {
    assert(name != nullptr);

    for (const auto& block : variables) {
        if (block.count(name) != 0) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<VariableSymbol> SymbolTable::getVariableByName(char* name) const {
    assert(name != nullptr);

    for (const auto& block : variables) {
        if (block.count(name) != 0) {
            return block.at(name);
        }
    }

    throw std::out_of_range("Variable not found");
}

unsigned int SymbolTable::getNextLocalVariableAddress() const {
    return nextLocalVariableAddress;
}

std::shared_ptr<FunctionSymbol> SymbolTable::addFunction(char* name, Type returnType, unsigned char argumentsNumber, const TokenOrigin& originPos) {
    assert(name != nullptr);

    if (hasFunction(name)) throw RedefinitionError(name, originPos, getFunctionByName(name)->originPos);

    auto symbol = std::make_shared<FunctionSymbol>(name, returnType, argumentsNumber, originPos);
    functions[copyName(name)] = symbol;
    return symbol;
}

bool SymbolTable::hasFunction(char* name) const {
    assert(name != nullptr);

    return functions.count(name) != 0;
}

std::shared_ptr<FunctionSymbol> SymbolTable::getFunctionByName(char* name) const {
    assert(name != nullptr);

    return functions.at(name);
}

void SymbolTable::enterFunction() {
    enterBlock();
    nextLocalVariableAddress = 0;
}

void SymbolTable::leaveFunction() {
    leaveBlock();
}

void SymbolTable::enterBlock() {
    variables.push_front(SymbolsMap<VariableSymbol>());
}

void SymbolTable::leaveBlock() {
    for (const auto& symbol : variables.front()) {
        free(symbol.first);
    }
    variables.pop_front();

    // Restore next variable address
    unsigned int maxLocalVariableAddress = 0u;
    for (const auto& symbol : variables.front()) {
        maxLocalVariableAddress = std::max(maxLocalVariableAddress, symbol.second->address);
    }
    nextLocalVariableAddress = maxLocalVariableAddress + VARIABLE_SIZE_IN_BYTES;
}

static inline char* copyName(const char* name) {
    char* nameCopy = (char*)calloc(strlen(name) + 1, sizeof(char)); // +1 is for '\0'
    strcpy(nameCopy, name);
    return nameCopy;
}

/**
 * @file
 */
#include <cstdlib>
#include "SymbolTable.h"

SymbolTable::~SymbolTable() {
    for (auto& symbol : symbols) {
        free(symbol.first);
    }

    while (!savedBlocks.empty()) {
        delete[] savedBlocks.top().first;
        savedBlocks.pop();
    }
}

VariableSymbol SymbolTable::addVariable(char* name) {
    if (symbols.count(name) == 0) {
        char* nameCopy = (char*)calloc(MAX_NAME_LENGTH, sizeof(char));
        for (size_t i = 0; i < MAX_NAME_LENGTH; ++i) {
            nameCopy[i] = name[i];
            if (nameCopy[i] == '\0') break;
        }

        VariableSymbol symbol(symbols.size() * VARIABLE_SIZE_IN_BYTES);
        symbols[nameCopy] = symbol;
        return symbol;
    }
    return symbols.at(name);
}

bool SymbolTable::hasVariable(char* name) const {
    return symbols.count(name) != 0;
}

VariableSymbol SymbolTable::getVariableByName(char* name) const {
    return symbols.at(name);
}

void SymbolTable::enterBlock() {
    char** currentVariableNames = new char*[symbols.size()];
    size_t i = 0;
    for (const auto& symbol : symbols) {
        currentVariableNames[i++] = symbol.first;
    }
    savedBlocks.emplace(currentVariableNames, symbols.size());
}

void SymbolTable::leaveBlock() {
    char** savedNames = savedBlocks.top().first;
    size_t savedNamesNumber = savedBlocks.top().second;
    savedBlocks.pop();

    std::map<char*, VariableSymbol, keyCompare> savedSymbols;
    for (size_t i = 0; i < savedNamesNumber; ++i) {
        if (symbols.count(savedNames[i]) != 0) {
            savedSymbols[savedNames[i]] = symbols[savedNames[i]];
        }
    }
    symbols.clear();
    symbols = savedSymbols;

    delete[] savedNames;
}

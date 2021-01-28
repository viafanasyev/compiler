/**
 * @file
 */
#ifndef COMPILER_COERCIONERROR_H
#define COMPILER_COERCIONERROR_H

#include <exception>
#include "TokenOrigin.h"
#include "../backend/SymbolTable.h"

class CoercionError : public std::exception {
protected:
    TokenOrigin position;
    char* message;

public:
    CoercionError(TokenOrigin position_, Type from, Type to);

    ~CoercionError() override;

    const char* what() const noexcept override;

    TokenOrigin at() const noexcept;

};

#endif // COMPILER_COERCIONERROR_H

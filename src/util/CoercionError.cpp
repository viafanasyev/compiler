/**
 * @file
 */
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "CoercionError.h"
#include "TokenOrigin.h"
#include "../backend/SymbolTable.h"

CoercionError::CoercionError(TokenOrigin position_, Type from, Type to) : position(position_) {
    const char* fromTypeString = TypeStrings[from];
    const char* toTypeString   = TypeStrings[to];
    size_t messageLen = 128;
    message = (char*)calloc(messageLen, sizeof(char));
    snprintf(message, messageLen, "Can't coerce %s to %s (%zu:%zu)", fromTypeString, toTypeString, position_.line, position_.column);
}

CoercionError::~CoercionError() {
    free(message);
}

const char* CoercionError::what() const noexcept {
    return message;
}

TokenOrigin CoercionError::at() const noexcept {
    return position;
}

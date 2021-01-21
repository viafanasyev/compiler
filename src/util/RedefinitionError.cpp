/**
 * @file
 */
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include "RedefinitionError.h"
#include "TokenOrigin.h"

RedefinitionError::RedefinitionError(const char* name, const TokenOrigin& newDefinition, const TokenOrigin& oldDefinition) {
    message = (char*)calloc(MAX_MESSAGE_LENGTH, sizeof(char));

    if (oldDefinition.line == INT64_MAX && oldDefinition.column == INT64_MAX) {
        snprintf(
            message, MAX_MESSAGE_LENGTH,
             "Redefinition of '%s' at %zu:%zu (previously defined internally)",
             name, newDefinition.line, newDefinition.column
        );
    } else {
        snprintf(
            message, MAX_MESSAGE_LENGTH,
            "Redefinition of '%s' at %zu:%zu (previously defined at %zu:%zu)",
            name, newDefinition.line, newDefinition.column, oldDefinition.line, oldDefinition.column
        );
    }
}

RedefinitionError::~RedefinitionError() {
    free(message);
}

const char* RedefinitionError::what() const noexcept {
    return message;
}

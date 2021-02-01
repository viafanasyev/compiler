/**
 * @file
 */
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include "ValueReassignmentError.h"
#include "TokenOrigin.h"

ValueReassignmentError::ValueReassignmentError(const TokenOrigin& declaration, const TokenOrigin& reassignment) {
    message = (char*)calloc(MAX_MESSAGE_LENGTH, sizeof(char));

    if (declaration.line == INT64_MAX && declaration.column == INT64_MAX) {
        snprintf(
            message, MAX_MESSAGE_LENGTH,
             "Value can't be reassigned (%zu:%zu, declared internally)",
             reassignment.line, reassignment.column
        );
    } else {
        snprintf(
            message, MAX_MESSAGE_LENGTH,
            "Value can't be reassigned (%zu:%zu, declared at %zu:%zu)",
            reassignment.line, reassignment.column, declaration.line, declaration.column
        );
    }
}

ValueReassignmentError::~ValueReassignmentError() {
    free(message);
}

const char* ValueReassignmentError::what() const noexcept {
    return message;
}

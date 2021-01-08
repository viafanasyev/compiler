/**
 * @file
 */
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "SyntaxError.h"
#include "tokenizer.h"

SyntaxError::SyntaxError(TokenOrigin position_, const char* cause_) {
    position = position_;
    size_t messageLen = strlen(cause_) + 4 + MAX_LONG_LENGTH + 1 + MAX_LONG_LENGTH + 1;
    message = (char*)calloc(messageLen, sizeof(char));
    snprintf(message, messageLen, "%s at %zu:%zu", cause_, position_.line, position_.column);
}

SyntaxError::SyntaxError(const char* cause_) {
    position = { INT64_MAX, INT64_MAX };
    size_t messageLen = strlen(cause_) + 1;
    message = (char*)calloc(messageLen, sizeof(char));
    snprintf(message, messageLen, "%s", cause_);
}

SyntaxError::~SyntaxError() {
    free(message);
}

const char* SyntaxError::what() const noexcept {
    return message;
}

TokenOrigin SyntaxError::at() const noexcept {
    return position;
}

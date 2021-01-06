/**
 * @file
 */
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "SyntaxError.h"

SyntaxError::SyntaxError(int position_, const char* cause_) {
    position = position_;
    size_t messageLen = strlen(cause_) + 4 + MAX_INT_LENGTH + 1;
    message = (char*)calloc(messageLen, sizeof(char));
    snprintf(message, messageLen, "%s at %d", cause_, position_);
}

SyntaxError::SyntaxError(const char* cause_) {
    position = -1;
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

int SyntaxError::at() const noexcept {
    return position;
}

/**
 * @file
 */
#ifndef COMPILER_SYNTAXERROR_H
#define COMPILER_SYNTAXERROR_H

#include <exception>
#include "TokenOrigin.h"

class SyntaxError : public std::exception {
protected:
    TokenOrigin position;
    char* message;

public:
    SyntaxError(TokenOrigin position_, const char* cause_);

    explicit SyntaxError(const char* cause_);

    ~SyntaxError() override;

    const char* what() const noexcept override;

    TokenOrigin at() const noexcept;
};

#endif // COMPILER_SYNTAXERROR_H

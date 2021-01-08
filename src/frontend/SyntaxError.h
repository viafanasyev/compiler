/**
 * @file
 */
#ifndef AST_BUILDER_SYNTAXERROR_H
#define AST_BUILDER_SYNTAXERROR_H

#include <exception>
#include "tokenizer.h"

class SyntaxError : public std::exception {
private:
    static constexpr int MAX_LONG_LENGTH = 19;

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

#endif // AST_BUILDER_SYNTAXERROR_H

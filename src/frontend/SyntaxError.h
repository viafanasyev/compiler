/**
 * @file
 */
#ifndef AST_BUILDER_SYNTAXERROR_H
#define AST_BUILDER_SYNTAXERROR_H

#include <exception>

class SyntaxError : public std::exception {
private:
    static constexpr int MAX_INT_LENGTH = 10;

protected:
    int position;
    char* message;

public:
    SyntaxError(int position_, const char* cause_);

    ~SyntaxError() override;

    const char* what() const noexcept override;

    int at() const noexcept;
};

#endif // AST_BUILDER_SYNTAXERROR_H

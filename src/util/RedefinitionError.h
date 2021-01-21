/**
 * @file
 */
#ifndef COMPILER_REDEFINITIONERROR_H
#define COMPILER_REDEFINITIONERROR_H

#include <exception>
#include "TokenOrigin.h"

class RedefinitionError : public std::exception {
private:
    static constexpr int MAX_MESSAGE_LENGTH = 1024;

protected:
    char* message;

public:
    RedefinitionError(const char* name, const TokenOrigin& newDefinition, const TokenOrigin& oldDefinition);

    ~RedefinitionError() override;

    const char* what() const noexcept override;
};



#endif // COMPILER_REDEFINITIONERROR_H

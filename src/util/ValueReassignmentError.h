/**
 * @file
 */
#ifndef COMPILER_VALUEREASSIGNMENTERROR_H
#define COMPILER_VALUEREASSIGNMENTERROR_H

#include <exception>
#include "TokenOrigin.h"

class ValueReassignmentError : public std::exception {
private:
    static constexpr int MAX_MESSAGE_LENGTH = 1024;

protected:
    char* message;

public:
    ValueReassignmentError(const TokenOrigin& declaration, const TokenOrigin& reassignment);

    ~ValueReassignmentError() override;

    const char* what() const noexcept override;
};



#endif // COMPILER_VALUEREASSIGNMENTERROR_H

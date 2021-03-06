/**
 * @file
 * @brief Definition and implementation of IR code label. Label can be used for jumps or calls.
 */
#ifndef COMPILER_LABEL_H
#define COMPILER_LABEL_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../util/constants.h"

struct Label {

private:
    static unsigned int nextId;

    char* name;

public:
    const unsigned int id;

    Label() : id(nextId++) {
        name = (char*)calloc(1 + MAX_INT_LENGTH + 1  /* = 'L' + id + '\0' */, sizeof(char));
        snprintf(name, 1 + MAX_INT_LENGTH + 1, "L%d", id);
    }

    explicit Label(const char* name_) : id(nextId++) {
        name = (char*)calloc(strlen(name_) + 1, sizeof(char)); // +1 is for '\0'
        strcpy(name, name_);
    }

    ~Label() {
        free(name);
    }

    inline char* getName() const {
        return name;
    }
};

#endif // COMPILER_LABEL_H

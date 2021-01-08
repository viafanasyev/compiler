/**
 * @file
 * @brief Source file for MappedFile class
 */
#include <cassert>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "MappedFile.h"

/**
 * Maps the given file using mmap function.
 * If the mapping fails, textPtr is set to nullptr and textSize is set to 0.
 * Constructor also ensures that the given file is POSIX-like (ends with '\\n') - just adds '\\n' to the end of the text.
 * @param[in] filePath path to the file to map
 */
MappedFile::MappedFile(const char* filePath) {
    assert(filePath != nullptr);

    int fd = open(filePath, O_RDONLY);
    if (fd < 0) {
        return;
    }

    struct stat statbuf{};
    if ((fstat(fd, &statbuf) < 0) || (statbuf.st_size == 0)) {
        close(fd);
        return;
    }

    textSize = statbuf.st_size + 1; // +1 - to add \n at the end of the file
    void* dataPtr = mmap(nullptr, textSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
    if (dataPtr == MAP_FAILED) {
        textSize = 0;
        return;
    }

    textPtr = static_cast<char*>(dataPtr);
    textPtr[textSize - 1] = '\n'; // Ensures that this file is a POSIX-like text file (ends with \n)
}

/**
 * Unmaps the file using munmap function if it was successfully mapped in constructor.
 */
MappedFile::~MappedFile() {
    if (textPtr != nullptr) {
        munmap(textPtr, textSize);
    }
}

char* MappedFile::getTextPtr() const {
    return textPtr;
}

size_t MappedFile::getTextSize() const {
    return textSize;
}
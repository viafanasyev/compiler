/**
 * @file
 * @brief Header file for MappedFile class
 */
#ifndef COMPILER_MAPPEDFILE_H
#define COMPILER_MAPPEDFILE_H

#include <cstddef>

/**
 * Represents a text file mapped by mmap function.
 * mmap is called during a constructor invocation.
 * munmap is called in destructor.
 * If the mapping fails, textPtr is set to nullptr and textSize is set to 0.
 * Otherwise, textPtr points to a text start and textSize is set to a text size in bytes.
 */
class MappedFile {
private:
    char* textPtr = nullptr;
    size_t textSize = 0;

public:

    /**
     * Maps the given file using mmap function.
     * If the mapping fails, textPtr is set to nullptr and textSize is set to 0.
     * Constructor also ensures that the given file is POSIX-like (ends with '\\n') - just adds '\\n' to the end of the text.
     * @param[in] filePath path to the file to map
     */
    explicit MappedFile(const char* filePath);

    MappedFile(MappedFile& mappedFile) = delete;
    MappedFile &operator=(const MappedFile&) = delete;

    /**
     * Unmaps the file using munmap function if it was successfully mapped in constructor.
     */
    ~MappedFile();

    char* getTextPtr() const;

    size_t getTextSize() const;
};


#endif //COMPILER_MAPPEDFILE_H
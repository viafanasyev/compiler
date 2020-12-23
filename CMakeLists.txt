cmake_minimum_required(VERSION 3.16)
project(compiler)

set(CMAKE_CXX_STANDARD 14)

add_compile_options(-Wall -Wextra -pedantic -Werror -Wfloat-equal)

add_executable(
        compiler
        src/main.cpp
        src/frontend/tokenizer.h
        src/frontend/tokenizer.cpp
        src/frontend/ast.h
        src/frontend/ast.cpp
        src/middleend/ast-optimizers.h
        src/middleend/ast-optimizers.cpp
        src/frontend/recursive_parser.h
        src/frontend/recursive_parser.cpp
        src/frontend/SyntaxError.cpp
        src/frontend/SyntaxError.h)

add_executable(
        tests
        test/main.cpp
        test/testlib.h
        test/testlib.cpp
        test/frontend/tokenizer_tests.cpp
        src/frontend/tokenizer.h
        src/frontend/tokenizer.cpp
        src/frontend/ast.h
        src/frontend/ast.cpp)
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
        src/util/SyntaxError.h
        src/util/SyntaxError.cpp
        src/MappedFile.h
        src/MappedFile.cpp
        src/backend/codegen.h
        src/backend/codegen.cpp
        src/backend/SymbolTable.h
        src/backend/SymbolTable.cpp
        src/stack-machine/src/stack-machine-utils.h
        src/stack-machine/src/stack-machine-utils.cpp
        src/stack-machine/src/stack-machine.h
        src/stack-machine/src/stack-machine.cpp
        src/stack-machine/src/arg-parser.h
        src/stack-machine/src/arg-parser.cpp
        src/util/TokenOrigin.h
        src/util/RedefinitionError.h
        src/util/RedefinitionError.cpp
        src/backend/Label.h
        src/backend/Label.cpp
        src/util/constants.h)

add_executable(
        tests
        test/main.cpp
        test/testlib.h
        test/testlib.cpp
        test/frontend/tokenizer_tests.cpp
        src/frontend/tokenizer.h
        src/frontend/tokenizer.cpp
        src/util/SyntaxError.h
        src/util/SyntaxError.cpp
        src/frontend/ast.h
        src/frontend/ast.cpp
        src/backend/codegen.h
        src/backend/codegen.cpp
        src/backend/SymbolTable.h
        src/backend/SymbolTable.cpp
        src/util/RedefinitionError.h
        src/util/RedefinitionError.cpp
        src/backend/Label.h
        src/backend/Label.cpp)
# Compiler

## Project description

This program is developed as a part of ISP RAS course.  

NOTE: This program runs only on UNIX-like OS. Also `dot` and `pdflatex` should be installed.

### Structure

* src/ : Main project
  * frontend/ : Parsing, AST building and etc.
    * ast.h, ast.cpp : Definition and implementation of AST node, AST building, visualization and TeX conversion functions;
    * tokenizer.h, tokenizer.cpp : Definition and implementation of tokens and tokenizer functions;
    * recursive_parser.h, recursive_parser.cpp : Definition and implementation of recursive parser;
    * SyntaxError.h, SyntaxError.cpp : Definition and implementation of exception that is thrown on syntax error;
  * middleend/ : AST optimizations
    * ast-optimizers.h, ast-optimizers.cpp : Definition and implementation of AST optimizers;
  * backend/ : IR generation
  * main.cpp : Entry point for the program.

* test/ : Tests and testing library
  * frontend/: Tests for compiler frontend
    * tokenizer_tests.cpp : Tests for tokenizer functions;
  * testlib.h, testlib.cpp : Library for testing with assertions and helper macros;
  * main.cpp : Entry point for tests. Just runs all tests.

* doc/ : doxygen documentation

* Doxyfile : doxygen config file

### Run

### Compiler

To run compiler execute next commands in terminal:
```shell script
cmake . && make
./compiler "(N * (N + 1)) / 2"
```

#### Tests

To run tests execute next commands in terminal:
```shell script
cmake . && make
./tests
```

### Documentation

Doxygen is used to create documentation. You can watch it by opening `doc/html/index.html` in browser.  

### OS

Program is developed under Ubuntu 20.04.1 LTS.
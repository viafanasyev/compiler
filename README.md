# Compiler

## Project description

This program is developed as a part of ISP RAS course.  

This program is a compiler for a <span style="font-size:0.75em;">currently unnamed</span> language. 
It can program build and show AST for code, compile and run programs on [pseudo-virtual-stack-machine](https://github.com/viafanasyev/stack-machine).

NOTE: This program runs only on UNIX-like OS. Also `dot` should be installed.

## Structure

* src/ : Main project
  * backend/ : IR generation
    * codegen.h, codegen.cpp : Definition and implementation of IR code generation functions - particularly, a CodegenVisitor for ASTNodes;
    * Label.h, Label.cpp : Definition and implementation of IR code label (used for jump and call instructions);
    * SymbolTable.h, SymbolTable.cpp : Definition and implementation of symbol table and symbols for variables and functions. Used to save symbols, their positions in memory and specific information (like labels for functions);
  * frontend/ : Parsing, AST building and etc.
    * ast.h, ast.cpp : Definition and implementation of AST node, AST building and visualization functions;
    * recursive_parser.h, recursive_parser.cpp : Definition and implementation of recursive parser;
    * tokenizer.h, tokenizer.cpp : Definition and implementation of tokens and tokenizer functions;
  * middleend/ : AST optimizations
    * ast-optimizers.h, ast-optimizers.cpp : Definition and implementation of AST optimizers;
  * stack-machine/ : stack machine that runs compiled program (see [GitHub repo](https://github.com/viafanasyev/stack-machine))
  * util/ : Utility classes, functions, etc.
    * constants.h : Useful constants like maximal variable name length;
    * RedefinitionError.h, RedefinitionError.cpp : Definition and implementation of exception that is thrown on variable or function being redefined;
    * SyntaxError.h, SyntaxError.cpp : Definition and implementation of exception that is thrown on syntax error;
    * TokenOrigin.h : Structure containing origin position of token;
  * main.cpp : Entry point for the program;
  * MappedFile.h, MappedFile.cpp : Represents a text file mapped by mmap function.

* test/ : Tests and testing library
  * frontend/: Tests for compiler frontend
    * tokenizer_tests.cpp : Tests for tokenizer functions;
  * testlib.h, testlib.cpp : Library for testing with assertions and helper macros;
  * main.cpp : Entry point for tests. Just runs all tests.

* doc/ : doxygen documentation

* Doxyfile : doxygen config file

## Language description

This language has (almost) a C-style syntax.   
Each program should have no-arg main function.  
Each statement should be terminated with `;`.
Each variable should be declared using `var` keyword: `var x = 42;`.  
There's no global variables.  
Each function should be defined using `func` keyword: `func foo(a, b, c) { ... }`.  
Each function (except main) should return value. Implicit `return 0` added to the end of each function, if there's no return.  

Like in C, only function defined above are visible in the current function.  
Variables can be shadowed in the nested scopes. For example:
```
{
    var x = 0;
    {
        var x = 1; <-- Shadows previous declared variable
    }
    <-- But x is equal to 0 here
}
```

There's no type system in the current version of this language, so every variable and function has type `double` (only exception is main function - it has type `void`).  
This means, that there's no operations like `%` (remainder operator).  
Note: `==` operator compares value with 1e-9 precision (`x == y` -> `|x - y| < 1e-9`), not by actual value.  

Language has next internal functions:
  * `read()` - reads number from console and returns it's value;
  * `print(x)` - prints value of `x` into console;
  * `sqrt(x)` - returns square root of the `x`;
  * `pow(x, y)` - returns `x` raised to the `y` power.

<br>

For list of all features to be done, see `TODO.md`.

### Examples

All examples can be found in `examples/` directory.

#### Recursive and non-recursive fibonacci
File: `fibonacci.txt`
```
func fib(n) {
    var cur = 0;
    var next = 1;
    while (n > 0) {
        next = cur + next;
        cur = next - cur;
        n = n - 1;
    }
    return cur;
}

func recFib(n) {
    if (n <= 2) return 1;
    return recFib(n - 1) + recFib(n - 2);
}

func main() {
    var n = read();
    while (n > 0) {
        print(fib(n));
        print(recFib(n));
        n = read();
    }
}
```

### Quadratic equation solver
File: `quadratic_equation.txt`
```
func solveLinear(a, b) {
    if (a == 0) {
        if (b == 0) {
            print(-1);
        } else {
            print(0);
        }
        return 0;
    }

    print(1);
    print(-b / a);
    return 0;
}

func solveQuadratic(a, b, c) {
    if (a == 0) {
        solveLinear(b, c);
        return 0;
    }

    var d = b*b - 4*a*c;
    if (d < 0) {
        print(0);
    } else if (d == 0) {
        print(1);
        print(-b / (2 * a));
    } else {
        var x1 = (-b - sqrt(d)) / (2 * a);
        var x2 = (-b + sqrt(d)) / (2 * a);
        print(2);
        print(x1);
        print(x2);
    }
    return 0;
}

func main() {
    var a = read();
    var b = read();
    var c = read();

    solveQuadratic(a, b, c);
}
```

### Grammar
```
G = OuterScopeStatements '\0'
OuterScopeStatements = (OuterScopeStatement)*
FunctionScopeStatements = (FunctionScopeStatement)*
OuterScopeStatement = FunctionDefinition
FunctionScopeStatement = Expression ';' | Assignment ';' | VariableDeclaration | Block | IfStatement | WhileStatement | ReturnStatement
Block = '{' FunctionScopeStatements '}'
IfStatement = 'if' '(' ComparisonExpression ')' FunctionScopeStatement ('else' FunctionScopeStatement)?
WhileStatement = 'while' '(' ComparisonExpression ')' FunctionScopeStatement
ComparisonExpression = Expression [< > == <= >=] Expression
FunctionDefinition = 'func' ID '(' ParametersList ')' Block
ParametersList = ( Variable (',' Variable)* )?
ReturnStatement = 'return' Expression ';'
VariableDeclaration = 'var' Variable ('=' Expression)? ';'
Expression = Term ([+ -] Term)*
Term = Factor ([* /] Factor)*
Factor = ('+' | '-') Factor | '(' Expression ')' | Number | Variable | FunctionCall
Assignment = Variable '=' Expression
FunctionCall = ID '(' ArgumentsList ')'
ArgumentsList = ( Expression (',' Expression)* )?
Variable = ID
Number = [0-9]+
ID = [a-z A-Z] [a-z A-Z 0-9]*
```

## Run

### Compiler

There are 3 modes compiler can be run in:
  * Compile
  * Print AST
  * Compile and run

To run compiler execute next commands in terminal:
```shell script
cmake . && make
./compiler code.txt     # Just compile program code from code.txt
./compiler code.txt ast # Print AST of the parsed program
./compiler code.txt run # Compile and run program on stack machine
```

### Tests

To run tests execute next commands in terminal:
```shell script
cmake . && make
./tests
```

## Documentation

Doxygen is used to create documentation. You can watch it by opening `doc/html/index.html` in browser.  

## OS

Program is developed under Ubuntu 20.04.1 LTS.
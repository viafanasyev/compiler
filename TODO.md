# TODO

### Language improvements
* Global variables
* Logical operators (`||`, `&&`, `!`)
* Bitwise operators (`|`, `&`, `~`, `^`)
* Type system

### Compiler improvements
* Put instructions into list instead of simply printing to file to perform optimizations
* Resolve labels and calls in two walks. Remove `mainFunction` creation from codegen.cpp:27
* Control flow analysis on instructions list. Don't generate implicit `return 0`, just check if function is terminated with `return` in each case.

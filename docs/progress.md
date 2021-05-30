# Kithare Development Progress

###### There were/are quite a few bumps that result in the slow development. Be patient.

## Main route
- [X] **Lexical analyzer (Lexer)**: Analyzes the string of the source such as handling numerals, identifiers, strings, character escapes which are present in the string. Which at the end returns a list of (lexical) tokens. *(Initially finished on 13th February 2021)*
- [X] **Parser**: Analyzes the list of lexical tokens and apply it with the grammar of the language such as import/include statements, variable declarations, function declarations, class declarations, resolve PEMDAS on arithmetic expressions, etc. to generate an Abstract Syntax Tree (AST). *(Initially finished on 9th April 2021)*
- [ ] **Semantic analyzer (Semanticizer)**: Analyzes the AST of the main source file and solve what identifiers refer to which, which function overload is it calling, should it cast it into another type to be passed into a function, where's the module source file it is importing, when should a variable be destructed, etc. to generate an Intermediate Representation (IR) tree. *(Will hopefully be finished on July)*
- [ ] **C transpiler**: Translates per piece of the IR tree into a string of C code, which then could be compiled with a C compiler (MSVC, GCC, MinGW, Clang, etc.) and be executed.
- [ ] **Bytecode assembler**: Translates per piece of the IR tree into a list of instructions of the bytecode. Which would be executed by the bytecode interpreter.
- [ ] **Bytecode interpreter**: Executes/interprets each of the bytecode instructions.

## Other possible future routes
- Direct compilation to Web Assembly without the C transpiler through emscripten.
- Native compilation to host machine without the C transpiler.
- Compilation to Java bytecode. (unlikely lol)

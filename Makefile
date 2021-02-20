COMPILER_ARGS = g++ source/main.cpp\
    source/lexer/lex.cpp source/lexer/token.cpp\
    source/parser/parse.cpp\
    source/runtime/environment.cpp\
    source/utility/string.cpp source/utility/utf8.cpp\
    -I include -o bin/kcr

ifeq ($(OS),Windows_NT)
COMPILER_ARGS += -D UNICODE -D _UNICODE -municode
endif

output:
    $(COMPILER_ARGS)

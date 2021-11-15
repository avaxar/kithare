/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/token.h>


khToken khToken_copy(const khToken* token) {
    khTokenValue token_value = token->value;

    switch (token->type) {
        case khTokenType_IDENTIFIER:
            token_value.identifier = khArray_byte_copy(&token->value.identifier);
            break;

        case khTokenType_STRING:
            token_value.string = khArray_char_copy(&token->value.string);
            break;

        case khTokenType_BUFFER:
            token_value.buffer = khArray_byte_copy(&token->value.buffer);
            break;

        default:
            break;
    }

    return (khToken){.type = token->type, .value = token_value};
}

void khToken_delete(khToken* token) {
    switch (token->type) {
        case khTokenType_IDENTIFIER:
            khArray_byte_delete(&token->value.identifier);
            break;

        case khTokenType_STRING:
            khArray_char_delete(&token->value.string);
            break;

        case khTokenType_BUFFER:
            khArray_byte_delete(&token->value.buffer);
            break;

        default:
            break;
    }
}

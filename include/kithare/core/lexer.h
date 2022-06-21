/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license,
 *     and it is available as a repository at https://github.com/Kithare/Kithare
 * Copyright (C) 2022 Kithare Organization at https://www.kithare.de
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <kithare/core/error.h>
#include <kithare/core/token.h>
#include <kithare/lib/array.h>
#include <kithare/lib/string.h>


kharray(khToken) kh_lexicate(khstring* string);

khToken kh_lexToken(char32_t** cursor);
khToken kh_lexWord(char32_t** cursor);
khToken kh_lexNumber(char32_t** cursor);
khToken kh_lexSymbol(char32_t** cursor);

char32_t kh_lexChar(char32_t** cursor, bool with_quotes, bool is_byte);
khstring kh_lexString(char32_t** cursor, bool is_buffer);

__uint128_t kh_lexInt(char32_t** cursor, uint8_t base, size_t max_length, bool* had_overflowed);
double kh_lexFloat(char32_t** cursor, uint8_t base);


#ifdef __cplusplus
}
#endif

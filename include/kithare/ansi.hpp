/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#define KH_ANSI_RESET "\033[0m"

#define KH_ANSI_FG_BLACK "\033[30m"
#define KH_ANSI_FG_RED "\033[31m"
#define KH_ANSI_FG_GREEN "\033[32m"
#define KH_ANSI_FG_YELLOW "\033[33m"
#define KH_ANSI_FG_BLUE "\033[34m"
#define KH_ANSI_FG_MAGENTA "\033[35m"
#define KH_ANSI_FG_CYAN "\033[36m"
#define KH_ANSI_FG_WHITE "\033[37m"

#define KH_ANSI_BG_BLACK "\033[40m"
#define KH_ANSI_BG_RED "\033[41m"
#define KH_ANSI_BG_GREEN "\033[42m"
#define KH_ANSI_BG_YELLOW "\033[43m"
#define KH_ANSI_BG_BLUE "\033[44m"
#define KH_ANSI_BG_MAGENTA "\033[45m"
#define KH_ANSI_BG_CYAN "\033[46m"
#define KH_ANSI_BG_WHITE "\033[47m"

#define KH_ANSI_FG_RGB(R, G, B) "\033[38;2;" #R ";" #G ";" #B "m"
#define KH_ANSI_BG_RGB(R, G, B) "\033[48;2;" #R ";" #B ";" #B "m"

#define KH_ANSI_BOLD "\033[1m"
#define KH_ANSI_DIM "\033[2m"
#define KH_ANSI_ITALIC "\033[3m"
#define KH_ANSI_UNDERLINE "\033[4m"
#define KH_ANSI_BLINKING "\033[5m"
#define KH_ANSI_INVERSE "\033[7m"
#define KH_ANSI_INVISIBLE "\033[8m"
#define KH_ANSI_STRIKETHROUGH "\033[9m"

#define KH_ANSI_HIDE_CURSOR "\033[?25l"
#define KH_ANSI_SHOW_CURSOR "\033[?25h"
#define KH_ANSI_SAVE_CURSOR "\033[s"
#define KH_ANSI_RESTORE_CURSOR "\033[u"
#define KH_ANSI_MOVE_UP_CURSOR(X) "\033[" #X "A"
#define KH_ANSI_MOVE_DOWN_CURSOR(X) "\033[" #X "B"
#define KH_ANSI_MOVE_RIGHT_CURSOR(X) "\033[" #X "C"
#define KH_ANSI_MOVE_LEFT_CURSOR(X) "\033[" #X "D"

#define KH_ANSI_CLEAR_SCREEN "\033[2J"
#define KH_ANSI_CLEAR_LINE "\033[2K"

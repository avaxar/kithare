#pragma once


namespace kh {
    struct Instruction {
        enum {
            /* RUNTIME INSTRUCTIONS */
            CLEAR = 0x0000, POP,
            LOAD_CONST8, LOAD_CONST16, LOAD_CONST32, LOAD_CONST64,
            LOAD_BUFFER, LOAD_STR, LOAD_UTF8_STR
        } type;

        struct {
            union {
                uint8  b8;
                uint16 b16;
                uint32 b32;
                uint64 b64;

                uint64 size;
            };

            std::vector<uint8> buffer;
            kh::String string;
        } value;
    };
}

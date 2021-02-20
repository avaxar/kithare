#pragma once

#include "utility/std.hpp"
#include "utility/string.hpp"
#include "runtime/instruction.hpp"


namespace kh {
    struct Function {
        Function(const size_t function_id, const size_t function_allocation, const std::vector<kh::Instruction> function_instructions) :
            id(function_id), allocation(function_allocation), instructions(function_instructions) {}

        size_t id;
        size_t allocation;
        std::vector<kh::Instruction> instructions;
    };
}

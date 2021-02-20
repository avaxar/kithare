#pragma once

#include "utility/std.hpp"
#include "utility/string.hpp"
#include "runtime/function.hpp"


namespace kh {
    class Environment {
    private:
        std::vector<kh::Function> functions;

        size_t global_allocation, local_allocation;
    public:
        kh::String name;

        Environment(const std::vector<kh::Function>& functions);
        Environment(const kh::Environment& copy); 
        ~Environment();
    };
}

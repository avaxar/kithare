#pragma once

#include <string>


namespace kh {
    class Exception {
    public:
        virtual ~Exception() {}
        virtual std::u32string format() const {
            return U"";
        }
    };
}

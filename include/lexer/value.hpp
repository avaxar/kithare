#pragma once

#include "utility/std.hpp"
#include "utility/string.hpp"
#include "lexer/type.hpp"


namespace kh {
	struct TokenValue {
		union {
			kh::Operator operator_type;
			kh::Symbol symbol_type;

			uint64 integer;
			double floating;
			double imaginary;

			uint32 character;
		};

		kh::String identifier_name;
		kh::String string;
		std::vector<uint8> buffer;
	};
}

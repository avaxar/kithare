#pragma once

#include "utility/std.hpp"


namespace kh {
	typedef std::basic_string<uint32> String;

	kh::String toString(const std::string& str);
	kh::String toString(const std::wstring& wstr);

	std::string fromString(const kh::String& str);
	std::wstring fromStringW(const kh::String& str);

	std::ostream& operator<<(std::ostream& out_stream, const kh::String& string);
	std::wostream& operator<<(std::wostream& wout_stream, const kh::String& string);
}

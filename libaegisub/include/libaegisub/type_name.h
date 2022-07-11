// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include <string>

namespace agi {
// Get the name of a type as a string in C syntax
// Currently supports primitives, pointers, references, const, and function pointers
template<typename T> struct type_name;

#define AGI_DEFINE_TYPE_NAME(type) \
	template<> struct type_name<type> { static const char *name() { return #type; }}

AGI_DEFINE_TYPE_NAME(bool);
AGI_DEFINE_TYPE_NAME(char);
AGI_DEFINE_TYPE_NAME(float);
AGI_DEFINE_TYPE_NAME(double);
AGI_DEFINE_TYPE_NAME(int);
AGI_DEFINE_TYPE_NAME(long);
AGI_DEFINE_TYPE_NAME(long long);
AGI_DEFINE_TYPE_NAME(unsigned int);
AGI_DEFINE_TYPE_NAME(unsigned long);
AGI_DEFINE_TYPE_NAME(unsigned long long);
AGI_DEFINE_TYPE_NAME(void);

#undef AGI_TYPE_NAME_PRIMITIVE

#define AGI_TYPE_NAME_MODIFIER(pre, type) \
	template<typename T> \
	struct type_name<T type> { \
		static std::string name() { \
			return std::string(type_name<T>::name()) + pre #type; \
		} \
	}

AGI_TYPE_NAME_MODIFIER("", *);
AGI_TYPE_NAME_MODIFIER("", &);
AGI_TYPE_NAME_MODIFIER(" ", const);

#undef AGI_TYPE_NAME_MODIFIER

template<typename First>
std::string function_args(bool is_first) {
	return std::string(is_first ? "" : ", ") + type_name<First>::name() + ")";
}

template<typename First, typename Second, typename... Rest>
std::string function_args(bool is_first) {
	return std::string(is_first ? "" : ", ") + type_name<First>::name() + function_args<Second, Rest...>(false);
}

template<typename Return, typename... Args>
struct type_name<Return (*)(Args...)> {
	static std::string name() {
		return std::string(type_name<Return>::name()) + " (*)(" + function_args<Args...>(true);
	}
};

template<typename Return>
struct type_name<Return (*)()> {
	static std::string name() {
		return std::string(type_name<Return>::name()) + " (*)()";
	}
};
}

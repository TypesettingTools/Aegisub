// Copyright (c) 2005, Niels Martin Hansen
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

/// @file string_codec.cpp
/// @brief Encode and decode strings so they can safely be stored inside fields in SSA/ASS files
/// @ingroup utility
///

// Functions for inline string encoding.
// See header file for details.

#include "config.h"

#include "string_codec.h"

#include <boost/format.hpp>

std::string inline_string_encode(const std::string &input) {
	std::string output;
	output.reserve(input.size());
	auto format = boost::format("#%02X");
	for (char c : input) {
		if (c <= 0x1F || c == 0x23 || c == 0x2C || c == 0x3A || c == 0x7C)
			output += str(format % c);
		else
			output += c;
	}
	return output;
}

std::string inline_string_decode(const std::string &input) {
	std::string output;
	output.reserve(input.size());
	for (size_t i = 0; i < input.size(); ++i) {
		if (input[i] != '#' || i + 2 > input.size())
			output += input[i];
		else {
			output += (char)strtol(input.substr(i + 1, 2).c_str(), nullptr, 16);
			i += 2;
		}
	}
	return output;
}

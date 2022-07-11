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

#include "visitor.h"

#include <ostream>
#include <string>

namespace agi {

class JsonWriter final : json::ConstVisitor {
	std::ostream &ostr;
	std::string indent;

	JsonWriter(std::ostream &ostr) : ostr(ostr) { }

	void Visit(json::Array const& array) override;
	void Visit(bool boolean) override;
	void Visit(double number) override;
	void Visit(int64_t number) override;
	void Visit(json::Null const& null) override;
	void Visit(json::Object const& object) override;
	void Visit(std::string const& string) override;
	void Visit(json::UnknownElement const& unknown);

public:
	template <typename T>
	static void Write(T const& value, std::ostream& ostr) {
		JsonWriter(ostr).Visit(value);
		ostr.flush();
	}
};

}

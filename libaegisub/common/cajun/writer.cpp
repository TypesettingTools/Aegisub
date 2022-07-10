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

#include "libaegisub/cajun/writer.h"

#include <cmath>
#include <iomanip>

namespace agi {
void JsonWriter::Visit(json::Array const& array) {
	if(array.empty()) {
		ostr << "[]";
		return;
	}

	indent += '\t';
	ostr << "[\n";

	bool first = true;
	for(auto const& entry : array) {
		if(!first) ostr << ",\n";
		first = false;

		ostr << indent;
		Visit(entry);
	}

	indent.pop_back();
	ostr << '\n' << indent << ']';
}

void JsonWriter::Visit(json::Object const& object) {
	if(object.empty()) {
		ostr << "{}";
		return;
	}

	indent += '\t';
	ostr << "{\n";

	bool first = true;
	for(auto const& entry : object) {
		if(!first) ostr << ",\n";
		first = false;

		ostr << indent;
		Visit(entry.first);
		ostr << " : ";
		Visit(entry.second);
	}

	indent.pop_back();
	ostr << '\n' << indent << '}';
}

void JsonWriter::Visit(double d) {
	ostr << std::setprecision(20) << d;

	double unused;
	if(!std::modf(d, &unused)) ostr << ".0";
}

void JsonWriter::Visit(std::string const& str) {
	ostr << '"';

	for(auto c : str) {
		switch(c) {
			case '"': ostr << "\\\""; break;
			case '\\': ostr << "\\\\"; break;
			case '\b': ostr << "\\b"; break;
			case '\f': ostr << "\\f"; break;
			case '\n': ostr << "\\n"; break;
			case '\r': ostr << "\\r"; break;
			case '\t': ostr << "\\t"; break;
			default: ostr << c; break;
		}
	}

	ostr << '"';
}

void JsonWriter::Visit(int64_t i) {
	ostr << i;
}
void JsonWriter::Visit(bool b) {
	ostr << (b ? "true" : "false");
}
void JsonWriter::Visit(json::Null const&) {
	ostr << "null";
}
void JsonWriter::Visit(json::UnknownElement const& unknown) {
	unknown.Accept(*this);
}
} // namespace agi

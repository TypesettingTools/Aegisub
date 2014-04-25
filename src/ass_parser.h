// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <array>
#include <map>
#include <memory>

class AssAttachment;
class AssFile;

class AssParser {
	AssFile *target;
	int version;
	std::unique_ptr<AssAttachment> attach;
	void (AssParser::*state)(std::string const&);

	void ParseAttachmentLine(std::string const& data);
	void ParseEventLine(std::string const& data);
	void ParseStyleLine(std::string const& data);
	void ParseScriptInfoLine(std::string const& data);
	void ParseFontLine(std::string const& data);
	void ParseGraphicsLine(std::string const& data);
	void ParseExtradataLine(std::string const &data);
	void UnknownLine(std::string const&) { }
public:
	AssParser(AssFile *target, int version);
	~AssParser();

	void AddLine(std::string const& data);
};

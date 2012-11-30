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

#ifndef AGI_PRE
#include <array>
#include <map>
#include <memory>

#include <wx/string.h>
#endif

#include "ass_entry.h"

class AssAttachment;
class AssFile;

class AssParser {
	AssFile *target;
	int version;
	std::unique_ptr<AssAttachment> attach;
	void (AssParser::*state)(wxString const&);
	std::array<AssEntry*, ENTRY_GROUP_MAX> insertion_positions;

	void InsertLine(AssEntry *entry);

	void ParseAttachmentLine(wxString const& data);
	void ParseEventLine(wxString const& data);
	void ParseStyleLine(wxString const& data);
	void ParseScriptInfoLine(wxString const& data);
	void ParseFontLine(wxString const& data);
	void ParseGraphicsLine(wxString const& data);
	void UnknownLine(wxString const&) { }
public:
	AssParser(AssFile *target, int version);
	~AssParser();
	void AddLine(wxString const& data);
};
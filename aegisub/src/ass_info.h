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
//
// Aegisub Project http://www.aegisub.org/

#include "ass_entry.h"

class AssInfo : public AssEntry {
	wxString key;
	wxString value;

public:
	AssInfo(AssInfo const& o) : key(o.key), value(o.value) { }
	AssInfo(wxString const& line) : key(line.BeforeFirst(':').Trim()), value(line.AfterFirst(':').Trim(false)) { }
	AssInfo(wxString const& key, wxString const& value) : key(key), value(value) { }

	AssEntry *Clone() const override { return new AssInfo(*this); }
	AssEntryGroup Group() const override { return ENTRY_INFO; }
	const wxString GetEntryData() const override { return key + ": " + value; }
	wxString GetSSAText() const override { return key.Lower() == "scripttype: v4.00+" ? "ScriptType: v4.00" : GetEntryData(); }

	wxString Key() const { return key; }
	wxString Value() const { return value; }
	void SetValue(wxString const& new_value) { value = new_value; }
};

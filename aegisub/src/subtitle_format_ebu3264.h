// Copyright (c) 2011 Niels Martin Hansen <nielsm@aegisub.org>
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


// This implements support for the EBU tech 3264 (1991) subtitling data exchange format.
// Work on support for this format was sponsored by Bandai.


#pragma once

#include "subtitle_format.h"

class Ebu3264SubtitleFormat : public SubtitleFormat {
public:
	wxString GetName();
	wxArrayString GetWriteWildcards();
	bool CanWriteFile(wxString filename);
	void WriteFile(wxString filename,wxString encoding);
};

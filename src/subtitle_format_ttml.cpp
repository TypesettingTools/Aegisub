// Copyright (c) 2026, 伤感咩吖
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/


#include "subtitle_format_ttml.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "lyrics_import.h"
#include "options.h"
#include <wx/wfstream.h>

#include <libaegisub/ass/time.h>

#include <stdexcept>
#include <utility>

TTMLSubtitleFormat::TTMLSubtitleFormat()
: SubtitleFormat("TTML Timed Text")
{
}

std::vector<std::string> TTMLSubtitleFormat::GetReadWildcards() const {
	return {"ttml", "dfxp", "xml"};
}

void TTMLSubtitleFormat::ReadFile(AssFile *target, agi::fs::path const& filename, agi::vfr::Framerate const&, const char *) const {
	try {
		wxFileInputStream input(filename.wstring());
		auto rows = lyrics::ParseTTML(input);
		target->LoadDefault(false, OPT_GET("Subtitle Format/TTML/Default Style Catalog")->GetString());
		for (auto& row : rows) {
			auto diag = new AssDialogue;
			diag->Start = agi::Time(static_cast<int>(row.start_ms));
			diag->End = agi::Time(static_cast<int>(row.end_ms));
			diag->Text = std::move(row.text);
			target->Events.push_back(*diag);
		}
	}
	catch (std::invalid_argument const& error) {
		throw SubtitleFormatParseError(error.what());
	}
}

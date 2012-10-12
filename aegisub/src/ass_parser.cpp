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

#include "config.h"

#include "ass_parser.h"

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"

AssParser::AssParser(AssFile *target, int version)
: target(target)
, version(version)
, attach(0)
{
}

AssParser::~AssParser() {
}

void AssParser::AddLine(wxString const& data) {
	// Is this line an attachment filename?
	bool isFilename = data.StartsWith("fontname: ") || data.StartsWith("filename: ");

	// If there's an attachment in progress, deal with it first as an
	// attachment data line can appear to be other things
	if (attach.get()) {
		// Check if it's valid data
		bool validData = data.size() > 0 && data.size() <= 80;
		for (size_t i = 0; i < data.size(); ++i) {
			if (data[i] < 33 || data[i] >= 97) {
				validData = false;
				break;
			}
		}

		// Data is over, add attachment to the file
		if (!validData || isFilename) {
			attach->Finish();
			target->Line.push_back(attach.release());
		}
		else {
			// Insert data
			attach->AddData(data);

			// Done building
			if (data.Length() < 80) {
				attach->Finish();
				target->Line.push_back(attach.release());
				return;
			}
		}
	}

	if (data.empty()) return;

	// Section header
	if (data[0] == '[' && data.Last() == ']') {
		// Ugly hacks to allow intermixed v4 and v4+ style sections
		const wxString low = data.Lower();
		wxString header = data;
		if (low == "[v4 styles]") {
			header = "[V4+ Styles]";
			version = 0;
		}
		else if (low == "[v4+ styles]") {
			header = "[V4+ Styles]";
			version = 1;
		}

		target->Line.push_back(new AssEntry(header, header));
		return;
	}

	// If the first nonblank line isn't a header pretend it starts with [Script Info]
	if (target->Line.empty())
		target->Line.push_back(new AssEntry("[Script Info]", "[Script Info]"));

	wxString group = target->Line.back()->group;
	wxString lowGroup = group.Lower();

	// Attachment
	if (lowGroup == "[fonts]" || lowGroup == "[graphics]") {
		if (isFilename) {
			attach.reset(new AssAttachment(data.Mid(10), group));
		}
		return;
	}

	// Dialogue
	if (lowGroup == "[events]") {
		if (data.StartsWith("Dialogue:") || data.StartsWith("Comment:"))
			target->Line.push_back(new AssDialogue(data));
		else if (data.StartsWith("Format:"))
			target->Line.push_back(new AssEntry("Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text", group));
		return;
	}

	// Style
	if (lowGroup == "[v4+ styles]") {
		if (data.StartsWith("Style:"))
			target->Line.push_back(new AssStyle(data, version));
		else if (data.StartsWith("Format:"))
			target->Line.push_back(new AssEntry("Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding", group));
		return;
	}

	// Script info
	if (lowGroup == "[script info]") {
		// Comment
		if (data.StartsWith(";")) {
			// Skip stupid comments added by other programs
			// Of course, we'll add our own in place later... ;)
			return;
		}

		if (data.StartsWith("ScriptType:")) {
			wxString versionString = data.Mid(11).Trim(true).Trim(false).Lower();
			int trueVersion;
			if (versionString == "v4.00")
				trueVersion = 0;
			else if (versionString == "v4.00+")
				trueVersion = 1;
			else
				throw "Unknown SSA file format version";
			if (trueVersion != version) {
				wxLogMessage("Warning: File has the wrong extension.");
				version = trueVersion;
			}
		}

		target->Line.push_back(new AssEntry(data, group));
		return;
	}

	// Unrecognized group
	target->Line.push_back(new AssEntry(data, group));
}

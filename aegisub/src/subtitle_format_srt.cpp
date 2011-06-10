// Copyright (c) 2006, Rodrigo Braz Monteiro
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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "config.h"

#include <wx/regex.h>
#include "subtitle_format_srt.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "colorspace.h"


DEFINE_SIMPLE_EXCEPTION(SRTParseError, SubtitleFormatParseError, "subtitle_io/parse/srt")


class SrtTagParser {

	struct FontAttribs {
		wxString face;
		wxString size;
		wxString color;
	};

	enum {
		// leave 0 unused so indexing an unknown tag in the map won't clash 
		TAG_BOLD_OPEN = 1,
		TAG_BOLD_CLOSE,
		TAG_ITALICS_OPEN,
		TAG_ITALICS_CLOSE,
		TAG_UNDERLINE_OPEN,
		TAG_UNDERLINE_CLOSE,
		TAG_STRIKEOUT_OPEN,
		TAG_STRIKEOUT_CLOSE,
		TAG_FONT_OPEN,
		TAG_FONT_CLOSE,
	};

	wxRegEx tag_matcher;
	wxRegEx attrib_matcher;
	std::map<wxString,int> tag_name_cases;

public:
	SrtTagParser()
	: tag_matcher(L"^(.*?)<(/?b|/?i|/?u|/?s|/?font)(.*?)>(.*)$", wxRE_ICASE|wxRE_ADVANCED)
	, attrib_matcher(L"^[[:space:]](face|size|color)=('.*?'|\".*?\"|[^[:space:]]+)", wxRE_ICASE|wxRE_ADVANCED)
	{
		if (!tag_matcher.IsValid())
			throw Aegisub::InternalError(L"Parsing SRT: Failed compiling tag matching regex", 0);
		if (!attrib_matcher.IsValid())
			throw Aegisub::InternalError(L"Parsing SRT: Failed compiling tag attribute matching regex", 0);

		tag_name_cases[L"b"]  = TAG_BOLD_OPEN;
		tag_name_cases[L"/b"] = TAG_BOLD_CLOSE;
		tag_name_cases[L"i"]  = TAG_ITALICS_OPEN;
		tag_name_cases[L"/i"] = TAG_ITALICS_CLOSE;
		tag_name_cases[L"u"]  = TAG_UNDERLINE_OPEN;
		tag_name_cases[L"/u"] = TAG_UNDERLINE_CLOSE;
		tag_name_cases[L"s"]  = TAG_STRIKEOUT_OPEN;
		tag_name_cases[L"/s"] = TAG_STRIKEOUT_CLOSE;
		tag_name_cases[L"font"] = TAG_FONT_OPEN;
		tag_name_cases[L"/font"] = TAG_FONT_CLOSE;
	}

	wxString ToAss(wxString srt)
	{
		int bold_level = 0;
		int italics_level = 0;
		int underline_level = 0;
		int strikeout_level = 0;
		std::vector<FontAttribs> font_stack;

		wxString ass; // result to be built

		while (!srt.IsEmpty())
		{
			if (!tag_matcher.Matches(srt))
			{
				// no more tags could be matched, end of string
				ass.append(srt);
				break;
			}

			// we found a tag, translate it
			wxString pre_text  = tag_matcher.GetMatch(srt, 1);
			wxString tag_name  = tag_matcher.GetMatch(srt, 2);
			wxString tag_attrs = tag_matcher.GetMatch(srt, 3);
			wxString post_text = tag_matcher.GetMatch(srt, 4);

			// the text before the tag goes through unchanged
			ass.append(pre_text);
			// the text after the tag is the input for next iteration
			srt = post_text;

			switch (tag_name_cases[tag_name.Lower()])
			{
			case TAG_BOLD_OPEN:
				if (bold_level == 0)
					ass.append(L"{\\b1}");
				bold_level++;
				break;
			case TAG_BOLD_CLOSE:
				if (bold_level == 1)
					ass.append(L"{\\b}");
				if (bold_level > 0)
					bold_level--;
				break;
			case TAG_ITALICS_OPEN:
				if (italics_level == 0)
					ass.append(L"{\\i1}");
				italics_level++;
				break;
			case TAG_ITALICS_CLOSE:
				if (italics_level == 1)
					ass.append(L"{\\i}");
				if (italics_level > 0)
					italics_level--;
				break;
			case TAG_UNDERLINE_OPEN:
				if (underline_level == 0)
					ass.append(L"{\\u1}");
				underline_level++;
				break;
			case TAG_UNDERLINE_CLOSE:
				if (underline_level == 1)
					ass.append(L"{\\u}");
				if (underline_level > 0)
					underline_level--;
				break;
			case TAG_STRIKEOUT_OPEN:
				if (strikeout_level == 0)
					ass.append(L"{\\s1}");
				strikeout_level++;
				break;
			case TAG_STRIKEOUT_CLOSE:
				if (strikeout_level == 1)
					ass.append(L"{\\s}");
				if (strikeout_level > 0)
					strikeout_level--;
				break;
			case TAG_FONT_OPEN:
				{
					// new attributes to fill in
					FontAttribs new_attribs;
					FontAttribs old_attribs;
					// start out with any previous ones on stack
					if (font_stack.size() > 0)
					{
						old_attribs = font_stack.back();
						new_attribs = old_attribs;
					}
					// now find all attributes on this font tag
					while (attrib_matcher.Matches(tag_attrs))
					{
						// get attribute name and values
						wxString attr_name = attrib_matcher.GetMatch(tag_attrs, 1);
						wxString attr_value = attrib_matcher.GetMatch(tag_attrs, 2);
						// clean them
						attr_name.MakeLower();
						if ((attr_value.StartsWith(L"'") && attr_value.EndsWith(L"'")) ||
							(attr_value.StartsWith(L"\"") && attr_value.EndsWith(L"\"")))
						{
							attr_value = attr_value.Mid(1, attr_value.Len()-2);
						}
						// handle the attributes
						if (attr_name == L"face")
						{
							new_attribs.face = wxString::Format(L"{\\fn%s}", attr_value.c_str());
						}
						else if (attr_name == L"size")
						{
							new_attribs.size = wxString::Format(L"{\\fs%s}", attr_value.c_str());
						}
						else if (attr_name == L"color")
						{
							wxColour wxcl = html_to_color(attr_value);
							wxString colorstr = AssColor(wxcl).GetASSFormatted(false, false, false);
							new_attribs.color = wxString::Format(L"{\\c%s}", colorstr.c_str());
						}
						// remove this attribute to prepare for the next
						size_t attr_pos, attr_len;
						attrib_matcher.GetMatch(&attr_pos, &attr_len, 0);
						tag_attrs.erase(attr_pos, attr_len);
					}
					// the attributes changed from old are then written out
					if (new_attribs.face != old_attribs.face)
						ass.append(new_attribs.face);
					if (new_attribs.size != old_attribs.size)
						ass.append(new_attribs.size);
					if (new_attribs.color != old_attribs.color)
						ass.append(new_attribs.color);
					// lastly dump the new attributes state onto the stack
					font_stack.push_back(new_attribs);
				}
				break;
			case TAG_FONT_CLOSE:
				{
					// this requires a font stack entry
					if (font_stack.empty())
						break;
					// get the current attribs
					FontAttribs cur_attribs = font_stack.back();
					// remove them from the stack
					font_stack.pop_back();
					// if there are a previous attrib set, restore to those,
					// otherwise restore to defaults
					if (font_stack.size() > 0)
					{
						const FontAttribs &old_attribs = font_stack.back();
						if (cur_attribs.face != old_attribs.face)
							ass.append(old_attribs.face);
						if (cur_attribs.size != old_attribs.face)
							ass.append(old_attribs.size);
						if (cur_attribs.color != old_attribs.color)
							ass.append(old_attribs.color);
					}
					else
					{
						if (!cur_attribs.face.IsEmpty())
							ass.append(L"{\\fn}");
						if (!cur_attribs.size.IsEmpty())
							ass.append(L"{\\fs}");
						if (!cur_attribs.color.IsEmpty())
							ass.append(L"{\\c}");
					}
				}
				break;
			default:
				// unknown tag, replicate it in the output
				ass.append(L"<").append(tag_name).append(tag_attrs).append(L">");
				break;
			}
		}

		// make it a little prettier, join tag groups
		ass.Replace(L"}{", L"", true);

		return ass;
	}

};


/////////////
// Can read?
bool SRTSubtitleFormat::CanReadFile(wxString filename) {
	return (filename.Right(4).Lower() == _T(".srt"));
}


//////////////
// Can write?
bool SRTSubtitleFormat::CanWriteFile(wxString filename) {
	return (filename.Right(4).Lower() == _T(".srt"));
}


////////////
// Get name
wxString SRTSubtitleFormat::GetName() {
	return _T("SubRip");
}


//////////////////////
// Get read wildcards
wxArrayString SRTSubtitleFormat::GetReadWildcards() {
	wxArrayString formats;
	formats.Add(_T("srt"));
	return formats;
}


///////////////////////
// Get write wildcards
wxArrayString SRTSubtitleFormat::GetWriteWildcards() {
	return GetReadWildcards();
}


/////////////
// Read file
void SRTSubtitleFormat::ReadFile(wxString filename,wxString encoding) {
	using namespace std;

	// Reader
	TextFileReader file(filename,encoding);

	// Default
	LoadDefault(false);

	// See parsing algorithm at <http://devel.aegisub.org/wiki/SubtitleFormats/SRT>

	// "hh:mm:ss,fff --> hh:mm:ss,fff" (e.g. "00:00:04,070 --> 00:00:10,04")
	/// @todo: move the full parsing of SRT timestamps here, instead of having it in AssTime
	wxRegEx timestamp_regex(L"^([0-9]{2}:[0-9]{2}:[0-9]{2},[0-9]{3}) --> ([0-9]{2}:[0-9]{2}:[0-9]{2},[0-9]{3})");
	if (!timestamp_regex.IsValid())
		throw Aegisub::InternalError(L"Parsing SRT: Failed compiling regex", 0);

	SrtTagParser tag_parser;

	int state = 1;
	int line_num = 0;
	int linebreak_debt = 0;
	AssDialogue *line = 0;
	while (file.HasMoreLines()) {
		wxString text_line = file.ReadLineFromFile();
		line_num++;
		text_line.Trim(true).Trim(false);

		switch (state) {
			case 1:
			{
				// start of file, no subtitles found yet
				if (text_line.IsEmpty())
					// ignore blank lines
					break;
				if (text_line.IsNumber()) {
					// found the line number, throw it away and hope for timestamps
					state = 2;
					break;
				}
				if (timestamp_regex.Matches(text_line)) {
					goto found_timestamps;
				}
				throw SRTParseError(wxString::Format(L"Parsing SRT: Expected subtitle index at line %d", line_num), 0);
			}
			case 2:
			{
				// want timestamps
				if (timestamp_regex.Matches(text_line) == false) {
					// bad format
					throw SRTParseError(wxString::Format(L"Parsing SRT: Expected timestamp pair at line %d", line_num), 0);
				}
found_timestamps:
				if (line != 0) {
					// finalise active line
					line->Text = tag_parser.ToAss(line->Text);
					line = 0;
				}
				// create new subtitle
				line = new AssDialogue();
				line->group = L"[Events]";
				line->Style = _T("Default");
				line->Comment = false;
				// this parsing should best be moved out of AssTime
				line->Start.ParseSRT(timestamp_regex.GetMatch(text_line, 1));
				line->End.ParseSRT(timestamp_regex.GetMatch(text_line, 2));
				// store pointer to subtitle, we'll continue working on it
				line->FixStartMS();
				Line->push_back(line);
				// next we're reading the text
				state = 3;
				break;
			}
			case 3:
			{
				// reading first line of subtitle text
				if (text_line.IsEmpty()) {
					// that's not very interesting... blank subtitle?
					state = 5;
					// no previous line that needs a line break after
					linebreak_debt = 0;
					break;
				}
				line->Text.Append(text_line);
				state = 4;
				break;
			}
			case 4:
			{
				// reading following line of subtitle text
				if (text_line.IsEmpty()) {
					// blank line, next may begin a new subtitle
					state = 5;
					// previous line needs a line break after
					linebreak_debt = 1;
					break;
				}
				line->Text.Append(L"\\N").Append(text_line);
				break;
			}
			case 5:
			{
				// blank line in subtitle text
				linebreak_debt++;
				if (text_line.IsEmpty()) {
					// multiple blank lines in a row, just add a line break...
					break;
				}
				if (text_line.IsNumber()) {
					// must be a subtitle index!
					// go for timestamps next
					state = 2;
					break;
				}
				if (timestamp_regex.Matches(text_line)) {
					goto found_timestamps;
				}
				// assume it's a continuation of the subtitle text
				// resolve our line break debt and append the line text
				while (linebreak_debt-- > 0)
					line->Text.Append(L"\\N");
				line->Text.Append(text_line);
				state = 4;
				break;
			}
			default:
			{
				throw Aegisub::InternalError(wxString::Format(L"Parsing SRT: Reached unexpected state %d", state), 0);
			}
		}
	}

	if (state == 1 || state == 2) {
		throw SRTParseError(L"Parsing SRT: Incomplete file", 0);
	}

	if (line) {
		// an unfinalised line
		line->Text = tag_parser.ToAss(line->Text);
	}
}


//////////////
// Write file
void SRTSubtitleFormat::WriteFile(wxString _filename,wxString encoding) {
	// Open file
	TextFileWriter file(_filename,encoding);

	// Convert to SRT
	CreateCopy();
	SortLines();
	StripComments();
	// Tags must be converted in two passes
	// First ASS style overrides are converted to SRT but linebreaks are kept
	ConvertTags(2,_T("\\N"),false);
	// Then we can recombine overlaps, this requires ASS style linebreaks
	RecombineOverlaps();
	MergeIdentical();
	// And finally convert linebreaks
	ConvertTags(0,_T("\r\n"),false);
	// Otherwise unclosed overrides might affect lines they shouldn't, see bug #809 for example

	// Write lines
	int i=1;
	using std::list;
	for (list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();cur++) {
		AssDialogue *current = AssEntry::GetAsDialogue(*cur);
		if (current && !current->Comment) {
			// Write line
			file.WriteLineToFile(wxString::Format(_T("%i"),i));
			file.WriteLineToFile(current->Start.GetSRTFormated() + _T(" --> ") + current->End.GetSRTFormated());
			file.WriteLineToFile(current->Text);
			file.WriteLineToFile(_T(""));

			i++;
		}
	}

	// Clean up
	ClearCopy();
}

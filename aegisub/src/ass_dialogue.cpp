// Copyright (c) 2005, Rodrigo Braz Monteiro
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

/// @file ass_dialogue.cpp
/// @brief Class for dialogue lines in subtitles
/// @ingroup subs_storage

#include "config.h"

#include "ass_dialogue.h"
#include "compat.h"
#include "subtitle_format.h"
#include "utils.h"

#include <libaegisub/of_type_adaptor.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include <wx/regex.h>
#include <wx/tokenzr.h>

using namespace boost::adaptors;

std::size_t hash_value(wxString const& s) {
	return wxStringHash()(s);
}

AssDialogue::AssDialogue()
: Comment(false)
, Layer(0)
, Start(0)
, End(5000)
, Style("Default")
{
	memset(Margin, 0, sizeof Margin);
}

AssDialogue::AssDialogue(AssDialogue const& that)
: Comment(that.Comment)
, Layer(that.Layer)
, Start(that.Start)
, End(that.End)
, Style(that.Style)
, Actor(that.Actor)
, Effect(that.Effect)
, Text(that.Text)
{
	memmove(Margin, that.Margin, sizeof Margin);
}

AssDialogue::AssDialogue(wxString const& data)
: Comment(false)
, Layer(0)
, Start(0)
, End(5000)
, Style("Default")
{
	if (!Parse(data))
		throw SubtitleFormatParseError(from_wx("Failed parsing line: " + data), 0);
}

AssDialogue::~AssDialogue () {
}

bool AssDialogue::Parse(wxString const& rawData) {
	size_t pos = 0;
	wxString temp;

	// Get type
	if (rawData.StartsWith("Dialogue:")) {
		Comment = false;
		pos = 10;
	}
	else if (rawData.StartsWith("Comment:")) {
		Comment = true;
		pos = 9;
	}
	else return false;

	wxStringTokenizer tkn(rawData.Mid(pos),",",wxTOKEN_RET_EMPTY_ALL);
	if (!tkn.HasMoreTokens()) return false;

	// Get first token and see if it has "Marked=" in it
	temp = tkn.GetNextToken().Trim(false).Trim(true);
	bool ssa = temp.Lower().StartsWith("marked=");

	// Get layer number
	if (ssa)
		Layer = 0;
	else {
		long templ;
		temp.ToLong(&templ);
		Layer = templ;
	}

	// Get start time
	if (!tkn.HasMoreTokens()) return false;
	Start = tkn.GetNextToken();

	// Get end time
	if (!tkn.HasMoreTokens()) return false;
	End = tkn.GetNextToken();

	// Get style
	if (!tkn.HasMoreTokens()) return false;
	Style = tkn.GetNextToken().Trim(true).Trim(false);

	// Get actor
	if (!tkn.HasMoreTokens()) return false;
	Actor = tkn.GetNextToken().Trim(true).Trim(false);

	// Get margins
	for (int i = 0; i < 3; ++i) {
		if (!tkn.HasMoreTokens()) return false;
		SetMarginString(tkn.GetNextToken().Trim(false).Trim(true), i);
	}

	if (!tkn.HasMoreTokens()) return false;
	Effect = tkn.GetNextToken().Trim(true).Trim(false);

	// Get text
	Text = rawData.Mid(pos + tkn.GetPosition());

	return true;
}

wxString AssDialogue::GetData(bool ssa) const {
	wxString s = Style;
	wxString a = Actor;
	wxString e = Effect;
	s.Replace(",",";");
	a.Replace(",",";");
	e.Replace(",",";");

	wxString str = wxString::Format(
		"%s: %s,%s,%s,%s,%s,%d,%d,%d,%s,%s",
		Comment ? "Comment" : "Dialogue",
		ssa ? "Marked=0" : wxString::Format("%01d", Layer),
		Start.GetAssFormated(),
		End.GetAssFormated(),
		s, a,
		Margin[0], Margin[1], Margin[2],
		e,
		Text.get());

	// Make sure that final has no line breaks
	str.Replace("\n", "");
	str.Replace("\r", "");

	return str;
}

const wxString AssDialogue::GetEntryData() const {
	return GetData(false);
}

wxString AssDialogue::GetSSAText() const {
	return GetData(true);
}

std::auto_ptr<boost::ptr_vector<AssDialogueBlock>> AssDialogue::ParseTags() const {
	boost::ptr_vector<AssDialogueBlock> Blocks;

	// Empty line, make an empty block
	if (Text.get().empty()) {
		Blocks.push_back(new AssDialogueBlockPlain);
		return Blocks.release();
	}

	int drawingLevel = 0;
	std::string text(from_wx(Text.get()));

	for (size_t len = text.size(), cur = 0; cur < len; ) {
		// Overrides block
		if (text[cur] == '{') {
			size_t end = text.find('}', cur);

			// VSFilter requires that override blocks be closed, while libass
			// does not. We match VSFilter here.
			if (end == std::string::npos)
				goto plain;

			++cur;
			// Get contents of block
			std::string work = text.substr(cur, end - cur);
			cur = end + 1;

			if (work.size() && work.find('\\') == std::string::npos) {
				//We've found an override block with no backslashes
				//We're going to assume it's a comment and not consider it an override block
				Blocks.push_back(new AssDialogueBlockComment(work));
			}
			else {
				// Create block
				AssDialogueBlockOverride *block = new AssDialogueBlockOverride(work);
				block->ParseTags();
				Blocks.push_back(block);

				// Look for \p in block
				for (auto const& tag : block->Tags) {
					if (tag.Name == "\\p")
						drawingLevel = tag.Params[0].Get<int>(0);
				}
			}

			continue;
		}

		// Plain-text/drawing block
plain:
		std::string work;
		size_t end = text.find('{', cur + 1);
		if (end == std::string::npos) {
			work = text.substr(cur);
			cur = len;
		}
		else {
			work = text.substr(cur, end - cur);
			cur = end;
		}

		if (drawingLevel == 0)
			Blocks.push_back(new AssDialogueBlockPlain(work));
		else
			Blocks.push_back(new AssDialogueBlockDrawing(work, drawingLevel));
	}

	return Blocks.release();
}

void AssDialogue::StripTags() {
	Text = GetStrippedText();
}

static std::string get_text(AssDialogueBlock &d) { return d.GetText(); }
void AssDialogue::UpdateText(boost::ptr_vector<AssDialogueBlock>& blocks) {
	if (blocks.empty()) return;
	Text = to_wx(join(blocks | transformed(get_text), ""));
}

void AssDialogue::SetMarginString(wxString const& origvalue, int which) {
	if (which < 0 || which > 2) throw InvalidMarginIdError();

	// Make it numeric
	wxString strvalue = origvalue;
	if (!strvalue.IsNumber()) {
		strvalue.clear();
		for (size_t i = 0; i < origvalue.Length(); ++i) {
			if (origvalue.Mid(i, 1).IsNumber()) {
				strvalue += origvalue.Mid(i, 1);
			}
		}
	}

	// Get value
	long value = 0;
	strvalue.ToLong(&value);
	Margin[which] = mid<int>(0, value, 9999);
}

wxString AssDialogue::GetMarginString(int which) const {
	if (which < 0 || which > 2) throw InvalidMarginIdError();
	return wxString::Format("%d", Margin[which]);
}

bool AssDialogue::CollidesWith(const AssDialogue *target) const {
	if (!target) return false;
	return ((Start < target->Start) ? (target->Start < End) : (Start < target->End));
}

static std::string get_text_p(AssDialogueBlock *d) { return d->GetText(); }
wxString AssDialogue::GetStrippedText() const {
	wxString ret;
	boost::ptr_vector<AssDialogueBlock> blocks(ParseTags());
	return to_wx(join(blocks | agi::of_type<AssDialogueBlockPlain>() | transformed(get_text_p), ""));
}

AssEntry *AssDialogue::Clone() const {
	return new AssDialogue(*this);
}

void AssDialogueBlockDrawing::TransformCoords(int mx, int my, double x, double y) {
	// HACK: Implement a proper parser ffs!!
	// Could use Spline but it'd be slower and this seems to work fine
	bool is_x = true;
	std::string final;

	boost::char_separator<char> sep(" ");
	for (auto const& cur : boost::tokenizer<boost::char_separator<char>>(text, sep)) {
		if (std::all_of(begin(cur), end(cur), isdigit)) {
			int val = boost::lexical_cast<int>(cur);
			if (is_x)
				val = (int)((val + mx) * x + .5);
			else
				val = (int)((val + my) * y + .5);
			final += std::to_string(val);
			final += ' ';
		}
		else if (cur.size() == 1) {
			char c = tolower(cur[0]);
			if (c == 'm' || c == 'n' || c == 'l' || c == 'b' || c == 's' || c == 'p' || c == 'c') {
				is_x = true;
				final += c;
				final += ' ';
			}
		}
	}

	// Write back final
	final.pop_back();
	text = final;
}

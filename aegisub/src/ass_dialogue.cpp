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
#include "subtitle_format.h"
#include "utils.h"

#include <libaegisub/of_type_adaptor.h>
#include <libaegisub/split.h>
#include <libaegisub/util.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/karma_generate.hpp>
#include <boost/spirit/include/karma_int.hpp>

using namespace boost::adaptors;

static int next_id = 0;

AssDialogue::AssDialogue()
: Id(++next_id)
, Comment(false)
, Layer(0)
, Start(0)
, End(5000)
, Style("Default")
{
	memset(Margin, 0, sizeof Margin);
}

AssDialogue::AssDialogue(AssDialogue const& that)
: Id(++next_id)
, Comment(that.Comment)
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

AssDialogue::AssDialogue(std::string const& data)
: Id(++next_id)
{
	Parse(data);
}

AssDialogue::~AssDialogue () {
}

class tokenizer {
	agi::StringRange str;
	boost::split_iterator<agi::StringRange::const_iterator> pos;

public:
	tokenizer(agi::StringRange const& str) : str(str) , pos(agi::Split(str, ',')) { }

	agi::StringRange next_tok() {
		if (pos.eof())
			throw SubtitleFormatParseError("Failed parsing line: " + std::string(str.begin(), str.end()), nullptr);
		return *pos++;
	}

	std::string next_str() { return agi::str(next_tok()); }
	std::string next_str_trim() { return agi::str(boost::trim_copy(next_tok())); }
};

void AssDialogue::Parse(std::string const& raw) {
	agi::StringRange str;
	if (boost::starts_with(raw, "Dialogue:")) {
		Comment = false;
		str = agi::StringRange(raw.begin() + 10, raw.end());
	}
	else if (boost::starts_with(raw, "Comment:")) {
		Comment = true;
		str = agi::StringRange(raw.begin() + 9, raw.end());
	}
	else
		throw SubtitleFormatParseError("Failed parsing line: " + raw, nullptr);

	tokenizer tkn(str);

	// Get first token and see if it has "Marked=" in it
	auto tmp = tkn.next_str_trim();
	bool ssa = boost::istarts_with(tmp, "marked=");

	// Get layer number
	if (ssa)
		Layer = 0;
	else
		Layer = boost::lexical_cast<int>(tmp);

	Start = tkn.next_str_trim();
	End = tkn.next_str_trim();
	Style = tkn.next_str_trim();
	Actor = tkn.next_str_trim();
	for (int& margin : Margin)
		margin = mid(0, boost::lexical_cast<int>(tkn.next_str()), 9999);
	Effect = tkn.next_str_trim();
	Text = std::string(tkn.next_tok().begin(), str.end());
}

void append_int(std::string &str, int v) {
	boost::spirit::karma::generate(back_inserter(str), boost::spirit::karma::int_, v);
	str += ',';
}

void append_str(std::string &out, std::string const& str) {
	out += str;
	out += ',';
}

void append_unsafe_str(std::string &out, std::string const& str) {
	if (str.find(',') == str.npos)
		out += str;
	else
		out += boost::replace_all_copy(str, ",", ";");
	out += ',';
}

std::string AssDialogue::GetData(bool ssa) const {
	std::string str = Comment ? "Comment: " : "Dialogue: ";
	str.reserve(51 + Style.get().size() + Actor.get().size() + Effect.get().size() + Text.get().size());

	if (ssa)
		append_str(str, "Marked=0");
	else
		append_int(str, Layer);
	append_str(str, Start.GetAssFormated());
	append_str(str, End.GetAssFormated());
	append_unsafe_str(str, Style);
	append_unsafe_str(str, Actor);
	for (auto margin : Margin)
		append_int(str, margin);
	append_unsafe_str(str, Effect);
	str += Text.get();

	if (str.find('\n') != str.npos || str.find('\r') != str.npos) {
		boost::replace_all(str, "\n", "");
		boost::replace_all(str, "\r", "");
	}

	return str;
}

const std::string AssDialogue::GetEntryData() const {
	return GetData(false);
}

std::string AssDialogue::GetSSAText() const {
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
	std::string const& text(Text.get());

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
				auto block = new AssDialogueBlockOverride(work);
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
	Text = join(blocks | transformed(get_text), "");
}

bool AssDialogue::CollidesWith(const AssDialogue *target) const {
	if (!target) return false;
	return ((Start < target->Start) ? (target->Start < End) : (Start < target->End));
}

static std::string get_text_p(AssDialogueBlock *d) { return d->GetText(); }
std::string AssDialogue::GetStrippedText() const {
	boost::ptr_vector<AssDialogueBlock> blocks(ParseTags());
	return join(blocks | agi::of_type<AssDialogueBlockPlain>() | transformed(get_text_p), "");
}

AssEntry *AssDialogue::Clone() const {
	auto clone = new AssDialogue(*this);
	*const_cast<int *>(&clone->Id) = Id;
	return clone;
}

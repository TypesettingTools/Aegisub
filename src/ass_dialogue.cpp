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

#include "ass_dialogue.h"
#include "subtitle_format.h"
#include "utils.h"

#include <libaegisub/of_type_adaptor.h>
#include <libaegisub/split.h>
#include <libaegisub/string.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/spirit/include/karma_generate.hpp>
#include <boost/spirit/include/karma_int.hpp>
#include <string_view>

using namespace boost::adaptors;

static int next_id = 0;

AssDialogue::AssDialogue() {
	Id = ++next_id;
}

AssDialogue::AssDialogue(AssDialogue const& that)
: AssDialogueBase(that)
, AssEntryListHook(that)
{
	Id = ++next_id;
}

AssDialogue::AssDialogue(AssDialogueBase const& that) : AssDialogueBase(that) { }

AssDialogue::AssDialogue(std::string const& data) {
	Id = ++next_id;
	Parse(data);
}

AssDialogue::~AssDialogue () { }

class tokenizer {
	std::string_view str;
	agi::split_iterator<char> pos;

public:
	tokenizer(std::string_view str) : str(str), pos(agi::Split(str, ',')) { }

	std::string_view next_tok() {
		if (pos.eof())
			throw SubtitleFormatParseError(agi::Str("Failed parsing line: ", str));
		return *pos++;
	}

	std::string next_str() { return std::string(next_tok()); }
	std::string next_str_trim() { return std::string(agi::Trim(next_tok())); }
};

void AssDialogue::Parse(std::string const& raw) {
	std::string_view str = raw;
	if (boost::starts_with(raw, "Dialogue:")) {
		Comment = false;
		str.remove_prefix(10);
	}
	else if (boost::starts_with(raw, "Comment:")) {
		Comment = true;
		str.remove_prefix(9);
	}
	else
		throw SubtitleFormatParseError("Failed parsing line: " + raw);

	tokenizer tkn(str);

	// Get first token and see if it has "Marked=" in it
	auto tmp = tkn.next_str_trim();
	bool ssa = boost::istarts_with(tmp, "marked=");

	// Get layer number
	Layer = ssa ? 0 : boost::lexical_cast<int>(tmp);

	Start = agi::Trim(tkn.next_tok());
	End = agi::Trim(tkn.next_tok());
	Style = tkn.next_str_trim();
	Actor = tkn.next_str_trim();
	for (int& margin : Margin)
		margin = mid(-9999, boost::lexical_cast<int>(tkn.next_tok()), 99999);
	Effect = tkn.next_str_trim();

	std::string text{tkn.next_tok().begin(), str.end()};

	if (text.size() > 1 && text[0] == '{' && text[1] == '=') {
		static const boost::regex extradata_test("^\\{(=\\d+)+\\}");
		boost::match_results<std::string::iterator> rematch;
		if (boost::regex_search(text.begin(), text.end(), rematch, extradata_test)) {
			std::string extradata_str = rematch.str(0);
			text = rematch.suffix().str();

			static const boost::regex idmatcher("=(\\d+)");
			auto start = extradata_str.begin();
			auto end = extradata_str.end();
			std::vector<uint32_t> ids;
			while (boost::regex_search(start, end, rematch, idmatcher)) {
				auto id = boost::lexical_cast<uint32_t>(rematch.str(1));
				ids.push_back(id);
				start = rematch.suffix().first;
			}
			ExtradataIds = ids;
		}
	}

	Text = text;
}

static void append_int(std::string &str, int v) {
	boost::spirit::karma::generate(back_inserter(str), boost::spirit::karma::int_, v);
	str += ',';
}

static void append_str(std::string &out, std::string const& str) {
	out += str;
	out += ',';
}

static void append_unsafe_str(std::string &out, std::string const& str) {
	for (auto c : str) {
		if (c == ',')
			out += ';';
		else
			out += c;
	}
	out += ',';
}

std::string AssDialogue::GetEntryData() const {
	std::string str = Comment ? "Comment: " : "Dialogue: ";
	str.reserve(51 + Style.get().size() + Actor.get().size() + Effect.get().size() + Text.get().size());

	append_int(str, Layer);
	append_str(str, Start.GetAssFormatted());
	append_str(str, End.GetAssFormatted());
	append_unsafe_str(str, Style);
	append_unsafe_str(str, Actor);
	for (auto margin : Margin)
		append_int(str, margin);
	append_unsafe_str(str, Effect);

	if (ExtradataIds.get().size() > 0) {
		str += '{';
		for (auto id : ExtradataIds.get()) {
			str += '=';
			boost::spirit::karma::generate(back_inserter(str), boost::spirit::karma::int_, id);
		}
		str += '}';
	}

	for (auto c : Text.get()) {
		if (c != '\n' && c != '\r')
			str += c;
	}

	return str;
}

std::vector<std::unique_ptr<AssDialogueBlock>> AssDialogue::ParseTags() const {
	std::vector<std::unique_ptr<AssDialogueBlock>> Blocks;

	// Empty line, make an empty block
	if (Text.get().empty()) {
		Blocks.push_back(std::make_unique<AssDialogueBlockPlain>());
		return Blocks;
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
				Blocks.push_back(std::make_unique<AssDialogueBlockComment>(work));
			}
			else {
				// Create block
				auto block = std::make_unique<AssDialogueBlockOverride>(work);
				block->ParseTags();

				// Look for \p in block
				for (auto const& tag : block->Tags) {
					if (tag.Name == "\\p")
						drawingLevel = tag.Params[0].Get<int>(0);
				}
				Blocks.push_back(std::move(block));
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
			Blocks.push_back(std::make_unique<AssDialogueBlockPlain>(work));
		else
			Blocks.push_back(std::make_unique<AssDialogueBlockDrawing>(work, drawingLevel));
	}

	return Blocks;
}

void AssDialogue::StripTags() {
	Text = GetStrippedText();
}

static std::string get_text(std::unique_ptr<AssDialogueBlock> &d) { return d->GetText(); }
void AssDialogue::UpdateText(std::vector<std::unique_ptr<AssDialogueBlock>>& blocks) {
	if (blocks.empty()) return;
	Text = agi::Join("", blocks | transformed(get_text));
}

bool AssDialogue::CollidesWith(const AssDialogue *target) const {
	if (!target) return false;
	return ((Start < target->Start) ? (target->Start < End) : (Start < target->End));
}

static std::string get_text_p(AssDialogueBlock *d) { return d->GetText(); }
std::string AssDialogue::GetStrippedText() const {
	auto blocks = ParseTags();
	return agi::Join("", blocks | agi::of_type<AssDialogueBlockPlain>() | transformed(get_text_p));
}

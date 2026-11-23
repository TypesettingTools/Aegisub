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
// Aegisub Project http://www.aegisub.org/

#include "subtitle_format.h"

#include "compat.h"
#include "subtitle_format_ass.h"
#include "subtitle_format_ebu3264.h"
#include "subtitle_format_encore.h"
#include "subtitle_format_microdvd.h"
#include "subtitle_format_mkv.h"
#include "subtitle_format_srt.h"
#include "subtitle_format_ssa.h"
#include "subtitle_format_transtation.h"
#include "subtitle_format_ttxt.h"
#include "subtitle_format_txt.h"

#include <libaegisub/string.h>

#include <algorithm>
#include <memory>
#include <wx/intl.h>

namespace {
	std::vector<std::unique_ptr<SubtitleFormat>> formats;
}

void SubtitleFormat::LoadFormats() {
	if (formats.empty()) {
		formats.emplace_back(std::make_unique<AssSubtitleFormat>());
		formats.emplace_back(std::make_unique<Ebu3264SubtitleFormat>());
		formats.emplace_back(std::make_unique<EncoreSubtitleFormat>());
		formats.emplace_back(std::make_unique<MKVSubtitleFormat>());
		formats.emplace_back(std::make_unique<MicroDVDSubtitleFormat>());
		formats.emplace_back(std::make_unique<SRTSubtitleFormat>());
		formats.emplace_back(std::make_unique<SsaSubtitleFormat>());
		formats.emplace_back(std::make_unique<TTXTSubtitleFormat>());
		formats.emplace_back(std::make_unique<TXTSubtitleFormat>());
		formats.emplace_back(std::make_unique<TranStationSubtitleFormat>());
	}
}

template<class Cont, class Pred>
SubtitleFormat *find_or_throw(Cont &container, Pred pred) {
	auto it = find_if(container.begin(), container.end(), pred);
	if (it == container.end())
		throw UnknownSubtitleFormatError("Subtitle format for extension not found");
	return it->get();
}

const SubtitleFormat *SubtitleFormat::GetReader(agi::fs::path const& filename, const char *encoding) {
	LoadFormats();
	return find_or_throw(formats, [&](std::unique_ptr<SubtitleFormat> const& f) {
		return f->CanReadFile(filename, encoding);
	});
}

const SubtitleFormat *SubtitleFormat::GetWriter(agi::fs::path const& filename) {
	LoadFormats();
	return find_or_throw(formats, [&](std::unique_ptr<SubtitleFormat> const& f) {
		return f->CanWriteFile(filename);
	});
}

std::string SubtitleFormat::GetWildcards(int mode) {
	LoadFormats();

	std::vector<std::string> all;
	std::string final;

	for (auto const& format : formats) {
		auto cur = mode == 0 ? format->GetReadWildcards() : format->GetWriteWildcards();
		if (cur.empty()) continue;

		for (auto& str : cur) str.insert(0, "*.");
		all.insert(all.end(), begin(cur), end(cur));
		agi::AppendStr(final, "|", format->GetName(), " (");
		agi::AppendJoin(final, ",", cur);
		final += ")|";
		agi::AppendJoin(final, ";", cur);
	}

	return agi::Str(from_wx(_("All Supported Formats")), " (", agi::Join(",", all), ")|", agi::Join(";", all), final);
}

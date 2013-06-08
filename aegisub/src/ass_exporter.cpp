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

/// @file ass_exporter.cpp
/// @brief Overall set-up and management of export operations
/// @ingroup export
///

#include "config.h"

#include "ass_exporter.h"

#include "ass_export_filter.h"
#include "ass_file.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "subtitle_format.h"

#include <memory>
#include <wx/sizer.h>

AssExporter::AssExporter(agi::Context *c)
: c(c)
, is_default(true)
{
}

void AssExporter::DrawSettings(wxWindow *parent, wxSizer *target_sizer) {
	is_default = false;
	for (auto& filter : *AssExportFilterChain::GetFilterList()) {
		// Make sure to construct static box sizer first, so it won't overlap
		// the controls on wxMac.
		wxSizer *box = new wxStaticBoxSizer(wxVERTICAL, parent, to_wx(filter.GetName()));
		wxWindow *window = filter.GetConfigDialogWindow(parent, c);
		if (window) {
			box->Add(window, 0, wxEXPAND, 0);
			target_sizer->Add(box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
			target_sizer->Show(box, false);
			Sizers[filter.GetName()] = box;
		}
		else {
			delete box;
		}
	}
}

void AssExporter::AddFilter(std::string const& name) {
	auto filter = AssExportFilterChain::GetFilter(name);

	if (!filter) throw "Filter not found: " + name;

	filters.push_back(filter);
}

std::vector<std::string> AssExporter::GetAllFilterNames() const {
	std::vector<std::string> names;
	for (auto& filter : *AssExportFilterChain::GetFilterList())
		names.emplace_back(filter.GetName());
	return names;
}

void AssExporter::Export(agi::fs::path const& filename, std::string const& charset, wxWindow *export_dialog) {
	AssFile subs(*c->ass);

	for (auto filter : filters) {
		filter->LoadSettings(is_default, c);
		filter->ProcessSubs(&subs, export_dialog);
	}

	const SubtitleFormat *writer = SubtitleFormat::GetWriter(filename);
	if (!writer)
		throw "Unknown file type.";

	writer->WriteFile(&subs, filename, charset);
}

wxSizer *AssExporter::GetSettingsSizer(std::string const& name) {
	auto pos = Sizers.find(name);
	return pos == Sizers.end() ? nullptr : pos->second;
}

std::string const& AssExporter::GetDescription(std::string const& name) const {
	auto filter = AssExportFilterChain::GetFilter(name);
	if (filter)
		return filter->GetDescription();
	throw "Filter not found: " + name;
}

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
//
// $Id$

/// @file ass_exporter.cpp
/// @brief Overall set-up and management of export operations
/// @ingroup export
///

#include "config.h"

#include "ass_export_filter.h"
#include "ass_exporter.h"
#include "ass_file.h"
#include "include/aegisub/context.h"

#include <libaegisub/scoped_ptr.h>

#ifndef AGI_PRE
#include <algorithm>
#endif

static inline std::list<AssExportFilter*>::const_iterator filter_list_begin() {
	return AssExportFilterChain::GetFilterList()->begin();
}

static inline std::list<AssExportFilter*>::const_iterator filter_list_end() {
	return AssExportFilterChain::GetFilterList()->end();
}

AssExporter::AssExporter(agi::Context *c)
: c(c)
, is_default(true)
{
}

AssExporter::~AssExporter () {
}

void AssExporter::DrawSettings(wxWindow *parent, wxSizer *target_sizer) {
	is_default = false;
	for (filter_iterator cur = filter_list_begin(); cur != filter_list_end(); ++cur) {
		// Make sure to construct static box sizer first, so it won't overlap
		// the controls on wxMac.
		wxSizer *box = new wxStaticBoxSizer(wxVERTICAL, parent, (*cur)->GetName());
		wxWindow *window = (*cur)->GetConfigDialogWindow(parent, c);
		if (window) {
			box->Add(window, 0, wxEXPAND, 0);
			target_sizer->Add(box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
			target_sizer->Show(box, false);
			Sizers[(*cur)->GetName()] = box;
		}
		else {
			delete box;
		}
	}
}

void AssExporter::AddFilter(wxString const& name) {
	AssExportFilter *filter = AssExportFilterChain::GetFilter(name);

	if (!filter) throw wxString::Format("Filter not found: %s", name);

	filters.push_back(filter);
}

void AssExporter::AddAutoFilters() {
	for (filter_iterator it = filter_list_begin(); it != filter_list_end(); ++it) {
		if ((*it)->GetAutoApply())
			filters.push_back(*it);
	}
}

wxArrayString AssExporter::GetAllFilterNames() const {
	wxArrayString names;
	transform(filter_list_begin(), filter_list_end(),
		std::back_inserter(names), std::mem_fun(&AssExportFilter::GetName));
	return names;
}

AssFile *AssExporter::ExportTransform(wxWindow *export_dialog, bool copy) {
	AssFile *subs = copy ? new AssFile(*c->ass) : c->ass;

	for (filter_iterator cur = filters.begin(); cur != filters.end(); cur++) {
		(*cur)->LoadSettings(is_default, c);
		(*cur)->ProcessSubs(subs, export_dialog);
	}

	return subs;
}

void AssExporter::Export(wxString const& filename, wxString const& charset, wxWindow *export_dialog) {
	agi::scoped_ptr<AssFile> subs(ExportTransform(export_dialog, true));
	subs->Save(filename, false, false, charset);
}

wxSizer *AssExporter::GetSettingsSizer(wxString const& name) {
	std::map<wxString, wxSizer*>::iterator pos = Sizers.find(name);
	if (pos == Sizers.end()) return 0;
	return pos->second;
}

wxString const& AssExporter::GetDescription(wxString const& name) const {
	AssExportFilter *filter = AssExportFilterChain::GetFilter(name);
	if (filter)
		return filter->GetDescription();
	throw wxString::Format("Filter not found: %s", name);
}

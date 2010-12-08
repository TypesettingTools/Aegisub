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
#include "frame_main.h"

/// @brief Constructor 
/// @param subs 
///
AssExporter::AssExporter (AssFile *subs) {
	OriginalSubs = subs;
	IsDefault = true;
}

/// @brief Destructor 
///
AssExporter::~AssExporter () {
}

/// @brief Draw control settings 
/// @param parent 
/// @param AddTo  
///
void AssExporter::DrawSettings(wxWindow *parent,wxSizer *AddTo) {
	IsDefault = false;
	wxWindow *window;
	wxSizer *box;
	FilterList::iterator begin = AssExportFilterChain::GetFilterList()->begin();
	FilterList::iterator end = AssExportFilterChain::GetFilterList()->end();
	for (FilterList::iterator cur=begin;cur!=end;cur++) {
		// Make sure to construct static box sizer first, so it won't overlap
		// the controls on wxMac.
		box = new wxStaticBoxSizer(wxVERTICAL,parent,(*cur)->RegisterName);
		window = (*cur)->GetConfigDialogWindow(parent);
		if (window) {
			box->Add(window,0,wxEXPAND,0);
			AddTo->Add(box,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
			AddTo->Show(box,false);
			Sizers[(*cur)->RegisterName] = box;
		}
		else {
			delete box;
		}
	}
}

/// @brief Add filter to chain 
/// @param name 
///
void AssExporter::AddFilter(wxString name) {
	// Get filter
	AssExportFilter *filter = NULL;
	FilterList::iterator begin = AssExportFilterChain::GetFilterList()->begin();
	FilterList::iterator end = AssExportFilterChain::GetFilterList()->end();
	for (FilterList::iterator cur=begin;cur!=end;cur++) {
		if ((*cur)->RegisterName == name) {
			filter = *cur;
		}
	}
	
	// Check
	if (!filter) throw wxString::Format(_T("Filter not found: %s"), name.c_str());

	// Add to list
	Filters.push_back(filter);
}

/// @brief Adds all autoexporting filters to chain 
///
void AssExporter::AddAutoFilters() {
	FilterList::iterator begin = AssExportFilterChain::GetFilterList()->begin();
	FilterList::iterator end = AssExportFilterChain::GetFilterList()->end();
	for (FilterList::iterator cur=begin;cur!=end;cur++) {
		if ((*cur)->autoExporter) {
			Filters.push_back(*cur);
		}
	}
}

/// @brief Get name of all filters 
/// @return 
///
wxArrayString AssExporter::GetAllFilterNames() {
	wxArrayString names;
	FilterList::iterator begin = AssExportFilterChain::GetFilterList()->begin();
	FilterList::iterator end = AssExportFilterChain::GetFilterList()->end();
	for (FilterList::iterator cur=begin;cur!=end;cur++) {
		if (!(*cur)->hidden) names.Add((*cur)->RegisterName);
	}
	return names;
}

/// @brief Transform for export 
/// @param export_dialog 
/// @return 
///
AssFile *AssExporter::ExportTransform(wxWindow *export_dialog,bool copy) {
	// Copy
	AssFile *Subs = copy ? new AssFile(*OriginalSubs) : OriginalSubs;

	// Run filters
	for (FilterList::iterator cur=Filters.begin();cur!=Filters.end();cur++) {
		(*cur)->LoadSettings(IsDefault);
		(*cur)->ProcessSubs(Subs, export_dialog);
	}

	// Done
	return Subs;
}

/// @brief Export 
/// @param filename      
/// @param charset       
/// @param export_dialog 
///
void AssExporter::Export(wxString filename, wxString charset, wxWindow *export_dialog) {
	std::auto_ptr<AssFile> Subs(ExportTransform(export_dialog,true));
	Subs->Save(filename,false,false,charset);
}

/// @brief Get window associated with name 
/// @param name 
/// @return 
///
wxSizer *AssExporter::GetSettingsSizer(wxString name) {
	SizerMap::iterator pos = Sizers.find(name);
	if (pos == Sizers.end()) return NULL;
	else return pos->second;
}

/// @brief Get description of filter 
/// @param name 
///
wxString AssExporter::GetDescription(wxString name) {
	FilterList::iterator begin = AssExportFilterChain::GetFilterList()->begin();
	FilterList::iterator end = AssExportFilterChain::GetFilterList()->end();
	for (FilterList::iterator cur=begin;cur!=end;cur++) {
		if ((*cur)->RegisterName == name) {
			return (*cur)->GetDescription();
		}
	}
	throw wxString::Format(_T("Filter not found: %s"), name.c_str());
}

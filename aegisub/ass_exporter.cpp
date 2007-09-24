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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "ass_exporter.h"
#include "ass_export_filter.h"
#include "ass_file.h"
#include "frame_main.h"


///////////////
// Constructor
AssExporter::AssExporter (AssFile *subs) {
	OriginalSubs = subs;
	IsDefault = true;
}


//////////////
// Destructor
AssExporter::~AssExporter () {
}


/////////////////////////
// Draw control settings
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


///////////////////////
// Add filter to chain
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


///////////////////////////////////////////
// Adds all autoexporting filters to chain
void AssExporter::AddAutoFilters() {
	FilterList::iterator begin = AssExportFilterChain::GetFilterList()->begin();
	FilterList::iterator end = AssExportFilterChain::GetFilterList()->end();
	for (FilterList::iterator cur=begin;cur!=end;cur++) {
		if ((*cur)->autoExporter) {
			Filters.push_back(*cur);
		}
	}
}


///////////////////////////
// Get name of all filters
wxArrayString AssExporter::GetAllFilterNames() {
	wxArrayString names;
	FilterList::iterator begin = AssExportFilterChain::GetFilterList()->begin();
	FilterList::iterator end = AssExportFilterChain::GetFilterList()->end();
	for (FilterList::iterator cur=begin;cur!=end;cur++) {
		if (!(*cur)->hidden) names.Add((*cur)->RegisterName);
	}
	return names;
}


////////////////////////
// Transform for export
AssFile *AssExporter::ExportTransform(wxWindow *export_dialog) {
	// Copy
	AssFile *Subs = new AssFile(*OriginalSubs);

	// Run filters
	for (FilterList::iterator cur=Filters.begin();cur!=Filters.end();cur++) {
		(*cur)->LoadSettings(IsDefault);
		(*cur)->ProcessSubs(Subs, export_dialog);
	}

	// Done
	return Subs;
}


//////////
// Export
void AssExporter::Export(wxString filename, wxString charset, wxWindow *export_dialog) {
	// Get transformation
	AssFile *Subs = ExportTransform(export_dialog);

	// Save
	Subs->Save(filename,false,false,charset);
	delete Subs;
}


///////////////////////////////////
// Get window associated with name
wxSizer *AssExporter::GetSettingsSizer(wxString name) {
	SizerMap::iterator pos = Sizers.find(name);
	if (pos == Sizers.end()) return NULL;
	else return pos->second;
}


/////////////////////////////
// Get description of filter
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

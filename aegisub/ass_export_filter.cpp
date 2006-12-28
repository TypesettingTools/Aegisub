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
#include "ass_export_filter.h"
#include "ass_file.h"


///////////////
// Constructor
AssExportFilter::AssExportFilter() {
	autoExporter = false;
	initialized = false;
	FilterList *fil = AssExportFilterChain::GetUnpreparedFilterList();
	fil->push_back(this);
}


//////////////
// Destructor
AssExportFilter::~AssExportFilter() {
	try {
		Unregister();
	}
	catch (...) {
		// Ignore error
	}
}


////////////
// Register
void AssExportFilter::Register (wxString name,int priority) {
	// Check if it's registered
	if (RegisterName != _T("")) {
		throw wxString::Format(_T("Register export filter: filter with name \"%s\" is already registered."), name.c_str());
	}

	// Remove pipes from name
	name.Replace(_T("|"),_T(""));

	// Check if name exists
	FilterList::iterator begin = AssExportFilterChain::GetFilterList()->begin();
	FilterList::iterator end = AssExportFilterChain::GetFilterList()->end();
	for (FilterList::iterator cur=begin;cur!=end;cur++) {
		if ((*cur)->RegisterName == name) {
			throw wxString::Format(_T("Register export filter: name \"%s\" already exists."), name.c_str());
		}
	}

	// Set name
	RegisterName = name;
	Priority = priority;

	// Look for place to insert
	bool inserted = false;
	for (FilterList::iterator cur=begin;cur!=end;cur++) {
		if ((*cur)->Priority < Priority) {
			AssExportFilterChain::GetFilterList()->insert(cur,this);
			inserted = true;
			break;
		}
	}
	if (!inserted) AssExportFilterChain::GetFilterList()->push_back(this);
}


//////////////
// Unregister
void AssExportFilter::Unregister () {
	// Check if it's registered
	if (!IsRegistered()) throw wxString::Format(_T("Unregister export filter: name \"%s\" is not registered."), RegisterName.c_str());

	// Unregister
	RegisterName = _T("");
	AssExportFilterChain::GetFilterList()->remove(this);
}


/////////////////////////////
// Checks if it's registered
bool AssExportFilter::IsRegistered() {
	// Check name
	if (RegisterName.IsEmpty()) {
		return false;
	}

	// Check list
	bool found = false;
	FilterList::iterator begin = AssExportFilterChain::GetFilterList()->begin();
	FilterList::iterator end = AssExportFilterChain::GetFilterList()->end();
	for (FilterList::iterator cur=begin;cur!=end;cur++) {
		if ((*cur) == this) {
			found = true;
			break;
		}
	}
	return found;
}


/////////////
// Get sizer
wxWindow *AssExportFilter::GetConfigDialogWindow(wxWindow *parent) {
	return NULL;
}


////////////////////
// Config dialog OK
void AssExportFilter::LoadSettings(bool IsDefault) {
}


//////////////////////
// Description reader
const wxString& AssExportFilter::GetDescription() const {
	return description;
}


///////////////
// Static list
AssExportFilterChain *AssExportFilterChain::instance=NULL;


////////////
// Get list
FilterList *AssExportFilterChain::GetFilterList() {
	if (instance == NULL) instance = new AssExportFilterChain();
	return &(instance->Filters);
}


///////////////////////
// Get unprepared list
FilterList *AssExportFilterChain::GetUnpreparedFilterList() {
	if (instance == NULL) instance = new AssExportFilterChain();
	return &(instance->Unprepared);
}


///////////////////
// Prepare filters
void AssExportFilterChain::PrepareFilters() {
	for (FilterList::iterator cur=instance->Unprepared.begin();cur!=instance->Unprepared.end();cur++) {
		(*cur)->Init();
	}
	instance->Unprepared.clear();
}

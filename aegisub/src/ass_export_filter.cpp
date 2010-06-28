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

/// @file ass_export_filter.cpp
/// @brief Baseclass for and management of single export filters
/// @ingroup export
///

#include "config.h"

#include "ass_export_filter.h"
#include "ass_file.h"


/// @brief Constructor 
///
AssExportFilter::AssExportFilter() {
	hidden = false;
	autoExporter = false;
	initialized = false;
	FilterList *fil = AssExportFilterChain::GetUnpreparedFilterList();
	fil->push_back(this);
}



/// @brief Destructor 
///
AssExportFilter::~AssExportFilter() {
	try {
		Unregister();
	}
	catch (...) {
		// Ignore error
	}
}



/// @brief Register 
/// @param name     
/// @param priority 
///
void AssExportFilter::Register (wxString name,int priority) {
	// Check if it's registered
	//   Changed this to an assert, since this kind of error should really only happen during dev. -jfs
	//   (Actually the list of regged filters should rather be looped through and check that this object isn't in.)
	assert(RegisterName == _T(""));

	// Remove pipes from name
	name.Replace(_T("|"),_T(""));

	int filter_copy = 0;
	wxString tmpnam;
	if (filter_copy == 0) {
		tmpnam = name;
	} else {
try_new_name:
		tmpnam = wxString::Format(_T("%s (%d)"), name.c_str(), filter_copy);
	}

	// Check if name exists
	FilterList::iterator begin = AssExportFilterChain::GetFilterList()->begin();
	FilterList::iterator end = AssExportFilterChain::GetFilterList()->end();
	for (FilterList::iterator cur=begin;cur!=end;cur++) {
		if ((*cur)->RegisterName == tmpnam) {
			// Instead of just failing and making a big noise about it, let multiple filters share name, but append something to the later arrivals -jfs
			filter_copy++;
			goto try_new_name;
		}
	}

	// Set name
	RegisterName = tmpnam;
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



/// @brief Unregister 
///
void AssExportFilter::Unregister () {
	// Check if it's registered
	if (!IsRegistered()) throw wxString::Format(_T("Unregister export filter: name \"%s\" is not registered."), RegisterName.c_str());

	// Unregister
	RegisterName = _T("");
	AssExportFilterChain::GetFilterList()->remove(this);
}



/// @brief Checks if it's registered 
/// @return 
///
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



/// @brief Get sizer 
/// @param parent 
/// @return 
///
wxWindow *AssExportFilter::GetConfigDialogWindow(wxWindow *parent) {
	return NULL;
}



/// @brief Config dialog OK 
/// @param IsDefault 
///
void AssExportFilter::LoadSettings(bool IsDefault) {
}



/// @brief Description reader 
/// @return 
///
const wxString& AssExportFilter::GetDescription() const {
	return description;
}



/// DOCME
std::auto_ptr<AssExportFilterChain> AssExportFilterChain::instance;



/// @brief Get list 
/// @return 
///
FilterList *AssExportFilterChain::GetFilterList() {
	if (!instance.get()) instance.reset(new AssExportFilterChain());
	return &(instance->Filters);
}



/// @brief Get unprepared list 
/// @return 
///
FilterList *AssExportFilterChain::GetUnpreparedFilterList() {
	if (!instance.get()) instance.reset(new AssExportFilterChain());
	return &(instance->Unprepared);
}



/// @brief Prepare filters 
///
void AssExportFilterChain::PrepareFilters() {
	if (!instance.get()) instance.reset(new AssExportFilterChain());
	for (FilterList::iterator cur=instance->Unprepared.begin();cur!=instance->Unprepared.end();cur++) {
		(*cur)->Init();
	}
	instance->Unprepared.clear();
}

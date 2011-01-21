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

#ifndef AGI_PRE
#include <algorithm>
#endif

#include "ass_export_filter.h"
#include "ass_file.h"

AssExportFilter::AssExportFilter(wxString const& name, wxString const& description, int priority)
: name(name)
, priority(priority)
, autoExporter(false)
, hidden(false)
, description(description)
{
}

void AssExportFilterChain::Register(AssExportFilter *filter) {
	// Remove pipes from name
	filter->name.Replace("|", "");

	FilterList::iterator begin = GetFilterList()->begin();
	FilterList::iterator end = GetFilterList()->end();

	int filter_copy = 0;
	wxString tmpnam = filter->name;
	if (false) {
try_new_name:
		tmpnam = wxString::Format("%s (%d)", filter->name, filter_copy);
	}

	// Check if name exists
	for (FilterList::iterator cur=begin;cur!=end;cur++) {
		if ((*cur)->name == tmpnam) {
			// Instead of just failing and making a big noise about it, let multiple filters share name, but append something to the later arrivals -jfs
			filter_copy++;
			goto try_new_name;
		}
	}

	filter->name = tmpnam;

	// Look for place to insert
	while (begin != end && (*begin)->priority >= filter->priority) ++begin;
	GetFilterList()->insert(begin, filter);
}

void AssExportFilterChain::Unregister(AssExportFilter *filter) {
	if (find(GetFilterList()->begin(), GetFilterList()->end(), filter) == GetFilterList()->end())
		throw wxString::Format("Unregister export filter: name \"%s\" is not registered.", filter->name);

	GetFilterList()->remove(filter);
}

FilterList *AssExportFilterChain::GetFilterList() {
	static FilterList instance;
	return &instance;
}

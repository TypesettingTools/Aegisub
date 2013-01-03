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

/// @file ass_export_filter.cpp
/// @brief Baseclass for and management of single export filters
/// @ingroup export
///

#include "config.h"

#include <algorithm>

#include "ass_export_filter.h"
#include "utils.h"

AssExportFilter::AssExportFilter(wxString const& name, wxString const& description, int priority)
: name(name)
, priority(priority)
, description(description)
{
}

void AssExportFilterChain::Register(AssExportFilter *filter) {
	// Remove pipes from name
	filter->name.Replace("|", "");

	int filter_copy = 1;
	wxString name = filter->name;
	// Find a unique name
	while (GetFilter(name))
		name = wxString::Format("%s (%d)", filter->name, filter_copy++);

	filter->name = name;

	// Look for place to insert
	FilterList::iterator begin = filters()->begin();
	FilterList::iterator end = filters()->end();
	while (begin != end && (*begin)->priority >= filter->priority) ++begin;
	filters()->insert(begin, filter);
}

void AssExportFilterChain::Unregister(AssExportFilter *filter) {
	auto it = remove(begin(*filters()), end(*filters()), filter);
	if (it == end(*filters()))
		throw wxString::Format("Unregister export filter: name \"%s\" is not registered.", filter->name);

	filters()->pop_back();
}

FilterList *AssExportFilterChain::filters() {
	static FilterList instance;
	return &instance;
}

const FilterList *AssExportFilterChain::GetFilterList() {
	return filters();
}

void AssExportFilterChain::Clear() {
	while (filters()->size() > 0) {
		AssExportFilter *f = filters()->back();
		delete f;
		if (filters()->size() && filters()->back() == f)
			filters()->pop_back();
	}
}

AssExportFilter *AssExportFilterChain::GetFilter(wxString const& name) {
	for (auto filter : *filters()) {
		if (filter->name == name)
			return filter;
	}
	return 0;
}

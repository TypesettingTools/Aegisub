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

/// @file ass_export_filter.h
/// @see ass_export_filter.cpp
/// @ingroup export
///

#pragma once

#ifndef AGI_PRE
#include <list>
#include <memory>

#include <wx/string.h>
#include <wx/window.h>
#endif

class AssFile;
class AssExportFilter;
class DialogExport;
class AssExporter;

/// DOCME
typedef std::list<AssExportFilter*> FilterList;

/// DOCME
/// @class AssExportFilterChain
/// @brief DOCME
///
/// DOCME
class AssExportFilterChain {
	friend class AssExporter;

	/// The list of registered filters
	FilterList Filters;

	/// Get the singleton instance
	static FilterList *GetFilterList();
	AssExportFilterChain() { }
public:
	/// Register an export filter
	static void Register(AssExportFilter *filter);
	/// Unregister an export filter; must have been registered
	static void Unregister(AssExportFilter *filter);
};

/// DOCME
/// @class AssExportFilter
/// @brief DOCME
///
/// DOCME
class AssExportFilter {
	friend class AssExporter;
	friend class AssExportFilterChain;

	/// This filter's name
	wxString name;

	/// Higher priority = run earlier
	int priority;

protected:
	/// Should this filter be used when sending subtitles to the subtitle provider
	bool autoExporter;

	/// Should this filter be hidden from the user
	bool hidden;

	/// User-visible description of this filter
	wxString description;

public:
	AssExportFilter(wxString const& name, wxString const& description, int priority = 0);
	virtual ~AssExportFilter() { };

	const wxString& GetDescription() const { return description; }

	/// Process subtitles
	virtual void ProcessSubs(AssFile *subs, wxWindow *export_dialog=0)=0;
	/// Draw setup controls
	virtual wxWindow *GetConfigDialogWindow(wxWindow *parent) { return 0; }
	/// Config dialog is done - extract data now.
	virtual void LoadSettings(bool IsDefault) { }
};


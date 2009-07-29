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


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/string.h>
#include <wx/window.h>
#include <list>


//////////////
// Prototypes
class AssFile;
class AssExportFilter;
class DialogExport;
class AssExporter;


////////////
// Typedefs
typedef std::list<AssExportFilter*> FilterList;


//////////////////////////////////////
// Singleton for storing filter chain
class AssExportFilterChain {
	friend class AssExportFilter;
	friend class AssExporter;

private:
	FilterList Filters;
	FilterList Unprepared;

	static AssExportFilterChain *instance;
	static FilterList *GetFilterList();
	static FilterList *GetUnpreparedFilterList();

public:
	static void PrepareFilters();
};


////////////////////////////
// Base export filter class
class AssExportFilter {
	friend class AssExporter;
	friend class AssExportFilterChain;

private:
	wxString RegisterName;
	int Priority;

protected:
	bool autoExporter;
	bool hidden;
	bool initialized;
	wxString description;

	void Register(wxString name,int priority=0);				// Register the filter with specific name. Higher priority filters get the file to process first.
	void Unregister();											// Unregister the filter instance
	bool IsRegistered();										// Is this instance registered as a filter?
	virtual void Init()=0;										// Tell it to initialize itself

public:
	AssExportFilter();
	virtual ~AssExportFilter();

	const wxString& GetDescription() const;

	virtual void ProcessSubs(AssFile *subs, wxWindow *export_dialog=0)=0;					// Process subtitles - this function must be overriden.
	virtual wxWindow *GetConfigDialogWindow(wxWindow *parent);	// Draw setup controls - this function may optionally be overridden.
	virtual void LoadSettings(bool IsDefault);					// Config dialog is done - extract data now.
};


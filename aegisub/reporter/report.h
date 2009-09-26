// Copyright (c) 2009, Amar Takhar <verm@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @file report.h
/// @see report.cpp

#ifndef R_PRECOMP
#include <map>

#include <wx/xml/xml.h>
#include <wx/listctrl.h>
#endif

/// @class Report
/// @brief Report generator.
class Report {

public:
	Report();
	~Report() {};

	void Fill(wxString *text, wxListView *listView);
	wxString AsText();

private:
	/// Comparison callback for nameMap.
	struct lst_comp {
		bool operator() (const wxString &a, const wxString &b) { return a.Cmp(b) < 0; }
	};

	/// Map of internal XML elements to human readable names.
	typedef std::map<std::string, std::string, lst_comp> nameMap;

	/// element->human name pairs.
	typedef std::pair<std::string, std::string> nPair;

	/// Struct to hold generatex XML Report.
	struct XMLReport {
		wxXmlDocument *doc;		/// Parent document.
		wxXmlNode *report;		/// Root node.
		wxXmlNode *general;		/// General.
		wxXmlNode *aegisub;		/// Aegisub related..
		wxXmlNode *hardware;	/// Hardware.
		wxXmlNode *windows;		/// Windows specific.
		wxXmlNode *unixx;		/// Unix specific.
		wxXmlNode *osx;			/// OS X specific.
	};
	XMLReport ReportCreate();
	XMLReport doc;

	void Add(wxXmlNode *parent, wxString node, wxString text);
	const nameMap HumanNames();
	nameMap nMap;
	void ProcessNode(wxXmlNode *node, wxString *text, wxListView *listView);
	wxLocale *locale;
};

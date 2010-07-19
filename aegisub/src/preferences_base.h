// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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

/// @file preferences_base.h
/// @brief Base preferences dialogue classes
/// @see preferences_base.cpp
/// @ingroup configuration_ui

class OptionPage: public wxScrolled<wxPanel> {
public:
	enum Style {
		PAGE_DEFAULT    =   0x00000000,
		PAGE_SCROLL     =   0x00000001,
		PAGE_SUB        =   0x00000002
	};

	wxSizer *sizer;

	OptionPage(wxTreebook *book, wxString name, int style = PAGE_DEFAULT);
	~OptionPage();

	wxFlexGridSizer* PageSizer(wxString name);
	void CellSkip(wxFlexGridSizer *&flex);
	void OptionAdd(wxFlexGridSizer *&flex, const wxString &name, const char *opt_name, double min=0, double max=100, double inc=1);
	void OptionChoice(wxFlexGridSizer *&flex, const wxString &name, const wxArrayString &choices, const char *opt_name);
	void OptionBrowse(wxFlexGridSizer *&flex, const wxString &name, BrowseType browse_type, const char *opt_name);
};

#define CLASS_PAGE(name)            \
	class name: public OptionPage { \
	public:                         \
		name(wxTreebook *book);     \
	};

CLASS_PAGE(General)
CLASS_PAGE(Subtitles)
CLASS_PAGE(Audio)
CLASS_PAGE(Video)
CLASS_PAGE(Interface)
CLASS_PAGE(Interface_Colours)
CLASS_PAGE(Interface_Hotkeys)
CLASS_PAGE(Paths)
CLASS_PAGE(File_Associations)
CLASS_PAGE(Backup)
CLASS_PAGE(Automation)
CLASS_PAGE(Advanced)
CLASS_PAGE(Advanced_Interface)
CLASS_PAGE(Advanced_Audio)
CLASS_PAGE(Advanced_Video)

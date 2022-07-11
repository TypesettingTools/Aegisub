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

/// @file preferences_base.h
/// @brief Base preferences dialogue classes
/// @see preferences_base.cpp
/// @ingroup configuration_ui

#include <wx/panel.h>
#include <wx/scrolwin.h>

class Preferences;
class wxControl;
class wxFlexGridSizer;
class wxSizer;
class wxString;
class wxTreebook;

class OptionPage : public wxScrolled<wxPanel> {
	template<class T>
	void Add(wxSizer *sizer, wxString const& label, T *control);
public:
	enum Style {
		PAGE_DEFAULT    =   0x00000000,
		PAGE_SCROLL     =   0x00000001,
		PAGE_SUB        =   0x00000002
	};

	wxSizer *sizer;
	Preferences *parent;
	wxFlexGridSizer *PageSizer(wxString name);

	void CellSkip(wxFlexGridSizer *flex);
	wxControl *OptionAdd(wxFlexGridSizer *flex, const wxString &name, const char *opt_name, double min=0, double max=INT_MAX, double inc=1);
	void OptionChoice(wxFlexGridSizer *flex, const wxString &name, const wxArrayString &choices, const char *opt_name);
	void OptionBrowse(wxFlexGridSizer *flex, const wxString &name, const char *opt_name, wxControl *enabler = nullptr, bool do_enable = false);
	void OptionFont(wxSizer *sizer, std::string opt_prefix);

	/// Enable ctrl only when cbx is checked
	void EnableIfChecked(wxControl *cbx, wxControl *ctrl);
	/// Enable ctrl only when cbx is not checked
	void DisableIfChecked(wxControl *cbx, wxControl *ctrl);

	OptionPage(wxTreebook *book, Preferences *parent, wxString name, int style = PAGE_DEFAULT);
};

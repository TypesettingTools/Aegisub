// Copyright(c) 2005, Rodrigo Braz Monteiro
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
// CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

#include <memory>
#include <string>
#include <wx/dialog.h>

class AssStyle;
class AssStyleStorage;
class PersistLocation;
class SubtitlesPreview;
class wxArrayString;
class wxCheckBox;
class wxChildFocusEvent;
class wxComboBox;
class wxCommandEvent;
class wxRadioBox;
class wxSpinCtrl;
class wxTextCtrl;
class wxThreadEvent;
class wxWindow;
namespace agi { struct Context; struct Color; }
template<typename T> class ValueEvent;

class DialogStyleEditor final : public wxDialog {
	agi::Context *c;
	std::unique_ptr<PersistLocation> persist;

	/// If true, the style was just created and so the user should not be
	/// asked if they want to change any existing lines should they rename
	/// the style
	bool is_new = false;

	bool updating = false;

	/// The style currently being edited
	AssStyle *style;

	/// Copy of style passed to the subtitles preview to avoid making changes
	/// before Apply is clicked
	std::unique_ptr<AssStyle> work;

	/// The style storage style is in, if applicable
	AssStyleStorage *store;

	wxTextCtrl *StyleName;
	wxComboBox *FontName;
	wxCheckBox *BoxBold;
	wxCheckBox *BoxItalic;
	wxCheckBox *BoxUnderline;
	wxCheckBox *BoxStrikeout;
	wxSpinCtrl *margin[3];
	wxRadioBox *Alignment;
	wxComboBox *OutlineType;
	wxComboBox *Encoding;
	wxTextCtrl *PreviewText;
	SubtitlesPreview *SubsPreview;

	void SetBitmapColor(int n,wxColour color);
	int AlignToControl(int n);
	int ControlToAlign(int n);
	int BorderStyleToControl(int n);
	int ControlToBorderStyle(int n);
	void UpdateWorkStyle();

	void OnChildFocus(wxChildFocusEvent &event);
	void OnCommandPreviewUpdate(wxCommandEvent &event);

	void OnPreviewTextChange(wxCommandEvent &event);
	void OnPreviewColourChange(ValueEvent<agi::Color> &event);

	/// @brief Maybe apply changes and maybe close the dialog
	/// @param apply Should changes be applied?
	/// @param close Should the dialog be closed?
	void Apply(bool apply,bool close);
	/// @brief Sets color for one of the four color buttons
	void OnSetColor(ValueEvent<agi::Color>& evt);

public:
	DialogStyleEditor(wxWindow *parent, AssStyle *style, agi::Context *c, AssStyleStorage *store, std::string const& new_name, wxArrayString const& font_list);
	~DialogStyleEditor();

	std::string GetStyleName() const;
};

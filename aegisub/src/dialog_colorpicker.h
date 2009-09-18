// Copyright (c) 2005, Niels Martin Hansen
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

/// @file dialog_colorpicker.h
/// @see dialog_colorpicker.cpp
/// @ingroup tools_ui
///




#ifndef AGI_PRE
#include <vector>

#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/colour.h>
#include <wx/dialog.h>
#include <wx/spinctrl.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#endif


/// DOCME
/// @class ColorPickerSpectrum
/// @brief DOCME
///
/// DOCME
class ColorPickerSpectrum : public wxControl {
public:

	/// DOCME
	enum PickerDirection {

		/// DOCME
		HorzVert,

		/// DOCME
		Horz,

		/// DOCME
		Vert
	};
private:

	/// DOCME

	/// DOCME
	int x, y;

	/// DOCME
	wxBitmap *background;

	/// DOCME
	PickerDirection direction;

	void OnPaint(wxPaintEvent &evt);
	void OnMouse(wxMouseEvent &evt);

public:
	ColorPickerSpectrum(wxWindow *parent, wxWindowID id, wxBitmap *_background, int xx, int yy, PickerDirection _direction, wxSize _size);

	void GetXY(int &xx, int &yy);
	void SetXY(int xx, int yy);
	void SetBackground(wxBitmap *new_background);

	DECLARE_EVENT_TABLE()
};

DECLARE_EVENT_TYPE(wxSPECTRUM_CHANGE, -1)



/// DOCME
/// @class ColorPickerRecent
/// @brief DOCME
///
/// DOCME
class ColorPickerRecent : public wxControl {
private:

	/// DOCME

	/// DOCME
	int rows, cols;

	/// DOCME
	int cellsize;

	/// DOCME
	wxPoint internal_control_offset;
	

	/// DOCME
	std::vector<wxColour> colors;
	

	/// DOCME
	bool background_valid;

	/// DOCME
	wxBitmap background;

	void OnClick(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnSize(wxSizeEvent &evt);

public:
	ColorPickerRecent(wxWindow *parent, wxWindowID id, int _cols, int _rows, int _cellsize);

	void LoadFromString(const wxString &recent_string);
	wxString StoreToString();
	void AddColor(wxColour color);

	/// @brief DOCME
	/// @param n 
	/// @return 
	///
	wxColour GetColor(int n) { return colors.at(n); }

	DECLARE_EVENT_TABLE()
};

DECLARE_EVENT_TYPE(wxRECENT_SELECT, -1)



/// DOCME
/// @class ColorPickerScreenDropper
/// @brief DOCME
///
/// DOCME
class ColorPickerScreenDropper : public wxControl {
private:

	/// DOCME
	wxBitmap capture;

	/// DOCME

	/// DOCME
	int resx, resy;

	/// DOCME
	int magnification;

	/// DOCME
	bool integrated_dropper;

	void OnMouse(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);

public:
	ColorPickerScreenDropper(wxWindow *parent, wxWindowID id, int _resx, int _resy, int _magnification, bool _integrated_dropper);

	void DropFromScreenXY(int x, int y);

	DECLARE_EVENT_TABLE()
};

DECLARE_EVENT_TYPE(wxDROPPER_SELECT, -1)


wxColour GetColorFromUser(wxWindow *parent, wxColour original);


/// DOCME
/// @class DialogColorPicker
/// @brief DOCME
///
/// DOCME
class DialogColorPicker : public wxDialog {
private:

	/// DOCME
	wxColour cur_color;

	/// DOCME
	bool updating_controls;

	/// DOCME
	bool spectrum_dirty;


	/// DOCME
	ColorPickerSpectrum *spectrum;

	/// DOCME
	ColorPickerSpectrum *slider;

	/// DOCME
	wxChoice *colorspace_choice;

	/// DOCME
	static const int slider_width = 10; // width in pixels of the color slider control


	/// DOCME
	wxSpinCtrl *rgb_input[3];

	/// DOCME
	wxBitmap *rgb_spectrum[3];	// x/y spectrum bitmap where color "i" is excluded from

	/// DOCME
	wxBitmap *rgb_slider[3];	// z spectrum for color "i"


	/// DOCME
	wxSpinCtrl *hsl_input[3];

	/// DOCME
	wxBitmap *hsl_spectrum;		// h/s spectrum

	/// DOCME
	wxBitmap *hsl_slider;		// l spectrum


	/// DOCME
	wxSpinCtrl *hsv_input[3];

	/// DOCME
	wxBitmap *hsv_spectrum;		// s/v spectrum

	/// DOCME
	wxBitmap *hsv_slider;		// h spectrum


	/// DOCME
	wxBitmap eyedropper_bitmap;

	/// DOCME
	wxPoint eyedropper_grab_point;

	/// DOCME
	bool eyedropper_is_grabbed;


	/// DOCME
	wxTextCtrl *ass_input;		// ASS hex format input

	/// DOCME
	wxTextCtrl *html_input;		// HTML hex format input


	/// DOCME
	wxStaticBitmap *preview_box;

	/// DOCME
	wxBitmap preview_bitmap;

	/// DOCME
	ColorPickerRecent *recent_box;

	/// DOCME
	ColorPickerScreenDropper *screen_dropper;

	/// DOCME
	wxStaticBitmap *screen_dropper_icon;

	void UpdateFromRGB();			// Update all other controls as a result of modifying an RGB control
	void UpdateFromHSL();			// Update all other controls as a result of modifying an HSL control
	void UpdateFromHSV();			// Update all other controls as a result of modifying an HSV control
	void UpdateFromASS();			// Update all other controls as a result of modifying the ASS format control
	void UpdateFromHTML();			// Update all other controls as a result of modifying the HTML format control
	void UpdateSpectrumDisplay();	// Redraw the spectrum display

	wxBitmap *MakeGBSpectrum();
	wxBitmap *MakeRBSpectrum();
	wxBitmap *MakeRGSpectrum();
	wxBitmap *MakeHSSpectrum();
	wxBitmap *MakeSVSpectrum();

	void OnSpinRGB(wxSpinEvent &evt);
	void OnSpinHSL(wxSpinEvent &evt);
	void OnSpinHSV(wxSpinEvent &evt);
	void OnChangeRGB(wxCommandEvent &evt);
	void OnChangeHSL(wxCommandEvent &evt);
	void OnChangeHSV(wxCommandEvent &evt);
	void OnChangeASS(wxCommandEvent &evt);
	void OnChangeHTML(wxCommandEvent &evt);
	void OnChangeMode(wxCommandEvent &evt);
	void OnSpectrumChange(wxCommandEvent &evt);
	void OnSliderChange(wxCommandEvent &evt);
	void OnRecentSelect(wxCommandEvent &evt); // also handles dropper pick
	void OnRGBAdjust(wxCommandEvent &evt);
	void OnDropperMouse(wxMouseEvent &evt);


	/// DOCME

	/// DOCME
	static int lastx, lasty;

public:
	DialogColorPicker(wxWindow *parent, wxColour initial_color);
	~DialogColorPicker();

	void SetColor(wxColour new_color);
	wxColour GetColor();

	DECLARE_EVENT_TABLE()
};


enum {

	/// DOCME
	SELECTOR_SPECTRUM = 4000,

	/// DOCME
	SELECTOR_SLIDER,

	/// DOCME
	SELECTOR_MODE,

	/// DOCME
	SELECTOR_RGB_R,

	/// DOCME
	SELECTOR_RGB_G,

	/// DOCME
	SELECTOR_RGB_B,

	/// DOCME
	SELECTOR_HSL_H,

	/// DOCME
	SELECTOR_HSL_S,

	/// DOCME
	SELECTOR_HSL_L,

	/// DOCME
	SELECTOR_HSV_H,

	/// DOCME
	SELECTOR_HSV_S,

	/// DOCME
	SELECTOR_HSV_V,

	/// DOCME
	SELECTOR_ASS_INPUT,

	/// DOCME
	SELECTOR_HTML_INPUT,

	/// DOCME
	SELECTOR_RECENT,

	/// DOCME
	SELECTOR_DROPPER,

	/// DOCME
	SELECTOR_DROPPER_PICK,

	/// DOCME
	BUTTON_RGBADJUST
};

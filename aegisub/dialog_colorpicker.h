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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//

#ifndef DIALOG_COLORPICKER_H
#define DIALOG_COLORPICKER_H


#include <wx/dialog.h>
#include <wx/spinctrl.h>
#include <wx/colour.h>
#include <wx/bitmap.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/statbmp.h>
#include <vector>


class ColorPickerSpectrum : public wxControl {
public:
	enum PickerDirection {
		HorzVert,
		Horz,
		Vert
	};
private:
	int x, y;
	wxBitmap *background;
	PickerDirection direction;

	void OnPaint(wxPaintEvent &evt);
	void OnMouse(wxMouseEvent &evt);

public:
	ColorPickerSpectrum(wxWindow *parent, wxWindowID id, wxBitmap *_background, int xx, int yy, PickerDirection _direction);

	void GetXY(int &xx, int &yy);
	void SetXY(int xx, int yy);
	void SetBackground(wxBitmap *new_background);

	DECLARE_EVENT_TABLE()
};

DECLARE_EVENT_TYPE(wxSPECTRUM_CHANGE, -1)


class ColorPickerRecent : public wxControl {
private:
	int rows, cols;
	int cellsize;
	wxPoint internal_control_offset;
	std::vector<wxColour> colors;

	void OnClick(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnSize(wxSizeEvent &evt);

public:
	ColorPickerRecent(wxWindow *parent, wxWindowID id, int _cols, int _rows, int _cellsize);

	void LoadFromString(const wxString &recent_string);
	wxString StoreToString();
	void AddColor(wxColour color);

	DECLARE_EVENT_TABLE()
};

DECLARE_EVENT_TYPE(wxRECENT_SELECT, -1)


class ColorPickerScreenDropper : public wxControl {
private:
	wxBitmap capture;
	int resx, resy;
	int magnification;
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

class DialogColorPicker : public wxDialog {
private:
	wxColour cur_color;
	bool updating_controls;
	bool spectrum_dirty;

	ColorPickerSpectrum *spectrum;
	ColorPickerSpectrum *slider;
	wxChoice *colorspace_choice;
	static const int slider_width = 10; // width in pixels of the color slider control

	// 0 = red, 1 = green, 2 = blue
	wxSpinCtrl *rgb_input[3];
	wxBitmap *rgb_spectrum[3];	// x/y spectrum bitmap where color "i" is excluded from
	wxBitmap *rgb_slider[3];	// z spectrum for color "i"

	// 0 = hue, 1 = saturation, 2 = luminance
	wxSpinCtrl *hsl_input[3];
	wxBitmap *hsl_spectrum;		// h/s spectrum
	wxBitmap *hsl_slider;		// l spectrum

	// 0 = hue, 1 = saturation, 2 = value
	wxSpinCtrl *hsv_input[3];
	wxBitmap *hsv_spectrum;		// s/v spectrum
	wxBitmap *hsv_slider;		// h spectrum

	wxBitmap eyedropper_bitmap;
	wxPoint eyedropper_grab_point;
	bool eyedropper_is_grabbed;

	wxTextCtrl *ass_input;		// ASS hex format input
	wxTextCtrl *html_input;		// HTML hex format input

	//wxWindow *preview_box;
	wxStaticBitmap *preview_box;
	wxBitmap preview_bitmap;
	ColorPickerRecent *recent_box;
	ColorPickerScreenDropper *screen_dropper;
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
	void OnDropperMouse(wxMouseEvent &evt);

	static int lastx, lasty;

public:
	DialogColorPicker(wxWindow *parent, wxColour initial_color);
	~DialogColorPicker();

	void SetColor(wxColour new_color);
	wxColour GetColor();

	DECLARE_EVENT_TABLE()
};


enum {
	SELECTOR_SPECTRUM = 4000,
	SELECTOR_SLIDER,
	SELECTOR_MODE,
	SELECTOR_RGB_R,
	SELECTOR_RGB_G,
	SELECTOR_RGB_B,
	SELECTOR_HSL_H,
	SELECTOR_HSL_S,
	SELECTOR_HSL_L,
	SELECTOR_HSV_H,
	SELECTOR_HSV_S,
	SELECTOR_HSV_V,
	SELECTOR_ASS_INPUT,
	SELECTOR_HTML_INPUT,
	SELECTOR_RECENT,
	SELECTOR_DROPPER,
	SELECTOR_DROPPER_PICK,
};


#endif

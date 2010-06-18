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

/// @file dialog_colorpicker.cpp
/// @brief Custom colour-selection dialogue box
/// @ingroup tools_ui
///

#include "config.h"

#ifndef AGI_PRE
#include <stdio.h>
#include <vector>

#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/dcscreen.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/gbsizer.h>
#include <wx/image.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbmp.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tokenzr.h>
#endif

#include "ass_style.h"
#include "colorspace.h"
#include "compat.h"
#include "dialog_colorpicker.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "options.h"
#include "utils.h"

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
	void SetBackground(wxBitmap *new_background, bool force = false);

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
	void OnMouse(wxMouseEvent &evt);

	/// DOCME

	/// DOCME
	static int lastx, lasty;

	ColorCallback callback;
	void *callbackUserdata;

public:
	DialogColorPicker(wxWindow *parent, wxColour initial_color, ColorCallback callback = NULL, void *userdata = NULL);
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

#ifdef WIN32

/// DOCME
#define STATIC_BORDER_FLAG wxSTATIC_BORDER
#else

/// DOCME
#define STATIC_BORDER_FLAG wxSIMPLE_BORDER
#endif

/// DOCME
static const int spectrum_horz_vert_arrow_size = 4;

/// @brief DOCME
/// @param parent      
/// @param id          
/// @param _background 
/// @param xx          
/// @param yy          
/// @param _direction  
/// @param _size       
///
ColorPickerSpectrum::ColorPickerSpectrum(wxWindow *parent, wxWindowID id, wxBitmap *_background, int xx, int yy, PickerDirection _direction, wxSize _size)
: wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE), x(xx), y(yy), background(_background), direction(_direction)
{
	_size.x += 2;
	_size.y += 2;

	if (direction == Vert) _size.x += spectrum_horz_vert_arrow_size + 1;
	if (direction == Horz) _size.y += spectrum_horz_vert_arrow_size + 1;

	SetClientSize(_size);
	SetMinSize(GetSize());
}

/// @brief DOCME
/// @param xx 
/// @param yy 
///
void ColorPickerSpectrum::GetXY(int &xx, int &yy)
{
	xx = x;
	yy = y;
}

/// @brief DOCME
/// @param xx 
/// @param yy 
///
void ColorPickerSpectrum::SetXY(int xx, int yy)
{
	if (x != xx || y != yy) {
		x = xx;
		y = yy;
		Refresh(false);
	}
}

/// @brief Set the background image for this spectrum
/// @param new_background New background image
/// @param force Repaint even if it appears to be the same image
void ColorPickerSpectrum::SetBackground(wxBitmap *new_background, bool force)
{
	if (background == new_background && !force) return;
	background = new_background;
	Refresh(false);
}

BEGIN_EVENT_TABLE(ColorPickerSpectrum, wxControl)
	EVT_PAINT(ColorPickerSpectrum::OnPaint)
	EVT_MOUSE_EVENTS(ColorPickerSpectrum::OnMouse)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(wxSPECTRUM_CHANGE)

/// @brief DOCME
/// @param evt 
/// @return 
///
void ColorPickerSpectrum::OnPaint(wxPaintEvent &evt)
{
	if (!background) return;

	int height = background->GetHeight();
	int width = background->GetWidth();
	wxPaintDC dc(this);

	wxMemoryDC memdc;
	memdc.SelectObject(*background);
	dc.Blit(1, 1, width, height, &memdc, 0, 0);

	wxPoint arrow[3];
	wxRect arrow_box;

	wxPen invpen(*wxWHITE, 3);
	invpen.SetCap(wxCAP_BUTT);
	dc.SetLogicalFunction(wxXOR);
	dc.SetPen(invpen);

	switch (direction) {
		case HorzVert:
			// Make a little cross
			dc.DrawLine(x-4, y+1, x+7, y+1);
			dc.DrawLine(x+1, y-4, x+1, y+7);
			break;
		case Horz:
			// Make a vertical line stretching all the way across
			dc.DrawLine(x+1, 1, x+1, height+1);
			// Points for arrow
			arrow[0] = wxPoint(x+1, height+2);
			arrow[1] = wxPoint(x+1-spectrum_horz_vert_arrow_size, height+2+spectrum_horz_vert_arrow_size);
			arrow[2] = wxPoint(x+1+spectrum_horz_vert_arrow_size, height+2+spectrum_horz_vert_arrow_size);

			arrow_box.SetLeft(0);
			arrow_box.SetTop(height + 2);
			arrow_box.SetRight(width + 1 + spectrum_horz_vert_arrow_size);
			arrow_box.SetBottom(height + 2 + spectrum_horz_vert_arrow_size);
			break;
		case Vert:
			// Make a horizontal line stretching all the way across
			dc.DrawLine(1, y+1, width+1, y+1);
			// Points for arrow
			arrow[0] = wxPoint(width+2, y+1);
			arrow[1] = wxPoint(width+2+spectrum_horz_vert_arrow_size, y+1-spectrum_horz_vert_arrow_size);
			arrow[2] = wxPoint(width+2+spectrum_horz_vert_arrow_size, y+1+spectrum_horz_vert_arrow_size);

			arrow_box.SetLeft(width + 2);
			arrow_box.SetTop(0);
			arrow_box.SetRight(width + 2 + spectrum_horz_vert_arrow_size);
			arrow_box.SetBottom(height + 1 + spectrum_horz_vert_arrow_size);
			break;
	}

	if (direction == Horz || direction == Vert) {
		wxBrush bgBrush;
		bgBrush.SetColour(GetBackgroundColour());
		dc.SetLogicalFunction(wxCOPY);
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(bgBrush);
		dc.DrawRectangle(arrow_box);

		// Arrow pointing at current point
		dc.SetBrush(*wxBLACK_BRUSH);
		dc.DrawPolygon(3, arrow);
	}

	// Border around the spectrum
	wxPen blkpen(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT), 1);
	blkpen.SetCap(wxCAP_BUTT);

	dc.SetLogicalFunction(wxCOPY);
	dc.SetPen(blkpen);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawRectangle(0, 0, background->GetWidth()+2, background->GetHeight()+2);
}

/// @brief DOCME
/// @param evt 
/// @return 
///
void ColorPickerSpectrum::OnMouse(wxMouseEvent &evt)
{
	evt.Skip();

	if (!evt.IsButton() && !evt.Dragging()) {
		return;
	}

	int newx = evt.GetX();
	if (newx < 0) newx = 0;
	if (newx >= GetClientSize().x) newx = GetClientSize().x-1;
	int newy = evt.GetY();
	if (newy < 0) newy = 0;
	if (newy >= GetClientSize().y) newy = GetClientSize().y-1;

	if (evt.LeftDown()) {
		CaptureMouse();
		SetCursor(wxCursor(wxCURSOR_BLANK));
	} else if (evt.LeftUp() && HasCapture()) {
		ReleaseMouse();
		SetCursor(wxNullCursor);
	}

	if (evt.LeftDown() || (HasCapture() && evt.LeftIsDown())) {
		SetXY(newx, newy);
		wxCommandEvent evt2(wxSPECTRUM_CHANGE, GetId());
		AddPendingEvent(evt2);
	}
}

/// @brief DOCME
/// @param parent    
/// @param id        
/// @param _cols     
/// @param _rows     
/// @param _cellsize 
///
ColorPickerRecent::ColorPickerRecent(wxWindow *parent, wxWindowID id, int _cols, int _rows, int _cellsize)
: wxControl(parent, id, wxDefaultPosition, wxDefaultSize, STATIC_BORDER_FLAG)
, rows(_rows)
, cols(_cols)
, cellsize(_cellsize)
, internal_control_offset(0,0)
, background_valid(false)
, background()
{
	LoadFromString(wxEmptyString);
	SetClientSize(cols*cellsize, rows*cellsize);
	SetMinSize(GetSize());
	SetMaxSize(GetSize());
	SetCursor(*wxCROSS_CURSOR);
}

/// @brief DOCME
/// @param recent_string 
///
void ColorPickerRecent::LoadFromString(const wxString &recent_string)
{
	colors.clear();
	wxStringTokenizer toker(recent_string, _T(" "), false);
	while (toker.HasMoreTokens()) {
		AssColor color;
		color.Parse(toker.NextToken());
		color.a = wxALPHA_OPAQUE;
		colors.push_back(color.GetWXColor());
	}
	while ((int)colors.size() < rows*cols) {
		colors.push_back(*wxBLACK);
	}
	
	background_valid = false;
}

/// @brief DOCME
/// @return 
///
wxString ColorPickerRecent::StoreToString()
{
	wxString res;
	for (int i = 0; i < rows*cols; i++) {
		AssColor color(colors[i]);
		res << color.GetASSFormatted(false, false, false) << _T(" ");
	}
	res.Trim(true);
	return res;
}

/// @brief DOCME
/// @param color 
///
void ColorPickerRecent::AddColor(wxColour color)
{
	for (std::vector<wxColor>::iterator i = colors.begin(); i != colors.end(); ++i) {
		if (color == *i) {
			colors.erase(i);
			break;
		}
	}
	
	colors.insert(colors.begin(), color);
	
	background_valid = false;
	
	Refresh(false);
}

BEGIN_EVENT_TABLE(ColorPickerRecent, wxControl)
	EVT_PAINT(ColorPickerRecent::OnPaint)
	EVT_LEFT_DOWN(ColorPickerRecent::OnClick)
	EVT_SIZE(ColorPickerRecent::OnSize)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(wxRECENT_SELECT)

/// @brief DOCME
/// @param evt 
/// @return 
///
void ColorPickerRecent::OnClick(wxMouseEvent &evt)
{
	int cx, cy, i;
	wxSize cs = GetClientSize();
	cx = (evt.GetX() - internal_control_offset.x) * cols / cs.x;
	cy = (evt.GetY() - internal_control_offset.y) * rows / cs.y;
	if (cx < 0 || cx > cols || cy < 0 || cy > rows) return;
	i = cols*cy + cx;
	if (i >= 0 && i < (int)colors.size()) {
		AssColor color(colors[i]);
		wxCommandEvent evnt(wxRECENT_SELECT, GetId());
		evnt.SetString(color.GetASSFormatted(false, false, false));
		AddPendingEvent(evnt);
	}
}

/// @brief DOCME
/// @param evt 
///
void ColorPickerRecent::OnPaint(wxPaintEvent &evt)
{
	wxPaintDC pdc(this);
	PrepareDC(pdc);

	if (!background_valid) {
		wxSize sz = pdc.GetSize();
	
		background = wxBitmap(sz.x, sz.y);
		wxMemoryDC dc(background);
		
		int i = 0;
		dc.SetPen(*wxTRANSPARENT_PEN);
	
		for (int cy = 0; cy < rows; cy++) {
			for (int cx = 0; cx < cols; cx++) {
				int x, y;
				x = cx * cellsize + internal_control_offset.x;
				y = cy * cellsize + internal_control_offset.y;
	
				dc.SetBrush(wxBrush(colors[i]));
				dc.DrawRectangle(x, y, x+cellsize, y+cellsize);
	
				i++;
			}
		}
		
		background_valid = true;
	}
	
	pdc.DrawBitmap(background, 0, 0, false);
}

/// @brief DOCME
/// @param evt 
///
void ColorPickerRecent::OnSize(wxSizeEvent &evt)
{
	wxSize size = GetClientSize();
	background_valid = false;
	//internal_control_offset.x = (size.GetWidth() - cellsize * cols) / 2;
	//internal_control_offset.y = (size.GetHeight() - cellsize * rows) / 2;
	Refresh();
}

/// @brief DOCME
/// @param parent              
/// @param id                  
/// @param _resx               
/// @param _resy               
/// @param _magnification      
/// @param _integrated_dropper 
///
ColorPickerScreenDropper::ColorPickerScreenDropper(wxWindow *parent, wxWindowID id, int _resx, int _resy, int _magnification, bool _integrated_dropper)
: wxControl(parent, id, wxDefaultPosition, wxDefaultSize, STATIC_BORDER_FLAG), resx(_resx), resy(_resy), magnification(_magnification), integrated_dropper(_integrated_dropper)
{
	SetClientSize(resx*magnification, resy*magnification);
	SetMinSize(GetSize());
	SetMaxSize(GetSize());
	SetCursor(*wxCROSS_CURSOR);

	capture = wxBitmap(resx, resy);
	wxMemoryDC capdc;
	capdc.SelectObject(capture);
	capdc.SetPen(*wxTRANSPARENT_PEN);
	capdc.SetBrush(*wxWHITE_BRUSH);
	capdc.DrawRectangle(0, 0, resx, resy);
}

BEGIN_EVENT_TABLE(ColorPickerScreenDropper, wxControl)
	EVT_PAINT(ColorPickerScreenDropper::OnPaint)
	EVT_MOUSE_EVENTS(ColorPickerScreenDropper::OnMouse)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(wxDROPPER_SELECT)

/// @brief DOCME
/// @param evt 
///
void ColorPickerScreenDropper::OnMouse(wxMouseEvent &evt)
{
	int x, y;
	x = evt.GetX() / magnification;
	y = evt.GetY() / magnification;

	if (HasCapture() && evt.LeftIsDown()) {

		wxPoint pos = ClientToScreen(evt.GetPosition());
		DropFromScreenXY(pos.x, pos.y);

	} else if (evt.LeftDown()) {

		if (x == 0 && y == 0 && integrated_dropper) {
			//SetCursor(*wxCROSS_CURSOR);
			CaptureMouse();

		} else if (x >= 0 && y >= 0 && x < resx && y < resy) {
			wxMemoryDC capdc;
			capdc.SelectObject(capture);
			wxColour color;
			capdc.GetPixel(x, y, &color);
			AssColor ass(color);
			wxCommandEvent evnt(wxDROPPER_SELECT, GetId());
			evnt.SetString(ass.GetASSFormatted(false, false, false));
			AddPendingEvent(evnt);
		}

	} else if (HasCapture() && evt.LeftUp()) {
		ReleaseMouse();
		//SetCursor(wxNullCursor);
	}
}

/// @brief DOCME
/// @param evt 
///
void ColorPickerScreenDropper::OnPaint(wxPaintEvent &evt)
{
	wxPaintDC pdc(this);
	wxMemoryDC capdc;
	capdc.SelectObject(capture);

	pdc.SetPen(*wxTRANSPARENT_PEN);

	for (int x = 0; x < resx; x++) {
		for (int y = 0; y < resy; y++) {
			if (x==0 && y==0 && integrated_dropper) continue;

			wxColour color;
			capdc.GetPixel(x, y, &color);
			pdc.SetBrush(wxBrush(color));

			pdc.DrawRectangle(x*magnification, y*magnification, magnification, magnification);
		}
	}

	if (integrated_dropper) {
		wxBrush cbrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
		pdc.SetBrush(cbrush);
		pdc.DrawRectangle(0, 0, magnification, magnification);
		cbrush.SetStyle(wxCROSSDIAG_HATCH);
		cbrush.SetColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
		pdc.SetBrush(cbrush);
		pdc.DrawRectangle(0, 0, magnification, magnification);
	}
}

/// @brief DOCME
/// @param x 
/// @param y 
///
void ColorPickerScreenDropper::DropFromScreenXY(int x, int y)
{
	wxMemoryDC capdc;
	capdc.SelectObject(capture);
	wxScreenDC screen;

	screen.StartDrawingOnTop();
	capdc.Blit(0, 0, resx, resy, &screen, x-resx/2, y-resy/2);
	screen.EndDrawingOnTop();

	Refresh(false);
}

/// @brief DOCME
/// @param parent   
/// @param original 
/// @return 
///
wxColour GetColorFromUser(wxWindow *parent, wxColour original, ColorCallback callback, void* userdata)
{
	DialogColorPicker dialog(parent, original, callback, userdata);
	if (dialog.ShowModal() == wxID_OK) {
		return dialog.GetColor();
	} else {
		if (callback) callback(userdata, original);
		return original;
	}
}

/// @brief Constructor
/// @param parent        
/// @param initial_color 
///
DialogColorPicker::DialogColorPicker(wxWindow *parent, wxColour initial_color, ColorCallback callback, void* userdata)
: wxDialog(parent, -1, _("Select Colour"), wxDefaultPosition, wxDefaultSize)
, callback(callback)
, callbackUserdata(userdata)
{
	rgb_spectrum[0] =
	rgb_spectrum[1] =
	rgb_spectrum[2] =
	hsl_spectrum    =
	hsv_spectrum    = 0;
	spectrum_dirty = true;

	// generate spectrum slider bar images
	wxImage sliderimg(slider_width, 256, true);
	unsigned char *oslid, *slid;

	// red
	oslid = slid = (unsigned char *)malloc(slider_width*256*3);
	if (!slid) throw std::bad_alloc();
	for (int  y = 0; y < 256; y++) {
		for (int x = 0; x < slider_width; x++) {
			*slid++ = clip_colorval(y);
			*slid++ = 0;
			*slid++ = 0;
		}
	}
	sliderimg.SetData(oslid);
	rgb_slider[0] = new wxBitmap(sliderimg);

	// green
	oslid = slid = (unsigned char *)malloc(slider_width*256*3);
	if (!slid) throw std::bad_alloc();
	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < slider_width; x++) {
			*slid++ = 0;
			*slid++ = clip_colorval(y);
			*slid++ = 0;
		}
	}
	sliderimg.SetData(oslid);
	rgb_slider[1] = new wxBitmap(sliderimg);

	// blue
	oslid = slid = (unsigned char *)malloc(slider_width*256*3);
	if (!slid) throw std::bad_alloc();
	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < slider_width; x++) {
			*slid++ = 0;
			*slid++ = 0;
			*slid++ = clip_colorval(y);
		}
	}
	sliderimg.SetData(oslid);
	rgb_slider[2] = new wxBitmap(sliderimg);

	// luminance
	oslid = slid = (unsigned char *)malloc(slider_width*256*3);
	if (!slid) throw std::bad_alloc();
	for (int y = 0; y < 256; y++) {
		int x = 0;
		for (; x < slider_width; x++) {
			*slid++ = clip_colorval(y);
			*slid++ = clip_colorval(y);
			*slid++ = clip_colorval(y);
		}
	}
	sliderimg.SetData(oslid);
	hsl_slider = new wxBitmap(sliderimg);

	oslid = slid = (unsigned char *)malloc(slider_width*256*3);
	if (!slid) throw std::bad_alloc();
	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < slider_width; x++) {
			hsv_to_rgb(y, 255, 255, slid, slid+1, slid+2);
			slid += 3;
		}
	}
	sliderimg.SetData(oslid);
	hsv_slider = new wxBitmap(sliderimg);

	// Create the controls for the dialog
	wxSizer *spectrum_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Colour spectrum"));
	spectrum = new ColorPickerSpectrum(this, SELECTOR_SPECTRUM, 0, -1, -1, ColorPickerSpectrum::HorzVert, wxSize(256, 256));
	slider = new ColorPickerSpectrum(this, SELECTOR_SLIDER, 0, -1, -1, ColorPickerSpectrum::Vert, wxSize(slider_width, 256));
	wxString modes[] = { _("RGB/R"), _("RGB/G"), _("RGB/B"), _("HSL/L"), _("HSV/H") };
	colorspace_choice = new wxChoice(this, SELECTOR_MODE, wxDefaultPosition, wxDefaultSize, 5, modes);

	wxSize colorinput_size(70, -1);
	wxSize colorinput_labelsize(40, -1);

	wxSizer *rgb_box = new wxStaticBoxSizer(wxHORIZONTAL, this, _("RGB colour"));
	rgb_input[0] = new wxSpinCtrl(this, SELECTOR_RGB_R, _T(""), wxDefaultPosition, colorinput_size, wxSP_ARROW_KEYS, 0, 255);
	rgb_input[1] = new wxSpinCtrl(this, SELECTOR_RGB_G, _T(""), wxDefaultPosition, colorinput_size, wxSP_ARROW_KEYS, 0, 255);
	rgb_input[2] = new wxSpinCtrl(this, SELECTOR_RGB_B, _T(""), wxDefaultPosition, colorinput_size, wxSP_ARROW_KEYS, 0, 255);

	wxSizer *hsl_box = new wxStaticBoxSizer(wxVERTICAL, this, _("HSL colour"));
	hsl_input[0] = new wxSpinCtrl(this, SELECTOR_HSL_H, _T(""), wxDefaultPosition, colorinput_size, wxSP_ARROW_KEYS, 0, 255);
	hsl_input[1] = new wxSpinCtrl(this, SELECTOR_HSL_S, _T(""), wxDefaultPosition, colorinput_size, wxSP_ARROW_KEYS, 0, 255);
	hsl_input[2] = new wxSpinCtrl(this, SELECTOR_HSL_L, _T(""), wxDefaultPosition, colorinput_size, wxSP_ARROW_KEYS, 0, 255);

	wxSizer *hsv_box = new wxStaticBoxSizer(wxVERTICAL, this, _("HSV colour"));
	hsv_input[0] = new wxSpinCtrl(this, SELECTOR_HSV_H, _T(""), wxDefaultPosition, colorinput_size, wxSP_ARROW_KEYS, 0, 255);
	hsv_input[1] = new wxSpinCtrl(this, SELECTOR_HSV_S, _T(""), wxDefaultPosition, colorinput_size, wxSP_ARROW_KEYS, 0, 255);
	hsv_input[2] = new wxSpinCtrl(this, SELECTOR_HSV_V, _T(""), wxDefaultPosition, colorinput_size, wxSP_ARROW_KEYS, 0, 255);

	ass_input = new wxTextCtrl(this, SELECTOR_ASS_INPUT, _T(""), wxDefaultPosition, colorinput_size);
	html_input = new wxTextCtrl(this, SELECTOR_HTML_INPUT, _T(""), wxDefaultPosition, colorinput_size);

	preview_bitmap = wxBitmap(40, 40, 24);
	preview_box = new wxStaticBitmap(this, -1, preview_bitmap, wxDefaultPosition, wxSize(40, 40), STATIC_BORDER_FLAG);

	recent_box = new ColorPickerRecent(this, SELECTOR_RECENT, 8, 4, 16);

	eyedropper_bitmap = GETIMAGE(eyedropper_tool_24);
	eyedropper_bitmap.SetMask(new wxMask(eyedropper_bitmap, wxColour(255, 0, 255)));
	screen_dropper_icon = new wxStaticBitmap(this, SELECTOR_DROPPER, eyedropper_bitmap, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER);
	screen_dropper = new ColorPickerScreenDropper(this, SELECTOR_DROPPER_PICK, 7, 7, 8, false);

	// Arrange the controls in a nice way
	wxSizer *spectop_sizer = new wxBoxSizer(wxHORIZONTAL);
	spectop_sizer->Add(new wxStaticText(this, -1, _("Spectrum mode:")), 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT|wxRIGHT, 5);
	spectop_sizer->Add(colorspace_choice, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
	spectop_sizer->Add(5, 5, 1, wxEXPAND);
	spectop_sizer->Add(preview_box, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT);
	wxSizer *spectrum_sizer = new wxFlexGridSizer(2, 5, 5);
	spectrum_sizer->Add(spectop_sizer, wxEXPAND);
	spectrum_sizer->AddStretchSpacer(1);
	spectrum_sizer->Add(spectrum);
	spectrum_sizer->Add(slider);
	spectrum_box->Add(spectrum_sizer, 0, wxALL, 3);

	wxFlexGridSizer *rgb_sizer = new wxFlexGridSizer(2, 5, 5);
	rgb_sizer->Add(new wxStaticText(this, -1, _("Red:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
	rgb_sizer->Add(rgb_input[0], 0);
	rgb_sizer->Add(new wxStaticText(this, -1, _("Green:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
	rgb_sizer->Add(rgb_input[1], 0);
	rgb_sizer->Add(new wxStaticText(this, -1, _("Blue:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
	rgb_sizer->Add(rgb_input[2], 0);
	rgb_sizer->AddGrowableCol(0,1);
	rgb_box->Add(rgb_sizer, 1, wxEXPAND | wxALL, 3);

	wxFlexGridSizer *ass_input_sizer = new wxFlexGridSizer(2, 5, 5);
	ass_input_sizer->Add(new wxStaticText(this, -1, _T("ASS:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
	ass_input_sizer->Add(ass_input, 0);
	ass_input_sizer->Add(new wxStaticText(this, -1, _T("HTML:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
	ass_input_sizer->Add(html_input, 0);
	ass_input_sizer->AddGrowableCol(0,1);
	rgb_box->Add(ass_input_sizer, 0, wxALL|wxCENTER|wxEXPAND, 3);

	wxFlexGridSizer *hsl_sizer = new wxFlexGridSizer(2, 5, 5);
	hsl_sizer->Add(new wxStaticText(this, -1, _("Hue:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
	hsl_sizer->Add(hsl_input[0], 0);
	hsl_sizer->Add(new wxStaticText(this, -1, _("Sat.:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
	hsl_sizer->Add(hsl_input[1], 0);
	hsl_sizer->Add(new wxStaticText(this, -1, _("Lum.:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
	hsl_sizer->Add(hsl_input[2], 0);
	hsl_sizer->AddGrowableCol(0,1);
	hsl_box->Add(hsl_sizer, 0, wxALL|wxEXPAND, 3);

	wxFlexGridSizer *hsv_sizer = new wxFlexGridSizer(2, 5, 5);
	hsv_sizer->Add(new wxStaticText(this, -1, _("Hue:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
	hsv_sizer->Add(hsv_input[0], 0);
	hsv_sizer->Add(new wxStaticText(this, -1, _("Sat.:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
	hsv_sizer->Add(hsv_input[1], 0);
	hsv_sizer->Add(new wxStaticText(this, -1, _("Value:"), wxDefaultPosition, colorinput_labelsize), 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
	hsv_sizer->Add(hsv_input[2], 0);
	hsv_sizer->AddGrowableCol(0,1);
	hsv_box->Add(hsv_sizer, 0, wxALL|wxEXPAND, 3);

	wxSizer *hsx_sizer = new wxBoxSizer(wxHORIZONTAL);
	hsx_sizer->Add(hsl_box);
	hsx_sizer->AddSpacer(5);
	hsx_sizer->Add(hsv_box);

	wxSizer *recent_sizer = new wxBoxSizer(wxVERTICAL);
	recent_sizer->Add(recent_box, 1, wxEXPAND);
	if (OPT_GET("Tool/Colour Picker/RGBAdjust Tool")->GetBool()) recent_sizer->Add(new wxButton(this,BUTTON_RGBADJUST,_T("rgbadjust()")), 0, wxEXPAND);

	wxSizer *picker_sizer = new wxBoxSizer(wxHORIZONTAL);
	picker_sizer->AddStretchSpacer();
	picker_sizer->Add(screen_dropper_icon, 0, wxALIGN_CENTER|wxRIGHT, 5);
	picker_sizer->Add(screen_dropper, 0, wxALIGN_CENTER);
	picker_sizer->AddStretchSpacer();
	picker_sizer->Add(recent_sizer, 0, wxALIGN_CENTER);
	picker_sizer->AddStretchSpacer();

	wxStdDialogButtonSizer *button_sizer = new wxStdDialogButtonSizer();
	button_sizer->AddButton(new wxButton(this,wxID_OK));
	button_sizer->AddButton(new wxButton(this,wxID_CANCEL));
	button_sizer->AddButton(new HelpButton(this,_("Colour Picker")));
	button_sizer->Realize();

	wxSizer *input_sizer = new wxBoxSizer(wxVERTICAL);
	input_sizer->Add(rgb_box, 0, wxALIGN_CENTER|wxEXPAND);
	input_sizer->AddSpacer(5);
	input_sizer->Add(hsx_sizer, 0, wxALIGN_CENTER|wxEXPAND);
	input_sizer->AddStretchSpacer(1);
	input_sizer->Add(picker_sizer, 0, wxALIGN_CENTER|wxEXPAND);
	input_sizer->AddStretchSpacer(2);
	input_sizer->Add(button_sizer, 0, wxALIGN_RIGHT|wxALIGN_BOTTOM);

	wxSizer *main_sizer = new wxBoxSizer(wxHORIZONTAL);
	main_sizer->Add(spectrum_box, 1, wxALL | wxEXPAND, 5);
	main_sizer->Add(input_sizer, 0, (wxALL&~wxLEFT)|wxEXPAND, 5);

	SetSizer(main_sizer);
	main_sizer->SetSizeHints(this);

	// Position window
	if (lastx == -1 && lasty == -1) {
		CenterOnParent();
	} else {
		Move(lastx, lasty);
	}

	// Fill the controls
	updating_controls = false;
	int mode = OPT_GET("Tool/Colour Picker/Mode")->GetInt();
	if (mode < 0 || mode > 4) mode = 3; // HSL default
	colorspace_choice->SetSelection(mode);
	SetColor(initial_color);
	recent_box->LoadFromString(lagi_wxString(OPT_GET("Tool/Colour Picker/Recent")->GetString()));

	// The mouse event handler for the Dropper control must be manually assigned
	// The EVT_MOUSE_EVENTS macro can't take a control id
	screen_dropper_icon->Connect(wxEVT_MOTION, wxMouseEventHandler(DialogColorPicker::OnDropperMouse), 0, this);
	screen_dropper_icon->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(DialogColorPicker::OnDropperMouse), 0, this);
	screen_dropper_icon->Connect(wxEVT_LEFT_UP, wxMouseEventHandler(DialogColorPicker::OnDropperMouse), 0, this);
}

/// @brief Destructor
///
DialogColorPicker::~DialogColorPicker()
{
	GetPosition(&lastx, &lasty);

	delete rgb_spectrum[0];
	delete rgb_spectrum[1];
	delete rgb_spectrum[2];
	delete hsl_spectrum;
	delete hsv_spectrum;
	delete rgb_slider[0];
	delete rgb_slider[1];
	delete rgb_slider[2];
	delete hsl_slider;
	delete hsv_slider;

	if (screen_dropper_icon->HasCapture()) screen_dropper_icon->ReleaseMouse();
}

/// @brief Sets the currently selected color, and updates all controls
/// @param new_color 
///
void DialogColorPicker::SetColor(wxColour new_color)
{
	cur_color = new_color;
	rgb_input[0]->SetValue(new_color.Red());
	rgb_input[1]->SetValue(new_color.Green());
	rgb_input[2]->SetValue(new_color.Blue());
	UpdateFromRGB();
}

/// @brief Get the currently selected color
/// @return 
///
wxColour DialogColorPicker::GetColor()
{
	recent_box->AddColor(cur_color);
	OPT_SET("Tool/Colour Picker/Recent")->SetString(STD_STR(recent_box->StoreToString()));
	return cur_color;
}

/// @brief Use the values entered in the RGB controls to update the other controls
/// @return 
///
void DialogColorPicker::UpdateFromRGB()
{
	if (updating_controls) return;
	updating_controls = true;

	unsigned char r, g, b, h, s, l, h2, s2, v2;
	r = rgb_input[0]->GetValue();
	g = rgb_input[1]->GetValue();
	b = rgb_input[2]->GetValue();
	rgb_to_hsl(r, g, b, &h, &s, &l);
	rgb_to_hsv(r, g, b, &h2, &s2, &v2);
	hsl_input[0]->SetValue(h);
	hsl_input[1]->SetValue(s);
	hsl_input[2]->SetValue(l);
	hsv_input[0]->SetValue(h2);
	hsv_input[1]->SetValue(s2);
	hsv_input[2]->SetValue(v2);
	cur_color = wxColour(r, g, b, wxALPHA_OPAQUE);
	ass_input->SetValue(AssColor(cur_color).GetASSFormatted(false, false, false));
	html_input->SetValue(color_to_html(cur_color));
	UpdateSpectrumDisplay();

	updating_controls = false;
}

/// @brief Use the values entered in the HSL controls to update the other controls
/// @return 
///
void DialogColorPicker::UpdateFromHSL()
{
	if (updating_controls) return;
	updating_controls = true;

	unsigned char r, g, b, h, s, l, h2, s2, v2;
	h = hsl_input[0]->GetValue();
	s = hsl_input[1]->GetValue();
	l = hsl_input[2]->GetValue();
	hsl_to_rgb(h, s, l, &r, &g, &b);
	hsl_to_hsv(h, s, l, &h2, &s2, &v2);
	rgb_input[0]->SetValue(r);
	rgb_input[1]->SetValue(g);
	rgb_input[2]->SetValue(b);
	hsv_input[0]->SetValue(h2);
	hsv_input[1]->SetValue(s2);
	hsv_input[2]->SetValue(v2);
	cur_color = wxColour(r, g, b, wxALPHA_OPAQUE);
	ass_input->SetValue(AssColor(cur_color).GetASSFormatted(false, false, false));
	html_input->SetValue(color_to_html(cur_color));
	UpdateSpectrumDisplay();

	updating_controls = false;
}

/// @brief DOCME
/// @return 
///
void DialogColorPicker::UpdateFromHSV()
{
	if (updating_controls) return;
	updating_controls = true;

	unsigned char r, g, b, h, s, l, h2, s2, v2;
	//int r, g, b, h2, s2, v2;
	h2 = hsv_input[0]->GetValue();
	s2 = hsv_input[1]->GetValue();
	v2 = hsv_input[2]->GetValue();
	hsv_to_rgb(h2, s2, v2, &r, &g, &b);
	hsv_to_hsl(h2, s2, v2, &h, &s, &l);
	rgb_input[0]->SetValue(r);
	rgb_input[1]->SetValue(g);
	rgb_input[2]->SetValue(b);
	hsl_input[0]->SetValue(h);
	hsl_input[1]->SetValue(s);
	hsl_input[2]->SetValue(l);
	cur_color = wxColour(r, g, b, wxALPHA_OPAQUE);
	ass_input->SetValue(AssColor(cur_color).GetASSFormatted(false, false, false));
	html_input->SetValue(color_to_html(cur_color));
	UpdateSpectrumDisplay();

	updating_controls = false;
}

/// @brief Use the value entered in the ASS hex control to update the other controls
/// @return 
///
void DialogColorPicker::UpdateFromASS()
{
	if (updating_controls) return;
	updating_controls = true;

	unsigned char r, g, b, h, s, l, h2, s2, v2;
	AssColor ass;
	ass.Parse(ass_input->GetValue());
	r = ass.r;
	g = ass.g;
	b = ass.b;
	rgb_to_hsl(r, g, b, &h, &s, &l);
	rgb_to_hsv(r, g, b, &h2, &s2, &v2);
	rgb_input[0]->SetValue(r);
	rgb_input[1]->SetValue(g);
	rgb_input[2]->SetValue(b);
	hsl_input[0]->SetValue(h);
	hsl_input[1]->SetValue(s);
	hsl_input[2]->SetValue(l);
	hsv_input[0]->SetValue(h2);
	hsv_input[1]->SetValue(s2);
	hsv_input[2]->SetValue(v2);
	cur_color = wxColour(r, g, b, wxALPHA_OPAQUE);
	html_input->SetValue(color_to_html(cur_color));
	UpdateSpectrumDisplay();

	updating_controls = false;
}

/// @brief DOCME
/// @return 
///
void DialogColorPicker::UpdateFromHTML()
{
	if (updating_controls) return;
	updating_controls = true;

	unsigned char r, g, b, h, s, l, h2, s2, v2;
	cur_color = html_to_color(html_input->GetValue());
	r = cur_color.Red();
	g = cur_color.Green();
	b = cur_color.Blue();
	rgb_to_hsl(r, g, b, &h, &s, &l);
	rgb_to_hsv(r, g, b, &h2, &s2, &v2);
	rgb_input[0]->SetValue(r);
	rgb_input[1]->SetValue(g);
	rgb_input[2]->SetValue(b);
	hsl_input[0]->SetValue(h);
	hsl_input[1]->SetValue(s);
	hsl_input[2]->SetValue(l);
	hsv_input[0]->SetValue(h2);
	hsv_input[1]->SetValue(s2);
	hsv_input[2]->SetValue(v2);
	cur_color = wxColour(r, g, b, wxALPHA_OPAQUE);
	ass_input->SetValue(AssColor(cur_color).GetASSFormatted(false, false, false));
	UpdateSpectrumDisplay();

	updating_controls = false;
}

/// @brief DOCME
///
void DialogColorPicker::UpdateSpectrumDisplay()
{
	int i = colorspace_choice->GetSelection();
	switch (i) {
		case 0:
			if (spectrum_dirty)
				spectrum->SetBackground(MakeGBSpectrum(), true);
			slider->SetBackground(rgb_slider[0]);
			slider->SetXY(0, rgb_input[0]->GetValue());
			spectrum->SetXY(rgb_input[2]->GetValue(), rgb_input[1]->GetValue());
			break;
		case 1:
			if (spectrum_dirty)
				spectrum->SetBackground(MakeRBSpectrum(), true);
			slider->SetBackground(rgb_slider[1]);
			slider->SetXY(0, rgb_input[1]->GetValue());
			spectrum->SetXY(rgb_input[2]->GetValue(), rgb_input[0]->GetValue());
			break;
		case 2:
			if (spectrum_dirty)
				spectrum->SetBackground(MakeRGSpectrum(), true);
			slider->SetBackground(rgb_slider[2]);
			slider->SetXY(0, rgb_input[2]->GetValue());
			spectrum->SetXY(rgb_input[1]->GetValue(), rgb_input[0]->GetValue());
			break;
		case 3:
			if (spectrum_dirty)
				spectrum->SetBackground(MakeHSSpectrum(), true);
			slider->SetBackground(hsl_slider);
			slider->SetXY(0, hsl_input[2]->GetValue());
			spectrum->SetXY(hsl_input[1]->GetValue(), hsl_input[0]->GetValue());
			break;
		case 4:
			if (spectrum_dirty)
				spectrum->SetBackground(MakeSVSpectrum(), true);
			slider->SetBackground(hsv_slider);
			slider->SetXY(0, hsv_input[0]->GetValue());
			spectrum->SetXY(hsv_input[1]->GetValue(), hsv_input[2]->GetValue());
			break;
	}
	spectrum_dirty = false;

	wxBitmap tempBmp = preview_box->GetBitmap();
	{
		wxMemoryDC previewdc;
		previewdc.SelectObject(tempBmp);
		previewdc.SetPen(*wxTRANSPARENT_PEN);
		previewdc.SetBrush(wxBrush(cur_color));
		previewdc.DrawRectangle(0, 0, 40, 40);
	}
	preview_box->SetBitmap(tempBmp);

	if (callback) callback(callbackUserdata, cur_color);
}

/// @brief DOCME
/// @return 
///
wxBitmap *DialogColorPicker::MakeGBSpectrum()
{
	if (rgb_spectrum[0]) delete rgb_spectrum[0];

	wxImage spectrum_image(256, 256, false);
	unsigned char *ospec, *spec;

	ospec = spec = (unsigned char *)malloc(256*256*3);
	for (int g = 0; g < 256; g++) {
		for (int b = 0; b < 256; b++) {
			*spec++ = cur_color.Red();
			*spec++ = g;
			*spec++ = b;
		}
	}
	spectrum_image.SetData(ospec);
	rgb_spectrum[0] = new wxBitmap(spectrum_image);

	return rgb_spectrum[0];
}

/// @brief DOCME
/// @return 
///
wxBitmap *DialogColorPicker::MakeRBSpectrum()
{
	if (rgb_spectrum[1]) delete rgb_spectrum[1];

	wxImage spectrum_image(256, 256, false);
	unsigned char *ospec, *spec;

	ospec = spec = (unsigned char *)malloc(256*256*3);
	for (int r = 0; r < 256; r++) {
		for (int b = 0; b < 256; b++) {
			*spec++ = r;
			*spec++ = cur_color.Green();
			*spec++ = b;
		}
	}
	spectrum_image.SetData(ospec);
	rgb_spectrum[1] = new wxBitmap(spectrum_image);

	return rgb_spectrum[1];
}

/// @brief DOCME
/// @return 
///
wxBitmap *DialogColorPicker::MakeRGSpectrum()
{
	if (rgb_spectrum[2]) delete rgb_spectrum[2];

	wxImage spectrum_image(256, 256, false);
	unsigned char *ospec, *spec;

	ospec = spec = (unsigned char *)malloc(256*256*3);
	for (int r = 0; r < 256; r++) {
		for (int g = 0; g < 256; g++) {
			*spec++ = r;
			*spec++ = g;
			*spec++ = cur_color.Blue();
		}
	}
	spectrum_image.SetData(ospec);
	rgb_spectrum[2] = new wxBitmap(spectrum_image);

	return rgb_spectrum[2];
}

/// @brief DOCME
/// @return 
///
wxBitmap *DialogColorPicker::MakeHSSpectrum()
{
	if (hsl_spectrum) delete hsl_spectrum;

	wxImage spectrum_image(256, 256, false);
	unsigned char *ospec, *spec;

	ospec = spec = (unsigned char *)malloc(256*256*3);
	int l = hsl_input[2]->GetValue();

	for (int h = 0; h < 256; h++) {
		unsigned char maxr, maxg, maxb;
		hsl_to_rgb(h, 255, l, &maxr, &maxg, &maxb);

		for (int s = 0; s < 256; s++) {
			*spec++ = maxr * s / 256 + (255-s) * l / 256;
			*spec++ = maxg * s / 256 + (255-s) * l / 256;
			*spec++ = maxb * s / 256 + (255-s) * l / 256;
		}
	}
	spectrum_image.SetData(ospec);
	hsl_spectrum = new wxBitmap(spectrum_image);

	return hsl_spectrum;
}

/// @brief DOCME
/// @return 
///
wxBitmap *DialogColorPicker::MakeSVSpectrum()
{
	if (hsv_spectrum) delete hsv_spectrum;

	wxImage spectrum_image(256, 256, false);
	unsigned char *ospec, *spec;

	ospec = spec = (unsigned char *)malloc(256*256*3);

	int h = hsv_input[0]->GetValue();
	unsigned char maxr, maxg, maxb;
	hsv_to_rgb(h, 255, 255, &maxr, &maxg, &maxb);

	for (int v = 0; v < 256; v++) {
		int rr, rg, rb;
		rr = (255-maxr) * v / 256;
		rg = (255-maxg) * v / 256;
		rb = (255-maxb) * v / 256;
		for (int s = 0; s < 256; s++) {
			int r, g, b;
			r = 255 - rr * s / 256 - (255-v);
			g = 255 - rg * s / 256 - (255-v);
			b = 255 - rb * s / 256 - (255-v);
			*spec++ = r;
			*spec++ = g;
			*spec++ = b;
		}
	}
	spectrum_image.SetData(ospec);
	hsv_spectrum = new wxBitmap(spectrum_image);

	return hsv_spectrum;
}

BEGIN_EVENT_TABLE(DialogColorPicker, wxDialog)
	EVT_SPINCTRL(SELECTOR_RGB_R, DialogColorPicker::OnSpinRGB)
	EVT_SPINCTRL(SELECTOR_RGB_G, DialogColorPicker::OnSpinRGB)
	EVT_SPINCTRL(SELECTOR_RGB_B, DialogColorPicker::OnSpinRGB)
	EVT_SPINCTRL(SELECTOR_HSL_H, DialogColorPicker::OnSpinHSL)
	EVT_SPINCTRL(SELECTOR_HSL_S, DialogColorPicker::OnSpinHSL)
	EVT_SPINCTRL(SELECTOR_HSL_L, DialogColorPicker::OnSpinHSL)
	EVT_SPINCTRL(SELECTOR_HSV_H, DialogColorPicker::OnSpinHSV)
	EVT_SPINCTRL(SELECTOR_HSV_S, DialogColorPicker::OnSpinHSV)
	EVT_SPINCTRL(SELECTOR_HSV_V, DialogColorPicker::OnSpinHSV)
	EVT_TEXT(SELECTOR_RGB_R, DialogColorPicker::OnChangeRGB)
	EVT_TEXT(SELECTOR_RGB_G, DialogColorPicker::OnChangeRGB)
	EVT_TEXT(SELECTOR_RGB_B, DialogColorPicker::OnChangeRGB)
	EVT_TEXT(SELECTOR_HSL_H, DialogColorPicker::OnChangeHSL)
	EVT_TEXT(SELECTOR_HSL_S, DialogColorPicker::OnChangeHSL)
	EVT_TEXT(SELECTOR_HSL_L, DialogColorPicker::OnChangeHSL)
	EVT_TEXT(SELECTOR_HSV_H, DialogColorPicker::OnChangeHSV)
	EVT_TEXT(SELECTOR_HSV_S, DialogColorPicker::OnChangeHSV)
	EVT_TEXT(SELECTOR_HSV_V, DialogColorPicker::OnChangeHSV)
	EVT_TEXT(SELECTOR_ASS_INPUT, DialogColorPicker::OnChangeASS)
	EVT_TEXT(SELECTOR_HTML_INPUT, DialogColorPicker::OnChangeHTML)
	EVT_CHOICE(SELECTOR_MODE, DialogColorPicker::OnChangeMode)
	EVT_COMMAND(SELECTOR_SPECTRUM, wxSPECTRUM_CHANGE, DialogColorPicker::OnSpectrumChange)
	EVT_COMMAND(SELECTOR_SLIDER, wxSPECTRUM_CHANGE, DialogColorPicker::OnSliderChange)
	EVT_COMMAND(SELECTOR_RECENT, wxRECENT_SELECT, DialogColorPicker::OnRecentSelect)
	EVT_COMMAND(SELECTOR_DROPPER_PICK, wxDROPPER_SELECT, DialogColorPicker::OnRecentSelect)
	EVT_BUTTON(BUTTON_RGBADJUST, DialogColorPicker::OnRGBAdjust)
	EVT_MOUSE_EVENTS(DialogColorPicker::OnMouse)
END_EVENT_TABLE()

/// @brief DOCME
/// @param evt 
///
void DialogColorPicker::OnSpinRGB(wxSpinEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromRGB();
}

/// @brief DOCME
/// @param evt 
///
void DialogColorPicker::OnSpinHSL(wxSpinEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromHSL();
}

/// @brief DOCME
/// @param evt 
///
void DialogColorPicker::OnSpinHSV(wxSpinEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromHSV();
}

/// @brief DOCME
/// @param evt 
///
void DialogColorPicker::OnChangeRGB(wxCommandEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromRGB();
}

/// @brief DOCME
/// @param evt 
///
void DialogColorPicker::OnChangeHSL(wxCommandEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromHSL();
}

/// @brief DOCME
/// @param evt 
///
void DialogColorPicker::OnChangeHSV(wxCommandEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromHSV();
}

/// @brief DOCME
/// @param evt 
///
void DialogColorPicker::OnChangeASS(wxCommandEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromASS();
}

/// @brief DOCME
/// @param evt 
///
void DialogColorPicker::OnChangeHTML(wxCommandEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	UpdateFromHTML();
}

/// @brief DOCME
/// @param evt 
///
void DialogColorPicker::OnChangeMode(wxCommandEvent &evt)
{
	if (!updating_controls)
		spectrum_dirty = true;
	OPT_SET("Tool/Colour Picker/Mode")->SetInt(colorspace_choice->GetSelection());
	UpdateSpectrumDisplay();
}

/// @brief DOCME
/// @param evt 
///
void DialogColorPicker::OnSpectrumChange(wxCommandEvent &evt)
{
	updating_controls = true;

	int i = colorspace_choice->GetSelection();
	int x, y;
	spectrum->GetXY(x, y);
	switch (i) {
		case 0:
			rgb_input[2]->SetValue(x);
			rgb_input[1]->SetValue(y);
			updating_controls = false;
			UpdateFromRGB();
			break;
		case 1:
			rgb_input[2]->SetValue(x);
			rgb_input[0]->SetValue(y);
			updating_controls = false;
			UpdateFromRGB();
			break;
		case 2:
			rgb_input[1]->SetValue(x);
			rgb_input[0]->SetValue(y);
			updating_controls = false;
			UpdateFromRGB();
			break;
		case 3:
			hsl_input[1]->SetValue(x);
			hsl_input[0]->SetValue(y);
			updating_controls = false;
			UpdateFromHSL();
			break;
		case 4:
			hsv_input[1]->SetValue(x);
			hsv_input[2]->SetValue(y);
			updating_controls = false;
			UpdateFromHSV();
			break;
	}

}

/// @brief DOCME
/// @param evt Ignored
///
void DialogColorPicker::OnSliderChange(wxCommandEvent &evt)
{
	spectrum_dirty = true;
	int i = colorspace_choice->GetSelection();
	int x, y; // only y is used, x is garbage for this control
	slider->GetXY(x, y);
	switch (i) {
		case 0:
			rgb_input[0]->SetValue(y);
			UpdateFromRGB();
			break;
		case 1:
			rgb_input[1]->SetValue(y);
			UpdateFromRGB();
			break;
		case 2:
			rgb_input[2]->SetValue(y);
			UpdateFromRGB();
			break;
		case 3:
			hsl_input[2]->SetValue(y);
			UpdateFromHSL();
			break;
		case 4:
			hsv_input[0]->SetValue(y);
			UpdateFromHSV();
			break;
	}
}

/// @brief DOCME
/// @param evt 
///
void DialogColorPicker::OnRecentSelect(wxCommandEvent &evt)
{
	// The colour picked is stored in the event string
	// Allows this event handler to be shared by recent and dropper controls
	// Ugly hack?
	AssColor color;
	color.Parse(evt.GetString());
	SetColor(color.GetWXColor());
}

/// @brief DOCME
/// @param evt 
///
void DialogColorPicker::OnDropperMouse(wxMouseEvent &evt)
{
	if (evt.LeftDown() && !screen_dropper_icon->HasCapture()) {
#ifdef WIN32
		screen_dropper_icon->SetCursor(wxCursor(_T("eyedropper_cursor")));
#else
		screen_dropper_icon->SetCursor(*wxCROSS_CURSOR);
#endif
		screen_dropper_icon->SetBitmap(wxNullBitmap);
		screen_dropper_icon->CaptureMouse();
		eyedropper_grab_point = evt.GetPosition();
		eyedropper_is_grabbed = false;
	}

	if (evt.LeftUp()) {

/// DOCME
#define ABS(x) (x < 0 ? -x : x)
		wxPoint ptdiff = evt.GetPosition() - eyedropper_grab_point;
		bool release_now = eyedropper_is_grabbed || ABS(ptdiff.x) + ABS(ptdiff.y) > 7;
		if (release_now) {
			screen_dropper_icon->ReleaseMouse();
			eyedropper_is_grabbed = false;
			screen_dropper_icon->SetCursor(wxNullCursor);
			screen_dropper_icon->SetBitmap(eyedropper_bitmap);
		} else {
			eyedropper_is_grabbed = true;
		}
	}

	if (screen_dropper_icon->HasCapture()) {
		wxPoint scrpos = screen_dropper_icon->ClientToScreen(evt.GetPosition());
		screen_dropper->DropFromScreenXY(scrpos.x, scrpos.y);
	}
}

/// @brief Hack to redirect events to the screen dropper icon
/// @param evt 
///
void DialogColorPicker::OnMouse(wxMouseEvent &evt)
{
	if (screen_dropper_icon->HasCapture()) {
		wxPoint dropper_pos = screen_dropper_icon->ScreenToClient(ClientToScreen(evt.GetPosition()));
		evt.m_x = dropper_pos.x;
		evt.m_y = dropper_pos.y;
		screen_dropper_icon->GetEventHandler()->ProcessEvent(evt);
	}
	else
		evt.Skip();
}

/// @brief rgbadjust() tool
/// @param evt 
///
void DialogColorPicker::OnRGBAdjust(wxCommandEvent &evt)
{
	wxColour cur = cur_color;
	wxColour old = recent_box->GetColor(0);
	double r = double(cur.Red()) / double(old.Red());
	double g = double(cur.Green()) / double(old.Green());
	double b = double(cur.Blue()) / double(old.Blue());
	wxString data = wxString::Format(L"rgbadjust(%g,%g,%g)", r, g, b);

	if (wxTheClipboard->Open())	{
		wxTheClipboard->SetData(new wxTextDataObject(data));
		wxTheClipboard->Close();
	}
}

/// DOCME
int DialogColorPicker::lastx = -1;

/// DOCME
int DialogColorPicker::lasty = -1;


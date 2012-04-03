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
#include <wx/rawbmp.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbmp.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tokenzr.h>
#endif

#include <libaegisub/scoped_ptr.h>

#include "ass_style.h"
#include "colorspace.h"
#include "compat.h"
#include "dialog_colorpicker.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "persist_location.h"
#include "utils.h"

/// DOCME
/// @class ColorPickerSpectrum
/// @brief DOCME
///
/// DOCME
class ColorPickerSpectrum : public wxControl {
public:
	enum PickerDirection {
		HorzVert,
		Horz,
		Vert
	};
private:
	int x;
	int y;

	/// DOCME
	wxBitmap *background;

	/// DOCME
	PickerDirection direction;

	void OnPaint(wxPaintEvent &evt);
	void OnMouse(wxMouseEvent &evt);

public:
	ColorPickerSpectrum(wxWindow *parent, PickerDirection direction, wxSize size);

	int GetX() const { return x; }
	int GetY() const { return y; }
	void SetXY(int xx, int yy);
	void SetBackground(wxBitmap *new_background, bool force = false);
};

/// DOCME
/// @class ColorPickerRecent
/// @brief DOCME
///
/// DOCME
class ColorPickerRecent : public wxControl {
	int rows;     ///< Number of rows of colors
	int cols;     ///< Number of cols of colors
	int cellsize; ///< Width/Height of each cell

	/// The colors currently displayed in the control
	std::vector<wxColour> colors;

	/// Does the background need to be regenerated?
	bool background_valid;

	/// Bitmap storing the cached background
	wxBitmap background;

	void OnClick(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);
	void OnSize(wxSizeEvent &evt);

public:
	ColorPickerRecent(wxWindow *parent, int cols, int rows, int cellsize);

	/// Load the colors to show from a string
	void LoadFromString(const wxString &recent_string = wxString());
	/// Save the colors currently shown to a string
	wxString StoreToString();
	/// Add a color to the beginning of the recent list
	void AddColor(wxColour color);
};

/// DOCME
/// @class ColorPickerScreenDropper
/// @brief DOCME
///
/// DOCME
class ColorPickerScreenDropper : public wxControl {
	/// DOCME
	wxBitmap capture;

	/// DOCME

	/// DOCME
	int resx, resy;

	/// DOCME
	int magnification;

	void OnMouse(wxMouseEvent &evt);
	void OnPaint(wxPaintEvent &evt);

public:
	ColorPickerScreenDropper(wxWindow *parent, int resx, int resy, int magnification);

	void DropFromScreenXY(int x, int y);
};

/// DOCME
/// @class DialogColorPicker
/// @brief DOCME
///
/// DOCME
class DialogColorPicker : public wxDialog {
	agi::scoped_ptr<PersistLocation> persist;

	wxColour cur_color; ///< Currently selected colour

	bool spectrum_dirty; ///< Does the spectrum image need to be regenerated?
	ColorPickerSpectrum *spectrum; ///< The 2D color spectrum
	ColorPickerSpectrum *slider; ///< The 1D slider for the color component not in the slider

	wxChoice *colorspace_choice; ///< The dropdown list to select colorspaces

	static const int slider_width = 10; ///< width in pixels of the color slider control

	wxSpinCtrl *rgb_input[3];
	wxBitmap *rgb_spectrum[3]; ///< x/y spectrum bitmap where color "i" is excluded from
	wxBitmap *rgb_slider[3];   ///< z spectrum for color "i"

	wxSpinCtrl *hsl_input[3];
	wxBitmap *hsl_spectrum; ///< h/s spectrum
	wxBitmap *hsl_slider;   ///< l spectrum

	wxSpinCtrl *hsv_input[3];
	wxBitmap *hsv_spectrum; ///< s/v spectrum
	wxBitmap *hsv_slider;   ///< h spectrum

	wxTextCtrl *ass_input;
	wxTextCtrl *html_input;

	/// The eyedropper is set to a blank icon when it's clicked, so store its normal bitmap
	wxBitmap eyedropper_bitmap;

	/// The point where the eyedropper was click, used to make it possible to either
	/// click the eyedropper or drag the eyedropper
	wxPoint eyedropper_grab_point;

	/// DOCME
	bool eyedropper_is_grabbed;

	wxStaticBitmap *preview_box; ///< A box which simply shows the current color
	ColorPickerRecent *recent_box; ///< A grid of recently used colors

	/// DOCME
	ColorPickerScreenDropper *screen_dropper;

	/// DOCME
	wxStaticBitmap *screen_dropper_icon;

	/// Update all other controls as a result of modifying an RGB control
	void UpdateFromRGB(bool dirty = true);
	/// Update all other controls as a result of modifying an HSL control
	void UpdateFromHSL(bool dirty = true);
	/// Update all other controls as a result of modifying an HSV control
	void UpdateFromHSV(bool dirty = true);
	/// Update all other controls as a result of modifying the ASS format control
	void UpdateFromASS();
	/// Update all other controls as a result of modifying the HTML format control
	void UpdateFromHTML();

	void SetRGB(unsigned char r, unsigned char g, unsigned char b);
	void SetHSL(unsigned char r, unsigned char g, unsigned char b);
	void SetHSV(unsigned char r, unsigned char g, unsigned char b);

	/// Redraw the spectrum display
	void UpdateSpectrumDisplay();

	wxBitmap *MakeGBSpectrum();
	wxBitmap *MakeRBSpectrum();
	wxBitmap *MakeRGSpectrum();
	wxBitmap *MakeHSSpectrum();
	wxBitmap *MakeSVSpectrum();

	/// Constructor helper function for making the color input box sizers
	template<int N, class Control>
	wxSizer *MakeColorInputSizer(wxString (&labels)[N], Control *(&inputs)[N]);

	void OnChangeMode(wxCommandEvent &evt);
	void OnSpectrumChange(wxCommandEvent &evt);
	void OnSliderChange(wxCommandEvent &evt);
	void OnRecentSelect(wxCommandEvent &evt); // also handles dropper pick
	void OnDropperMouse(wxMouseEvent &evt);
	void OnMouse(wxMouseEvent &evt);
	void OnCaptureLost(wxMouseCaptureLostEvent&);

	ColorCallback callback;
	void *callbackUserdata;

public:
	DialogColorPicker(wxWindow *parent, wxColour initial_color, ColorCallback callback = NULL, void *userdata = NULL);
	~DialogColorPicker();

	void SetColor(wxColour new_color);
	wxColour GetColor();
};

/// DOCME
static const int spectrum_horz_vert_arrow_size = 4;

ColorPickerSpectrum::ColorPickerSpectrum(wxWindow *parent, PickerDirection direction, wxSize size)
: wxControl(parent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
, x(-1)
, y(-1)
, background(0)
, direction(direction)
{
	size.x += 2;
	size.y += 2;

	if (direction == Vert) size.x += spectrum_horz_vert_arrow_size + 1;
	if (direction == Horz) size.y += spectrum_horz_vert_arrow_size + 1;

	SetClientSize(size);
	SetMinSize(GetSize());

	Bind(wxEVT_LEFT_DOWN, &ColorPickerSpectrum::OnMouse, this);
	Bind(wxEVT_LEFT_UP, &ColorPickerSpectrum::OnMouse, this);
	Bind(wxEVT_MOTION, &ColorPickerSpectrum::OnMouse, this);
	Bind(wxEVT_PAINT, &ColorPickerSpectrum::OnPaint, this);
}

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


wxDEFINE_EVENT(EVT_SPECTRUM_CHANGE, wxCommandEvent);

void ColorPickerSpectrum::OnPaint(wxPaintEvent &)
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

void ColorPickerSpectrum::OnMouse(wxMouseEvent &evt)
{
	evt.Skip();

	// We only care about mouse move events during a drag
	if (evt.Moving())
		return;

	if (evt.LeftDown()) {
		CaptureMouse();
		SetCursor(wxCursor(wxCURSOR_BLANK));
	}
	else if (evt.LeftUp() && HasCapture()) {
		ReleaseMouse();
		SetCursor(wxNullCursor);
	}

	if (evt.LeftDown() || (HasCapture() && evt.LeftIsDown())) {
		int newx = mid(0, evt.GetX(), GetClientSize().x - 1);
		int newy = mid(0, evt.GetY(), GetClientSize().y - 1);
		SetXY(newx, newy);
		wxCommandEvent evt2(EVT_SPECTRUM_CHANGE, GetId());
		AddPendingEvent(evt2);
	}
}

#ifdef WIN32
#define STATIC_BORDER_FLAG wxSTATIC_BORDER
#else
#define STATIC_BORDER_FLAG wxSIMPLE_BORDER
#endif

ColorPickerRecent::ColorPickerRecent(wxWindow *parent, int cols, int rows, int cellsize)
: wxControl(parent, -1, wxDefaultPosition, wxDefaultSize, STATIC_BORDER_FLAG)
, rows(rows)
, cols(cols)
, cellsize(cellsize)
, background_valid(false)
{
	LoadFromString();
	SetClientSize(cols*cellsize, rows*cellsize);
	SetMinSize(GetSize());
	SetMaxSize(GetSize());
	SetCursor(*wxCROSS_CURSOR);

	Bind(wxEVT_PAINT, &ColorPickerRecent::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &ColorPickerRecent::OnClick, this);
	Bind(wxEVT_SIZE, &ColorPickerRecent::OnSize, this);
}

void ColorPickerRecent::LoadFromString(const wxString &recent_string)
{
	colors.clear();
	wxStringTokenizer toker(recent_string, " ", false);
	while (toker.HasMoreTokens()) {
		AssColor color;
		color.Parse(toker.NextToken());
		color.a = 0; // opaque
		colors.push_back(color.GetWXColor());
	}
	while ((int)colors.size() < rows*cols) {
		colors.push_back(*wxBLACK);
	}

	background_valid = false;
}

wxString ColorPickerRecent::StoreToString()
{
	wxString res;
	for (int i = 0; i < rows*cols; i++) {
		res << AssColor(colors[i]).GetASSFormatted(false, false, false) << " ";
	}
	return res.Trim(true);
}

void ColorPickerRecent::AddColor(wxColour color)
{
	std::vector<wxColour>::iterator existing = find(colors.begin(), colors.end(), color);
	if (existing != colors.end())
		rotate(colors.begin(), existing, existing + 1);
	else
		colors.insert(colors.begin(), color);

	background_valid = false;

	Refresh(false);
}

wxDEFINE_EVENT(EVT_RECENT_SELECT, wxCommandEvent);

void ColorPickerRecent::OnClick(wxMouseEvent &evt)
{
	wxSize cs = GetClientSize();
	int cx = evt.GetX() * cols / cs.x;
	int cy = evt.GetY() * rows / cs.y;
	if (cx < 0 || cx > cols || cy < 0 || cy > rows) return;
	int i = cols*cy + cx;

	if (i >= 0 && i < (int)colors.size()) {
		wxCommandEvent evnt(EVT_RECENT_SELECT, GetId());
		evnt.SetString(AssColor(colors[i]).GetASSFormatted(false, false, false));
		AddPendingEvent(evnt);
	}
}

void ColorPickerRecent::OnPaint(wxPaintEvent &)
{
	wxPaintDC pdc(this);
	PrepareDC(pdc);

	if (!background_valid) {
		wxSize sz = pdc.GetSize();

		background = wxBitmap(sz.x, sz.y);
		wxMemoryDC dc(background);

		dc.SetPen(*wxTRANSPARENT_PEN);

		for (int cy = 0; cy < rows; cy++) {
			for (int cx = 0; cx < cols; cx++) {
				int x = cx * cellsize;
				int y = cy * cellsize;

				dc.SetBrush(wxBrush(colors[cy * cols + cx]));
				dc.DrawRectangle(x, y, x+cellsize, y+cellsize);
			}
		}

		background_valid = true;
	}

	pdc.DrawBitmap(background, 0, 0, false);
}

void ColorPickerRecent::OnSize(wxSizeEvent &)
{
	background_valid = false;
	Refresh();
}

ColorPickerScreenDropper::ColorPickerScreenDropper(wxWindow *parent, int resx, int resy, int magnification)
: wxControl(parent, -1, wxDefaultPosition, wxDefaultSize, STATIC_BORDER_FLAG)
, resx(resx)
, resy(resy)
, magnification(magnification)
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

	Bind(wxEVT_PAINT, &ColorPickerScreenDropper::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &ColorPickerScreenDropper::OnMouse, this);
}

wxDEFINE_EVENT(EVT_DROPPER_SELECT, wxCommandEvent);

void ColorPickerScreenDropper::OnMouse(wxMouseEvent &evt)
{
	int x = evt.GetX() / magnification;
	int y = evt.GetY() / magnification;

	if (x >= 0 && y >= 0 && x < resx && y < resy) {
		wxColour color;
#ifdef __WXMAC__
		// wxMemoryDC::GetPixel() isn't implemented on OS X
		// Work around it by reading pixel data from the bitmap instead
		wxAlphaPixelData cappd(capture);
		wxAlphaPixelData::Iterator cappdi(cappd);
		cappdi.MoveTo(cappd, x, y);
		color.Set(cappdi.Red(), cappdi.Green(), cappdi.Blue());
#else
		wxMemoryDC capdc(capture);
		capdc.GetPixel(x, y, &color);
#endif
		color = wxColour(color.Red(), color.Green(), color.Blue(), wxALPHA_OPAQUE);
		wxCommandEvent evnt(EVT_DROPPER_SELECT, GetId());
		evnt.SetString(AssColor(color).GetASSFormatted(false, false, false));
		AddPendingEvent(evnt);
	}
}

void ColorPickerScreenDropper::OnPaint(wxPaintEvent &)
{
	wxPaintDC pdc(this);

#ifdef __WXMAC__
	// See OnMouse() above
	wxAlphaPixelData cappd(capture);
	wxAlphaPixelData::Iterator cappdi(cappd);
#else
	wxMemoryDC capdc(capture);
#endif

	pdc.SetPen(*wxTRANSPARENT_PEN);

	for (int x = 0; x < resx; x++) {
		for (int y = 0; y < resy; y++) {
			wxColour color;
#ifdef __WXMAC__
			cappdi.MoveTo(cappd, x, y);
			color.Set(cappdi.Red(), cappdi.Green(), cappdi.Blue());
#else
			capdc.GetPixel(x, y, &color);
#endif
			pdc.SetBrush(wxBrush(color));
			pdc.DrawRectangle(x*magnification, y*magnification, magnification, magnification);
		}
	}
}

void ColorPickerScreenDropper::DropFromScreenXY(int x, int y)
{
	wxMemoryDC capdc(capture);
	wxScreenDC screen;

#ifdef __WXMAC__
	wxBitmap screenbmp = screen.GetAsBitmap().GetSubBitmap(wxRect(x-resx/2, y-resy/2, resx, resy));
	capdc.DrawBitmap(screenbmp, 0, 0);
#else
	screen.StartDrawingOnTop();
	capdc.Blit(0, 0, resx, resy, &screen, x-resx/2, y-resy/2);
	screen.EndDrawingOnTop();
#endif

	Refresh(false);
}

wxColour GetColorFromUser(wxWindow *parent, wxColour original, ColorCallback callback, void* userdata)
{
	DialogColorPicker dialog(parent, original, callback, userdata);
	if (dialog.ShowModal() == wxID_OK)
		original = dialog.GetColor();
	if (callback)
		callback(userdata, original);
	return original;
}

static wxBitmap *make_rgb_image(int width, int offset) {
	unsigned char *oslid = (unsigned char *)calloc(width * 256 * 3, 1);
	unsigned char *slid = oslid + offset;
	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < width; x++) {
			*slid = clip_colorval(y);
			slid += 3;
		}
	}
	wxImage img(width, 256, oslid);
	return new wxBitmap(img);
}

DialogColorPicker::DialogColorPicker(wxWindow *parent, wxColour initial_color, ColorCallback callback, void* userdata)
: wxDialog(parent, -1, _("Select Color"))
, callback(callback)
, callbackUserdata(userdata)
{
	memset(rgb_spectrum, 0, sizeof rgb_spectrum);
	hsl_spectrum = 0;
	hsv_spectrum = 0;

	// generate spectrum slider bar images
	rgb_slider[0] = make_rgb_image(slider_width, 0);
	rgb_slider[1] = make_rgb_image(slider_width, 1);
	rgb_slider[2] = make_rgb_image(slider_width, 2);

	// luminance
	unsigned char *slid = (unsigned char *)malloc(slider_width*256*3);
	for (int y = 0; y < 256; y++) {
		memset(slid + y * slider_width * 3, y, slider_width * 3);
	}
	wxImage sliderimg(slider_width, 256, slid);
	hsl_slider = new wxBitmap(sliderimg);

	slid = (unsigned char *)malloc(slider_width*256*3);
	for (int y = 0; y < 256; y++) {
		unsigned char rgb[3];
		hsv_to_rgb(y, 255, 255, rgb, rgb + 1, rgb + 2);
		for (int x = 0; x < slider_width; x++) {
			memcpy(slid + y * slider_width * 3 + x * 3, rgb, 3);
		}
	}
	sliderimg.SetData(slid);
	hsv_slider = new wxBitmap(sliderimg);

	// Create the controls for the dialog
	wxSizer *spectrum_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Color spectrum"));
	spectrum = new ColorPickerSpectrum(this, ColorPickerSpectrum::HorzVert, wxSize(256, 256));
	slider = new ColorPickerSpectrum(this, ColorPickerSpectrum::Vert, wxSize(slider_width, 256));
	wxString modes[] = { _("RGB/R"), _("RGB/G"), _("RGB/B"), _("HSL/L"), _("HSV/H") };
	colorspace_choice = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, 5, modes);

	wxSize colorinput_size(70, -1);
	wxSize colorinput_labelsize(40, -1);

	wxSizer *rgb_box = new wxStaticBoxSizer(wxHORIZONTAL, this, _("RGB color"));
	wxSizer *hsl_box = new wxStaticBoxSizer(wxVERTICAL, this, _("HSL color"));
	wxSizer *hsv_box = new wxStaticBoxSizer(wxVERTICAL, this, _("HSV color"));
	for (int i = 0; i < 3; ++i) {
		rgb_input[i] = new wxSpinCtrl(this, -1, "", wxDefaultPosition, colorinput_size, wxSP_ARROW_KEYS, 0, 255);
		hsl_input[i] = new wxSpinCtrl(this, -1, "", wxDefaultPosition, colorinput_size, wxSP_ARROW_KEYS, 0, 255);
		hsv_input[i] = new wxSpinCtrl(this, -1, "", wxDefaultPosition, colorinput_size, wxSP_ARROW_KEYS, 0, 255);
	}

	ass_input = new wxTextCtrl(this, -1, "", wxDefaultPosition, colorinput_size);
	html_input = new wxTextCtrl(this, -1, "", wxDefaultPosition, colorinput_size);

	preview_box = new wxStaticBitmap(this, -1, wxBitmap(40, 40, 24), wxDefaultPosition, wxSize(40, 40), STATIC_BORDER_FLAG);
	recent_box = new ColorPickerRecent(this, 8, 4, 16);

	eyedropper_bitmap = GETIMAGE(eyedropper_tool_24);
	eyedropper_bitmap.SetMask(new wxMask(eyedropper_bitmap, wxColour(255, 0, 255)));
	screen_dropper_icon = new wxStaticBitmap(this, -1, eyedropper_bitmap, wxDefaultPosition, wxDefaultSize, wxRAISED_BORDER);
	screen_dropper = new ColorPickerScreenDropper(this, 7, 7, 8);

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

	wxString rgb_labels[] = { _("Red:"), _("Green:"), _("Blue:") };
	rgb_box->Add(MakeColorInputSizer(rgb_labels, rgb_input), 1, wxALL|wxEXPAND, 3);

	wxString ass_labels[] = { "ASS:", "HTML:" };
	wxTextCtrl *ass_ctrls[] = { ass_input, html_input };
	rgb_box->Add(MakeColorInputSizer(ass_labels, ass_ctrls), 0, wxALL|wxCENTER|wxEXPAND, 3);

	wxString hsl_labels[] = { _("Hue:"), _("Sat.:"), _("Lum.:") };
	hsl_box->Add(MakeColorInputSizer(hsl_labels, hsl_input), 0, wxALL|wxEXPAND, 3);

	wxString hsv_labels[] = { _("Hue:"), _("Sat.:"), _("Value:") };
	hsv_box->Add(MakeColorInputSizer(hsv_labels, hsv_input), 0, wxALL|wxEXPAND, 3);

	wxSizer *hsx_sizer = new wxBoxSizer(wxHORIZONTAL);
	hsx_sizer->Add(hsl_box);
	hsx_sizer->AddSpacer(5);
	hsx_sizer->Add(hsv_box);

	wxSizer *recent_sizer = new wxBoxSizer(wxVERTICAL);
	recent_sizer->Add(recent_box, 1, wxEXPAND);

	wxSizer *picker_sizer = new wxBoxSizer(wxHORIZONTAL);
	picker_sizer->AddStretchSpacer();
	picker_sizer->Add(screen_dropper_icon, 0, wxALIGN_CENTER|wxRIGHT, 5);
	picker_sizer->Add(screen_dropper, 0, wxALIGN_CENTER);
	picker_sizer->AddStretchSpacer();
	picker_sizer->Add(recent_sizer, 0, wxALIGN_CENTER);
	picker_sizer->AddStretchSpacer();

	wxStdDialogButtonSizer *button_sizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP);

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

	SetSizerAndFit(main_sizer);

	persist.reset(new PersistLocation(this, "Tool/Colour Picker"));

	using std::tr1::bind;
	for (int i = 0; i < 3; ++i) {
		rgb_input[i]->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, bind(&DialogColorPicker::UpdateFromRGB, this, true));
		rgb_input[i]->Bind(wxEVT_COMMAND_TEXT_UPDATED, bind(&DialogColorPicker::UpdateFromRGB, this, true));
		hsl_input[i]->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, bind(&DialogColorPicker::UpdateFromHSL, this, true));
		hsl_input[i]->Bind(wxEVT_COMMAND_TEXT_UPDATED, bind(&DialogColorPicker::UpdateFromHSL, this, true));
		hsv_input[i]->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, bind(&DialogColorPicker::UpdateFromHSV, this, true));
		hsv_input[i]->Bind(wxEVT_COMMAND_TEXT_UPDATED, bind(&DialogColorPicker::UpdateFromHSV, this, true));
	}
	ass_input->Bind(wxEVT_COMMAND_TEXT_UPDATED, bind(&DialogColorPicker::UpdateFromASS, this));
	html_input->Bind(wxEVT_COMMAND_TEXT_UPDATED, bind(&DialogColorPicker::UpdateFromHTML, this));

	screen_dropper_icon->Bind(wxEVT_MOTION, &DialogColorPicker::OnDropperMouse, this);
	screen_dropper_icon->Bind(wxEVT_LEFT_DOWN, &DialogColorPicker::OnDropperMouse, this);
	screen_dropper_icon->Bind(wxEVT_LEFT_UP, &DialogColorPicker::OnDropperMouse, this);
	screen_dropper_icon->Bind(wxEVT_MOUSE_CAPTURE_LOST, &DialogColorPicker::OnCaptureLost, this);
	Bind(wxEVT_MOTION, &DialogColorPicker::OnMouse, this);
	Bind(wxEVT_LEFT_DOWN, &DialogColorPicker::OnMouse, this);
	Bind(wxEVT_LEFT_UP, &DialogColorPicker::OnMouse, this);

	spectrum->Bind(EVT_SPECTRUM_CHANGE, &DialogColorPicker::OnSpectrumChange, this);
	slider->Bind(EVT_SPECTRUM_CHANGE, &DialogColorPicker::OnSliderChange, this);
	recent_box->Bind(EVT_RECENT_SELECT, &DialogColorPicker::OnRecentSelect, this);
	screen_dropper->Bind(EVT_DROPPER_SELECT, &DialogColorPicker::OnRecentSelect, this);

	colorspace_choice->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &DialogColorPicker::OnChangeMode, this);

	button_sizer->GetHelpButton()->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&HelpButton::OpenPage, "Colour Picker"));

	// Fill the controls
	int mode = OPT_GET("Tool/Colour Picker/Mode")->GetInt();
	if (mode < 0 || mode > 4) mode = 3; // HSL default
	colorspace_choice->SetSelection(mode);
	SetColor(initial_color);
	recent_box->LoadFromString(lagi_wxString(OPT_GET("Tool/Colour Picker/Recent")->GetString()));
}

template<int N, class Control>
wxSizer *DialogColorPicker::MakeColorInputSizer(wxString (&labels)[N], Control *(&inputs)[N])
{
	wxFlexGridSizer * sizer = new wxFlexGridSizer(2, 5, 5);
	for (int i = 0; i < N; ++i) {
		sizer->Add(new wxStaticText(this, -1, labels[i], wxDefaultPosition, wxSize(40, -1)), 1, wxALIGN_CENTER_VERTICAL|wxEXPAND);
		sizer->Add(inputs[i]);
	}
	sizer->AddGrowableCol(0,1);
	return sizer;
}


/// @brief Destructor
DialogColorPicker::~DialogColorPicker()
{
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
void DialogColorPicker::SetColor(wxColour new_color)
{
	SetRGB(new_color.Red(), new_color.Green(), new_color.Blue());
	spectrum_dirty = true;
	UpdateFromRGB();
}

/// @brief Get the currently selected color
wxColour DialogColorPicker::GetColor()
{
	recent_box->AddColor(cur_color);
	OPT_SET("Tool/Colour Picker/Recent")->SetString(STD_STR(recent_box->StoreToString()));
	return cur_color;
}

void DialogColorPicker::SetRGB(unsigned char r, unsigned char g, unsigned char b)
{
	rgb_input[0]->SetValue(r);
	rgb_input[1]->SetValue(g);
	rgb_input[2]->SetValue(b);
	cur_color = wxColour(r, g, b, wxALPHA_OPAQUE);
}

void DialogColorPicker::SetHSL(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char h, s, l;
	rgb_to_hsl(r, g, b, &h, &s, &l);
	hsl_input[0]->SetValue(h);
	hsl_input[1]->SetValue(s);
	hsl_input[2]->SetValue(l);
}

void DialogColorPicker::SetHSV(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char h, s, v;
	rgb_to_hsv(r, g, b, &h, &s, &v);
	hsv_input[0]->SetValue(h);
	hsv_input[1]->SetValue(s);
	hsv_input[2]->SetValue(v);
}

/// @brief Use the values entered in the RGB controls to update the other controls
void DialogColorPicker::UpdateFromRGB(bool dirty)
{
	unsigned char r, g, b;
	r = rgb_input[0]->GetValue();
	g = rgb_input[1]->GetValue();
	b = rgb_input[2]->GetValue();
	SetHSL(r, g, b);
	SetHSV(r, g, b);
	cur_color = wxColour(r, g, b, wxALPHA_OPAQUE);
	ass_input->ChangeValue(AssColor(cur_color).GetASSFormatted(false, false, false));
	html_input->ChangeValue(color_to_html(cur_color));

	if (dirty)
		spectrum_dirty = true;
	UpdateSpectrumDisplay();
}

/// @brief Use the values entered in the HSL controls to update the other controls
void DialogColorPicker::UpdateFromHSL(bool dirty)
{
	unsigned char r, g, b, h, s, l;
	h = hsl_input[0]->GetValue();
	s = hsl_input[1]->GetValue();
	l = hsl_input[2]->GetValue();
	hsl_to_rgb(h, s, l, &r, &g, &b);
	SetRGB(r, g, b);
	SetHSV(r, g, b);

	ass_input->ChangeValue(AssColor(cur_color).GetASSFormatted(false, false, false));
	html_input->ChangeValue(color_to_html(cur_color));

	if (dirty)
		spectrum_dirty = true;
	UpdateSpectrumDisplay();
}

/// @brief Use the values entered in the HSV controls to update the other controls
void DialogColorPicker::UpdateFromHSV(bool dirty)
{
	unsigned char r, g, b, h, s, v;
	h = hsv_input[0]->GetValue();
	s = hsv_input[1]->GetValue();
	v = hsv_input[2]->GetValue();
	hsv_to_rgb(h, s, v, &r, &g, &b);
	SetRGB(r, g, b);
	SetHSL(r, g, b);
	ass_input->ChangeValue(AssColor(cur_color).GetASSFormatted(false, false, false));
	html_input->ChangeValue(color_to_html(cur_color));

	if (dirty)
		spectrum_dirty = true;
	UpdateSpectrumDisplay();
}

/// @brief Use the value entered in the ASS hex control to update the other controls
void DialogColorPicker::UpdateFromASS()
{
	unsigned char r, g, b;
	AssColor ass;
	ass.Parse(ass_input->GetValue());
	r = ass.r;
	g = ass.g;
	b = ass.b;
	SetRGB(r, g, b);
	SetHSL(r, g, b);
	SetHSV(r, g, b);
	html_input->ChangeValue(color_to_html(cur_color));

	spectrum_dirty = true;
	UpdateSpectrumDisplay();
}

/// @brief Use the value entered in the HTML hex control to update the other controls
void DialogColorPicker::UpdateFromHTML()
{
	unsigned char r, g, b;
	cur_color = html_to_color(html_input->GetValue());
	r = cur_color.Red();
	g = cur_color.Green();
	b = cur_color.Blue();
	SetRGB(r, g, b);
	SetHSL(r, g, b);
	SetHSV(r, g, b);
	ass_input->ChangeValue(AssColor(cur_color).GetASSFormatted(false, false, false));

	spectrum_dirty = true;
	UpdateSpectrumDisplay();
}

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

wxBitmap *DialogColorPicker::MakeGBSpectrum()
{
	delete rgb_spectrum[0];

	wxImage spectrum_image(256, 256, false);
	unsigned char *ospec, *spec;

	ospec = spec = (unsigned char *)malloc(256*256*3);
	if (!spec) throw std::bad_alloc();

	for (int g = 0; g < 256; g++) {
		for (int b = 0; b < 256; b++) {
			*spec++ = cur_color.Red();
			*spec++ = g;
			*spec++ = b;
		}
	}
	spectrum_image.SetData(ospec);

	return rgb_spectrum[0] = new wxBitmap(spectrum_image);
}

wxBitmap *DialogColorPicker::MakeRBSpectrum()
{
	delete rgb_spectrum[1];

	unsigned char *ospec, *spec;
	ospec = spec = (unsigned char *)malloc(256*256*3);
	if (!spec) throw std::bad_alloc();

	for (int r = 0; r < 256; r++) {
		for (int b = 0; b < 256; b++) {
			*spec++ = r;
			*spec++ = cur_color.Green();
			*spec++ = b;
		}
	}

	wxImage spectrum_image(256, 256, ospec);
	return rgb_spectrum[1] = new wxBitmap(spectrum_image);
}

wxBitmap *DialogColorPicker::MakeRGSpectrum()
{
	delete rgb_spectrum[2];

	unsigned char *ospec, *spec;
	ospec = spec = (unsigned char *)malloc(256*256*3);
	if (!spec) throw std::bad_alloc();

	for (int r = 0; r < 256; r++) {
		for (int g = 0; g < 256; g++) {
			*spec++ = r;
			*spec++ = g;
			*spec++ = cur_color.Blue();
		}
	}

	wxImage spectrum_image(256, 256, ospec);
	return rgb_spectrum[2] = new wxBitmap(spectrum_image);
}

wxBitmap *DialogColorPicker::MakeHSSpectrum()
{
	delete hsl_spectrum;

	unsigned char *ospec, *spec;
	ospec = spec = (unsigned char *)malloc(256*256*3);
	if (!spec) throw std::bad_alloc();

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

	wxImage spectrum_image(256, 256, ospec);
	return hsl_spectrum = new wxBitmap(spectrum_image);
}

wxBitmap *DialogColorPicker::MakeSVSpectrum()
{
	delete hsv_spectrum;

	unsigned char *ospec, *spec;
	ospec = spec = (unsigned char *)malloc(256*256*3);
	if (!spec) throw std::bad_alloc();

	int h = hsv_input[0]->GetValue();
	unsigned char maxr, maxg, maxb;
	hsv_to_rgb(h, 255, 255, &maxr, &maxg, &maxb);

	for (int v = 0; v < 256; v++) {
		int rr = (255-maxr) * v / 256;
		int rg = (255-maxg) * v / 256;
		int rb = (255-maxb) * v / 256;
		for (int s = 0; s < 256; s++) {
			*spec++ = 255 - rr * s / 256 - (255-v);
			*spec++ = 255 - rg * s / 256 - (255-v);
			*spec++ = 255 - rb * s / 256 - (255-v);
		}
	}

	wxImage spectrum_image(256, 256, ospec);
	return hsv_spectrum = new wxBitmap(spectrum_image);
}

void DialogColorPicker::OnChangeMode(wxCommandEvent &)
{
	spectrum_dirty = true;
	OPT_SET("Tool/Colour Picker/Mode")->SetInt(colorspace_choice->GetSelection());
	UpdateSpectrumDisplay();
}

void DialogColorPicker::OnSpectrumChange(wxCommandEvent &)
{
	int i = colorspace_choice->GetSelection();
	switch (i) {
		case 0:
			rgb_input[2]->SetValue(spectrum->GetX());
			rgb_input[1]->SetValue(spectrum->GetY());
			break;
		case 1:
			rgb_input[2]->SetValue(spectrum->GetX());
			rgb_input[0]->SetValue(spectrum->GetY());
			break;
		case 2:
			rgb_input[1]->SetValue(spectrum->GetX());
			rgb_input[0]->SetValue(spectrum->GetY());
			break;
		case 3:
			hsl_input[1]->SetValue(spectrum->GetX());
			hsl_input[0]->SetValue(spectrum->GetY());
			break;
		case 4:
			hsv_input[1]->SetValue(spectrum->GetX());
			hsv_input[2]->SetValue(spectrum->GetY());
			break;
	}

	switch (i) {
		case 0: case 1: case 2:
			UpdateFromRGB(false);
			break;
		case 3:
			UpdateFromHSL(false);
			break;
		case 4:
			UpdateFromHSV(false);
			break;
	}
}

void DialogColorPicker::OnSliderChange(wxCommandEvent &)
{
	spectrum_dirty = true;
	int i = colorspace_choice->GetSelection();
	switch (i) {
		case 0: case 1: case 2:
			rgb_input[i]->SetValue(slider->GetY());
			UpdateFromRGB(false);
			break;
		case 3:
			hsl_input[2]->SetValue(slider->GetY());
			UpdateFromHSL(false);
			break;
		case 4:
			hsv_input[0]->SetValue(slider->GetY());
			UpdateFromHSV(false);
			break;
	}
}

void DialogColorPicker::OnRecentSelect(wxCommandEvent &evt)
{
	// The colour picked is stored in the event string
	// Allows this event handler to be shared by recent and dropper controls
	// Ugly hack?
	AssColor color;
	color.Parse(evt.GetString());
	SetColor(color.GetWXColor());
}

void DialogColorPicker::OnDropperMouse(wxMouseEvent &evt)
{
	if (evt.LeftDown() && !screen_dropper_icon->HasCapture()) {
#ifdef WIN32
		screen_dropper_icon->SetCursor(wxCursor("eyedropper_cursor"));
#else
		screen_dropper_icon->SetCursor(*wxCROSS_CURSOR);
#endif
		screen_dropper_icon->SetBitmap(wxNullBitmap);
		screen_dropper_icon->CaptureMouse();
		eyedropper_grab_point = evt.GetPosition();
		eyedropper_is_grabbed = false;
	}

	if (evt.LeftUp()) {
		wxPoint ptdiff = evt.GetPosition() - eyedropper_grab_point;
		bool release_now = eyedropper_is_grabbed || abs(ptdiff.x) + abs(ptdiff.y) > 7;
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

void DialogColorPicker::OnCaptureLost(wxMouseCaptureLostEvent&) {
	eyedropper_is_grabbed = false;
	screen_dropper_icon->SetCursor(wxNullCursor);
	screen_dropper_icon->SetBitmap(eyedropper_bitmap);
}

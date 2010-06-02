// Copyright (c) 2005-2010, Rodrigo Braz Monteiro
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

/// @file video_display.h
/// @see video_display.cpp
/// @ingroup video main_ui
///


#ifndef AGI_PRE
#include <wx/glcanvas.h>
#include <wx/combobox.h>
#include <wx/textctrl.h>
#include <memory>
#endif

// Prototypes
class VideoSlider;
class VideoBox;
class VideoOutGL;
class IVisualTool;

struct VideoState {
	int x;
	int y;
	int w;
	int h;
	VideoState() : x(-1), y(-1), w(-1), h(-1) { }
};

/// @class VideoDisplay
/// @brief DOCME
class VideoDisplay: public wxGLCanvas {
	/// The unscaled size of the displayed video
	wxSize origSize;

	/// The frame number currently being displayed
	int currentFrame;

	/// The width of the canvas in screen pixels
	int w;
	/// The height of the canvas in screen pixels
	int h;

	/// Screen pixels between the left of the canvas and the left of the video
	int viewport_x;
	/// The width of the video in screen pixels
	int viewport_width;
	/// Screen pixels between the bottom of the canvas and the bottom of the video; used for glViewport
	int viewport_bottom;
	/// Screen pixels between the bottom of the canvas and the top of the video; used for coordinate space conversion
	int viewport_top;
	/// The height of the video in screen pixels
	int viewport_height;

	/// Lock to disable mouse updates during resize operations
	bool locked;

	/// @brief Draw an overscan mask
	/// @param sizeH  The amount of horizontal overscan on one side
	/// @param sizeV  The amount of vertical overscan on one side
	/// @param colour The color of the mask
	/// @param alpha  The alpha of the mask
	void DrawOverscanMask(int sizeH, int sizeV, wxColor color, double alpha) const;

	/// @brief Paint event 
	void OnPaint(wxPaintEvent& event);
	/// @brief Key event handler
	/// @param event 
	void OnKey(wxKeyEvent &event);
	/// @brief Mouse event handler
	/// @param event 
	void OnMouseEvent(wxMouseEvent& event);

	/// @brief NOP event handler
	void OnEraseBackground(wxEraseEvent &) {}
	/// @brief Recalculate video positioning and scaling when the available area or zoom changes
	/// @param event
	void OnSizeEvent(wxSizeEvent &event);

	/// @brief Copy coordinates of the mouse to the clipboard
	void OnCopyCoords(wxCommandEvent &);
	/// @brief Copy the currently display frame to the clipboard, with subtitles
	void OnCopyToClipboard(wxCommandEvent &);
	/// @brief Save the currently display frame to a file, with subtitles
	void OnSaveSnapshot(wxCommandEvent &);
	/// @brief Copy the currently display frame to the clipboard, without subtitles
	void OnCopyToClipboardRaw(wxCommandEvent &);
	/// @brief Save the currently display frame to a file, without subtitles
	void OnSaveSnapshotRaw(wxCommandEvent &);

	/// The current zoom level, where 1.0 = 100%
	double zoomValue;

	/// The video position slider
	VideoSlider *ControlSlider;

	/// The display for the the video position relative to the current subtitle line
	wxTextCtrl *SubsPosition;

	/// The display for the absolute time of the video position
	wxTextCtrl *PositionDisplay;

	/// The video renderer
	std::auto_ptr<VideoOutGL> videoOut;

	/// The active visual typesetting tool
	std::auto_ptr<IVisualTool> tool;
	/// The current tool's ID
	int activeMode;
	/// The toolbar used by individual typesetting tools
	wxToolBar* toolBar;


	void OnMode(const wxCommandEvent &event);
	void SetMode(int mode);
	/// @brief Switch the active tool to a new object of the specified class
	/// @param T The class of the new visual typsetting tool
	template <class T> void SetTool();

	/// The current script width
	int scriptW;
	/// The current script height
	int scriptH;

	VideoState video;

public:
	/// The VideoBox this display is contained in
	VideoBox *box;

	/// The dropdown box for selecting zoom levels
	wxComboBox *zoomBox;

	/// Whether the display can be freely resized by the user
	bool freeSize;

	/// @brief Constructor
	/// @param parent Pointer to a parent window.
	/// @param id     Window identifier. If -1, will automatically create an identifier.
	/// @param pos    Window position. wxDefaultPosition is (-1, -1) which indicates that wxWidgets should generate a default position for the window.
	/// @param size   Window size. wxDefaultSize is (-1, -1) which indicates that wxWidgets should generate a default size for the window. If no suitable size can be found, the window will be sized to 20x20 pixels so that the window is visible but obviously not correctly sized.
	/// @param style  Window style.
	/// @param name   Window name.
	VideoDisplay(VideoBox *box, VideoSlider *ControlSlider, wxTextCtrl *PositionDisplay, wxTextCtrl *SubsPosition, wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr);
	~VideoDisplay();
	/// @brief Reset the size of the display to the video size
	void Reset();

	/// @brief Set this video display to the given frame
	/// @frameNumber The desired frame number
	void SetFrame(int frameNumber);
	/// @brief Get the number of the currently displayed framed
	int GetFrame() const { return currentFrame; }
	/// @brief Set the range of valid frame numbers for the slider
	/// @from Minimum frame number
	/// @to Maximum frame number; must be >= from or strange things may happen
	void SetFrameRange(int from, int to);

	/// @brief Render the currently visible frame
	void Render();

	/// @brief Set the cursor to either default or blank
	/// @param show Whether or not the cursor should be visible
	void ShowCursor(bool show);
	/// @brief Set the size of the display based on the current zoom and video resolution
	void UpdateSize();
	/// @brief Set the zoom level
	/// @param value The new zoom level
	void SetZoom(double value);
	/// @brief Set the zoom level to that indicated by the dropdown
	void SetZoomFromBox();
	/// @brief Get the current zoom level
	double GetZoom() const;

	/// @brief Convert a point from screen to script coordinate frame
	/// @param x x coordinate; in/out
	/// @param y y coordinate; in/out
	void ToScriptCoords(int *x, int *y) const;
	/// @brief Convert a point from script to screen coordinate frame
	/// @param x x coordinate; in/out
	/// @param y y coordinate; in/out
	void FromScriptCoords(int *x, int *y) const;


	DECLARE_EVENT_TABLE()
};

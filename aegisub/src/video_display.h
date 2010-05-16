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
class VisualTool;
class VideoBox;
class VideoOutGL;

/// @class VideoDisplay
/// @brief DOCME
class VideoDisplay: public wxGLCanvas {
private:
	/// The current visual typesetting mode
	int visualMode;

	/// The unscaled size of the displayed video
	wxSize origSize;

	/// The frame number currently being displayed
	int currentFrame;

	/// The width of the display
	int w;
	/// The height of the display
	int h;

	/// The x-coordinate of the bottom left of the area containing video.
	/// Always zero unless the display is detatched and is wider than the video.
	int dx1;
	/// The width of the screen area containing video
	int dx2;
	/// The y-coordinate of the bottom left of the area containing video.
	/// Always zero unless the display is detatched and is taller than the video.
	int dy1;
	/// The height of the screen area containing video
	int dy2;

	/// The last seen x position of the mouse; stored for some context menu commands
	int mouse_x;
	/// The last seen y position of the mouse; stored for some context menu commands
	int mouse_y;

	/// Lock to disable mouse updates during resize operations
	bool locked;

	/// @brief Draw the appropriate overscan masks for the current aspect ratio
	void DrawTVEffects(int sh, int sw);
	/// @brief Draw an overscan mask 
	/// @param sizeH  The amount of horizontal overscan on one side
	/// @param sizeV  The amount of vertical overscan on one side
	/// @param colour The color of the mask
	/// @param alpha  The alpha of the mask
	void DrawOverscanMask(int sizeH,int sizeV,wxColour color,double alpha=0.5);

	/// @brief Paint event 
	void OnPaint(wxPaintEvent& event);
	/// @brief Handle keypress events for switching visual typesetting modes
	/// @param event
	void OnKey(wxKeyEvent &event);
	/// @brief Handle mouse events
	/// @param event 
	void OnMouseEvent(wxMouseEvent& event);

	/// @brief NOP event handler
	void OnEraseBackground(wxEraseEvent &event) {}
	/// @brief Handle resize events
	/// @param event
	void OnSizeEvent(wxSizeEvent &event);

	/// @brief Copy coordinates of the mouse to the clipboard
	void OnCopyCoords(wxCommandEvent &event);
	/// @brief Copy the currently display frame to the clipboard, with subtitles
	void OnCopyToClipboard(wxCommandEvent &event);
	/// @brief Save the currently display frame to a file, with subtitles
	void OnSaveSnapshot(wxCommandEvent &event);
	/// @brief Copy the currently display frame to the clipboard, without subtitles
	void OnCopyToClipboardRaw(wxCommandEvent &event);
	/// @brief Save the currently display frame to a file, without subtitles
	void OnSaveSnapshotRaw(wxCommandEvent &event);

	/// The current zoom level, where 1.0 = 100%
	double zoomValue;

	/// The video position slider
	VideoSlider *ControlSlider;

	/// The display for the the video position relative to the current subtitle line
	wxTextCtrl *SubsPosition;

	/// The display for the absolute time of the video position
	wxTextCtrl *PositionDisplay;

	/// The current visual typesetting tool
	VisualTool *visual;

	/// The video renderer
	std::auto_ptr<VideoOutGL> videoOut;

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
	/// @brief Convert mouse coordinates relative to the display to coordinates relative to the video
	/// @param x X coordinate
	/// @param y Y coordinate
	void ConvertMouseCoords(int &x,int &y);
	/// @brief Set the size of the display based on the current zoom and video resolution
	void UpdateSize();
	/// @brief Set the zoom level
	/// @param value The new zoom level
	void SetZoom(double value);
	/// @brief Set the zoom level to that indicated by the dropdown
	void SetZoomFromBox();
	/// @brief Get the current zoom level
	double GetZoom();
	/// @brief Set the current visual typesetting mode
	/// @param mode The new mode
	/// @param render Whether the display should be rerendered
	void SetVisualMode(int mode, bool render = false);

	/// @brief Event handler for the secondary toolbar which some visual tools use
	void OnSubTool(wxCommandEvent &event);

	DECLARE_EVENT_TABLE()
};

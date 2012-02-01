// Copyright (c) 2005, Rodrigo Braz Monteiro
// Copyright (c) 2009-2010, Niels Martin Hansen
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

/// @file audio_display.cpp
/// @brief Display audio in the main UI
/// @ingroup audio_ui
///

#include "config.h"

#ifndef AGI_PRE
#include <algorithm>

#include <wx/dcbuffer.h>
#include <wx/dcclient.h>
#include <wx/mousestate.h>
#endif

#include "ass_time.h"
#include "audio_colorscheme.h"
#include "audio_controller.h"
#include "audio_display.h"
#include "audio_renderer.h"
#include "audio_renderer_spectrum.h"
#include "audio_renderer_waveform.h"
#include "audio_timing.h"
#include "block_cache.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"
#include "main.h"
#include "selection_controller.h"
#include "utils.h"
#include "video_context.h"

/// @brief Colourscheme-based UI colour provider
///
/// This class provides UI colours corresponding to the supplied audio colour
/// scheme.
///
/// SetColourScheme must be called to set the active colour scheme before
/// colours can be retrieved
class UIColours {
	wxColour light_colour;         ///< Light unfocused colour from the colour scheme
	wxColour dark_colour;          ///< Dark unfocused colour from the colour scheme
	wxColour sel_colour;           ///< Selection unfocused colour from the colour scheme
	wxColour light_focused_colour; ///< Light focused colour from the colour scheme
	wxColour dark_focused_colour;  ///< Dark focused colour from the colour scheme
	wxColour sel_focused_colour;   ///< Selection focused colour from the colour scheme

	bool focused; ///< Use the focused colours?
public:
	/// Constructor
	UIColours() : focused(false) { }

	/// Set the colour scheme to load colours from
	/// @param name Name of the colour scheme
	void SetColourScheme(std::string const& name)
	{
		std::string opt_prefix = "Colour/Schemes/" + name + "/UI/";
		light_colour = lagi_wxColour(OPT_GET(opt_prefix + "Light")->GetColour());
		dark_colour = lagi_wxColour(OPT_GET(opt_prefix + "Dark")->GetColour());
		sel_colour = lagi_wxColour(OPT_GET(opt_prefix + "Selection")->GetColour());

		opt_prefix = "Colour/Schemes/" + name + "/UI Focused/";
		light_focused_colour = lagi_wxColour(OPT_GET(opt_prefix + "Light")->GetColour());
		dark_focused_colour = lagi_wxColour(OPT_GET(opt_prefix + "Dark")->GetColour());
		sel_focused_colour = lagi_wxColour(OPT_GET(opt_prefix + "Selection")->GetColour());
	}

	/// Set whether to use the focused or unfocused colours
	/// @param focused If true, focused colours will be returned
	void SetFocused(bool focused) { this->focused = focused; }

	/// Get the current Light colour
	wxColour Light() const { return focused ? light_focused_colour : light_colour; }
	/// Get the current Dark colour
	wxColour Dark() const { return focused ? dark_focused_colour : dark_colour; }
	/// Get the current Selection colour
	wxColour Selection() const { return focused ? sel_focused_colour : sel_colour; }
};

class AudioDisplayScrollbar : public AudioDisplayInteractionObject {
	static const int height = 10;
	static const int min_width = 10;

	wxRect bounds;
	wxRect thumb;

	bool dragging;   ///< user is dragging with the primary mouse button

	int data_length; ///< total amount of data in control
	int page_length; ///< amount of data in one page
	int position;    ///< first item displayed

	int sel_start;   ///< first data item in selection
	int sel_length;  ///< number of data items in selection

	UIColours colours; ///< Colour provider

	/// Containing display to send scroll events to
	AudioDisplay *display;

	// Recalculate thumb bounds from position and length data
	void RecalculateThumb()
	{
		thumb.width = std::max<int>(min_width, (int64_t)bounds.width * page_length / data_length);
		thumb.height = height;
		thumb.x = int((int64_t)bounds.width * position / data_length);
		thumb.y = bounds.y;
	}

public:
	AudioDisplayScrollbar(AudioDisplay *display)
	: dragging(false)
	, data_length(1)
	, page_length(1)
	, position(0)
	, sel_start(-1)
	, sel_length(0)
	, display(display)
	{
	}

	/// The audio display has changed size
	void SetDisplaySize(const wxSize &display_size)
	{
		bounds.x = 0;
		bounds.y = display_size.y - height;
		bounds.width = display_size.x;
		bounds.height = height;
		page_length = display_size.x;

		RecalculateThumb();
	}

	void SetColourScheme(std::string const& name)
	{
		colours.SetColourScheme(name);
	}

	const wxRect & GetBounds() const { return bounds; }
	int GetPosition() const { return position; }

	int SetPosition(int new_position)
	{
		// These two conditionals can't be swapped, otherwise the position can become
		// negative if the entire data is shorter than one page.
		if (new_position + page_length >= data_length)
			new_position = data_length - page_length - 1;
		if (new_position < 0)
			new_position = 0;

		position = new_position;
		RecalculateThumb();

		return position;
	}

	void SetSelection(int new_start, int new_length)
	{
		sel_start = (int64_t)new_start * bounds.width / data_length;
		sel_length = (int64_t)new_length * bounds.width / data_length;
	}

	void ChangeLengths(int new_data_length, int new_page_length)
	{
		data_length = new_data_length;
		page_length = new_page_length;

		RecalculateThumb();
	}

	bool OnMouseEvent(wxMouseEvent &event)
	{
		if (event.LeftIsDown())
		{
			const int thumb_left = event.GetPosition().x - thumb.width/2;
			const int data_length_less_page = data_length - page_length;
			const int shaft_length_less_thumb = bounds.width - thumb.width;

			display->ScrollPixelToLeft((int64_t)data_length_less_page * thumb_left / shaft_length_less_thumb);

			dragging = true;
		}
		else if (event.LeftUp())
		{
			dragging = false;
		}

		return dragging;
	}

	void Paint(wxDC &dc, bool has_focus)
	{
		colours.SetFocused(has_focus);

		dc.SetPen(wxPen(colours.Light()));
		dc.SetBrush(wxBrush(colours.Dark()));
		dc.DrawRectangle(bounds);

		if (sel_length > 0 && sel_start >= 0)
		{
			dc.SetPen(wxPen(colours.Selection()));
			dc.SetBrush(wxBrush(colours.Selection()));
			dc.DrawRectangle(wxRect(sel_start, bounds.y, sel_length, bounds.height));
		}

		dc.SetPen(wxPen(colours.Light()));
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(bounds);

		dc.SetPen(wxPen(colours.Light()));
		dc.SetBrush(wxBrush(colours.Light()));
		dc.DrawRectangle(thumb);
	}
};

const int AudioDisplayScrollbar::min_width;


class AudioDisplayTimeline : public AudioDisplayInteractionObject {
	int duration;        ///< Total duration in ms
	double ms_per_pixel; ///< Milliseconds per pixel
	int pixel_left;      ///< Leftmost visible pixel (i.e. scroll position)

	wxRect bounds;

	wxPoint drag_lastpos;
	bool dragging;

	enum Scale {
		Sc_Millisecond,
		Sc_Centisecond,
		Sc_Decisecond,
		Sc_Second,
		Sc_Decasecond,
		Sc_Minute,
		Sc_Decaminute,
		Sc_Hour,
		Sc_Decahour, // If anyone needs this they should reconsider their project
		Sc_MAX = Sc_Decahour
	};
	Scale scale_minor;
	int scale_major_modulo; ///< If minor_scale_mark_index % scale_major_modulo == 0 the mark is a major mark
	double scale_minor_divisor; ///< Absolute scale-mark index multiplied by this number gives sample index for scale mark

	AudioDisplay *display; ///< Containing audio display

	UIColours colours; ///< Colour provider

public:
	AudioDisplayTimeline(AudioDisplay *display)
	: duration(0)
	, ms_per_pixel(1.0)
	, pixel_left(0)
	, dragging(false)
	, display(display)
	{
		int width, height;
		display->GetTextExtent("0123456789:.", &width, &height);
		bounds.height = height + 4;
	}

	void SetColourScheme(std::string const& name)
	{
		colours.SetColourScheme(name);
	}

	void SetDisplaySize(const wxSize &display_size)
	{
		// The size is without anything that goes below the timeline (like scrollbar)
		bounds.width = display_size.x;
		bounds.x = 0;
		bounds.y = 0;
	}

	int GetHeight() const { return bounds.height; }
	const wxRect & GetBounds() const { return bounds; }

	void ChangeAudio(int new_duration)
	{
		duration = new_duration;
	}

	void ChangeZoom(double new_ms_per_pixel)
	{
		ms_per_pixel = new_ms_per_pixel;

		double px_sec = 1000.0 / ms_per_pixel;

		if (px_sec > 3000) {
			scale_minor = Sc_Millisecond;
			scale_minor_divisor = 1.0;
			scale_major_modulo = 10;
		} else if (px_sec > 300) {
			scale_minor = Sc_Centisecond;
			scale_minor_divisor = 10.0;
			scale_major_modulo = 10;
		} else if (px_sec > 30) {
			scale_minor = Sc_Decisecond;
			scale_minor_divisor = 100.0;
			scale_major_modulo = 10;
		} else if (px_sec > 3) {
			scale_minor = Sc_Second;
			scale_minor_divisor = 1000.0;
			scale_major_modulo = 10;
		} else if (px_sec > 1.0/3.0) {
			scale_minor = Sc_Decasecond;
			scale_minor_divisor = 10000.0;
			scale_major_modulo = 6;
		} else if (px_sec > 1.0/9.0) {
			scale_minor = Sc_Minute;
			scale_minor_divisor = 60000.0;
			scale_major_modulo = 10;
		} else if (px_sec > 1.0/90.0) {
			scale_minor = Sc_Decaminute;
			scale_minor_divisor = 600000.0;
			scale_major_modulo = 6;
		} else {
			scale_minor = Sc_Hour;
			scale_minor_divisor = 3600000.0;
			scale_major_modulo = 10;
		}
	}

	void SetPosition(int new_pixel_left)
	{
		pixel_left = std::max(new_pixel_left, 0);
	}

	bool OnMouseEvent(wxMouseEvent &event)
	{
		if (event.LeftDown())
		{
			drag_lastpos = event.GetPosition();
			dragging = true;
		}
		else if (event.LeftIsDown())
		{
			display->ScrollPixelToLeft(pixel_left - event.GetPosition().x + drag_lastpos.x);

			drag_lastpos = event.GetPosition();
			dragging = true;
		}
		else if (event.LeftUp())
		{
			dragging = false;
		}

		return dragging;
	}

	void Paint(wxDC &dc)
	{
		int bottom = bounds.y + bounds.height;

		// Background
		dc.SetPen(wxPen(colours.Dark()));
		dc.SetBrush(wxBrush(colours.Dark()));
		dc.DrawRectangle(bounds);

		// Top line
		dc.SetPen(wxPen(colours.Light()));
		dc.DrawLine(bounds.x, bottom-1, bounds.x+bounds.width, bottom-1);

		// Prepare for writing text
		dc.SetTextBackground(colours.Dark());
		dc.SetTextForeground(colours.Light());

		// Figure out the first scale mark to show
		int ms_left = int(pixel_left * ms_per_pixel);
		int next_scale_mark = int(ms_left / scale_minor_divisor);
		if (next_scale_mark * scale_minor_divisor < ms_left)
			next_scale_mark += 1;
		assert(next_scale_mark * scale_minor_divisor >= ms_left);

		// Draw scale marks
		int next_scale_mark_pos;
		int last_text_right = -1;
		int last_hour = -1, last_minute = -1;
		if (duration < 3600) last_hour = 0; // Trick to only show hours if audio is longer than 1 hour
		do {
			next_scale_mark_pos = int(next_scale_mark * scale_minor_divisor / ms_per_pixel) - pixel_left;
			bool mark_is_major = next_scale_mark % scale_major_modulo == 0;

			if (mark_is_major)
				dc.DrawLine(next_scale_mark_pos, bottom-6, next_scale_mark_pos, bottom-1);
			else
				dc.DrawLine(next_scale_mark_pos, bottom-4, next_scale_mark_pos, bottom-1);

			// Print time labels on major scale marks
			if (mark_is_major && next_scale_mark_pos > last_text_right)
			{
				double mark_time = next_scale_mark * scale_minor_divisor / 1000.0;
				int mark_hour = (int)(mark_time / 3600);
				int mark_minute = (int)(mark_time / 60) % 60;
				double mark_second = mark_time - mark_hour*3600 - mark_minute*60;

				wxString time_string;
				bool changed_hour = mark_hour != last_hour;
				bool changed_minute = mark_minute != last_minute;

				if (changed_hour)
				{
					time_string = wxString::Format("%d:%02d:", mark_hour, mark_minute);
					last_hour = mark_hour;
					last_minute = mark_minute;
				}
				else if (changed_minute)
				{
					time_string = wxString::Format("%d:", mark_minute);
					last_minute = mark_minute;
				}
				if (scale_minor >= Sc_Decisecond)
					time_string += wxString::Format("%02d", (int)mark_second);
				else if (scale_minor == Sc_Centisecond)
					time_string += wxString::Format("%02.1f", mark_second);
				else
					time_string += wxString::Format("%02.2f", mark_second);

				int tw, th;
				dc.GetTextExtent(time_string, &tw, &th);
				last_text_right = next_scale_mark_pos + tw;

				dc.DrawText(time_string, next_scale_mark_pos, 0);
			}

			next_scale_mark += 1;

		} while (next_scale_mark_pos < bounds.width);
	}
};

class AudioMarkerInteractionObject : public AudioDisplayInteractionObject {
	// Object-pair being interacted with
	AudioMarker *marker;
	AudioTimingController *timing_controller;
	// Audio display drag is happening on
	AudioDisplay *display;
	// Audio controller managing it all
	AudioController *controller;
	// Mouse button used to initiate the drag
	wxMouseButton button_used;
	// Default to snapping to snappable markers
	bool default_snap;
	// Range in pixels to snap at
	int snap_range;

public:
	AudioMarkerInteractionObject(AudioMarker *marker, AudioTimingController *timing_controller, AudioDisplay *display, AudioController *controller, wxMouseButton button_used)
	: marker(marker)
	, timing_controller(timing_controller)
	, display(display)
	, controller(controller)
	, button_used(button_used)
	, default_snap(OPT_GET("Audio/Snap/Enable")->GetBool())
	, snap_range(OPT_GET("Audio/Snap/Distance")->GetInt())
	{
	}

	bool OnMouseEvent(wxMouseEvent &event)
	{
		if (event.Dragging())
		{
			timing_controller->OnMarkerDrag(
				marker,
				display->TimeFromRelativeX(event.GetPosition().x),
				default_snap != event.ShiftDown() ? display->TimeFromAbsoluteX(snap_range) : 0);
		}

		// We lose the marker drag if the button used to initiate it goes up
		return !event.ButtonUp(button_used);
	}
};

class AudioStyleRangeMerger : public AudioRenderingStyleRanges {
	typedef std::map<int, AudioRenderingStyle> style_map;
public:
	typedef style_map::iterator iterator;

private:
	style_map points;

	void Split(int point)
	{
		iterator it = points.lower_bound(point);
		if (it == points.end() || it->first != point)
		{
			assert(it != points.begin());
			points[point] = (--it)->second;
		}
	}

	void Restyle(int start, int end, AudioRenderingStyle style)
	{
		assert(points.lower_bound(end) != points.end());
		for (iterator pt = points.lower_bound(start); pt->first < end; ++pt)
		{
			if (style > pt->second)
				pt->second = style;
		}
	}

public:
	AudioStyleRangeMerger()
	{
		points[0] = AudioStyle_Normal;
	}

	void AddRange(int start, int end, AudioRenderingStyle style)
	{

		if (start < 0) start = 0;
		if (end < start) return;

		Split(start);
		Split(end);
		Restyle(start, end, style);
	}

	iterator begin() { return points.begin(); }
	iterator end() { return points.end(); }
};

AudioDisplay::AudioDisplay(wxWindow *parent, AudioController *controller, agi::Context *context)
: wxWindow(parent, -1, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS|wxBORDER_SIMPLE)
, audio_open_connection(controller->AddAudioOpenListener(&AudioDisplay::OnAudioOpen, this))
, context(context)
, audio_renderer(new AudioRenderer)
, controller(controller)
, scrollbar(new AudioDisplayScrollbar(this))
, timeline(new AudioDisplayTimeline(this))
, dragged_object(0)
{
	style_ranges[0] = AudioStyle_Normal;

	scroll_left = 0;
	pixel_audio_width = 0;
	scale_amplitude = 1.0;

	track_cursor_pos = -1;

	audio_renderer->SetAmplitudeScale(scale_amplitude);
	SetZoomLevel(0);

	SetMinClientSize(wxSize(-1, 70));
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	SetThemeEnabled(false);

	Bind(wxEVT_LEFT_DOWN, &AudioDisplay::OnMouseEvent, this);
	Bind(wxEVT_MIDDLE_DOWN, &AudioDisplay::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_DOWN, &AudioDisplay::OnMouseEvent, this);
	Bind(wxEVT_LEFT_UP, &AudioDisplay::OnMouseEvent, this);
	Bind(wxEVT_MIDDLE_UP, &AudioDisplay::OnMouseEvent, this);
	Bind(wxEVT_RIGHT_UP, &AudioDisplay::OnMouseEvent, this);
	Bind(wxEVT_MOTION, &AudioDisplay::OnMouseEvent, this);
	Bind(wxEVT_ENTER_WINDOW, &AudioDisplay::OnMouseEvent, this);
	Bind(wxEVT_LEAVE_WINDOW, &AudioDisplay::OnMouseEvent, this);
}


AudioDisplay::~AudioDisplay()
{
}


void AudioDisplay::ScrollBy(int pixel_amount)
{
	ScrollPixelToLeft(scroll_left + pixel_amount);
}


void AudioDisplay::ScrollPixelToLeft(int pixel_position)
{
	const int client_width = GetClientRect().GetWidth();

	if (pixel_position + client_width >= pixel_audio_width)
		pixel_position = pixel_audio_width - client_width;
	if (pixel_position < 0)
		pixel_position = 0;

	scroll_left = pixel_position;
	scrollbar->SetPosition(scroll_left);
	timeline->SetPosition(scroll_left);
	Refresh();
}


void AudioDisplay::ScrollTimeRangeInView(const TimeRange &range)
{
	int client_width = GetClientRect().GetWidth();
	int range_begin = AbsoluteXFromTime(range.begin());
	int range_end = AbsoluteXFromTime(range.end());
	int range_len = range_end - range_begin;

	// Is everything already in view?
	if (range_begin >= scroll_left && range_end <= scroll_left+client_width)
		return;

	// For the rest of the calculation, remove 5 % from each side of the client area.
	// The leftadjust is the amount to subtract from the final scroll_left value.
	int leftadjust = client_width / 20;
	client_width = client_width * 9 / 10;

	// The entire range can fit inside the view, center it
	if (range_len < client_width)
	{
		ScrollPixelToLeft(range_begin - (client_width-range_len)/2 - leftadjust);
	}

	// Range doesn't fit in view and we're viewing a middle part of it, just leave it alone
	else if (range_begin < scroll_left+leftadjust && range_end > scroll_left+leftadjust+client_width)
	{
		// nothing
	}

	// Right edge is in view, scroll it as far to the right as possible
	else if (range_end >= scroll_left+leftadjust && range_end < scroll_left+leftadjust+client_width)
	{
		ScrollPixelToLeft(range_end - client_width - leftadjust);
	}

	// Nothing is in view or the left edge is in view, scroll left edge as far to the left as possible
	else
	{
		ScrollPixelToLeft(range_begin - leftadjust);
	}
}

void AudioDisplay::SetZoomLevel(int new_zoom_level)
{
	zoom_level = new_zoom_level;

	const int factor = GetZoomLevelFactor(zoom_level);
	const int base_pixels_per_second = 50; /// @todo Make this customisable
	const double base_ms_per_pixel = 1000.0 / base_pixels_per_second;
	const double new_ms_per_pixel = 100.0 * base_ms_per_pixel / factor;

	if (ms_per_pixel != new_ms_per_pixel)
	{
		int client_width = GetClientSize().GetWidth();
		double center_time = (scroll_left + client_width / 2) * ms_per_pixel;

		ms_per_pixel = new_ms_per_pixel;
		pixel_audio_width = std::max(1, int(controller->GetDuration() / ms_per_pixel));

		audio_renderer->SetMillisecondsPerPixel(ms_per_pixel);
		scrollbar->ChangeLengths(pixel_audio_width, client_width);
		timeline->ChangeZoom(ms_per_pixel);

		ScrollTimeToCenter(center_time);
		Refresh();
	}
}


int AudioDisplay::GetZoomLevel() const
{
	return zoom_level;
}


wxString AudioDisplay::GetZoomLevelDescription(int level) const
{
	const int factor = GetZoomLevelFactor(level);
	const int base_pixels_per_second = 50; /// @todo Make this customisable along with the above
	const int second_pixels = 100 * base_pixels_per_second / factor;

	return wxString::Format(_("%d%%, %d pixel/second"), factor, second_pixels);
}


int AudioDisplay::GetZoomLevelFactor(int level)
{
	int factor = 100;

	if (level > 0)
	{
		factor += 25 * level;
	}
	else if (level < 0)
	{
		if (level >= -5)
			factor += 10 * level;
		else if (level >= -11)
			factor = 50 + (level+5) * 5;
		else
			factor = 20 + level + 11;
		if (factor <= 0)
			factor = 1;
	}

	return factor;
}


void AudioDisplay::SetAmplitudeScale(float scale)
{
	audio_renderer->SetAmplitudeScale(scale);
	Refresh();
}

void AudioDisplay::ReloadRenderingSettings()
{
	std::string colour_scheme_name;

	if (OPT_GET("Audio/Spectrum")->GetBool())
	{
		colour_scheme_name = OPT_GET("Colour/Audio Display/Spectrum")->GetString();
		AudioSpectrumRenderer *audio_spectrum_renderer = new AudioSpectrumRenderer(colour_scheme_name);

		int64_t spectrum_quality = OPT_GET("Audio/Renderer/Spectrum/Quality")->GetInt();
#ifdef WITH_FFTW3
		// FFTW is so fast we can afford to upgrade quality by two levels
		spectrum_quality += 2;
#endif
		spectrum_quality = mid<int64_t>(0, spectrum_quality, 5);

		// Quality indexes:        0  1  2  3   4   5
		int spectrum_width[]    = {8, 9, 9, 9, 10, 11};
		int spectrum_distance[] = {8, 8, 7, 6,  6,  5};

		audio_spectrum_renderer->SetResolution(
			spectrum_width[spectrum_quality],
			spectrum_distance[spectrum_quality]);

		audio_renderer_provider.reset(audio_spectrum_renderer);
	}
	else
	{
		colour_scheme_name = OPT_GET("Colour/Audio Display/Waveform")->GetString();
		audio_renderer_provider.reset(new AudioWaveformRenderer(colour_scheme_name));
	}

	audio_renderer->SetRenderer(audio_renderer_provider.get());
	scrollbar->SetColourScheme(colour_scheme_name);
	timeline->SetColourScheme(colour_scheme_name);

	Refresh();
}



BEGIN_EVENT_TABLE(AudioDisplay, wxWindow)
	EVT_PAINT(AudioDisplay::OnPaint)
	EVT_SIZE(AudioDisplay::OnSize)
	EVT_SET_FOCUS(AudioDisplay::OnFocus)
	EVT_KILL_FOCUS(AudioDisplay::OnFocus)
	EVT_KEY_DOWN(AudioDisplay::OnKeyDown)
END_EVENT_TABLE()


void AudioDisplay::OnPaint(wxPaintEvent&)
{
	wxAutoBufferedPaintDC dc(this);

	wxRect audio_bounds(0, audio_top, GetClientSize().GetWidth(), audio_height);
	bool redraw_scrollbar = false;
	bool redraw_timeline = false;

	wxPoint client_org = GetClientAreaOrigin();
	for (wxRegionIterator region(GetUpdateRegion()); region; ++region)
	{
		wxRect updrect = region.GetRect();
		// Work around wxMac issue, client border offsets update rectangles but does
		// not affect drawing coordinates.
#ifdef __WXMAC__
		updrect.x += client_org.x; updrect.y += client_org.y;
#endif

		redraw_scrollbar |= scrollbar->GetBounds().Intersects(updrect);
		redraw_timeline |= timeline->GetBounds().Intersects(updrect);

		if (audio_bounds.Intersects(updrect))
		{
			TimeRange updtime(
				std::max(0, TimeFromRelativeX(updrect.x - foot_size)),
				std::max(0, TimeFromRelativeX(updrect.x + updrect.width + foot_size)));

			PaintAudio(dc, updtime, updrect);
			PaintMarkers(dc, updtime);
			PaintLabels(dc, updtime);
		}
	}

	if (track_cursor_pos >= 0)
	{
		PaintTrackCursor(dc);
	}

	if (redraw_scrollbar)
		scrollbar->Paint(dc, HasFocus());
	if (redraw_timeline)
		timeline->Paint(dc);
}

void AudioDisplay::PaintAudio(wxDC &dc, TimeRange updtime, wxRect updrect)
{
	std::map<int, int>::iterator pt = style_ranges.upper_bound(updtime.begin());
	std::map<int, int>::iterator pe = style_ranges.upper_bound(updtime.end());

	if (pt != style_ranges.begin())
		--pt;

	while (pt != pe)
	{
		AudioRenderingStyle range_style = static_cast<AudioRenderingStyle>(pt->second);
		int range_x1 = std::max(updrect.x, RelativeXFromTime(pt->first));
		int range_x2 = (++pt == pe) ? updrect.x + updrect.width : RelativeXFromTime(pt->first);

		if (range_x2 > range_x1)
		{
			audio_renderer->Render(dc, wxPoint(range_x1, audio_top), range_x1 + scroll_left, range_x2 - range_x1, range_style);
		}
	}
}

void AudioDisplay::PaintMarkers(wxDC &dc, TimeRange updtime)
{
	AudioMarkerVector markers;
	controller->GetMarkers(updtime, markers);
	if (markers.empty()) return;

	wxDCPenChanger pen_retainer(dc, wxPen());
	wxDCBrushChanger brush_retainer(dc, wxBrush());
	for (AudioMarkerVector::iterator marker_i = markers.begin(); marker_i != markers.end(); ++marker_i)
	{
		const AudioMarker *marker = *marker_i;
		int marker_x = RelativeXFromTime(marker->GetPosition());

		dc.SetPen(marker->GetStyle());
		dc.DrawLine(marker_x, audio_top, marker_x, audio_top+audio_height);

		if (marker->GetFeet() == AudioMarker::Feet_None) continue;

		dc.SetBrush(wxBrush(marker->GetStyle().GetColour()));
		dc.SetPen(*wxTRANSPARENT_PEN);

		if (marker->GetFeet() & AudioMarker::Feet_Left)
			PaintFoot(dc, marker_x, -1);
		if (marker->GetFeet() & AudioMarker::Feet_Right)
			PaintFoot(dc, marker_x, 1);
	}
}

void AudioDisplay::PaintFoot(wxDC &dc, int marker_x, int dir)
{
	wxPoint foot_top[3] = { wxPoint(foot_size * dir, 0), wxPoint(0, 0), wxPoint(0, foot_size) };
	wxPoint foot_bot[3] = { wxPoint(foot_size * dir, 0), wxPoint(0, -foot_size), wxPoint(0, 0) };
	dc.DrawPolygon(3, foot_top, marker_x, audio_top);
	dc.DrawPolygon(3, foot_bot, marker_x, audio_top+audio_height);
}

void AudioDisplay::PaintLabels(wxDC &dc, TimeRange updtime)
{
	std::vector<AudioLabelProvider::AudioLabel> labels;
	controller->GetLabels(updtime, labels);
	if (labels.empty()) return;

	wxDCFontChanger fc(dc);
	wxFont font = dc.GetFont();
	font.SetWeight(wxFONTWEIGHT_BOLD);
	dc.SetFont(font);
	dc.SetTextForeground(*wxWHITE);
	for (size_t i = 0; i < labels.size(); ++i)
	{
		wxSize extent = dc.GetTextExtent(labels[i].text);
		int left = RelativeXFromTime(labels[i].range.begin());
		int width = AbsoluteXFromTime(labels[i].range.length());

		// If it doesn't fit, truncate
		if (width < extent.GetWidth())
		{
			dc.SetClippingRegion(left, audio_top + 4, width, extent.GetHeight());
			dc.DrawText(labels[i].text, left, audio_top + 4);
			dc.DestroyClippingRegion();
		}
		// Otherwise center in the range
		else
		{
			dc.DrawText(labels[i].text, left + (width - extent.GetWidth()) / 2, audio_top + 4);
		}
	}
}

void AudioDisplay::PaintTrackCursor(wxDC &dc) {
	wxDCPenChanger penchanger(dc, wxPen(*wxWHITE));
	dc.DrawLine(track_cursor_pos-scroll_left, audio_top, track_cursor_pos-scroll_left, audio_top+audio_height);

	if (track_cursor_label.empty()) return;

	wxDCFontChanger fc(dc);
	wxFont font = dc.GetFont();
	font.SetWeight(wxFONTWEIGHT_BOLD);
	dc.SetFont(font);

	wxSize label_size(dc.GetTextExtent(track_cursor_label));
	wxPoint label_pos(track_cursor_pos - scroll_left - label_size.x/2, audio_top + 2);
	label_pos.x = mid(2, label_pos.x, GetClientSize().GetWidth() - label_size.x - 2);

	int old_bg_mode = dc.GetBackgroundMode();
	dc.SetBackgroundMode(wxTRANSPARENT);

	// Draw border
	dc.SetTextForeground(wxColour(64, 64, 64));
	dc.DrawText(track_cursor_label, label_pos.x+1, label_pos.y+1);
	dc.DrawText(track_cursor_label, label_pos.x+1, label_pos.y-1);
	dc.DrawText(track_cursor_label, label_pos.x-1, label_pos.y+1);
	dc.DrawText(track_cursor_label, label_pos.x-1, label_pos.y-1);

	// Draw fill
	dc.SetTextForeground(*wxWHITE);
	dc.DrawText(track_cursor_label, label_pos.x, label_pos.y);
	dc.SetBackgroundMode(old_bg_mode);

	label_pos.x -= 2;
	label_pos.y -= 2;
	label_size.IncBy(4, 4);
	// If the rendered text changes size we have to draw it an extra time to make sure the entire thing was drawn
	bool need_extra_redraw = track_cursor_label_rect.GetSize() != label_size;
	track_cursor_label_rect.SetPosition(label_pos);
	track_cursor_label_rect.SetSize(label_size);
	if (need_extra_redraw)
		RefreshRect(track_cursor_label_rect, false);
}

void AudioDisplay::SetDraggedObject(AudioDisplayInteractionObject *new_obj)
{
	dragged_object = new_obj;

	if (dragged_object && !HasCapture())
		CaptureMouse();
	else if (!dragged_object && HasCapture())
		ReleaseMouse();
}


void AudioDisplay::SetTrackCursor(int new_pos, bool show_time)
{
	if (new_pos != track_cursor_pos)
	{
		int old_pos = track_cursor_pos;
		track_cursor_pos = new_pos;

		RefreshRect(wxRect(old_pos - scroll_left - 0, audio_top, 1, audio_height), false);
		RefreshRect(wxRect(new_pos - scroll_left - 0, audio_top, 1, audio_height), false);

		// Make sure the old label gets cleared away
		RefreshRect(track_cursor_label_rect, false);

		if (show_time)
		{
			AssTime new_label_time = TimeFromAbsoluteX(track_cursor_pos);
			track_cursor_label = new_label_time.GetASSFormated();
			track_cursor_label_rect.x += new_pos - old_pos;
			RefreshRect(track_cursor_label_rect, false);
		}
		else
		{
			track_cursor_label_rect.SetSize(wxSize(0,0));
			track_cursor_label.Clear();
		}
	}
}


void AudioDisplay::RemoveTrackCursor()
{
	SetTrackCursor(-1, false);
}


void AudioDisplay::OnMouseEvent(wxMouseEvent& event)
{
	// If we have focus, we get mouse move events on Mac even when the mouse is
	// outside our client rectangle, we don't want those.
	if (event.Moving() && !GetClientRect().Contains(event.GetPosition()))
	{
		event.Skip();
		return;
	}

	if (event.IsButton() || (event.Entering() && OPT_GET("Audio/Auto/Focus")->GetBool()))
		SetFocus();

	// Handle any ongoing drag
	if (dragged_object && HasCapture())
	{
		if (!dragged_object->OnMouseEvent(event))
		{
			SetDraggedObject(0);
			SetCursor(wxNullCursor);
		}
		return;
	}
	else
	{
		// Something is wrong, we might have lost capture somehow.
		// Fix state and pretend it didn't happen.
		SetDraggedObject(0);
		SetCursor(wxNullCursor);
	}

	wxPoint mousepos = event.GetPosition();

	AudioDisplayInteractionObject *new_obj = 0;
	// Check for scrollbar action
	if (scrollbar->GetBounds().Contains(mousepos))
	{
		new_obj = scrollbar.get();
	}
	// Check for timeline action
	else if (timeline->GetBounds().Contains(mousepos))
	{
		SetCursor(wxCursor(wxCURSOR_SIZEWE));
		new_obj = timeline.get();
		
	}

	if (new_obj)
	{
		if (!controller->IsPlaying())
			RemoveTrackCursor();
		if (new_obj->OnMouseEvent(event))
			SetDraggedObject(new_obj);
		return;
	}

	if (event.Leaving() && !controller->IsPlaying())
	{
		RemoveTrackCursor();
		return;
	}

	if (event.MiddleIsDown())
	{
		context->videoController->JumpToTime(TimeFromRelativeX(mousepos.x), agi::vfr::EXACT);
		return;
	}

	if (event.Moving() && !controller->IsPlaying())
	{
		SetTrackCursor(scroll_left + mousepos.x, OPT_GET("Audio/Display/Draw/Cursor Time")->GetBool());
	}

	AudioTimingController *timing = controller->GetTimingController();
	if (!timing) return;
	int drag_sensitivity = int(OPT_GET("Audio/Start Drag Sensitivity")->GetInt() * ms_per_pixel);
	int snap_sensitivity = OPT_GET("Audio/Snap/Enable")->GetBool() != event.ShiftDown() ? int(OPT_GET("Audio/Snap/Distance")->GetInt() * ms_per_pixel) : 0;

	// Not scrollbar, not timeline, no button action
	if (event.Moving())
	{
		int timepos = TimeFromRelativeX(mousepos.x);

		if (timing->IsNearbyMarker(timepos, drag_sensitivity))
			SetCursor(wxCursor(wxCURSOR_SIZEWE));
		else
			SetCursor(wxNullCursor);
	}

	if (event.LeftDown() || event.RightDown())
	{
		int timepos = TimeFromRelativeX(mousepos.x);
		AudioMarker *marker = event.LeftDown() ?
			timing->OnLeftClick(timepos, drag_sensitivity, snap_sensitivity) :
			timing->OnRightClick(timepos, drag_sensitivity, snap_sensitivity);

		if (marker)
		{
			RemoveTrackCursor();
			audio_marker.reset(new AudioMarkerInteractionObject(marker, timing, this, controller, (wxMouseButton)event.GetButton()));
			SetDraggedObject(audio_marker.get());
			return;
		}
	}
}

void AudioDisplay::OnKeyDown(wxKeyEvent& event)
{
	if (!hotkey::check("Audio", context, event.GetKeyCode(), event.GetUnicodeKey(), event.GetModifiers()))
		event.Skip();
}

void AudioDisplay::OnSize(wxSizeEvent &)
{
	// We changed size, update the sub-controls' internal data and redraw
	wxSize size = GetClientSize();

	timeline->SetDisplaySize(wxSize(size.x, scrollbar->GetBounds().y));
	scrollbar->SetDisplaySize(size);

	if (controller->GetTimingController())
	{
		TimeRange sel(controller->GetTimingController()->GetPrimaryPlaybackRange());
		scrollbar->SetSelection(AbsoluteXFromTime(sel.begin()), AbsoluteXFromTime(sel.length()));
	}

	audio_height = size.GetHeight();
	audio_height -= scrollbar->GetBounds().GetHeight();
	audio_height -= timeline->GetHeight();
	audio_renderer->SetHeight(audio_height);

	audio_top = timeline->GetHeight();

	Refresh();
}


void AudioDisplay::OnFocus(wxFocusEvent &)
{
	// The scrollbar indicates focus so repaint that
	RefreshRect(scrollbar->GetBounds(), false);
}


void AudioDisplay::OnAudioOpen(AudioProvider *provider)
{
	if (!audio_renderer_provider)
		ReloadRenderingSettings();

	audio_renderer->SetAudioProvider(provider);
	audio_renderer->SetCacheMaxSize(OPT_GET("Audio/Renderer/Spectrum/Memory Max")->GetInt() * 1024 * 1024);

	timeline->ChangeAudio(controller->GetDuration());

	ms_per_pixel = 0;
	SetZoomLevel(zoom_level);

	Refresh();

	if (provider)
	{
		if (connections.empty())
		{
			connections.push_back(controller->AddAudioCloseListener(&AudioDisplay::OnAudioOpen, this, (AudioProvider*)0));
			connections.push_back(controller->AddPlaybackPositionListener(&AudioDisplay::OnPlaybackPosition, this));
			connections.push_back(controller->AddPlaybackStopListener(&AudioDisplay::RemoveTrackCursor, this));
			connections.push_back(controller->AddTimingControllerListener(&AudioDisplay::OnStyleRangesChanged, this));
			connections.push_back(controller->AddMarkerMovedListener(&AudioDisplay::OnMarkerMoved, this));
			connections.push_back(controller->AddSelectionChangedListener(&AudioDisplay::OnSelectionChanged, this));
			connections.push_back(controller->AddStyleRangesChangedListener(&AudioDisplay::OnStyleRangesChanged, this));
			connections.push_back(OPT_SUB("Audio/Spectrum", &AudioDisplay::ReloadRenderingSettings, this));
			connections.push_back(OPT_SUB("Audio/Display/Waveform Style", &AudioDisplay::ReloadRenderingSettings, this));
			connections.push_back(OPT_SUB("Colour/Audio Display/Spectrum", &AudioDisplay::ReloadRenderingSettings, this));
			connections.push_back(OPT_SUB("Colour/Audio Display/Waveform", &AudioDisplay::ReloadRenderingSettings, this));
			connections.push_back(OPT_SUB("Audio/Renderer/Spectrum/Quality", &AudioDisplay::ReloadRenderingSettings, this));
		}
	}
	else
	{
		connections.clear();
	}
}

void AudioDisplay::OnPlaybackPosition(int ms)
{
	int pixel_position = AbsoluteXFromTime(ms);
	SetTrackCursor(pixel_position, false);

	if (OPT_GET("Audio/Lock Scroll on Cursor")->GetBool())
	{
		int client_width = GetClientSize().GetWidth();
		int edge_size = client_width / 20;
		if (scroll_left > 0 && pixel_position < scroll_left + edge_size)
		{
			ScrollPixelToLeft(std::max(pixel_position - edge_size, 0));
		}
		else if (scroll_left + client_width < std::min(pixel_audio_width - 1, pixel_position + edge_size))
		{
			ScrollPixelToLeft(std::min(pixel_position - client_width + edge_size, pixel_audio_width - client_width - 1));
		}
	}
}

void AudioDisplay::OnSelectionChanged()
{
	TimeRange sel(controller->GetPrimaryPlaybackRange());
	scrollbar->SetSelection(AbsoluteXFromTime(sel.begin()), AbsoluteXFromTime(sel.length()));

	if (OPT_GET("Audio/Auto/Scroll")->GetBool())
	{
		ScrollTimeRangeInView(sel);
	}

	RefreshRect(scrollbar->GetBounds(), false);
}

void AudioDisplay::OnStyleRangesChanged()
{
	if (!controller->GetTimingController()) return;

	AudioStyleRangeMerger asrm;
	controller->GetTimingController()->GetRenderingStyles(asrm);

	std::map<int, int> old_style_ranges;
	swap(old_style_ranges, style_ranges);
	style_ranges.insert(asrm.begin(), asrm.end());

	std::map<int, int>::iterator old_style_it = old_style_ranges.begin();
	std::map<int, int>::iterator new_style_it = style_ranges.begin();

	int old_style = old_style_it->second;
	int new_style = new_style_it->second;
	int range_start = 0;

	// Repaint each range which has changed
	while (old_style_it != old_style_ranges.end() || new_style_it != style_ranges.end())
	{
		if (new_style_it == style_ranges.end() || (old_style_it != old_style_ranges.end() && old_style_it->first <= new_style_it->first))
		{
			if (old_style != new_style)
				Redraw(range_start, old_style_it->first);
			old_style = old_style_it->second;
			range_start = old_style_it->first;
			++old_style_it;
		}
		else
		{
			if (old_style != new_style)
				Redraw(range_start, new_style_it->first);
			new_style = new_style_it->second;
			range_start = new_style_it->first;
			++new_style_it;
		}
	}

	// Fill in the last style range
	if (old_style != new_style)
	{
		Redraw(range_start, TimeFromRelativeX(GetClientSize().GetWidth()));
	}
}

void AudioDisplay::Redraw(int time_start, int time_end)
{
	if (time_start == time_end) return;

	time_start = RelativeXFromTime(time_start) - foot_size;
	time_end = RelativeXFromTime(time_end) + foot_size;

	if (time_end >= 0 && time_start <= GetClientSize().GetWidth())
	{
		RefreshRect(wxRect(time_start, audio_top, time_end - time_start, audio_height), false);
	}
}

void AudioDisplay::OnMarkerMoved()
{
	/// @todo investigate if it's worth refreshing only the changed spots
	RefreshRect(wxRect(0, audio_top, GetClientSize().GetWidth(), audio_height), false);
}

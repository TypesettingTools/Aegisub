// Copyright (c) 2005, Rodrigo Braz Monteiro
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

///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <math.h>

#include <vector>

#include <wx/filename.h>
#include <wx/tglbtn.h>
#endif

#include "ass_file.h"
#include "audio_box.h"
#include "audio_display.h"
#include "audio_karaoke.h"
#ifdef _DEBUG
#include "audio_provider_dummy.h"
#endif
#include "colorspace.h"
#include "compat.h"
#include "fft.h"
#include "hotkeys.h"
#include "main.h"
#include "standard_paths.h"
#include "subs_edit_box.h"
#include "subs_edit_ctrl.h"
#include "subs_grid.h"
#include "timeedit_ctrl.h"
#include "utils.h"
#include "video_context.h"

#ifdef __WXMAC__

/// DOCME
# define AudioDisplayWindowStyle wxWANTS_CHARS
#else

/// DOCME
# define AudioDisplayWindowStyle wxSUNKEN_BORDER | wxWANTS_CHARS
#endif

/// @brief Constructor 
/// @param parent 
AudioDisplay::AudioDisplay(wxWindow *parent)
: wxWindow (parent, -1, wxDefaultPosition, wxSize(200,OPT_GET("Audio/Display Height")->GetInt()), AudioDisplayWindowStyle , _T("Audio Display"))
, grid(0)
{
	// Set variables
	origImage = NULL;
	spectrumDisplay = NULL;
	spectrumDisplaySelected = NULL;
	spectrumRenderer = NULL;
	ScrollBar = NULL;
	dialogue = NULL;
	karaoke = NULL;
	peak = NULL;
	min = NULL;
	hasSel = false;
	diagUpdated = false;
	NeedCommit = false;
	loaded = false;
	temporary = false;
	blockUpdate = false;
	dontReadTimes = false;
	holding = false;
	draggingScale = false;
	Position = 0;
	PositionSample = 0;
	oldCurPos = 0;
	scale = 1.0f;
	provider = NULL;
	player = NULL;
	hold = 0;
	samples = 0;
	samplesPercent = 100;
	hasFocus = (wxWindow::FindFocus() == this);
	needImageUpdate = false;
	needImageUpdateWeak = true;
	playingToEnd = false;

	// Init
	UpdateTimer.SetOwner(this,Audio_Update_Timer);
	GetClientSize(&w,&h);
	h -= OPT_GET("Audio/Display/Draw/Timeline")->GetBool() ? 20 : 0;
	SetSamplesPercent(50,false);

	// Set cursor
	//wxCursor cursor(wxCURSOR_BLANK);
	//SetCursor(cursor);

	//wxLog::SetActiveTarget(new wxLogWindow(NULL,_T("Log"),true,false));
}

/// @brief Destructor 
AudioDisplay::~AudioDisplay() {
	if (player) player->CloseStream();
	delete provider;
	delete player;
	delete origImage;
	delete spectrumRenderer;
	delete spectrumDisplay;
	delete spectrumDisplaySelected;
	delete[] peak;
	delete[] min;
	provider = NULL;
	player = NULL;
	origImage = NULL;
	spectrumRenderer = NULL;
	spectrumDisplay = NULL;
	spectrumDisplaySelected = NULL;
	peak = NULL;
	min = NULL;
}

/// @brief Reset 
void AudioDisplay::Reset() {
	hasSel = false;
	diagUpdated = false;
	NeedCommit = false;
	karaoke->enabled = false;
	karaoke->syllables.clear();
	box->karaokeMode = false;
	box->KaraokeButton->SetValue(false);
	dialogue = NULL;
}

/// @brief Update image 
/// @param weak 
void AudioDisplay::UpdateImage(bool weak) {
	// Update samples
	UpdateSamples();

	// Set image as needing to be redrawn
	needImageUpdate = true;
	if (weak == false && needImageUpdateWeak == true) {
		needImageUpdateWeak = false;
	}
	Refresh(false);
}

/// @brief Actually update the image on the display
/// This is where most actual drawing of the audio display happens, or other functions
/// to draw specific parts are called from.
void AudioDisplay::DoUpdateImage() {
	// Loaded?
	if (!loaded || !provider) return;

	// Needs updating?
	if (!needImageUpdate) return;
	bool weak = needImageUpdateWeak;

	// Prepare bitmap
	int timelineHeight = OPT_GET("Audio/Display/Draw/Timeline")->GetBool() ? 20 : 0;
	int displayH = h+timelineHeight;
	if (origImage) {
		if (origImage->GetWidth() != w || origImage->GetHeight() != displayH) {
			delete origImage;
			origImage = NULL;
		}
	}

	// Options
	bool draw_boundary_lines = OPT_GET("Audio/Display/Draw/Secondary Lines")->GetBool();
	bool draw_selection_background = OPT_GET("Audio/Display/Draw/Selection Background")->GetBool();
	bool drawKeyframes = OPT_GET("Audio/Display/Draw/Keyframes")->GetBool();

	// Invalid dimensions
	if (w == 0 || displayH == 0) return;

	// New bitmap
	if (!origImage) origImage = new wxBitmap(w,displayH,-1);

	// Is spectrum?
	bool spectrum = false;
	if (provider && OPT_GET("Audio/Spectrum")->GetBool()) {
		spectrum = true;
	}

	// Draw image to be displayed
	wxMemoryDC dc;
	dc.SelectObject(*origImage);

	// Black background
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(lagi_wxColour(OPT_GET("Colour/Audio Display/Background/Background")->GetColour())));
	dc.DrawRectangle(0,0,w,h);

	// Selection position
	hasSel = false;
	hasKaraoke = karaoke->enabled;
	selStart = 0;
	selEnd = 0;
	lineStart = 0;
	lineEnd = 0;
	selStartCap = 0;
	selEndCap = 0;
	int64_t drawSelStart = 0;
	int64_t drawSelEnd = 0;
	if (dialogue) {
		GetDialoguePos(lineStart,lineEnd,false);
		hasSel = true;
		if (hasKaraoke) {
			GetKaraokePos(selStartCap,selEndCap,true);
			GetKaraokePos(drawSelStart,drawSelEnd,false);
			selStart = lineStart;
			selEnd = lineEnd;
		}
		else {
			GetDialoguePos(selStartCap,selEndCap,true);
			selStart = lineStart;
			selEnd = lineEnd;
			drawSelStart = lineStart;
			drawSelEnd = lineEnd;
		}
	}

	// Draw selection bg
	if (hasSel && drawSelStart < drawSelEnd && draw_selection_background) {
		if (NeedCommit && !karaoke->enabled) dc.SetBrush(wxBrush(lagi_wxColour(OPT_GET("Colour/Audio Display/Background/Selection Modified")->GetColour())));
		else dc.SetBrush(wxBrush(lagi_wxColour(OPT_GET("Colour/Audio Display/Background/Background")->GetColour())));
		dc.DrawRectangle(drawSelStart,0,drawSelEnd-drawSelStart,h);
	}

	// Draw spectrum
	if (spectrum) {
		DrawSpectrum(dc,weak);
	}

	// Waveform
	else if (provider) {
		DrawWaveform(dc,weak);
	}

	// Nothing
	else {
		dc.DrawLine(0,h/2,w,h/2);
	}

	// Draw seconds boundaries
	if (draw_boundary_lines) {
		int64_t start = Position*samples;
		int rate = provider->GetSampleRate();
		int pixBounds = rate / samples;
		dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Audio Display/Seconds Boundaries")->GetColour()),1,wxDOT));
		if (pixBounds >= 8) {
			for (int x=0;x<w;x++) {
				if (((x*samples)+start) % rate < samples) {
					dc.DrawLine(x,0,x,h);
				}
			}
		}
	}

	// Draw current frame
	if (OPT_GET("Audio/Display/Draw/Video Position")->GetBool()) {
		if (VideoContext::Get()->IsLoaded()) {
			dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Audio Display/Play Cursor")->GetColour())));
			int x = GetXAtMS(VideoContext::Get()->TimeAtFrame(VideoContext::Get()->GetFrameN()));
			dc.DrawLine(x,0,x,h);
		}
	}

	// Draw keyframes
	if (drawKeyframes && VideoContext::Get()->KeyFramesLoaded()) {
		DrawKeyframes(dc);
	}

	// Draw previous line
	DrawInactiveLines(dc);

	if (hasSel) {
		// Draw boundaries
		if (true) {
			// Draw start boundary
			int selWidth = OPT_GET("Audio/Line Boundaries Thickness")->GetInt();
			dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Audio Display/Line boundary Start")->GetColour())));
			dc.SetBrush(wxBrush(lagi_wxColour(OPT_GET("Colour/Audio Display/Line boundary Start")->GetColour())));
			dc.DrawRectangle(lineStart-selWidth/2+1,0,selWidth,h);
			wxPoint points1[3] = { wxPoint(lineStart,0), wxPoint(lineStart+10,0), wxPoint(lineStart,10) };
			wxPoint points2[3] = { wxPoint(lineStart,h-1), wxPoint(lineStart+10,h-1), wxPoint(lineStart,h-11) };
			dc.DrawPolygon(3,points1);
			dc.DrawPolygon(3,points2);

			// Draw end boundary
			dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Audio Display/Line boundary End")->GetColour())));
			dc.SetBrush(wxBrush(lagi_wxColour(OPT_GET("Colour/Audio Display/Line boundary End")->GetColour())));
			dc.DrawRectangle(lineEnd-selWidth/2+1,0,selWidth,h);
			wxPoint points3[3] = { wxPoint(lineEnd,0), wxPoint(lineEnd-10,0), wxPoint(lineEnd,10) };
			wxPoint points4[3] = { wxPoint(lineEnd,h-1), wxPoint(lineEnd-10,h-1), wxPoint(lineEnd,h-11) };
			dc.DrawPolygon(3,points3);
			dc.DrawPolygon(3,points4);
		}

		// Draw karaoke
		if (hasKaraoke) {
			try {
				// Prepare
				wxPen curPen(lagi_wxColour(OPT_GET("Colour/Audio Display/Syllable Boundaries")->GetColour()),1,wxDOT);
				dc.SetPen(curPen);
				wxFont curFont(9,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"),wxFONTENCODING_SYSTEM);
				dc.SetFont(curFont);
				if (!spectrum) dc.SetTextForeground(lagi_wxColour(OPT_GET("Colour/Audio Display/Syllable Text")->GetColour()));
				else dc.SetTextForeground(wxColour(255,255,255));
				size_t karn = karaoke->syllables.size();
				int64_t pos1,pos2;
				int len,curpos;
				wxCoord tw=0,th=0;
				AudioKaraokeSyllable *curSyl;
				wxString temptext;

				// Draw syllables
				for (size_t i=0;i<karn;i++) {
					curSyl = &karaoke->syllables.at(i);
					len = curSyl->duration*10;
					curpos = curSyl->start_time*10;
					if (len != -1) {
						pos1 = GetXAtMS(curStartMS+curpos);
						pos2 = GetXAtMS(curStartMS+len+curpos);
						dc.DrawLine(pos2,0,pos2,h);
						temptext = curSyl->text;
						temptext.Trim(true);
						temptext.Trim(false);
						GetTextExtent(temptext,&tw,&th,NULL,NULL,&curFont);
						dc.DrawText(temptext,(pos1+pos2-tw)/2,4);
					}
				}
			}
			catch (...) {
				// FIXME?
			}
		}
	}

	// Modified text
	if (NeedCommit) {
		dc.SetFont(wxFont(9,wxDEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"))); // FIXME: hardcoded font name
		dc.SetTextForeground(wxColour(255,0,0));
		if (selStart <= selEnd) {
			dc.DrawText(_T("Modified"),4,4);
		}
		else {
			dc.DrawText(_T("Negative time"),4,4);
		}
	}

	// Draw timescale
	if (timelineHeight) {
		DrawTimescale(dc);
	}

	// Draw selection border
	if (hasFocus) {
		dc.SetPen(*wxGREEN_PEN);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(0,0,w,h);
	}

	// Done
	needImageUpdate = false;
	needImageUpdateWeak = true;
}

/// @brief Draw other lines than the current active
/// @param dc The DC to draw to.
/// Draws markers for inactive lines, eg. the previous line, per configuration.
void AudioDisplay::DrawInactiveLines(wxDC &dc) {
	// Check if there is anything to do
	int shadeType = OPT_GET("Audio/Inactive Lines Display Mode")->GetInt();
	if (shadeType == 0) return;

	// Spectrum?
	bool spectrum = false;
	if (provider && OPT_GET("Audio/Spectrum")->GetBool()) {
		spectrum = true;
	}

	// Set options
	dc.SetBrush(wxBrush(lagi_wxColour(OPT_GET("Colour/Audio Display/Line Boundary Inactive Line")->GetColour())));
	int selWidth = OPT_GET("Audio/Line Boundaries Thickness")->GetInt();
	AssDialogue *shade;
	int shadeX1,shadeX2;
	int shadeFrom,shadeTo;

	// Only previous
	if (shadeType == 1) {
		shadeFrom = this->line_n-1;
		shadeTo = shadeFrom+1;
	}

	// All
	else {
		shadeFrom = 0;
		shadeTo = grid->GetRows();
	}
	
	for (int j=shadeFrom;j<shadeTo;j++) {
		if (j == line_n) continue;
		if (j < 0) continue;
		shade = grid->GetDialogue(j);

		if (shade) {
			// Get coordinates
			shadeX1 = GetXAtMS(shade->Start.GetMS());
			shadeX2 = GetXAtMS(shade->End.GetMS());
			if (shadeX2 < 0 || shadeX1 > w) continue;

			// Draw over waveform
			if (!spectrum) {
				// Selection
				int selX1 = MAX(0,GetXAtMS(curStartMS));
				int selX2 = MIN(w,GetXAtMS(curEndMS));

				// Get ranges (x1->x2, x3->x4).
				int x1 = MAX(0,shadeX1);
				int x2 = MIN(w,shadeX2);
				int x3 = MAX(x1,selX2);
				int x4 = MAX(x2,selX2);

				// Clip first range
				x1 = MIN(x1,selX1);
				x2 = MIN(x2,selX1);

				// Set pen and draw
				dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Audio Display/Waveform Inactive")->GetColour())));
				for (int i=x1;i<x2;i++) dc.DrawLine(i,peak[i],i,min[i]-1);
				for (int i=x3;i<x4;i++) dc.DrawLine(i,peak[i],i,min[i]-1);
			}

			// Draw boundaries
			dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Audio Display/Line Boundary Inactive Line")->GetColour())));
			dc.DrawRectangle(shadeX1-selWidth/2+1,0,selWidth,h);
			dc.DrawRectangle(shadeX2-selWidth/2+1,0,selWidth,h);
		}
	}
}

/// @brief Draw keyframe markers
/// @param dc The DC to draw to.
void AudioDisplay::DrawKeyframes(wxDC &dc) {
	std::vector<int> KeyFrames = VideoContext::Get()->GetKeyFrames();
	int nKeys = (int)KeyFrames.size();
	dc.SetPen(wxPen(wxColour(255,0,255),1));

	// Get min and max frames to care about
	int minFrame = VideoContext::Get()->FrameAtTime(GetMSAtX(0),agi::vfr::START);
	int maxFrame = VideoContext::Get()->FrameAtTime(GetMSAtX(w),agi::vfr::END);

	// Scan list
	for (int i=0;i<nKeys;i++) {
		int cur = KeyFrames[i];
		if (cur >= minFrame && cur <= maxFrame) {
			int x = GetXAtMS(VideoContext::Get()->TimeAtFrame(cur,agi::vfr::START));
			dc.DrawLine(x,0,x,h);
		}
		else if (cur > maxFrame) break;
	}
}

/// @brief Draw timescale at bottom of audio display
/// @param dc The DC to draw to.
void AudioDisplay::DrawTimescale(wxDC &dc) {
	// Set size
	int timelineHeight = OPT_GET("Audio/Display/Draw/Timeline")->GetBool() ? 20 : 0;

	// Set colours
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(0,h,w,timelineHeight);
	dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT));
	dc.DrawLine(0,h,w,h);
	dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT));
	dc.DrawLine(0,h+1,w,h+1);
	dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
	wxFont scaleFont;
	scaleFont.SetFaceName(_T("Tahoma")); // FIXME: hardcoded font name
	scaleFont.SetPointSize(8);
	dc.SetFont(scaleFont);

	// Timescale ticks
	int64_t start = Position*samples;
	int rate = provider->GetSampleRate();
	for (int i=1;i<32;i*=2) {
		int pixBounds = rate / (samples * 4 / i);
		if (pixBounds >= 8) {
			for (int x=0;x<w;x++) {
				int64_t pos = (x*samples)+start;
				// Second boundary
				if (pos % rate < samples) {
					dc.DrawLine(x,h+2,x,h+8);

					// Draw text
					wxCoord textW,textH;
					int hr = 0;
					int m = 0;
					int s = pos/rate;
					while (s >= 3600) {
						s -= 3600;
						hr++;
					}
					while (s >= 60) {
						s -= 60;
						m++;
					}
					wxString text;
					if (hr) text = wxString::Format(_T("%i:%02i:%02i"),hr,m,s);
					else if (m) text = wxString::Format(_T("%i:%02i"),m,s);
					else text = wxString::Format(_T("%i"),s);
					dc.GetTextExtent(text,&textW,&textH,NULL,NULL,&scaleFont);
					dc.DrawText(text,MAX(0,x-textW/2)+1,h+8);
				}

				// Other
				else if (pos % (rate / 4 * i) < samples) {
					dc.DrawLine(x,h+2,x,h+5);
				}
			}
			break;
		}
	}
}

/// @brief Draw audio waveform
/// @param dc   The DC to draw to.
/// @param weak False if the visible portion of the display has changed.
void AudioDisplay::DrawWaveform(wxDC &dc,bool weak) {
	// Prepare Waveform
	if (!weak || peak == NULL || min == NULL) {
		if (peak) delete[] peak;
		if (min) delete[] min;
		peak = new int[w];
		min = new int[w];
	}

	// Get waveform
	if (!weak) {
		provider->GetWaveForm(min,peak,Position*samples,w,h,samples,scale);
	}

	// Draw pre-selection
	if (!hasSel) selStartCap = w;
	dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Audio Display/Waveform")->GetColour())));
	for (int64_t i=0;i<selStartCap;i++) {
		dc.DrawLine(i,peak[i],i,min[i]-1);
	}

	if (hasSel) {
		// Draw selection
		if (OPT_GET("Audio/Display/Draw/Selection Background")->GetBool()) {
			if (NeedCommit && !karaoke->enabled) dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Audio Display/Waveform Modified")->GetColour())));
			else dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Audio Display/Waveform Selected")->GetColour())));
		}
		for (int64_t i=selStartCap;i<selEndCap;i++) {
			dc.DrawLine(i,peak[i],i,min[i]-1);
		}

		// Draw post-selection
		dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Audio Display/Waveform")->GetColour())));
		for (int64_t i=selEndCap;i<w;i++) {
			dc.DrawLine(i,peak[i],i,min[i]-1);
		}
	}
}

/// @brief Draw spectrum analyzer 
/// @param finaldc The DC to draw to.
/// @param weak    False if the visible portion of the display has changed.
/// @bug Slow when non-weak and the selection has to be drawn, see:
/// @ticket{951} Spectrum view scrolls/updates considerably slower when selection is visible
void AudioDisplay::DrawSpectrum(wxDC &finaldc,bool weak) {
	if (!weak || !spectrumDisplay || spectrumDisplay->GetWidth() != w || spectrumDisplay->GetHeight() != h) {
		if (spectrumDisplay) {
			delete spectrumDisplay;
			delete spectrumDisplaySelected;
			spectrumDisplay = 0;
			spectrumDisplaySelected = 0;
		}
		weak = false;
	}

	if (!weak) {
		if (!spectrumRenderer)
			spectrumRenderer = new AudioSpectrum(provider);

		spectrumRenderer->SetScaling(scale);

		unsigned char *img = (unsigned char *)malloc(h*w*3); // wxImage requires using malloc

		// Use a slightly slower, but simple way
		// Always draw the spectrum for the entire width
		// Hack: without those divs by 2 the display is horizontally compressed
		spectrumRenderer->RenderRange(Position*samples, (Position+w)*samples, false, img, 0, w, w, h);

		// The spectrum bitmap will have been deleted above already, so just make a new one
		wxImage imgobj(w, h, img, false);
		spectrumDisplay = new wxBitmap(imgobj);
	}

	if (hasSel && selStartCap < selEndCap && !spectrumDisplaySelected) {
		// There is a visible selection and we don't have a rendered one
		// This should be done regardless whether we're "weak" or not
		// Assume a few things were already set up when things were first rendered though
		unsigned char *img = (unsigned char *)malloc(h*w*3);
		spectrumRenderer->RenderRange(Position*samples, (Position+w)*samples, true, img, 0, w, w, h);
		wxImage imgobj(w, h, img, false);
		spectrumDisplaySelected = new wxBitmap(imgobj);
	}

	// Draw
	wxMemoryDC dc;
	dc.SelectObject(*spectrumDisplay);
	finaldc.Blit(0,0,w,h,&dc,0,0);

	if (hasSel && spectrumDisplaySelected && selStartCap < selEndCap) {
		dc.SelectObject(*spectrumDisplaySelected);
		finaldc.Blit(selStartCap, 0, selEndCap-selStartCap, h, &dc, selStartCap, 0);
	}
}

/// @brief Get selection position 
/// @param selStart 
/// @param selEnd   
/// @param cap      
void AudioDisplay::GetDialoguePos(int64_t &selStart,int64_t &selEnd, bool cap) {
	selStart = GetXAtMS(curStartMS);
	selEnd = GetXAtMS(curEndMS);

	if (cap) {
		if (selStart < 0) selStart = 0;
		if (selEnd < 0) selEnd = 0;
		if (selStart >= w) selStart = w-1;
		if (selEnd >= w) selEnd = w-1;
	}
}

/// @brief Get karaoke position 
/// @param karStart 
/// @param karEnd   
/// @param cap      
void AudioDisplay::GetKaraokePos(int64_t &karStart,int64_t &karEnd, bool cap) {
	try {
		// Wrap around
		int nsyls = (int)karaoke->syllables.size();
		if (karaoke->curSyllable == -1) {
			karaoke->SetSyllable(nsyls-1);
		}
		if (karaoke->curSyllable >= nsyls) karaoke->curSyllable = nsyls-1;

		// Get positions
		int pos = karaoke->syllables.at(karaoke->curSyllable).start_time;
		int len = karaoke->syllables.at(karaoke->curSyllable).duration;
		karStart = GetXAtMS(curStartMS+pos*10);
		karEnd = GetXAtMS(curStartMS+pos*10+len*10);

		// Cap
		if (cap) {
			if (karStart < 0) karStart = 0;
			if (karEnd < 0) karEnd = 0;
			if (karStart >= w) karStart = w-1;
			if (karEnd >= w) karEnd = w-1;
		}
	}
	catch (...) {
	}
}

/// @brief Update 
/// @return 
void AudioDisplay::Update() {
	if (blockUpdate) return;
	if (loaded) {
		if (OPT_GET("Audio/Auto/Scroll")->GetBool())
			MakeDialogueVisible();
		else
			UpdateImage(true);
	}
}

/// @brief Recreate the image 
void AudioDisplay::RecreateImage() {
	GetClientSize(&w,&h);
	h -= OPT_GET("Audio/Display/Draw/Timeline")->GetBool() ? 20 : 0;
	delete origImage;
	origImage = NULL;
	UpdateImage(false);
}

/// @brief Make dialogue visible 
/// @param force 
void AudioDisplay::MakeDialogueVisible(bool force) {
	// Variables
	int startShow=0, endShow=0;
	if (karaoke->enabled) {
		// In karaoke mode the syllable and as much as possible towards the end of the line should be shown
		int dummy = 0;
		GetTimesSelection(startShow, dummy);
		GetTimesDialogue(dummy, endShow);
	} else {
		GetTimesSelection(startShow,endShow);
	}
	int startPos = GetSampleAtMS(startShow);
	int endPos = GetSampleAtMS(endShow);
	int startX = GetXAtMS(startShow);
	int endX = GetXAtMS(endShow);

	if (force || startX < 50 || endX > w-50) {
		if (startX < 50 || endX - startX >= w) {
			// Make sure the left edge of the selection is at least 50 pixels from the edge of the display
			UpdatePosition(startPos - 50*samples, true);
		} else {
			// Otherwise center the selection in display
			UpdatePosition((startPos+endPos-w*samples)/2,true);
		}
	}

	// Update
	UpdateImage();
}

/// @brief Set position 
/// @param pos 
void AudioDisplay::SetPosition(int pos) {
	Position = pos;
	PositionSample = pos * samples;
	UpdateImage();
}

/// @brief Update position 
/// @param pos      
/// @param IsSample 
/// @return 
void AudioDisplay::UpdatePosition (int pos,bool IsSample) {
	// Safeguards
	if (!provider) return;
	if (IsSample) pos /= samples;
	int len = provider->GetNumSamples() / samples;
	if (pos < 0) pos = 0;
	if (pos >= len) pos = len-1;

	// Set
	Position = pos;
	PositionSample = pos*samples;
	UpdateScrollbar();
}

/// @brief Note: aka Horizontal Zoom Set samples in percentage 
/// @param percent 
/// @param update  
/// @param pivot   
/// @return 
void AudioDisplay::SetSamplesPercent(int percent,bool update,float pivot) {
	// Calculate
	if (percent < 1) percent = 1;
	if (percent > 100) percent = 100;
	if (samplesPercent == percent) return;
	samplesPercent = percent;

	// Update
	if (update) {
		// Center scroll
		int oldSamples = samples;
		UpdateSamples();
		PositionSample += int64_t((oldSamples-samples)*w*pivot);
		if (PositionSample < 0) PositionSample = 0;

		// Update
		UpdateSamples();
		UpdateScrollbar();
		UpdateImage();
		Refresh(false);
	}
}

/// @brief Update samples 
/// @return 
void AudioDisplay::UpdateSamples() {
	// Set samples
	if (!provider) return;
	if (w) {
		int64_t totalSamples = provider->GetNumSamples();
		int total = totalSamples / w;
		int max = 5760000 / w;	// 2 minutes at 48 kHz maximum
		if (total > max) total = max;
		int min = 8;
		if (total < min) total = min;
		int range = total-min;
		samples = int(range*pow(samplesPercent/100.0,3)+min);

		// Set position
		int length = w * samples;
		if (PositionSample + length > totalSamples) {
			PositionSample = totalSamples - length;
			if (PositionSample < 0) PositionSample = 0;
			if (samples) Position = PositionSample / samples;
		}
	}
}

/// @brief Set scale 
/// @param _scale 
/// @return 
void AudioDisplay::SetScale(float _scale) {
	if (scale == _scale) return;
	scale = _scale;
	UpdateImage();
}

/// @brief Load from file 
/// @param file 
/// @return 
void AudioDisplay::SetFile(wxString file) {
	// Unload
	if (file.IsEmpty()) try {
		try {
			if (player) player->CloseStream();
		}
		catch (const wxChar *e) {
			wxLogError(e);
		}
		delete provider;
		delete player;
		delete spectrumRenderer;
		provider = NULL;
		player = NULL;
		spectrumRenderer = NULL;
		try {
			Reset();
		}
		catch (const wxChar *e) {
			wxLogError(e);
		}

		loaded = false;
		temporary = false;
		StandardPaths::SetPathValue(_T("?audio"),_T(""));
	}
	catch (wxString e) {
		wxLogError(e);
	}
	catch (const wxChar *e) {
		wxLogError(e);
	}
	catch (...) {
		wxLogError(_T("Unknown error unloading audio"));
	}

	// Load
	else {
		SetFile(_T(""));
		try {
			// Get provider
			bool is_dummy = false;
#ifdef _DEBUG
			if (file == _T("?dummy")) {
				is_dummy = true;
				provider = new DummyAudioProvider(150*60*1000, false); // 150 minutes non-noise
			} else if (file == _T("?noise")) {
				is_dummy = true;
				provider = new DummyAudioProvider(150*60*1000, true); // 150 minutes noise
			} else {
				provider = AudioProviderFactoryManager::GetAudioProvider(file);
			}
#else
			provider = AudioProviderFactoryManager::GetAudioProvider(file);
#endif

			// Get player
			player = AudioPlayerFactoryManager::GetAudioPlayer();
			player->SetDisplayTimer(&UpdateTimer);
			player->SetProvider(provider);
			player->OpenStream();
			loaded = true;

			// Add to recent
			if (!is_dummy) {
				config::mru->Add("Audio", STD_STR(file));
				wxFileName fn(file);
				StandardPaths::SetPathValue(_T("?audio"),fn.GetPath());
			}

			// Update
			UpdateImage();
		}
		catch (const wxChar *e) {
			if (player) { delete player; player = 0; }
			if (provider) { delete provider; provider = 0; }
			wxLogError(e);
		}
		catch (wxString &err) {
			if (player) { delete player; player = 0; }
			if (provider) { delete provider; provider = 0; }
			wxMessageBox(err,_T("Error loading audio"),wxICON_ERROR | wxOK);
		}
		catch (...) {
			if (player) { delete player; player = 0; }
			if (provider) { delete provider; provider = 0; }
			wxLogError(_T("Unknown error loading audio"));
		}
	}
	
	if (!loaded) return;

	assert(loaded == (provider != NULL));

	// Set default selection
	AssDialogue *dlg = grid->GetActiveLine();
	SetDialogue(grid,dlg,grid->GetDialogueIndex(dlg));
}

/// @brief Load from video 
void AudioDisplay::SetFromVideo() {
	if (VideoContext::Get()->IsLoaded()) {
		wxString extension = VideoContext::Get()->videoName.Right(4);
		extension.LowerCase();

		if (extension != _T(".d2v")) SetFile(VideoContext::Get()->videoName);
	}
}

/// @brief Reload audio 
void AudioDisplay::Reload() {
	if (provider) SetFile(provider->GetFilename());
}

/// @brief Update scrollbar 
/// @return 
void AudioDisplay::UpdateScrollbar() {
	if (!provider) return;
	int page = w/12;
	int len = provider->GetNumSamples() / samples / 12;
	Position = PositionSample / samples;
	ScrollBar->SetScrollbar(Position/12,page,len,int(page*0.7),true);
}

/// @brief Gets the sample number at the x coordinate 
/// @param x 
/// @return 
int64_t AudioDisplay::GetSampleAtX(int x) {
	return (x+Position)*samples;
}

/// @brief Gets the x coordinate corresponding to sample 
/// @param n 
/// @return 
int AudioDisplay::GetXAtSample(int64_t n) {
	return samples ? (n/samples)-Position : 0;
}

/// @brief Get MS from X 
/// @param x 
/// @return 
int AudioDisplay::GetMSAtX(int64_t x) {
	return (PositionSample+(x*samples)) * 1000 / provider->GetSampleRate();
}

/// @brief Get X from MS 
/// @param ms 
/// @return 
int AudioDisplay::GetXAtMS(int64_t ms) {
	return ((ms * provider->GetSampleRate() / 1000)-PositionSample)/samples;
}

/// @brief Get MS At sample 
/// @param x 
/// @return 
int AudioDisplay::GetMSAtSample(int64_t x) {
	return x * 1000 / provider->GetSampleRate();
}

/// @brief Get Sample at MS 
/// @param ms 
/// @return 
int64_t AudioDisplay::GetSampleAtMS(int64_t ms) {
	return ms * provider->GetSampleRate() / 1000;
}

/// @brief Play 
/// @param start 
/// @param end   
/// @return 
void AudioDisplay::Play(int start,int end) {
	Stop();

	// Check provider
	if (!provider) {
		return;
	}

	// Set defaults
	playingToEnd = end < 0;
	int64_t num_samples = provider->GetNumSamples();
	start = GetSampleAtMS(start);
	if (end != -1) end = GetSampleAtMS(end);
	else end = num_samples-1;

	// Sanity checking
	if (start < 0) start = 0;
	if (start >= num_samples) start = num_samples-1;
	if (end >= num_samples) end = num_samples-1;
	if (end < start) end = start;

	// Redraw the image to avoid any junk left over from mouse movements etc
	// See issue #598
	UpdateImage(true);

	// Call play
	player->Play(start,end-start);
}

/// @brief Stop 
void AudioDisplay::Stop() {
	if (VideoContext::Get()->IsPlaying()) VideoContext::Get()->Stop();
	if (player) player->Stop();
}

/// @brief Get samples of dialogue 
/// @param start 
/// @param end   
/// @return 
void AudioDisplay::GetTimesDialogue(int &start,int &end) {
	if (!dialogue) {
		start = 0;
		end = 0;
		return;
	}

	start = dialogue->Start.GetMS();
	end = dialogue->End.GetMS();
}

/// @brief Get samples of selection 
/// @param start 
/// @param end   
/// @return 
void AudioDisplay::GetTimesSelection(int &start,int &end) {
	start = 0;
	end = 0;
	if (!dialogue) return;

	try {
		if (karaoke->enabled) {
			int pos = karaoke->syllables.at(karaoke->curSyllable).start_time;
			int len = karaoke->syllables.at(karaoke->curSyllable).duration;
			start = curStartMS+pos*10;
			end = curStartMS+pos*10+len*10;
		}
		else {
			start = curStartMS;
			end = curEndMS;
		}
	}
	catch (...) {}
}

/// @brief Set the current selection 
/// @param start 
/// @param end   
void AudioDisplay::SetSelection(int start, int end) {
	curStartMS = start;
	curEndMS = end;
	Update();
}

/// @brief Set dialogue 
/// @param _grid 
/// @param diag  
/// @param n     
/// @return 
void AudioDisplay::SetDialogue(SubtitlesGrid *_grid,AssDialogue *diag,int n) {
	// Actual parameters
	if (_grid) {
		// Set variables
		grid = _grid;
		line_n = n;
		dialogue = diag;

		// Set flags
		diagUpdated = false;
		NeedCommit = false;

		// Set times
		if (dialogue && !dontReadTimes && OPT_GET("Audio/Grab Times on Select")->GetBool()) {
			int s = dialogue->Start.GetMS();
			int e = dialogue->End.GetMS();

			// Never do it for 0:00:00.00->0:00:00.00 lines
			if (s != 0 || e != 0) {
				curStartMS = s;
				curEndMS = e;
			}
		}
	}

	// Read karaoke data
	if (dialogue && karaoke->enabled) {
		NeedCommit = karaoke->LoadFromDialogue(dialogue);

		// Reset karaoke pos
		if (karaoke->curSyllable == -1) karaoke->SetSyllable((int)karaoke->syllables.size()-1);
		else karaoke->SetSyllable(0);
	}

	// Update
	Update();
}

/// @brief Commit changes 
/// @param nextLine 
/// @return 
void AudioDisplay::CommitChanges (bool nextLine) {
	// Loaded?
	if (!loaded) return;

	// Check validity
	bool textNeedsCommit = grid->GetDialogue(line_n)->Text != grid->editBox->TextEdit->GetText();
	bool timeNeedsCommit = grid->GetDialogue(line_n)->Start.GetMS() != curStartMS || grid->GetDialogue(line_n)->End.GetMS() != curEndMS;
	if (timeNeedsCommit || textNeedsCommit) NeedCommit = true;
	bool wasKaraSplitting = false;
	bool validCommit = true;
	if (!karaoke->enabled && !karaoke->splitting) {
		if (!NeedCommit || curEndMS < curStartMS) validCommit = false;
	}

	// Update karaoke
	int karaSelStart = 0, karaSelEnd = -1;
	if (karaoke->enabled) {
		wasKaraSplitting = karaoke->splitting;
		karaoke->Commit();
		// Get karaoke selection
		karaSelStart = karaoke->syllables.size();
		for (size_t k = 0; k < karaoke->syllables.size(); ++k) {
			if (karaoke->syllables[k].selected) {
				if ((signed)k < karaSelStart) karaSelStart = k;
				if ((signed)k > karaSelEnd) karaSelEnd = k;
			}
		}
	}
	
	// Get selected rows
	wxArrayInt sel = grid->GetSelection();

	// Commit ok?
	if (validCommit) {
		// Reset flags
		diagUpdated = false;
		NeedCommit = false;

		// Update dialogues
		blockUpdate = true;
		AssDialogue *curDiag;
		for (size_t i=0;i<sel.GetCount();i++) {
			if (grid->IsInSelection(line_n)) curDiag = grid->GetDialogue(sel[i]);
			else curDiag = grid->GetDialogue(line_n);
			if (timeNeedsCommit) {
				curDiag->Start.SetMS(curStartMS);
				curDiag->End.SetMS(curEndMS);
			}
			if (!karaoke->enabled && textNeedsCommit) {
				// If user was editing karaoke stuff, that should take precedence of manual changes in the editbox,
				// so only update from editbox when not in kara mode
				curDiag->Text = grid->editBox->TextEdit->GetText();
			}
			if (!grid->IsInSelection(line_n)) break;
		}

		// Update edit box
		grid->editBox->StartTime->Update();
		grid->editBox->EndTime->Update();
		grid->editBox->Duration->Update();

		// Update grid
		grid->editBox->Update(!karaoke->enabled);
		grid->ass->Commit(_T(""));
		grid->CommitChanges();
		karaoke->SetSelection(karaSelStart, karaSelEnd);
		blockUpdate = false;
	}

	// Next line (ugh what a condition, can this be simplified?)
	if (nextLine && !karaoke->enabled && OPT_GET("Audio/Next Line on Commit")->GetBool() && !wasKaraSplitting) {
		// Insert a line if it doesn't exist
		int nrows = grid->GetRows();
		if (nrows == line_n + 1) {
			AssDialogue *def = new AssDialogue;
			def->Start = grid->GetDialogue(line_n)->End;
			def->End = grid->GetDialogue(line_n)->End;
			def->End.SetMS(def->End.GetMS()+OPT_GET("Timing/Default Duration")->GetInt());
			def->Style = grid->GetDialogue(line_n)->Style;
			grid->InsertLine(def,line_n,true);
			curStartMS = curEndMS;
			curEndMS = curStartMS + OPT_GET("Timing/Default Duration")->GetInt();
		}
		else if (grid->GetDialogue(line_n+1)->Start.GetMS() == 0 && grid->GetDialogue(line_n+1)->End.GetMS() == 0) {
			curStartMS = curEndMS;
			curEndMS = curStartMS + OPT_GET("Timing/Default Duration")->GetInt();
		}
		else {
			curStartMS = grid->GetDialogue(line_n+1)->Start.GetMS();
			curEndMS = grid->GetDialogue(line_n+1)->End.GetMS();
		}
		
		// Go to next
		dontReadTimes = true;
		ChangeLine(1,sel.GetCount() > 1 ? true : false);
		dontReadTimes = false;
	}

	Update();
}

/// @brief Add lead 
/// @param in  
/// @param out 
void AudioDisplay::AddLead(bool in,bool out) {
	// Lead in
	if (in) {
		curStartMS -= OPT_GET("Audio/Lead/IN")->GetInt();
		if (curStartMS < 0) curStartMS = 0;
	}

	// Lead out
	if (out) {
		curEndMS += OPT_GET("Audio/Lead/OUT")->GetInt();
	}

	// Set changes
	UpdateTimeEditCtrls();
	NeedCommit = true;
	if (OPT_GET("Audio/Auto/Commit")->GetBool()) CommitChanges();
	Update();
}

///////////////
// Event table
BEGIN_EVENT_TABLE(AudioDisplay, wxWindow)
    EVT_MOUSE_EVENTS(AudioDisplay::OnMouseEvent)
    EVT_PAINT(AudioDisplay::OnPaint)
	EVT_SIZE(AudioDisplay::OnSize)
	EVT_TIMER(Audio_Update_Timer,AudioDisplay::OnUpdateTimer)
	EVT_KEY_DOWN(AudioDisplay::OnKeyDown)
	EVT_SET_FOCUS(AudioDisplay::OnGetFocus)
	EVT_KILL_FOCUS(AudioDisplay::OnLoseFocus)
END_EVENT_TABLE()

/// @brief Paint 
/// @param event 
/// @return 
void AudioDisplay::OnPaint(wxPaintEvent& event) {
	if (w == 0 || h == 0) return;
	DoUpdateImage();

	wxPaintDC dc(this);
	if (origImage) dc.DrawBitmap(*origImage,0,0);
}

/// @brief Mouse event 
/// @param event 
/// @return 
void AudioDisplay::OnMouseEvent(wxMouseEvent& event) {
	// Get x,y
	int64_t x = event.GetX();
	int64_t y = event.GetY();
	bool karMode = karaoke->enabled;
	bool shiftDown = event.m_shiftDown;
	int timelineHeight = OPT_GET("Audio/Display/Draw/Timeline")->GetBool() ? 20 : 0;

	// Leaving event
	if (event.Leaving()) {
		event.Skip();
		return;
	}

	if (!player || !provider) {
		event.Skip();
		return;
	}

	// Is inside?
	bool inside = false;
	bool onScale = false;
	if (x >= 0 && y >= 0 && x < w) {
		if (y < h) {
			inside = true;

			// Get focus
			if (wxWindow::FindFocus() != this && OPT_GET("Audio/Auto/Focus")->GetBool()) SetFocus();
		}
		else if (y < h+timelineHeight) onScale = true;
	}

	// Buttons
	bool leftIsDown = event.ButtonIsDown(wxMOUSE_BTN_LEFT);
	bool rightIsDown = event.ButtonIsDown(wxMOUSE_BTN_RIGHT);
	bool buttonIsDown = leftIsDown || rightIsDown;
	bool leftClick = event.ButtonDown(wxMOUSE_BTN_LEFT);
	bool rightClick = event.ButtonDown(wxMOUSE_BTN_RIGHT);
	bool middleClick = event.Button(wxMOUSE_BTN_MIDDLE);
	bool buttonClick = leftClick || rightClick;
	bool defCursor = true;

	// Click type
	if (buttonClick && !holding) {
		holding = true;
		CaptureMouse();
	}
	if (!buttonIsDown && holding) {
		holding = false;
		if (HasCapture()) ReleaseMouse();
	}

	// Mouse wheel
	if (event.GetWheelRotation() != 0) {
		// Zoom or scroll?
		bool zoom = shiftDown;
		if (OPT_GET("Audio/Wheel Default to Zoom")->GetBool()) zoom = !zoom;

		// Zoom
		if (zoom) {
#ifdef __APPLE__
			// Reverse scroll directions on Apple... ugly hack
			// Otherwise left=right and right=left on systems that support four-way scrolling.
			int step = -event.GetWheelRotation() / event.GetWheelDelta();
#else
			int step = event.GetWheelRotation() / event.GetWheelDelta();
#endif
			int value = box->HorizontalZoom->GetValue()+step;
			box->HorizontalZoom->SetValue(value);
			SetSamplesPercent(value,true,float(x)/float(w));
		}

		// Scroll
		else {
			int step = -event.GetWheelRotation() * w / 360;
			UpdatePosition(Position+step,false);
			UpdateImage();
		}
	}

	// Cursor drawing
	if (player && !player->IsPlaying() && origImage) {
		// Draw bg
		wxClientDC dc(this);
		if (origImage) dc.DrawBitmap(*origImage,0,0);

		if (inside) {
			// Draw cursor
			dc.SetLogicalFunction(wxINVERT);
			dc.DrawLine(x,0,x,h);

			// Time
			if (OPT_GET("Audio/Display/Draw/Cursor Time")->GetBool()) {
				// Time string
				AssTime time;
				time.SetMS(GetMSAtX(x));
				wxString text = time.GetASSFormated();

				// Calculate metrics
				// FIXME: Hardcoded font name
				wxFont font(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"));
				dc.SetFont(font);
				int tw,th;
				GetTextExtent(text,&tw,&th,NULL,NULL,&font);

				// Set inversion
				bool left = true;
				if (x > w/2) left = false;

				// Text coordinates
				int dx;
				dx = x - tw/2;
				if (dx < 4) dx = 4;
				int max = w - tw - 4;
				if (dx > max) dx = max;
				int dy = 4;
				if (karMode) dy += th;

				// Draw text
				dc.SetTextForeground(wxColour(64,64,64));
				dc.DrawText(text,dx+1,dy-1);
				dc.DrawText(text,dx+1,dy+1);
				dc.DrawText(text,dx-1,dy-1);
				dc.DrawText(text,dx-1,dy+1);
				dc.SetTextForeground(wxColour(255,255,255));
				dc.DrawText(text,dx,dy);
			}
		}
	}

	// Scale dragging
	if ((hold == 0 && onScale) || draggingScale) {
		if (event.ButtonDown(wxMOUSE_BTN_LEFT)) {
			lastDragX = x;
			draggingScale = true;
		}
		else if (holding) {
			int delta = lastDragX - x;
			lastDragX = x;
			UpdatePosition(Position + delta);
			UpdateImage();
			Refresh(false);
			SetCursor(wxNullCursor);
			return;
		}
		else draggingScale = false;
	}

	// Outside
	if (!inside && hold == 0) return;

	// Left click
	if (leftClick) {
		SetFocus();
	}

	// Right click
	if (rightClick) {
		SetFocus();
		if (karaoke->enabled) {
			int syl = GetSyllableAtX(x);
			if (syl != -1) {
				int start = karaoke->syllables.at(syl).start_time * 10 + dialogue->Start.GetMS();
				int count = karaoke->syllables.at(syl).duration * 10;
				Play(start, start+count);
			}
		}
	}

	// Middle click
	if (middleClick) {
		SetFocus();
		if (VideoContext::Get()->IsLoaded()) {
			VideoContext::Get()->JumpToTime(GetMSAtX(x));
		}
	}

	// Timing
	if (hasSel) {
		bool updated = false;
							
		// Grab start/end
		if (hold == 0) {
			bool gotGrab = false;
			bool karTime = karMode && !
#ifdef __APPLE__
				event.CmdDown();
#else
				event.ControlDown();
#endif

			// Line timing mode
			if (!karTime) {
				// Grab start
				if (abs64 (x - selStart) < 6 && OPT_GET("Audio/Display/Dragging Times")->GetBool()==false) {
					wxCursor cursor(wxCURSOR_SIZEWE);
					SetCursor(cursor);
					defCursor = false;
					if (buttonClick) {
						hold = 1;
						gotGrab = true;
					}
				}

				// Grab end
				else if (abs64 (x - selEnd) < 6 && OPT_GET("Audio/Display/Dragging Times")->GetBool()==false) {
					wxCursor cursor(wxCURSOR_SIZEWE);
					SetCursor(cursor);
					defCursor = false;
					if (buttonClick) {
						hold = 2;
						gotGrab = true;
					}
				}

				// Dragging nothing, time from scratch
				else {
					if (buttonClick) {
						if (leftClick) hold = 3;
						else hold = 2;
						lastX = x;
						gotGrab = true;
					}
				}
			}

			// Karaoke mode
			else {
				// Look for a syllable
				int64_t pos,len,curpos;
				AudioKaraokeSyllable *curSyl;
				size_t karn = karaoke->syllables.size();
				for (size_t i=0;i<karn;i++) {
					curSyl = &karaoke->syllables.at(i);
					len = curSyl->duration*10;
					curpos = curSyl->start_time*10;
					if (len != -1) {
						pos = GetXAtMS(curStartMS+len+curpos);

						// Grabbing syllable boundary
						if (abs64 (x - pos) < 7) {
							wxCursor cursor(wxCURSOR_SIZEWE);
							SetCursor(cursor);
							defCursor = false;
							if (event.LeftIsDown()) {
								hold = 4;
								holdSyl = (int)i;
								gotGrab = true;
							}
							break;
						}
					}
				}

				// No syllable found, select if possible
				if (hold == 0 && leftClick) {
					int syl = GetSyllableAtX(x);
					if (syl != -1) {
						karaoke->SetSyllable(syl);
						updated = true;
					}
				}
			}
		}

		// Drag start/end
		if (hold != 0) {
			// Dragging
			if (buttonIsDown) {
				// Drag from nothing or straight timing
				if (hold == 3 && buttonIsDown) {
					if (!karMode) {
						if (leftIsDown) curStartMS = GetMSAtX(x);
						else curEndMS = GetMSAtX(x);
						updated = true;
						diagUpdated = true;

						if (leftIsDown && abs((long)(x-lastX)) > OPT_GET("Audio/Start Drag Sensitivity")->GetInt()) {
							selStart = lastX;
							selEnd = x;
							curStartMS = GetBoundarySnap(GetMSAtX(lastX),10,event.ShiftDown(),true);
							curEndMS = GetMSAtX(x);
							hold = 2;
						}
					}
				}

				// Drag start
				if (hold == 1 && buttonIsDown) {
					// Set new value
					if (x != selStart) {
						int snapped = GetBoundarySnap(GetMSAtX(x),10,event.ShiftDown(),true);
						selStart = GetXAtMS(snapped);
						if (selStart > selEnd) {
							int temp = selStart;
							selStart = selEnd;
							selEnd = temp;
							hold = 2;
							curEndMS = snapped;
							snapped = GetMSAtX(selStart);
						}
						curStartMS = snapped;
						updated = true;
						diagUpdated = true;
					}
				}

				// Drag end
				if (hold == 2 && buttonIsDown) {
					// Set new value
					if (x != selEnd) {
						int snapped = GetBoundarySnap(GetMSAtX(x),10,event.ShiftDown(),false);
						selEnd = GetXAtMS(snapped);
						//selEnd = GetBoundarySnap(x,event.ShiftDown()?0:10,false);
						if (selStart > selEnd) {
							int temp = selStart;
							selStart = selEnd;
							selEnd = temp;
							hold = 1;
							curStartMS = snapped;
							snapped = GetMSAtX(selEnd);
						}
						curEndMS = snapped;
						updated = true;
						diagUpdated = true;
					}
				}

				// Drag karaoke
				if (hold == 4 && leftIsDown) {
					// Set new value
					int curpos,len,pos,nkar;
					AudioKaraokeSyllable *curSyl=NULL,*nextSyl=NULL;
					curSyl = &karaoke->syllables.at(holdSyl);
					nkar = (int)karaoke->syllables.size();
					if (holdSyl < nkar-1) {
						nextSyl = &karaoke->syllables.at(holdSyl+1);
					}
					curpos = curSyl->start_time;
					len = curSyl->duration;
					pos = GetXAtMS(curStartMS+(len+curpos)*10);
					if (x != pos) {
						// Calculate delta in centiseconds
						int delta = ((int64_t)(x-pos)*samples*100)/provider->GetSampleRate();

						// Apply delta
						int deltaMode = 0;
						if (shiftDown) deltaMode = 1;
						// else if (ctrlDown) deltaMode = 2;
						bool result = karaoke->SyllableDelta(holdSyl,delta,deltaMode);
						if (result) {
							updated = true;
							diagUpdated = true;
						}
					}
				}
			}

			// Release
			else {
				// Commit changes
				if (diagUpdated) {
					diagUpdated = false;
					NeedCommit = true;
					if (curStartMS <= curEndMS) {
						UpdateTimeEditCtrls();
						if (OPT_GET("Audio/Auto/Commit")->GetBool()) CommitChanges();
					}

					else UpdateImage(true);
				}

				// Update stuff
				SetCursor(wxNullCursor);
				hold = 0;
			}
		}

		// Update stuff
		if (updated) {
			if (diagUpdated) NeedCommit = true;
			if (karaoke->enabled && !playingToEnd) {
				AudioKaraokeSyllable &syl = karaoke->syllables[karaoke->curSyllable];
				player->SetEndPosition(GetSampleAtMS(curStartMS + (syl.start_time+syl.duration)*10));
			} else if (!playingToEnd) {
				player->SetEndPosition(GetSampleAtX(selEnd));
			}
			if (hold != 0) {
				wxCursor cursor(wxCURSOR_SIZEWE);
				SetCursor(cursor);
			}
			UpdateImage(true);
		}
	}

	// Not holding
	else {
		hold = 0;
	}

	// Restore cursor
	if (defCursor) SetCursor(wxNullCursor);
}

/// @brief Get snap to boundary 
/// @param ms        
/// @param rangeX    
/// @param shiftHeld 
/// @param start     
/// @return 
int AudioDisplay::GetBoundarySnap(int ms,int rangeX,bool shiftHeld,bool start) {
	// Range?
	if (rangeX <= 0) return ms;

	// Convert range into miliseconds
	int rangeMS = rangeX*samples*1000 / provider->GetSampleRate();

	// Keyframe boundaries
	wxArrayInt boundaries;
	bool snapKey = OPT_GET("Audio/Display/Snap/Keyframes")->GetBool();
	if (shiftHeld) snapKey = !snapKey;
	if (snapKey && VideoContext::Get()->KeyFramesLoaded() && OPT_GET("Audio/Display/Draw/Keyframes")->GetBool()) {
		int64_t keyMS;
		std::vector<int> keyFrames = VideoContext::Get()->GetKeyFrames();
		int frame;
		for (unsigned int i=0;i<keyFrames.size();i++) {
			frame = keyFrames[i];
			if (!start) frame--;
			if (frame < 0) frame = 0;
			keyMS = VideoContext::Get()->TimeAtFrame(frame,start ? agi::vfr::START : agi::vfr::END);
			//if (start) keyX++;
			if (GetXAtMS(keyMS) >= 0 && GetXAtMS(keyMS) < w) boundaries.Add(keyMS);
		}
	}

	// Other subtitles' boundaries
	int inactiveType = OPT_GET("Audio/Inactive Lines Display Mode")->GetInt();
	bool snapLines = OPT_GET("Audio/Display/Snap/Other Lines")->GetBool();
	if (shiftHeld) snapLines = !snapLines;
	if (snapLines && (inactiveType == 1 || inactiveType == 2)) {
		AssDialogue *shade;
		int shadeX1,shadeX2;
		int shadeFrom,shadeTo;

		// Get range
		if (inactiveType == 1) {
			shadeFrom = this->line_n-1;
			shadeTo = shadeFrom+1;
		}
		else {
			shadeFrom = 0;
			shadeTo = grid->GetRows();
		}

		for (int j=shadeFrom;j<shadeTo;j++) {
			if (j == line_n) continue;
			shade = grid->GetDialogue(j);

			if (shade) {
				// Get coordinates
				shadeX1 = GetXAtMS(shade->Start.GetMS());
				shadeX2 = GetXAtMS(shade->End.GetMS());
				if (shadeX1 >= 0 && shadeX1 < w) boundaries.Add(shade->Start.GetMS());
				if (shadeX2 >= 0 && shadeX2 < w) boundaries.Add(shade->End.GetMS());
			}
		}
	}

	// See if ms falls within range of any of them
	int minDist = rangeMS+1;
	int bestMS = ms;
	for (unsigned int i=0;i<boundaries.Count();i++) {
		if (abs(ms-boundaries[i]) < minDist) {
			bestMS = boundaries[i];
			minDist = abs(ms-boundaries[i]);
		}
	}

	// Return best match
	return bestMS;
}

/// @brief Size event 
/// @param event 
void AudioDisplay::OnSize(wxSizeEvent &event) {
	// Set size
	GetClientSize(&w,&h);
	h -= OPT_GET("Audio/Display/Draw/Timeline")->GetBool() ? 20 : 0;

	// Update image
	UpdateSamples();
	if (samples) {
		UpdatePosition(PositionSample / samples);
	}
	UpdateImage();
	
	// Update scrollbar
	UpdateScrollbar();
}

/// @brief Timer event 
/// @param event 
/// @return 
void AudioDisplay::OnUpdateTimer(wxTimerEvent &event) {
	if (!origImage)
		return;

	// Get lock and check if it's OK
	if (player->GetMutex()) {
		wxMutexLocker locker(*player->GetMutex());
		if (!locker.IsOk()) return;
	}
		
	if (!player->IsPlaying()) return;

	// Get DCs
	//wxMutexGuiEnter();
	wxClientDC dc(this);

	// Draw cursor
	int curpos = -1;
	if (player->IsPlaying()) {
		int64_t curPos = player->GetCurrentPosition();
		if (curPos > player->GetStartPosition() && curPos < player->GetEndPosition()) {
			// Scroll if needed
			int posX = GetXAtSample(curPos);
			bool fullDraw = false;
			bool centerLock = false;
			bool scrollToCursor = OPT_GET("Audio/Lock Scroll on Cursor")->GetBool();
			if (centerLock) {
				int goTo = MAX(0,curPos - w*samples/2);
				if (goTo >= 0) {
					UpdatePosition(goTo,true);
					UpdateImage();
					fullDraw = true;
				}
			}
			else {
				if (scrollToCursor) {
					if (posX < 80 || posX > w-80) {
						int goTo = MAX(0,curPos - 80*samples);
						if (goTo >= 0) {
							UpdatePosition(goTo,true);
							UpdateImage();
							fullDraw = true;
						}
					}
				}
			}

			// Draw cursor
			wxMemoryDC src;
			curpos = GetXAtSample(curPos);
			if (curpos >= 0 && curpos < GetClientSize().GetWidth()) {
				dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Audio Display/Play Cursor")->GetColour())));
				src.SelectObject(*origImage);
				if (fullDraw) {
					//dc.Blit(0,0,w,h,&src,0,0);
					dc.DrawLine(curpos,0,curpos,h);
					//dc.Blit(0,0,curpos-10,h,&src,0,0);
					//dc.Blit(curpos+10,0,w-curpos-10,h,&src,curpos+10,0);
				}
				else {
					dc.Blit(oldCurPos,0,1,h,&src,oldCurPos,0);
					dc.DrawLine(curpos,0,curpos,h);
				}
			}
		}
		else {
			if (curPos > player->GetEndPosition() + 8192) {
				player->Stop();
			}
			wxMemoryDC src;
			src.SelectObject(*origImage);
			dc.Blit(oldCurPos,0,1,h,&src,oldCurPos,0);
		}
	}

	// Restore background
	else {
		wxMemoryDC src;
		src.SelectObject(*origImage);
		dc.Blit(oldCurPos,0,1,h,&src,oldCurPos,0);
	}
	oldCurPos = curpos;
}

/// @brief Key down 
/// @param event 
void AudioDisplay::OnKeyDown(wxKeyEvent &event) {
	int key = event.GetKeyCode();
#ifdef __APPLE__
	Hotkeys.SetPressed(key,event.m_metaDown,event.m_altDown,event.m_shiftDown);
#else
	Hotkeys.SetPressed(key,event.m_controlDown,event.m_altDown,event.m_shiftDown);
#endif

	// Accept
	if (Hotkeys.IsPressed(_T("Audio Commit"))) {
		CommitChanges(true);
		//ChangeLine(1);
	}

	// Accept (SSA's "Grab times")
	if (Hotkeys.IsPressed(_T("Audio Commit Alt"))) {
		CommitChanges(true);
	}

	// Accept (stay)
	if (Hotkeys.IsPressed(_T("Audio Commit (Stay)"))) {
		CommitChanges();
	}

	// Previous
	if (Hotkeys.IsPressed(_T("Audio Prev Line")) || Hotkeys.IsPressed(_T("Audio Prev Line Alt"))) {
		Prev();
	}

	// Next
	if (Hotkeys.IsPressed(_T("Audio Next Line")) || Hotkeys.IsPressed(_T("Audio Next Line Alt"))) {
		Next();
	}

	// Play
	if (Hotkeys.IsPressed(_T("Audio Play")) || Hotkeys.IsPressed(_T("Audio Play Alt"))) {
		int start=0,end=0;
		GetTimesSelection(start,end);
		Play(start,end);
	}

	// Play/Stop
	if (Hotkeys.IsPressed(_T("Audio Play or Stop"))) {
		if (player->IsPlaying()) Stop();
		else {
			int start=0,end=0;
			GetTimesSelection(start,end);
			Play(start,end);
		}
	}

	// Stop
	if (Hotkeys.IsPressed(_T("Audio Stop"))) {
		Stop();
	}

	// Increase length
	if (Hotkeys.IsPressed(_T("Audio Karaoke Increase Len"))) {
		if (karaoke->enabled) {
			bool result = karaoke->SyllableDelta(karaoke->curSyllable,1,0);
			if (result) diagUpdated = true;
		}
	}

	// Increase length (shift)
	if (Hotkeys.IsPressed(_T("Audio Karaoke Increase Len Shift"))) {
		if (karaoke->enabled) {
			bool result = karaoke->SyllableDelta(karaoke->curSyllable,1,1);
			if (result) diagUpdated = true;
		}
	}

	// Decrease length
	if (Hotkeys.IsPressed(_T("Audio Karaoke Decrease Len"))) {
		if (karaoke->enabled) {
			bool result = karaoke->SyllableDelta(karaoke->curSyllable,-1,0);
			if (result) diagUpdated = true;
		}
	}

	// Decrease length (shift)
	if (Hotkeys.IsPressed(_T("Audio Karaoke Decrease Len Shift"))) {
		if (karaoke->enabled) {
			bool result = karaoke->SyllableDelta(karaoke->curSyllable,-1,1);
			if (result) diagUpdated = true;
		}
	}

	// Move backwards
	if (Hotkeys.IsPressed(_T("Audio Scroll Left"))) {
		UpdatePosition(Position-128,false);
		UpdateImage();
	}

	// Move forward
	if (Hotkeys.IsPressed(_T("Audio Scroll Right"))) {
		UpdatePosition(Position+128,false);
		UpdateImage();
	}

	// Play first 500 ms
	if (Hotkeys.IsPressed(_T("Audio Play First 500ms"))) {
		int start=0,end=0;
		GetTimesSelection(start,end);
		int e = start+500;
		if (e > end) e = end;
		Play(start,e);
	}

	// Play last 500 ms
	if (Hotkeys.IsPressed(_T("Audio Play Last 500ms"))) {
		int start=0,end=0;
		GetTimesSelection(start,end);
		int s = end-500;
		if (s < start) s = start;
		Play(s,end);
	}

	// Play 500 ms before
	if (Hotkeys.IsPressed(_T("Audio Play 500ms Before"))) {
		int start=0,end=0;
		GetTimesSelection(start,end);
		Play(start-500,start);
	}

	// Play 500 ms after
	if (Hotkeys.IsPressed(_T("Audio Play 500ms After"))) {
		int start=0,end=0;
		GetTimesSelection(start,end);
		Play(end,end+500);
	}

	// Play to end of file
	if (Hotkeys.IsPressed(_T("Audio Play To End"))) {
		int start=0,end=0;
		GetTimesSelection(start,end);
		Play(start,-1);
	}

	// Play original line
	if (Hotkeys.IsPressed(_T("Audio Play Original Line"))) {
		int start=0,end=0;
		GetTimesDialogue(start,end);
		SetSelection(start, end);
		Play(start,end);
	}

	// Lead in
	if (Hotkeys.IsPressed(_T("Audio Add Lead In"))) {
		AddLead(true,false);
	}

	// Lead out
	if (Hotkeys.IsPressed(_T("Audio Add Lead Out"))) {
		AddLead(false,true);
	}

	// Update
	if (diagUpdated) {
		diagUpdated = false;
		NeedCommit = true;
		if (OPT_GET("Audio/Auto/Commit")->GetBool() && curStartMS <= curEndMS) CommitChanges();
		else UpdateImage(true);
	}
}

/// @brief Change line 
/// @param delta 
/// @param block 
/// @return 
void AudioDisplay::ChangeLine(int delta, bool block) {
	if (dialogue) {
		// Get next line number and make sure it's within bounds
		int next;
		if (block && grid->IsInSelection(line_n)) next = grid->GetLastSelRow()+delta;
		else next = line_n+delta;

		if (next == -1) next = 0;
		if (next == grid->GetRows()) next = grid->GetRows() - 1;

		// Set stuff
		NeedCommit = false;
		dialogue = NULL;
		grid->SetActiveLine(grid->GetDialogue(next));
		grid->SelectRow(next);
		grid->MakeCellVisible(next,0,true);
		if (!dialogue) UpdateImage(true);
		else UpdateImage(false);
		line_n = next;
	}
}

/// @brief Next 
/// @param play 
/// @return 
void AudioDisplay::Next(bool play) {
	// Karaoke
	if (karaoke->enabled) {
		int nextSyl = karaoke->curSyllable+1;
		bool needsUpdate = true;

		// Last syllable; jump to next
		if (nextSyl >= (signed int)karaoke->syllables.size()) {
			// Already last?
			if (line_n == grid->GetRows()-1) return;

			if (NeedCommit) {
				int result = wxMessageBox(_("Do you want to commit your changes? If you choose No, they will be discarded."),_("Commit?"),wxYES_NO | wxCANCEL | wxICON_QUESTION);
				//int result = wxNO;
				if (result == wxYES) {
					CommitChanges();
				}
				else if (result == wxCANCEL) {
					karaoke->curSyllable = (int)karaoke->syllables.size()-1;
					return;
				}
			}
			nextSyl = 0;
			karaoke->curSyllable = 0;
			ChangeLine(1);
			needsUpdate = false;
		}

		// Set syllable
		karaoke->SetSyllable(nextSyl);
		if (needsUpdate) Update();
		int start=0,end=0;
		GetTimesSelection(start,end);
		if (play) Play(start,end);
	}

	// Plain mode
	else {
		ChangeLine(1);
	}

}

/// @brief Previous 
/// @param play 
/// @return 
void AudioDisplay::Prev(bool play) {
	// Karaoke
	if (karaoke->enabled) {
		int nextSyl = karaoke->curSyllable-1;
		bool needsUpdate = true;

		// First syllable; jump line
		if (nextSyl < 0) {
			// Already first?
			if (line_n == 0) return;

			if (NeedCommit) {
				int result = wxMessageBox(_("Do you want to commit your changes? If you choose No, they will be discarded."),_("Commit?"),wxYES_NO | wxCANCEL);
				if (result == wxYES) {
					CommitChanges();
				}
				else if (result == wxCANCEL) {
					karaoke->curSyllable = 0;
					return;
				}
			}
			karaoke->curSyllable = -1;
			ChangeLine(-1);
			needsUpdate = false;
		}

		// Set syllable
		karaoke->SetSyllable(nextSyl);
		if (needsUpdate) Update();
		int start=0,end=0;
		GetTimesSelection(start,end);
		if (play) Play(start,end);
	}

	// Plain mode
	else {
		ChangeLine(-1);
	}

}

/// @brief Gets syllable at x position 
/// @param x 
/// @return 
int AudioDisplay::GetSyllableAtX(int x) {
	if (!karaoke->enabled) return -1;
	int ms = GetMSAtX(x);
	size_t syllables = karaoke->syllables.size();;
	int sylstart,sylend;

	// Find a matching syllable
	for (size_t i=0;i<syllables;i++) {
		sylstart = karaoke->syllables.at(i).start_time*10 + curStartMS;
		sylend = karaoke->syllables.at(i).duration*10 + sylstart;
		if (ms >= sylstart && ms < sylend) {
			return (int)i;
		}
	}
	return -1;
}

/// @brief Focus events 
/// @param event 
void AudioDisplay::OnGetFocus(wxFocusEvent &event) {
	if (!hasFocus) {
		hasFocus = true;
		UpdateImage(true);
	}
}

/// @brief DOCME
/// @param event 
void AudioDisplay::OnLoseFocus(wxFocusEvent &event) {
	if (hasFocus && loaded) {
		hasFocus = false;
		UpdateImage(true);
		Refresh(false);
	}
}

/// @brief Update time edit controls 
void AudioDisplay::UpdateTimeEditCtrls() {
	grid->editBox->StartTime->SetTime(curStartMS);
	grid->editBox->EndTime->SetTime(curEndMS);
	grid->editBox->Duration->SetTime(curEndMS-curStartMS);
}


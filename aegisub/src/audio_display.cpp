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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "config.h"

#include <wx/tglbtn.h>
#include <wx/filename.h>
#include <math.h>
#include <vector>
#include "audio_display.h"
#include "audio_provider_stream.h"
#include "main.h"
#include "ass_dialogue.h"
#include "subs_grid.h"
#include "ass_file.h"
#include "subs_edit_box.h"
#include "options.h"
#include "audio_karaoke.h"
#include "audio_box.h"
#include "fft.h"
#include "video_context.h"
#include "vfr.h"
#include "colorspace.h"
#include "hotkeys.h"
#include "utils.h"
#include "timeedit_ctrl.h"
#include "standard_paths.h"
#ifdef _DEBUG
#include "audio_provider_dummy.h"
#endif


#ifdef __WXMAC__
# define AudioDisplayWindowStyle wxWANTS_CHARS
#else
# define AudioDisplayWindowStyle wxSUNKEN_BORDER | wxWANTS_CHARS
#endif

///////////////
// Constructor
AudioDisplay::AudioDisplay(wxWindow *parent)
: wxWindow (parent, -1, wxDefaultPosition, wxSize(200,Options.AsInt(_T("Audio Display Height"))), AudioDisplayWindowStyle , _T("Audio Display"))
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
	scrubbing = false;
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
	h -= Options.AsBool(_T("Audio Draw Timeline")) ? 20 : 0;
	SetSamplesPercent(50,false);

	// Set cursor
	//wxCursor cursor(wxCURSOR_BLANK);
	//SetCursor(cursor);

	//wxLog::SetActiveTarget(new wxLogWindow(NULL,_T("Log"),true,false));
}


//////////////
// Destructor
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


/////////
// Reset
void AudioDisplay::Reset() {
	wxLogDebug(_T("AudioDisplay::Reset"));
	hasSel = false;
	diagUpdated = false;
	NeedCommit = false;
	karaoke->enabled = false;
	karaoke->syllables.clear();
	box->karaokeMode = false;
	box->KaraokeButton->SetValue(false);
	dialogue = NULL;
}


////////////////
// Update image
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

void AudioDisplay::DoUpdateImage() {
	// Loaded?
	if (!loaded || !provider) return;

	// Needs updating?
	if (!needImageUpdate) return;
	bool weak = needImageUpdateWeak;

	// Prepare bitmap
	int timelineHeight = Options.AsBool(_T("Audio Draw Timeline")) ? 20 : 0;
	int displayH = h+timelineHeight;
	if (origImage) {
		if (origImage->GetWidth() != w || origImage->GetHeight() != displayH) {
			delete origImage;
			origImage = NULL;
		}
	}

	// Options
	bool draw_boundary_lines = Options.AsBool(_T("Audio Draw Secondary Lines"));
	bool draw_selection_background = Options.AsBool(_T("Audio Draw Selection Background"));
	bool drawKeyframes = Options.AsBool(_T("Audio Draw Keyframes"));

	// Invalid dimensions
	if (w == 0 || displayH == 0) return;

	// New bitmap
	if (!origImage) origImage = new wxBitmap(w,displayH,-1);

	// Is spectrum?
	bool spectrum = false;
	if (provider && Options.AsBool(_T("Audio Spectrum"))) {
		spectrum = true;
	}

	// Draw image to be displayed
	wxMemoryDC dc;
	dc.SelectObject(*origImage);

	// Black background
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(Options.AsColour(_T("Audio Background"))));
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
		if (NeedCommit && !karaoke->enabled) dc.SetBrush(wxBrush(Options.AsColour(_T("Audio Selection Background Modified"))));
		else dc.SetBrush(wxBrush(Options.AsColour(_T("Audio Selection Background"))));
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
		dc.SetPen(wxPen(Options.AsColour(_T("Audio Seconds Boundaries")),1,wxDOT));
		if (pixBounds >= 8) {
			for (int x=0;x<w;x++) {
				if (((x*samples)+start) % rate < samples) {
					dc.DrawLine(x,0,x,h);
				}
			}
		}
	}

	// Draw current frame
	if (Options.AsBool(_T("Audio Draw Video Position"))) {
		if (VideoContext::Get()->IsLoaded()) {
			dc.SetPen(wxPen(Options.AsColour(_T("Audio Play Cursor")),2,wxLONG_DASH));
			int x = GetXAtMS(VFR_Output.GetTimeAtFrame(VideoContext::Get()->GetFrameN()));
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
			int selWidth = Options.AsInt(_T("Audio Line boundaries Thickness"));
			dc.SetPen(wxPen(Options.AsColour(_T("Audio Line boundary start"))));
			dc.SetBrush(wxBrush(Options.AsColour(_T("Audio Line boundary start"))));
			dc.DrawRectangle(lineStart-selWidth/2+1,0,selWidth,h);
			wxPoint points1[3] = { wxPoint(lineStart,0), wxPoint(lineStart+10,0), wxPoint(lineStart,10) };
			wxPoint points2[3] = { wxPoint(lineStart,h-1), wxPoint(lineStart+10,h-1), wxPoint(lineStart,h-11) };
			dc.DrawPolygon(3,points1);
			dc.DrawPolygon(3,points2);

			// Draw end boundary
			dc.SetPen(wxPen(Options.AsColour(_T("Audio Line boundary end"))));
			dc.SetBrush(wxBrush(Options.AsColour(_T("Audio Line boundary end"))));
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
				wxPen curPen(Options.AsColour(_T("Audio Syllable boundaries")),1,wxDOT);
				dc.SetPen(curPen);
				wxFont curFont(9,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"),wxFONTENCODING_SYSTEM);
				dc.SetFont(curFont);
				if (!spectrum) dc.SetTextForeground(Options.AsColour(_T("Audio Syllable text")));
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


///////////////////////
// Draw Inactive Lines
void AudioDisplay::DrawInactiveLines(wxDC &dc) {
	// Check if there is anything to do
	int shadeType = Options.AsInt(_T("Audio Inactive Lines Display Mode"));
	if (shadeType == 0) return;

	// Spectrum?
	bool spectrum = false;
	if (provider && Options.AsBool(_T("Audio Spectrum"))) {
		spectrum = true;
	}

	// Set options
	dc.SetBrush(wxBrush(Options.AsColour(_T("Audio Line boundary inactive line"))));
	int selWidth = Options.AsInt(_T("Audio Line boundaries Thickness"));
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
				dc.SetPen(wxPen(Options.AsColour(_T("Audio Waveform Inactive"))));
				for (int i=x1;i<x2;i++) dc.DrawLine(i,peak[i],i,min[i]-1);
				for (int i=x3;i<x4;i++) dc.DrawLine(i,peak[i],i,min[i]-1);
			}

			// Draw boundaries
			dc.SetPen(wxPen(Options.AsColour(_T("Audio Line boundary inactive line"))));
			dc.DrawRectangle(shadeX1-selWidth/2+1,0,selWidth,h);
			dc.DrawRectangle(shadeX2-selWidth/2+1,0,selWidth,h);
		}
	}
}


//////////////////
// Draw keyframes
void AudioDisplay::DrawKeyframes(wxDC &dc) {
	wxArrayInt KeyFrames = VideoContext::Get()->GetKeyFrames();
	int nKeys = (int)KeyFrames.Count();
	dc.SetPen(wxPen(wxColour(255,0,255),1));

	// Get min and max frames to care about
	int minFrame = VFR_Output.GetFrameAtTime(GetMSAtX(0),true);
	int maxFrame = VFR_Output.GetFrameAtTime(GetMSAtX(w),true);

	// Scan list
	for (int i=0;i<nKeys;i++) {
		int cur = KeyFrames[i];
		if (cur >= minFrame && cur <= maxFrame) {
			int x = GetXAtMS(VFR_Output.GetTimeAtFrame(cur,true));
			dc.DrawLine(x,0,x,h);
		}
		else if (cur > maxFrame) break;
	}
}


//////////////////
// Draw timescale
void AudioDisplay::DrawTimescale(wxDC &dc) {
	// Set size
	int timelineHeight = Options.AsBool(_T("Audio Draw Timeline")) ? 20 : 0;

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


////////////
// Waveform
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
	dc.SetPen(wxPen(Options.AsColour(_T("Audio Waveform"))));
	for (int64_t i=0;i<selStartCap;i++) {
		dc.DrawLine(i,peak[i],i,min[i]-1);
	}

	if (hasSel) {
		// Draw selection
		if (Options.AsBool(_T("Audio Draw Selection Background"))) {
			if (NeedCommit && !karaoke->enabled) dc.SetPen(wxPen(Options.AsColour(_T("Audio Waveform Modified"))));
			else dc.SetPen(wxPen(Options.AsColour(_T("Audio Waveform Selected"))));
		}
		for (int64_t i=selStartCap;i<selEndCap;i++) {
			dc.DrawLine(i,peak[i],i,min[i]-1);
		}

		// Draw post-selection
		dc.SetPen(wxPen(Options.AsColour(_T("Audio Waveform"))));
		for (int64_t i=selEndCap;i<w;i++) {
			dc.DrawLine(i,peak[i],i,min[i]-1);
		}
	}
}


//////////////////////////
// Draw spectrum analyzer
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

//////////////////////////
// Get selection position
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


////////////////////////
// Get karaoke position
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


//////////
// Update
void AudioDisplay::Update() {
	if (blockUpdate) return;
	if (loaded) {
		if (Options.AsBool(_T("Audio Autoscroll")))
			MakeDialogueVisible();
		else
			UpdateImage(true);
	}
}


//////////////////////
// Recreate the image
void AudioDisplay::RecreateImage() {
	GetClientSize(&w,&h);
	h -= Options.AsBool(_T("Audio Draw Timeline")) ? 20 : 0;
	delete origImage;
	origImage = NULL;
	UpdateImage(false);
}


/////////////////////////
// Make dialogue visible
void AudioDisplay::MakeDialogueVisible(bool force) {
	wxLogDebug(_T("AudioDisplay::MakeDialogueVisible(force=%d)"), force?1:0);
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


////////////////
// Set position
void AudioDisplay::SetPosition(int pos) {
	wxLogDebug(_T("AudioDisplay::SetPosition(pos=%d)"), pos);
	Position = pos;
	PositionSample = pos * samples;
	UpdateImage();
}


///////////////////
// Update position
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


/////////////////////////////
// Set samples in percentage
// Note: aka Horizontal Zoom
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


//////////////////
// Update samples
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


/////////////
// Set scale
void AudioDisplay::SetScale(float _scale) {
	if (scale == _scale) return;
	scale = _scale;
	UpdateImage();
}


//////////////////
// Load from file
void AudioDisplay::SetFile(wxString file) {
	wxLogDebug(_T("AudioDisplay::SetFile(file=%s)"), file.c_str());
	// Unload
	if (file.IsEmpty()) try {
		wxLogDebug(_T("AudioDisplay::SetFile: file is empty, just closing audio"));
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
		wxLogDebug(_T("AudioDisplay::SetFile: unloading old file"));
		SetFile(_T(""));
		try {
			// Get provider
			wxLogDebug(_T("AudioDisplay::SetFile: get audio provider"));
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
			wxLogDebug(_T("AudioDisplay::SetFile: get audio player"));
			player = AudioPlayerFactoryManager::GetAudioPlayer();
			player->SetDisplayTimer(&UpdateTimer);
			player->SetProvider(provider);
			player->OpenStream();
			loaded = true;

			// Add to recent
			if (!is_dummy) {
				wxLogDebug(_T("AudioDisplay::SetFile: add to recent"));
				Options.AddToRecentList(file,_T("Recent aud"));
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
			wxLogDebug(_T("AudioDisplay::SetFile: gotcha!"));
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
	wxLogDebug(_T("AudioDisplay::SetFile: set default selection"));
	int n = grid->editBox->linen;
	SetDialogue(grid,grid->GetDialogue(n),n);
	wxLogDebug(_T("AudioDisplay::SetFile: returning"));
}


///////////////////
// Load from video
void AudioDisplay::SetFromVideo() {
	wxLogDebug(_T("AudioDisplay::SetFromVideo"));
	if (VideoContext::Get()->IsLoaded()) {
		wxString extension = VideoContext::Get()->videoName.Right(4);
		extension.LowerCase();

		if (extension != _T(".d2v")) SetFile(VideoContext::Get()->videoName);
	}
}


////////////////
// Reload audio
void AudioDisplay::Reload() {
	wxLogDebug(_T("AudioDisplay::Reload"));
	if (provider) SetFile(provider->GetFilename());
}


////////////////////
// Update scrollbar
void AudioDisplay::UpdateScrollbar() {
	if (!provider) return;
	int page = w/12;
	int len = provider->GetNumSamples() / samples / 12;
	Position = PositionSample / samples;
	ScrollBar->SetScrollbar(Position/12,page,len,int(page*0.7),true);
}


//////////////////////////////////////////////
// Gets the sample number at the x coordinate
int64_t AudioDisplay::GetSampleAtX(int x) {
	return (x+Position)*samples;
}


/////////////////////////////////////////////////
// Gets the x coordinate corresponding to sample
int AudioDisplay::GetXAtSample(int64_t n) {
	return samples ? (n/samples)-Position : 0;
}


/////////////////
// Get MS from X
int AudioDisplay::GetMSAtX(int64_t x) {
	return (PositionSample+(x*samples)) * 1000 / provider->GetSampleRate();
}


/////////////////
// Get X from MS
int AudioDisplay::GetXAtMS(int64_t ms) {
	return ((ms * provider->GetSampleRate() / 1000)-PositionSample)/samples;
}


////////////////////
// Get MS At sample
int AudioDisplay::GetMSAtSample(int64_t x) {
	return x * 1000 / provider->GetSampleRate();
}


////////////////////
// Get Sample at MS
int64_t AudioDisplay::GetSampleAtMS(int64_t ms) {
	return ms * provider->GetSampleRate() / 1000;
}


////////
// Play
void AudioDisplay::Play(int start,int end) {
	wxLogDebug(_T("AudioDisplay::Play"));
	Stop();

	// Check provider
	if (!provider) {
		wxLogDebug(_T("AudioDisplay::Play: no audio provider"));
		return;
	}

	// Set defaults
	wxLogDebug(_T("AudioDisplay::Play: initialising playback"));
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
	wxLogDebug(_T("AudioDisplay::Play: playback started, returning"));
}


////////
// Stop
void AudioDisplay::Stop() {
	wxLogDebug(_T("AudioDisplay::Stop"));
	if (VideoContext::Get()->IsPlaying()) VideoContext::Get()->Stop();
	if (player) player->Stop();
}


///////////////////////////
// Get samples of dialogue
void AudioDisplay::GetTimesDialogue(int &start,int &end) {
	wxLogDebug(_T("AudioDisplay::GetTimesDialogue"));
	if (!dialogue) {
		start = 0;
		end = 0;
		return;
	}

	start = dialogue->Start.GetMS();
	end = dialogue->End.GetMS();
}


////////////////////////////
// Get samples of selection
void AudioDisplay::GetTimesSelection(int &start,int &end) {
	wxLogDebug(_T("AudioDisplay::GetTimesSelection"));
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


/////////////////////////////
// Set the current selection
void AudioDisplay::SetSelection(int start, int end) {
	wxLogDebug(_T("AudioDisplay::SetSelection(start=%d, end=%d)"), start, end);
	curStartMS = start;
	curEndMS = end;
	Update();
}


////////////////
// Set dialogue
void AudioDisplay::SetDialogue(SubtitlesGrid *_grid,AssDialogue *diag,int n) {
	wxLogDebug(_T("AudioDisplay::SetDialogue"));
	// Actual parameters
	if (_grid) {
		wxLogDebug(_T("AudioDisplay::SetDialogue: has grid"));
		// Set variables
		grid = _grid;
		line_n = n;
		dialogue = diag;

		// Set flags
		diagUpdated = false;
		NeedCommit = false;

		// Set times
		if (dialogue && !dontReadTimes && Options.AsBool(_T("Audio grab times on select"))) {
			wxLogDebug(_T("AudioDisplay::SetDialogue: grabbing times"));
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
		wxLogDebug(_T("AudioDisplay::SetDialogue: in karaoke mode, loading new line into karaoke control"));
		NeedCommit = karaoke->LoadFromDialogue(dialogue);

		// Reset karaoke pos
		wxLogDebug(_T("AudioDisplay::SetDialogue: resetting karaoke position"));
		if (karaoke->curSyllable == -1) karaoke->SetSyllable((int)karaoke->syllables.size()-1);
		else karaoke->SetSyllable(0);
	}

	// Update
	Update();
	wxLogDebug(_T("AudioDisplay::SetDialogue: returning"));
}


//////////////////
// Commit changes
void AudioDisplay::CommitChanges (bool nextLine) {
	wxLogDebug(_T("AudioDisplay::CommitChanges(nextLine=%d)"), nextLine?1:0);
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
		wxLogDebug(_T("AudioDisplay::CommitChanges: karaoke enabled, committing it"));
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
		wxLogDebug(_T("AudioDisplay::CommitChanges: karaSelStart=%d karaSelEnd=%d"), karaSelStart, karaSelEnd);
	}
	
	// Get selected rows
	wxArrayInt sel = grid->GetSelection();

	// Commit ok?
	if (validCommit) {
		wxLogDebug(_T("AudioDisplay::CommitChanges: valid commit"));
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
			curDiag->UpdateData();
			if (!grid->IsInSelection(line_n)) break;
		}

		// Update edit box
		wxLogDebug(_T("AudioDisplay::CommitChanges: updating time edit boxes"));
		grid->editBox->StartTime->Update();
		grid->editBox->EndTime->Update();
		grid->editBox->Duration->Update();

		// Update grid
		wxLogDebug(_T("AudioDisplay::CommitChanges: update grid"));
		grid->editBox->Update(!karaoke->enabled);
		grid->ass->FlagAsModified(_T(""));
		grid->CommitChanges();
		karaoke->SetSelection(karaSelStart, karaSelEnd);
		blockUpdate = false;
	}

	// Next line (ugh what a condition, can this be simplified?)
	if (nextLine && !karaoke->enabled && Options.AsBool(_T("Audio Next Line on Commit")) && !wasKaraSplitting) {
		wxLogDebug(_T("AudioDisplay::CommitChanges: going to next line"));
		// Insert a line if it doesn't exist
		int nrows = grid->GetRows();
		if (nrows == line_n + 1) {
			wxLogDebug(_T("AudioDisplay::CommitChanges: was on last line, inserting new"));
			AssDialogue *def = new AssDialogue;
			def->Start = grid->GetDialogue(line_n)->End;
			def->End = grid->GetDialogue(line_n)->End;
			def->End.SetMS(def->End.GetMS()+Options.AsInt(_T("Timing Default Duration")));
			def->Style = grid->GetDialogue(line_n)->Style;
			grid->InsertLine(def,line_n,true);
			curStartMS = curEndMS;
			curEndMS = curStartMS + Options.AsInt(_T("Timing Default Duration"));
		}
		else if (grid->GetDialogue(line_n+1)->Start.GetMS() == 0 && grid->GetDialogue(line_n+1)->End.GetMS() == 0) {
			curStartMS = curEndMS;
			curEndMS = curStartMS + Options.AsInt(_T("Timing Default Duration"));
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
	wxLogDebug(_T("AudioDisplay::CommitChanges: returning"));
}


////////////
// Add lead
void AudioDisplay::AddLead(bool in,bool out) {
	// Lead in
	if (in) {
		curStartMS -= Options.AsInt(_T("Audio Lead in"));
		if (curStartMS < 0) curStartMS = 0;
	}

	// Lead out
	if (out) {
		curEndMS += Options.AsInt(_T("Audio Lead out"));
	}

	// Set changes
	UpdateTimeEditCtrls();
	NeedCommit = true;
	if (Options.AsBool(_T("Audio Autocommit"))) CommitChanges();
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


/////////
// Paint
void AudioDisplay::OnPaint(wxPaintEvent& event) {
	if (w == 0 || h == 0) return;
	DoUpdateImage();

	wxPaintDC dc(this);
	if (origImage) dc.DrawBitmap(*origImage,0,0);
}


///////////////
// Mouse event
void AudioDisplay::OnMouseEvent(wxMouseEvent& event) {
	// Get x,y
	int64_t x = event.GetX();
	int64_t y = event.GetY();
	bool karMode = karaoke->enabled;
	bool shiftDown = event.m_shiftDown;
	int timelineHeight = Options.AsBool(_T("Audio Draw Timeline")) ? 20 : 0;

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
			if (wxWindow::FindFocus() != this && Options.AsBool(_T("Audio Autofocus"))) SetFocus();
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
		if (Options.AsBool(_T("Audio Wheel Default To Zoom"))) zoom = !zoom;

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
		dc.DrawBitmap(*origImage,0,0);

		if (inside) {
			// Draw cursor
			dc.SetLogicalFunction(wxINVERT);
			dc.DrawLine(x,0,x,h);

			// Time
			if (Options.AsBool(_T("Audio Draw Cursor Time"))) {
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
			VideoContext::Get()->JumpToTime(GetMSAtX(x),true);
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
				if (abs64 (x - selStart) < 6 && Options.AsBool(_T("Disable Dragging Times"))==false) {
					wxCursor cursor(wxCURSOR_SIZEWE);
					SetCursor(cursor);
					defCursor = false;
					if (buttonClick) {
						hold = 1;
						gotGrab = true;
					}
				}

				// Grab end
				else if (abs64 (x - selEnd) < 6 && Options.AsBool(_T("Disable Dragging Times"))==false) {
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

						if (leftIsDown && abs((long)(x-lastX)) > Options.AsInt(_T("Audio Start Drag Sensitivity"))) {
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
						if (Options.AsBool(_T("Audio Autocommit"))) CommitChanges();
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


////////////////////////
// Get snap to boundary
int AudioDisplay::GetBoundarySnap(int ms,int rangeX,bool shiftHeld,bool start) {
	// Range?
	if (rangeX <= 0) return ms;

	// Convert range into miliseconds
	int rangeMS = rangeX*samples*1000 / provider->GetSampleRate();

	// Keyframe boundaries
	wxArrayInt boundaries;
	bool snapKey = Options.AsBool(_T("Audio snap to keyframes"));
	if (shiftHeld) snapKey = !snapKey;
	if (snapKey && VideoContext::Get()->KeyFramesLoaded() && Options.AsBool(_T("Audio Draw Keyframes"))) {
		int64_t keyMS;
		wxArrayInt keyFrames = VideoContext::Get()->GetKeyFrames();
		int frame;
		for (unsigned int i=0;i<keyFrames.Count();i++) {
			frame = keyFrames[i];
			if (!start) frame--;
			if (frame < 0) frame = 0;
			keyMS = VFR_Output.GetTimeAtFrame(frame,start);
			//if (start) keyX++;
			if (GetXAtMS(keyMS) >= 0 && GetXAtMS(keyMS) < w) boundaries.Add(keyMS);
		}
	}

	// Other subtitles' boundaries
	int inactiveType = Options.AsInt(_T("Audio Inactive Lines Display Mode"));
	bool snapLines = Options.AsBool(_T("Audio snap to other lines"));
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


//
// SCRUBBING CODE, REMOVED FROM THE FUNCTION ABOVE
/*
	// Stop scrubbing
	bool scrubButton = false && event.ButtonIsDown(wxMOUSE_BTN_MIDDLE);
	if (scrubbing && !scrubButton) {
		// Release mouse
		scrubbing = false;
		if (HasCapture()) ReleaseMouse();

		// Stop player
		player->Stop();
		player->SetProvider(provider);
		delete scrubProvider;
	}

	// Start scrubbing
	if (!scrubbing && scrubButton && provider->GetChannels() == 1) {
		// Get mouse
		CaptureMouse();
		scrubbing = true;

		// Initialize provider
		player->Stop();
		scrubProvider = new StreamAudioProvider();
		scrubProvider->SetParams(provider->GetChannels(),provider->GetSampleRate(),provider->GetBytesPerSample());
		player->SetProvider(scrubProvider);

		// Set variables
		scrubLastPos = GetSampleAtX(x);
		scrubTime = clock();
		scrubLastRate = provider->GetSampleRate();
	}

	// Scrub
	if (scrubbing && scrubButton) {
		// Get current data
		int64_t exactPos = MAX(0,GetSampleAtX(x));
		int curScrubTime = clock();
		int scrubDeltaTime = curScrubTime - scrubTime;
		bool invert = exactPos < scrubLastPos;
		int64_t curScrubPos = exactPos;

		if (scrubDeltaTime > 0) {
			// Get derived data
			int rateChange = provider->GetSampleRate()/20;
			int curRate = MID(int(scrubLastRate-rateChange),abs(int(exactPos - scrubLastPos)) * CLOCKS_PER_SEC / scrubDeltaTime,int(scrubLastRate+rateChange));
			if (abs(curRate-scrubLastRate) < rateChange) curRate = scrubLastRate;
			curScrubPos = scrubLastPos + (curRate * scrubDeltaTime / CLOCKS_PER_SEC * (invert ? -1 : 1));
			int64_t scrubDelta = curScrubPos - scrubLastPos;
			scrubLastRate = curRate;

			// Copy data to buffer
			if (scrubDelta != 0) {
				// Create buffer
				int bufSize = scrubDeltaTime * scrubProvider->GetSampleRate() / CLOCKS_PER_SEC;
				short *buf = new short[bufSize];

				// Flag as inverted, if necessary
				if (invert) scrubDelta = -scrubDelta;

				// Copy data from original provider to temp buffer
				short *temp = new short[scrubDelta];
				provider->GetAudio(temp,MIN(curScrubPos,scrubLastPos),scrubDelta);

				// Scale
				float scale = float(double(scrubDelta) / double(bufSize));
				float start,end;
				int istart,iend;
				float tempfinal;
				for (int i=0;i<bufSize;i++) {
					start = i*scale;
					end = (i+1)*scale;
					istart = (int) start;
					iend = MIN((int) end,scrubDelta-1);
					if (istart == iend) tempfinal = temp[istart] * (end - start);
					else {
						tempfinal = temp[istart] * (1 + istart - start) + temp[iend] * (end - iend);
						for (int j=istart+1;j<iend;j++) tempfinal += temp[i];
					}
					buf[i] = tempfinal / scale;
				}
				//int len = MIN(bufSize,scrubDelta);
				//for (int i=0;i<len;i++) buf[i] = temp[i];
				//for (int i=len;i<bufSize;i++) buf[i] = 0;
				delete temp;

				// Invert
				if (invert) {
					short aux;
					for (int i=0;i<bufSize/2;i++) {
						aux = buf[i];
						buf[i] = buf[bufSize-i-1];
						buf[bufSize-i-1] = aux;
					}
				}

				// Send data to provider
				scrubProvider->Append(buf,bufSize);
				if (!player->IsPlaying()) player->Play(0,~0ULL);
				delete buf;
			}
		}

		// Update last pos and time
		scrubLastPos = curScrubPos;
		scrubTime = curScrubTime;

		// Return
		return;
	}

*/


//////////////
// Size event
void AudioDisplay::OnSize(wxSizeEvent &event) {
	// Set size
	GetClientSize(&w,&h);
	h -= Options.AsBool(_T("Audio Draw Timeline")) ? 20 : 0;

	// Update image
	UpdateSamples();
	if (samples) {
		UpdatePosition(PositionSample / samples);
	}
	UpdateImage();
	
	// Update scrollbar
	UpdateScrollbar();
}


///////////////
// Timer event
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
			bool scrollToCursor = Options.AsBool(_T("Audio lock scroll on cursor"));
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
				dc.SetPen(wxPen(Options.AsColour(_T("Audio Play cursor"))));
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


////////////
// Key down
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
		if (Options.AsBool(_T("Audio Autocommit")) && curStartMS <= curEndMS) CommitChanges();
		else UpdateImage(true);
	}
}


///////////////
// Change line
void AudioDisplay::ChangeLine(int delta, bool block) {
	wxLogDebug(_T("AudioDisplay::ChangeLine(delta=%d)"), delta);
	if (dialogue) {
		wxLogDebug(_T("AudioDisplay::ChangeLine: has dialogue"));
		// Get next line number and make sure it's within bounds
		int next;
		if (block && grid->IsInSelection(line_n)) next = grid->GetLastSelRow()+delta;
		else next = line_n+delta;

		if (next == -1) next = 0;
		if (next == grid->GetRows()) next = grid->GetRows() - 1;
		wxLogDebug(_T("AudioDisplay::ChangeLine: next=%i"), next);

		// Set stuff
		NeedCommit = false;
		dialogue = NULL;
		grid->editBox->SetToLine(next);
		grid->SelectRow(next);
		grid->MakeCellVisible(next,0,true);
		if (!dialogue) UpdateImage(true);
		else UpdateImage(false);
		line_n = next;
	}
	wxLogDebug(_T("AudioDisplay::ChangeLine: returning"));
}


////////
// Next
void AudioDisplay::Next(bool play) {
	wxLogDebug(_T("AudioDisplay::Next"));
	// Karaoke
	if (karaoke->enabled) {
		wxLogDebug(_T("AudioDisplay::Next: karaoke enables, going to next syllable"));
		int nextSyl = karaoke->curSyllable+1;
		bool needsUpdate = true;

		// Last syllable; jump to next
		if (nextSyl >= (signed int)karaoke->syllables.size()) {
			wxLogDebug(_T("AudioDisplay::Next: last syllable on line"));
			// Already last?
			if (line_n == grid->GetRows()-1) return;

			if (NeedCommit) {
				wxLogDebug(_T("AudioDisplay::Next: uncommitted karaoke changes"));
				int result = wxMessageBox(_("Do you want to commit your changes? If you choose No, they will be discarded."),_("Commit?"),wxYES_NO | wxCANCEL | wxICON_QUESTION);
				//int result = wxNO;
				if (result == wxYES) {
					CommitChanges();
				}
				else if (result == wxCANCEL) {
					wxLogDebug(_T("AudioDisplay::Next: cancelled, returning"));
					karaoke->curSyllable = (int)karaoke->syllables.size()-1;
					return;
				}
			}
			wxLogDebug(_T("AudioDisplay::Next: going to next line"));
			nextSyl = 0;
			karaoke->curSyllable = 0;
			ChangeLine(1);
			needsUpdate = false;
		}

		// Set syllable
		wxLogDebug(_T("AudioDisplay::Next: set syllable"));
		karaoke->SetSyllable(nextSyl);
		if (needsUpdate) Update();
		int start=0,end=0;
		GetTimesSelection(start,end);
		if (play) Play(start,end);
	}

	// Plain mode
	else {
		wxLogDebug(_T("AudioDisplay::Next: going to next line"));
		ChangeLine(1);
	}

	wxLogDebug(_T("AudioDisplay::Next: returning"));
}


////////////
// Previous
void AudioDisplay::Prev(bool play) {
	wxLogDebug(_T("AudioDisplay::Prev"));
	// Karaoke
	if (karaoke->enabled) {
		wxLogDebug(_T("AudioDisplay::Prev: karaoke enabled, going to prev syllable"));
		int nextSyl = karaoke->curSyllable-1;
		bool needsUpdate = true;

		// First syllable; jump line
		if (nextSyl < 0) {
			wxLogDebug(_T("AudioDisplay::Prev: prev syllable on prev line"));
			// Already first?
			if (line_n == 0) return;

			if (NeedCommit) {
				wxLogDebug(_T("AudioDisplay::Prev: uncommitted karaoke changes"));
				int result = wxMessageBox(_("Do you want to commit your changes? If you choose No, they will be discarded."),_("Commit?"),wxYES_NO | wxCANCEL);
				if (result == wxYES) {
					CommitChanges();
				}
				else if (result == wxCANCEL) {
					karaoke->curSyllable = 0;
					wxLogDebug(_T("AudioDisplay::Prev: cancelled, returning"));
					return;
				}
			}
			wxLogDebug(_T("AudioDisplay::Prev: going to prev line"));
			karaoke->curSyllable = -1;
			ChangeLine(-1);
			needsUpdate = false;
		}

		// Set syllable
		wxLogDebug(_T("AudioDisplay::Prev: set syllable"));
		karaoke->SetSyllable(nextSyl);
		if (needsUpdate) Update();
		int start=0,end=0;
		GetTimesSelection(start,end);
		if (play) Play(start,end);
	}

	// Plain mode
	else {
		wxLogDebug(_T("AudioDisplay::Prev: going to prev line"));
		ChangeLine(-1);
	}

	wxLogDebug(_T("AudioDisplay::Prev: returning"));
}


///////////////////////////////
// Gets syllable at x position
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


////////////////
// Focus events
void AudioDisplay::OnGetFocus(wxFocusEvent &event) {
	if (!hasFocus) {
		hasFocus = true;
		UpdateImage(true);
	}
}

void AudioDisplay::OnLoseFocus(wxFocusEvent &event) {
	if (hasFocus && loaded) {
		hasFocus = false;
		UpdateImage(true);
		Refresh(false);
	}
}


//////////////////////////////
// Update time edit controls
void AudioDisplay::UpdateTimeEditCtrls() {
	grid->editBox->StartTime->SetTime(curStartMS,true);
	grid->editBox->EndTime->SetTime(curEndMS,true);
	grid->editBox->Duration->SetTime(curEndMS-curStartMS,true);
}

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
#include <wx/tglbtn.h>
#include "audio_display.h"
#include "main.h"
#include "ass_dialogue.h"
#include "subs_grid.h"
#include "ass_file.h"
#include "subs_edit_box.h"
#include "options.h"
#include "audio_karaoke.h"
#include "audio_box.h"
#include "fft.h"
#include "video_display.h"
#include "vfr.h"
#include "colorspace.h"
#include "hotkeys.h"
#include "utils.h"


///////////////
// Constructor
AudioDisplay::AudioDisplay(wxWindow *parent,VideoDisplay *display)
: wxWindow (parent, -1, wxDefaultPosition, wxSize(200,Options.AsInt(_T("Audio Display Height"))), wxSUNKEN_BORDER | wxWANTS_CHARS , _T("Audio Display"))
{
	// Set variables
	origImage = NULL;
	spectrumDisplay = NULL;
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
	video = display;
	hold = 0;
	hasFocus = (wxWindow::FindFocus() == this);

	// Init
	UpdateTimer.SetOwner(this,Audio_Update_Timer);
	GetClientSize(&w,&h);
	h -= 20;
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
	delete spectrumDisplay;
	delete peak;
	delete min;
}


/////////
// Reset
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


////////////////
// Update image
void AudioDisplay::UpdateImage(bool weak) {
	// Prepare bitmap
	int displayH = h+20;
	if (origImage) {
		if (origImage->GetWidth() != w || origImage->GetHeight() != displayH) {
			delete origImage;
			origImage = NULL;
		}
	}

	bool draw_boundary_lines = Options.AsBool(_T("Audio Draw Secondary Lines"));
	bool draw_selection_background = Options.AsBool(_T("Audio Draw Selection Background"));

	// Invalid dimensions
	if (w == 0 || displayH == 0) return;

	// New bitmap
	if (!origImage) origImage = new wxBitmap(w,displayH,-1);

	// Is spectrum?
	bool spectrum = false;
	if (provider && Options.AsBool(_T("Audio Spectrum"))) {
		spectrum = true;
	}

	// Update samples
	UpdateSamples();

	// Draw image to be displayed
	wxMemoryDC dc;
	dc.SelectObject(*origImage);
	dc.BeginDrawing();

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
	if (dialogue) {
		GetDialoguePos(lineStart,lineEnd,false);
		hasSel = true;
		if (hasKaraoke) {
			GetKaraokePos(selStartCap,selEndCap,true);
			GetKaraokePos(selStart,selEnd,false);
		}
		else {
			GetDialoguePos(selStartCap,selEndCap,true);
			selStart = lineStart;
			selEnd = lineEnd;
		}
	}

	// Draw selection bg
	if (hasSel && selStart < selEnd && draw_selection_background) {
		if (NeedCommit && !karaoke->enabled) dc.SetBrush(wxBrush(Options.AsColour(_T("Audio Selection Background Modified"))));
		else dc.SetBrush(wxBrush(Options.AsColour(_T("Audio Selection Background"))));
		dc.DrawRectangle(selStart,0,selEnd-selStart,h);
	}

	// Draw spectrum
	if (spectrum) {
		DrawSpectrum(dc,weak);

		// Invert the selection, if any
		if (hasSel && selStart < selEnd && Options.AsBool(_T("Audio Spectrum invert selection"))) {
			dc.Blit(selStart, 0, selEnd-selStart, h, &dc, selStart, 0, wxSRC_INVERT);
		}
	}

	// Draw seconds boundaries
	if (draw_boundary_lines) {
		__int64 start = Position*samples;
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

	// Draw keyframes
	if (video->loaded && draw_boundary_lines) {
		int nKeys = video->KeyFrames.Count();
		dc.SetPen(wxPen(wxColour(255,0,255),1));

		// Get min and max frames to care about
		int minFrame = VFR_Output.GetFrameAtTime(GetMSAtX(0),true);
		int maxFrame = VFR_Output.GetFrameAtTime(GetMSAtX(w),true);

		// Scan list
		for (int i=0;i<nKeys;i++) {
			int cur = video->KeyFrames[i];
			if (cur >= minFrame && cur <= maxFrame) {
				int x = GetXAtMS(VFR_Output.GetTimeAtFrame(cur,true));
				dc.DrawLine(x,0,x,h);
			}
			else if (cur > maxFrame) break;
		}
	}

	// Waveform
	if (provider) {
		if (!spectrum) DrawWaveform(dc,weak);
	}

	// Nothing
	else {
		dc.DrawLine(0,h/2,w,h/2);
	}

	// Draw previous line
	int shadeType = Options.AsInt(_T("Audio Inactive Lines Display Mode"));
	if (shadeType == 1 || shadeType == 2) {
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
			shade = grid->GetDialogue(j);

			if (shade) {
				// Get coordinates
				shadeX1 = GetXAtMS(shade->Start.GetMS());
				shadeX2 = GetXAtMS(shade->End.GetMS());
				if (shadeX2 < 0 || shadeX1 > w) continue;

				// Draw over waveform
				if (!spectrum) {
					int x1 = MAX(0,shadeX1);
					int x2 = MIN(w,shadeX2);
					dc.SetPen(wxPen(Options.AsColour(_T("Audio Waveform Inactive"))));
					for (__int64 i=x1;i<x2;i++) {
						dc.DrawLine(i,peak[i],i,min[i]-1);
					}
				}

				// Draw boundaries
				dc.SetPen(wxPen(Options.AsColour(_T("Audio Line boundary inactive line"))));
				dc.DrawRectangle(shadeX1-selWidth/2+1,0,selWidth,h);
				dc.DrawRectangle(shadeX2-selWidth/2+1,0,selWidth,h);
			}
		}
	}

	if (hasSel) {
		// Draw boundaries
		int selWidth = Options.AsInt(_T("Audio Line boundaries Thickness"));
		dc.SetPen(wxPen(Options.AsColour(_T("Audio Line boundary start"))));
		dc.SetBrush(wxBrush(Options.AsColour(_T("Audio Line boundary start"))));
		dc.DrawRectangle(lineStart-selWidth/2+1,0,selWidth,h);
		dc.SetPen(wxPen(Options.AsColour(_T("Audio Line boundary end"))));
		dc.SetBrush(wxBrush(Options.AsColour(_T("Audio Line boundary end"))));
		dc.DrawRectangle(lineEnd-selWidth/2+1,0,selWidth,h);

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
				__int64 pos1,pos2;
				int len,curpos;
				wxCoord tw=0,th=0;
				KaraokeSyllable *curSyl;
				wxString temptext;

				// Draw syllables
				for (size_t i=0;i<karn;i++) {
					curSyl = &karaoke->syllables.at(i);
					len = curSyl->length*10;
					curpos = curSyl->position*10;
					if (len != -1) {
						pos1 = GetXAtMS(curStartMS+curpos);
						pos2 = GetXAtMS(curStartMS+len+curpos);
						dc.DrawLine(pos2,0,pos2,h);
						temptext = curSyl->contents;
						temptext.Trim(true);
						temptext.Trim(false);
						GetTextExtent(temptext,&tw,&th,NULL,NULL,&curFont);
						dc.DrawText(temptext,(pos1+pos2-tw)/2,h-th-4);
					}
				}
			}
			catch (...) {
			}
		}
	}

	// Modified text
	if (NeedCommit) {
		dc.SetFont(wxFont(9,wxDEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana")));
		dc.SetTextForeground(wxColour(255,0,0));
		if (selStart <= selEnd) {
			dc.DrawText(_T("Modified"),4,4);
		}
		else {
			dc.DrawText(_T("Negative time"),4,4);
		}
	}

	// Draw selection border
	if (hasFocus) {
		dc.SetPen(*wxGREEN_PEN);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(0,0,w,h);
	}

	// Draw timescale
	dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(0,h,w,20);
	dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT));
	dc.DrawLine(0,h,w,h);
	dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DHIGHLIGHT));
	dc.DrawLine(0,h+1,w,h+1);
	dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
	wxFont scaleFont;
	scaleFont.SetFaceName(_T("Tahoma"));
	scaleFont.SetPointSize(8);
	dc.SetFont(scaleFont);

	// Timescale ticks
	__int64 start = Position*samples;
	int rate = provider->GetSampleRate();
	for (int i=1;i<32;i*=2) {
		int pixBounds = rate / (samples * 4 / i);
		if (pixBounds >= 8) {
			for (int x=0;x<w;x++) {
				__int64 pos = (x*samples)+start;
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

	// Done
	dc.EndDrawing();

	Refresh(false);
}


////////////
// Waveform
void AudioDisplay::DrawWaveform(wxDC &dc,bool weak) {
	// Prepare Waveform
	if (!weak || peak == NULL || min == NULL) {
		if (peak) delete peak;
		if (min) delete min;
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
	for (__int64 i=0;i<selStartCap;i++) {
		dc.DrawLine(i,peak[i],i,min[i]-1);
	}

	if (hasSel) {
		// Draw selection
		if (Options.AsBool(_T("Audio Draw Selection Background"))) {
			if (NeedCommit && !karaoke->enabled) dc.SetPen(wxPen(Options.AsColour(_T("Audio Waveform Modified"))));
			else dc.SetPen(wxPen(Options.AsColour(_T("Audio Waveform Selected"))));
		}
		for (__int64 i=selStartCap;i<selEndCap;i++) {
			dc.DrawLine(i,peak[i],i,min[i]-1);
		}

		// Draw post-selection
		dc.SetPen(wxPen(Options.AsColour(_T("Audio Waveform"))));
		for (__int64 i=selEndCap;i<w;i++) {
			dc.DrawLine(i,peak[i],i,min[i]-1);
		}
	}
}


static int spectrumColorMap[256];
static unsigned short spectrumColorMap16[256];
static bool colorMapsGenerated = false;


//////////////////////////
// Draw spectrum analyzer
void AudioDisplay::DrawSpectrum(wxDC &finaldc,bool weak) {
	// Spectrum bitmap
	if (!weak || !spectrumDisplay || spectrumDisplay->GetWidth() != w || spectrumDisplay->GetHeight() != h) {
		if (spectrumDisplay) delete spectrumDisplay;
		//spectrumDisplay = new wxBitmap(w,h);
		weak = false;
	}

	if (!weak) {
		// Generate colors
		//int colorMap[256];
		//unsigned short colorMap16[256];
		if (!colorMapsGenerated) {
			unsigned char r,g,b;
			for (int i=0;i<256;i++) {
				//HSVtoRGB(255-i,1.0f - 0.3f*(i/255.0f),0.3f + 0.7f*(i/255.0f),r,g,b);
				//hsv_to_rgb(255-i, 1.0f - 0.3f * (i/255.0f), 0.3f + 0.7f * (i/255.0f), &r, &g, &b);
				hsv_to_rgb(255 - i, 255 - i * 3/10, 255*3/10 + i * 7/10, &r, &g, &b);
				spectrumColorMap[i] = b | (g<<8) | (r<<16);
				spectrumColorMap16[i] = ((r>>3)<<11) | ((g>>2)<<5) | b>>3;
			}
			colorMapsGenerated = true;
		}
		int depth = wxDisplayDepth();

		// Prepare arrays
		int cutOff = Options.AsInt(_T("Audio Spectrum Cutoff"));
		int window = 1 << Options.AsInt(_T("Audio Spectrum Window"));
		int totalLen = w*samples+window;
		float *raw_float = new float[totalLen];
		short *raw_int = new short[totalLen];
		float *in = raw_float;

		// Fill input
		__int64 start = Position*samples;
		provider->GetAudio(raw_int,start,totalLen);
		for (int j=0;j<totalLen;j++) {
			raw_float[j] = (float)raw_int[j];
		}
		delete raw_int;

		// More arrays
		float *out_r = new float[window];
		float *out_i = new float[window];
		int *data = new int[w*h*depth/32];
		int *write_ptr = data;
		unsigned short *write_ptr16 = (unsigned short *)data;
	
		// Copy to spectrum bitmap
		//dc.BeginDrawing();

		// Prepare variables
		int halfwindow = window/2;
		int posThres = int(double(halfwindow-cutOff)/double(h)*0.5/scale + 0.5);
		float mult = float(h)/float(halfwindow-cutOff)/255.0f;

		// Calculation loop
		for (int i=0;i<w;i++) {
			// Calculate start for current bar
			__int64 curStart = i*samples-(window/2);
			if (curStart < 0) curStart = 0;

			// Position input
			in = raw_float + curStart;

			// Perform the FFT
			FFT fft;
			fft.Transform(window,in,out_r,out_i);

			// Draw bar
			float accum = 0;
			int accumPos = posThres;
			int y = h;
			int intensity;
			float t1,t2;

			// Position pointer
			write_ptr = data+i+h*w;
			write_ptr16 = ((unsigned short*)data)+(i+h*w);

			// Draw loop
			for (int j=cutOff;j<halfwindow;j++) {
				// Calculate magnitude and add to accumulator
				t1 = out_r[j];
				t1 *= t1;
				t2 = out_i[j];
				t2 *= t2;
				t2 += t1;
				accum += sqrt(t2);

				// When accumulator goes over, plot point
				accumPos--;
				if (accumPos == 0) {
					intensity = int(accum*mult);
					if (intensity > 255) intensity = 255;
					if (intensity < 0) intensity = 0;

					// Write and go to next row
					if (depth == 32) {
						write_ptr -= w;
						*write_ptr = spectrumColorMap[intensity];
					}
					else if (depth == 16) {
						write_ptr16 -= w;
						*write_ptr16 = spectrumColorMap16[intensity];
					}

					//dc.SetPen(pen[intensity]);
					//dc.DrawPoint(i,y);
					y--;
					if (y == 0) break;
					accumPos = posThres;
					accum = 0;
				}
			}
		}

		// Clear top of image
		int filly = h - (halfwindow-cutOff)/posThres;
		if (filly < 0) filly = 0;
		if (depth == 32) {
			int color = spectrumColorMap[0];
			for (int y=0;y<filly;y++) {
				write_ptr = data+y*w;
				for (int x=0;x<w;x++) {
					*write_ptr = color;
					write_ptr++;
				}
			}
		}
		else {
			unsigned short color = spectrumColorMap16[0];
			for (int y=0;y<filly;y++) {
				write_ptr16 = ((unsigned short*)data)+y*w;
				for (int x=0;x<w;x++) {
					*write_ptr16 = color;
					write_ptr16++;
				}
			}
		}

		// Clear memory
		delete out_r;
		delete out_i;
		delete raw_float;

		// Create image
		spectrumDisplay = new wxBitmap((const char*)data,w,h,depth);
	}

	// Draw
	wxMemoryDC dc;
	dc.SelectObject(*spectrumDisplay);
	finaldc.Blit(0,0,w,h,&dc,0,0);
}


//////////////////////////
// Get selection position
void AudioDisplay::GetDialoguePos(__int64 &selStart,__int64 &selEnd, bool cap) {
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
void AudioDisplay::GetKaraokePos(__int64 &karStart,__int64 &karEnd, bool cap) {
	try {
		// Wrap around
		int nsyls = karaoke->syllables.size();
		if (karaoke->curSyllable == -1) {
			karaoke->SetSyllable(nsyls-1);
		}
		if (karaoke->curSyllable >= nsyls) karaoke->curSyllable = nsyls-1;

		// Get positions
		int pos = karaoke->syllables.at(karaoke->curSyllable).position;
		int len = karaoke->syllables.at(karaoke->curSyllable).length;
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


/////////////////////////
// Make dialogue visible
void AudioDisplay::MakeDialogueVisible(bool force) {
	// Variables
	int temp1=0,temp2=0;
	GetTimesSelection(temp1,temp2);
	int startPos = GetSampleAtMS(temp1);
	int endPos = GetSampleAtMS(temp2);
	int startX = GetXAtMS(temp1);
	int endX = GetXAtMS(temp2);

	if (force || startX < 50 || endX > w-50) {
		UpdatePosition((startPos+endPos-w*samples)/2,true);
	}

	// Update
	UpdateImage();
}


////////////////
// Set position
void AudioDisplay::SetPosition(int pos) {
	Position = pos;
	PositionSample = pos * samples;
	UpdateImage();
}


///////////////////
// Update position
void AudioDisplay::UpdatePosition (int pos,bool IsSample) {
	// Safeguards
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
		PositionSample += (oldSamples-samples)*w*pivot;
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
	__int64 totalSamples = provider->GetNumSamples();
	int total = totalSamples / w;
	int max = 5760000 / w;	// 2 minutes at 48 kHz maximum
	if (total > max) total = max;
	int min = 8;
	if (total < min) total = min;
	int range = total-min;
	samples = range*pow(samplesPercent/100.0,3)+min;

	// Set position
	int length = w * samples;
	if (PositionSample + length > totalSamples) {
		PositionSample = totalSamples - length;
		if (PositionSample < 0) PositionSample = 0;
		Position = PositionSample / samples;
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
void AudioDisplay::SetFile(wxString file, VideoProvider *vprovider) {
	// Unload
	if (file.IsEmpty()) {
		if (player) player->CloseStream();
		delete provider;
		delete player;
		provider = NULL;
		player = NULL;
		Reset();

		loaded = false;
	}

	// Load
	else {
		SetFile(_T(""));
		try {
			// Get provider
			provider = AudioProvider::GetAudioProvider(file, this, vprovider);

			// Get player
			player = AudioPlayer::GetAudioPlayer();
			player->SetDisplayTimer(&UpdateTimer);
			player->SetProvider(provider);
			player->OpenStream();
			loaded = true;

			// Add to recent
			Options.AddToRecentList(file,_T("Recent aud"));

			// Update
			UpdateImage();
		}
		catch (wxString &err) {
			wxMessageBox(err,_T("Error loading audio"),wxICON_ERROR | wxOK);
		}
	}

	assert(loaded == (provider != NULL));

	// Set default selection
	int n = grid->editBox->linen;
	SetDialogue(grid,grid->GetDialogue(n),n);
}


///////////////////
// Load from video
void AudioDisplay::SetFromVideo() {
	if (video->loaded) {
		wxString extension = video->videoName.Right(4);
		extension.LowerCase();

		if (extension != _T(".d2v"))
			SetFile(video->videoName, video->provider);
	}
}


////////////////////
// Update scrollbar
void AudioDisplay::UpdateScrollbar() {
	int page = w/12;
	int len = provider->GetNumSamples() / samples / 12;
	Position = PositionSample / samples;
	ScrollBar->SetScrollbar(Position/12,page,len,page*0.7,true);
}


//////////////////////////////////////////////
// Gets the sample number at the x coordinate
__int64 AudioDisplay::GetSampleAtX(int x) {
	return (x+Position)*samples;
}


/////////////////////////////////////////////////
// Gets the x coordinate corresponding to sample
int AudioDisplay::GetXAtSample(__int64 n) {
	return (n/samples)-Position;
}


/////////////////
// Get MS from X
int AudioDisplay::GetMSAtX(__int64 x) {
	return (PositionSample+(x*samples)) * 1000 / provider->GetSampleRate();
}


/////////////////
// Get X from MS
int AudioDisplay::GetXAtMS(__int64 ms) {
	return ((ms * provider->GetSampleRate() / 1000)-PositionSample)/samples;
}


////////////////////
// Get MS At sample
int AudioDisplay::GetMSAtSample(__int64 x) {
	return x * 1000 / provider->GetSampleRate();
}


////////////////////
// Get Sample at MS
__int64 AudioDisplay::GetSampleAtMS(__int64 ms) {
	return ms * provider->GetSampleRate() / 1000;
}


////////
// Play
void AudioDisplay::Play(int start,int end) {
	// Check provider
	if (!provider) {
		// Load temporary provider from video
		if (video->loaded) {
			try {
				// Get provider
				provider = AudioProvider::GetAudioProvider(video->videoName, this, video->provider,0);

				// Get player
				player = AudioPlayer::GetAudioPlayer();
				player->SetDisplayTimer(&UpdateTimer);
				player->SetProvider(provider);
				player->OpenStream();
				temporary = true;
			}
			catch (...) {
				return;
			}
		}
		if (!provider) return;
	}

	// Set defaults
	__int64 num_samples = provider->GetNumSamples();
	start = GetSampleAtMS(start);
	if (end != -1) end = GetSampleAtMS(end);
	else end = num_samples-1;

	// Sanity checking
	if (start < 0) start = 0;
	if (start >= num_samples) start = num_samples-1;
	if (end < 0) end = 0;
	if (end >= num_samples) end = num_samples-1;
	if (end < start) end = start;

	// Call play
	player->Play(start,end-start);
}


////////
// Stop
void AudioDisplay::Stop() {
	if (!player) return;

	player->Stop();
}


///////////////////////////
// Get samples of dialogue
void AudioDisplay::GetTimesDialogue(int &start,int &end) {
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
	start = 0;
	end = 0;
	if (!dialogue) return;

	try {
		if (karaoke->enabled) {
			int pos = karaoke->syllables.at(karaoke->curSyllable).position;
			int len = karaoke->syllables.at(karaoke->curSyllable).length;
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
	curStartMS = start;
	curEndMS = end;
	Update();
}


////////////////
// Set dialogue
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
		if (dialogue && !dontReadTimes) {
			curStartMS = dialogue->Start.GetMS();
			curEndMS = dialogue->End.GetMS();
		}
	}

	// Read karaoke data
	if (dialogue && karaoke->enabled) {
		NeedCommit = karaoke->LoadFromDialogue(dialogue);

		// Reset karaoke pos
		if (karaoke->curSyllable == -1) karaoke->SetSyllable(karaoke->syllables.size()-1);
		else karaoke->SetSyllable(0);
	}

	// Update
	Update();
}


//////////////////
// Commit changes
void AudioDisplay::CommitChanges () {
	if (!Options.AsBool(_T("Audio SSA Mode")) && !box->audioKaraoke->splitting) {
		// Check if there's any need to commit
		if (!NeedCommit) return;

		// Check if selection is valid
		if (curEndMS < curStartMS) {
			wxMessageBox(_T("Start time must be before end time!"),_T("Error commiting"),wxICON_ERROR);
			return;
		}
	}

	// Reset flags
	diagUpdated = false;
	NeedCommit = false;

	// Update karaoke
	int karSyl = 0;
	bool wasKaraSplitting = false;
	if (karaoke->enabled) {
		wasKaraSplitting = box->audioKaraoke->splitting;
		karaoke->Commit();
		karSyl = karaoke->curSyllable;
	}

	// Update dialogues
	blockUpdate = true;
	wxArrayInt sel = grid->GetSelection();
	int sels = sel.Count();
	AssDialogue *curDiag;
	for (int i=-1;i<sels;i++) {
		if (i == -1) curDiag = dialogue;
		else {
			curDiag = grid->GetDialogue(sel[i]);
			if (curDiag == dialogue) continue;
		}

		curDiag->Start.SetMS(curStartMS);
		curDiag->End.SetMS(curEndMS);
		curDiag->UpdateData();
	}

	// Update grid
	grid->editBox->Update(!karaoke->enabled);
	grid->ass->FlagAsModified();
	grid->CommitChanges();
	karaoke->curSyllable = karSyl;
	blockUpdate = false;

	// If in SSA mode, select next line and "move timing forward" (unless the user was splitting karaoke)
	if (Options.AsBool(_T("Audio SSA Mode")) && Options.AsBool(_T("Audio SSA Next Line on Commit")) && !wasKaraSplitting) {
		dontReadTimes = true;
		Next();
		dontReadTimes = false;
		curStartMS = curEndMS;
		curEndMS = curStartMS + Options.AsInt(_T("Timing Default Duration"));
	}

	Update();
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

	wxPaintDC dc(this);
	dc.BeginDrawing();
	dc.DrawBitmap(*origImage,0,0);
	dc.EndDrawing();
}


///////////////
// Mouse event
void AudioDisplay::OnMouseEvent(wxMouseEvent& event) {
	// Get x,y
	__int64 x = event.GetX();
	__int64 y = event.GetY();
	bool karMode = karaoke->enabled;
	bool shiftDown = event.m_shiftDown;
	bool ctrlDown = event.m_controlDown;

	// Leaving event
	if (event.Leaving()) {
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
		else if (y < h+20) onScale = true;
	}

	// Click type
	if (event.ButtonDown(wxMOUSE_BTN_LEFT) && !holding) {
		holding = true;
		CaptureMouse();
	}
	if (!event.ButtonIsDown(wxMOUSE_BTN_LEFT) && holding) {
		holding = false;
		ReleaseMouse();
	}

	// Mouse wheel
	if (event.GetWheelRotation() != 0) {
		// Zoom or scroll?
		bool zoom = shiftDown;
		if (Options.AsBool(_T("Audio Wheel Default To Zoom"))) zoom = !zoom;

		// Zoom
		if (zoom) {
			int step = -event.GetWheelRotation() / event.GetWheelDelta();
			int value = box->HorizontalZoom->GetValue()+step;
			box->HorizontalZoom->SetValue(value);
			SetSamplesPercent(value,true,float(x)/float(w));
		}

		// Scroll
		else {
			int step = event.GetWheelRotation() * w / 360;
			UpdatePosition(Position+step,false);
			UpdateImage();
		}
	}

	// Cursor drawing
	if (!player->IsPlaying()) {
		// Draw bg
		wxClientDC dc(this);
		dc.BeginDrawing();
		dc.DrawBitmap(*origImage,0,0);

		if (inside) {
			// Draw cursor
			dc.SetLogicalFunction(wxINVERT);
			dc.DrawLine(x,0,x,h);

			// Time string
			AssTime time;
			time.SetMS(GetMSAtX(x));
			wxString text = time.GetASSFormated();

			// Calculate metrics
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

			// Draw text
			dc.SetTextForeground(wxColour(64,64,64));
			dc.DrawText(text,dx+1,dy-1);
			dc.DrawText(text,dx+1,dy+1);
			dc.DrawText(text,dx-1,dy-1);
			dc.DrawText(text,dx-1,dy+1);
			dc.SetTextForeground(wxColour(255,255,255));
			dc.DrawText(text,dx,dy);
		}

		// End drawing
		dc.EndDrawing();
	}

	// Scale dragging
	if (onScale || draggingScale) {
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
		}
		else draggingScale = false;
	}

	// Outside
	if (!inside) return;

	// Left/middle click
	if (event.ButtonDown(wxMOUSE_BTN_LEFT) || event.Button(wxMOUSE_BTN_MIDDLE)) {
		SetFocus();
	}

	// Right click
	if (event.ButtonDown(wxMOUSE_BTN_RIGHT)) {
		SetFocus();
		if (karaoke->enabled) {
			int syl = GetSyllableAtX(x);
			if (syl != -1) {
				int start = karaoke->syllables.at(syl).position * 10 + dialogue->Start.GetMS();
				int count = karaoke->syllables.at(syl).length * 10;
				player->Play(GetSampleAtMS(start),GetSampleAtMS(count));
			}
		}
	}

	bool defCursor = true;

	// Substation alpha mode
	if (Options.AsBool(_T("Audio SSA Mode")) && !karaoke->enabled && dialogue) {
		bool mod = false;

		// Set start
		if (event.ButtonDown(wxMOUSE_BTN_LEFT)) {
			curStartMS = GetMSAtX(x);
			mod = true;
		}

		// Set end
		if (event.ButtonDown(wxMOUSE_BTN_RIGHT)) {
			curEndMS = GetMSAtX(x);
			mod = true;
			player->SetEndPosition(GetSampleAtX(x));
		}

		// Modified, commit changes
		if (mod) {
			// Swap if needed
			if (false && curStartMS > curEndMS) {
				int aux = curStartMS;
				curStartMS = curEndMS;
				curEndMS = aux;
			}

			// Commit
			NeedCommit = true;
			if (Options.AsBool(_T("Audio SSA Allow Autocommit")) && Options.AsBool(_T("Audio Autocommit")) && curStartMS <= curEndMS)
				CommitChanges();
			else 
				UpdateImage(true);
		}
	}

	// Standard mode
	else {
		// Moving around
		if (hasSel) {
			// Grab start/end
			if (hold == 0) {
				bool gotGrab = false;

				// Grab start
				if (!karMode) {
					if (abs64 (x - selStart) < 6) {
						wxCursor cursor(wxCURSOR_SIZEWE);
						SetCursor(cursor);
						defCursor = false;
						if (event.LeftIsDown()) {
							hold = 1;
							gotGrab = true;
						}
					}

					// Grab end
					else if (abs64 (x - selEnd) < 6) {
						wxCursor cursor(wxCURSOR_SIZEWE);
						SetCursor(cursor);
						defCursor = false;
						if (event.LeftIsDown()) {
							hold = 2;
							gotGrab = true;
						}
					}
				}

				// Grabbing a syllable
				if (!gotGrab && karMode) {
					__int64 pos,len,curpos;
					KaraokeSyllable *curSyl;
					size_t karn = karaoke->syllables.size();
					for (size_t i=0;i<karn;i++) {
						curSyl = &karaoke->syllables.at(i);
						len = curSyl->length*10;
						curpos = curSyl->position*10;
						if (len != -1) {
							pos = GetXAtMS(curStartMS+len+curpos);
							if (abs64 (x - pos) < 4) {
								wxCursor cursor(wxCURSOR_SIZEWE);
								SetCursor(cursor);
								defCursor = false;
								if (event.LeftIsDown()) {
									hold = 4;
									holdSyl = i;
									gotGrab = true;
								}
								break;
							}
						}
					}
				}

				// Dragging nothing, time from scratch
				if (!gotGrab) {
					if (event.ButtonIsDown(wxMOUSE_BTN_LEFT)) {
						hold = 3;
						lastX = x;
						gotGrab = true;
					}
				}
			}

			// Drag start/end
			if (hold != 0) {
				// Dragging
				if (event.ButtonIsDown(wxMOUSE_BTN_LEFT)) {
					bool updated = false;

					// Drag from nothing
					if (hold == 3) {
						if (!karMode) {
							if (x != lastX) {
								selStart = x;
								selEnd = x;
								curEndMS = GetMSAtX(selEnd);
								curStartMS = GetMSAtX(selStart);
								hold = 2;
							}
						}
					}

					// Drag start
					if (hold == 1) {
						// Set new value
						if (x != selStart) {
							selStart = x;
							if (selStart > selEnd) {
								int temp = selStart;
								selStart = selEnd;
								selEnd = temp;
								hold = 2;
								curEndMS = GetMSAtX(selEnd);
							}
							curStartMS = GetMSAtX(selStart);
							updated = true;
							diagUpdated = true;
						}
					}

					// Drag end
					if (hold == 2) {
						// Set new value
						if (x != selEnd) {
							selEnd = x;
							if (selStart > selEnd) {
								int temp = selStart;
								selStart = selEnd;
								selEnd = temp;
								hold = 1;
								curStartMS = GetMSAtX(selStart);
							}
							curEndMS = GetMSAtX(selEnd);
							updated = true;
							diagUpdated = true;
						}
					}

					// Drag karaoke
					if (hold == 4) {
						// Set new value
						int curpos,len,pos,nkar;
						KaraokeSyllable *curSyl=NULL,*nextSyl=NULL;
						curSyl = &karaoke->syllables.at(holdSyl);
						nkar = karaoke->syllables.size();
						if (holdSyl < nkar-1) {
							nextSyl = &karaoke->syllables.at(holdSyl+1);
						}
						curpos = curSyl->position;
						len = curSyl->length;
						pos = GetXAtMS(curStartMS+(len+curpos)*10);
						if (x != pos) {
							// Calculate delta in centiseconds
							int delta = ((__int64)(x-pos)*samples*100)/provider->GetSampleRate();

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

					// Update stuff
					if (updated) {
						player->SetEndPosition(GetSampleAtX(selEnd));
						wxCursor cursor(wxCURSOR_SIZEWE);
						SetCursor(cursor);
						UpdateImage(true);
					}
				}

				// Release
				else {
					// Commit changes
					if (diagUpdated) {
						diagUpdated = false;
						NeedCommit = true;
						if (Options.AsBool(_T("Audio Autocommit")) && curStartMS <= curEndMS) 
							CommitChanges();
						else 
							UpdateImage(true);
					}

					// Single click on nothing
					else if (hold == 3) {
						// Select syllable
						if (karaoke->enabled) {
							int syl = GetSyllableAtX(x);
							if (syl != -1) {
								karaoke->SetSyllable(syl);
								UpdateImage(true);
							}
						}
					}

					// Update stuff
					SetCursor(wxNullCursor);
					hold = 0;
				}
			}
		}
		else {
			hold = 0;
		}
	}

	// Restore cursor
	if (defCursor) SetCursor(wxNullCursor);
}


//////////////
// Size event
void AudioDisplay::OnSize(wxSizeEvent &event) {
	// Set size
	GetClientSize(&w,&h);
	h -= 20;

	// Update image
	UpdateImage();
	
	// Update scrollbar
	UpdateScrollbar();
}


///////////////
// Timer event
void AudioDisplay::OnUpdateTimer(wxTimerEvent &event) {
	// Get lock and check if it's OK
	if (player->GetMutex()) {
		wxMutexLocker locker(*player->GetMutex());
		if (!locker.IsOk()) return;
	}
		
	if (!player->IsPlaying()) return;

	// Get DCs
	//wxMutexGuiEnter();
	wxClientDC dc(this);
	dc.BeginDrawing();

	// Draw cursor
	int curpos = -1;
	if (player->IsPlaying()) {
		__int64 curPos = player->GetCurrentPosition();
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

	// Done
	dc.EndDrawing();
	//wxMutexGuiLeave();
}


////////////
// Key down
void AudioDisplay::OnKeyDown(wxKeyEvent &event) {
	int key = event.GetKeyCode();
	Hotkeys.SetPressed(key,event.m_controlDown,event.m_altDown,event.m_shiftDown);

	// Accept
	if (Hotkeys.IsPressed(_T("Audio Commit"))) {
		CommitChanges();
		ChangeLine(1);
	}

	// Accept (SSA's "Grab times")
	if (Hotkeys.IsPressed(_T("Audio Commit Alt"))) {
		CommitChanges();
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
		if (player->IsPlaying()) Stop();
		else {
			int start=0,end=0;
			GetTimesSelection(start,end);
			Play(start,end);
		}
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

	// Play original line
	if (Hotkeys.IsPressed(_T("Audio Play Original Line"))) {
		int start=0,end=0;
		GetTimesDialogue(start,end);
		if (Options.AsBool(_T("Audio SSA Mode"))) {
			SetSelection(start, end);
		}
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
		if ((Options.AsBool(_T("Audio SSA Allow Autocommit")) || Options.AsBool(_T("Audio SSA Mode")) == false) && Options.AsBool(_T("Audio Autocommit")) && curStartMS <= curEndMS)
			CommitChanges();
		else
			UpdateImage(true);
	}
}


///////////////
// Change line
void AudioDisplay::ChangeLine(int delta) {
	if (dialogue) {
		// Get next line number and make sure it's within bounds
		int next = line_n+delta;
		if (next == -1) next = 0;
		if (next == grid->GetRows()) next = grid->GetRows() - 1;

		// Set stuff
		NeedCommit = false;
		dialogue = NULL;
		grid->editBox->SetToLine(next);
		grid->SelectRow(next);
		if (!dialogue) UpdateImage(true);
		else UpdateImage(false);
		line_n = next;
	}
}


////////
// Next
void AudioDisplay::Next() {
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
					karaoke->curSyllable = karaoke->syllables.size()-1;
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
		Play(start,end);
	}

	// Plain mode
	else {
		ChangeLine(1);
	}
}


////////////
// Previous
void AudioDisplay::Prev() {
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
		Play(start,end);
	}

	// Plain mode
	else {
		ChangeLine(-1);
	}
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
		sylstart = karaoke->syllables.at(i).position*10 + curStartMS;
		sylend = karaoke->syllables.at(i).length*10 + sylstart;
		if (ms >= sylstart && ms < sylend) {
			return i;
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


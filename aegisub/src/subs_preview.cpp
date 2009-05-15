// Copyright (c) 2007, Rodrigo Braz Monteiro
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


////////////
// Includes
#include "config.h"

#include <wx/msgdlg.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include "ass_style.h"
#include "subs_preview.h"
#include "video_provider_dummy.h"
#include "subtitles_provider_manager.h"
#include "ass_file.h"


///////////////
// Constructor
SubtitlesPreview::SubtitlesPreview(wxWindow *parent,int id,wxPoint pos,wxSize size,int winStyle,wxColour col)
: wxWindow(parent,id,pos,size,winStyle)
{
	AssStyle temp;
	bmp = NULL;
	style = NULL;
	vid = NULL;
	SetStyle(&temp);
	SetText(_T("preview"));
	SetSizeHints(size.GetWidth(),size.GetHeight(),-1,-1);
	backColour = col;
}


//////////////
// Destructor
SubtitlesPreview::~SubtitlesPreview() {
	delete bmp;
	bmp = NULL;
	delete style;
	style = NULL;
	delete vid;
	vid = NULL;
}


/////////////
// Set style
void SubtitlesPreview::SetStyle(AssStyle *_style) {
	// Prepare style
	AssStyle *tmpStyle = AssEntry::GetAsStyle(_style->Clone());
	tmpStyle->name = _T("Preview");
	tmpStyle->alignment = 5;
	for (int i=0;i<4;i++) tmpStyle->Margin[i] = 0;
	tmpStyle->UpdateData();

	// See if it's any different from the current
	if (style) {
		if (tmpStyle->IsEqualTo(style)) {
			delete tmpStyle;
			return;
		}

		delete style;
	}

	// Update
	style = tmpStyle;
	UpdateBitmap();
}


////////////
// Set text
void SubtitlesPreview::SetText(wxString text) {
	if (text != showText) {
		showText = text;
		UpdateBitmap();
	}
}


////////////////
// Update image
void SubtitlesPreview::UpdateBitmap(int w,int h) {
	// Visible?
	if (!IsShownOnScreen()) return;

	// Get size
	if (w == -1) {
		w = GetClientSize().GetWidth();
		h = GetClientSize().GetHeight();
	}

	// Delete old bmp if needed
	if (bmp) {
		if (bmp->GetWidth() != w || bmp->GetHeight() != h) {
			delete bmp;
			bmp = NULL;
		}
	}

	// Create bitmap
	if (!bmp) {
		bmp = new wxBitmap(w,h,-1);
	}

	// Get AegiVideoFrame
	if (!vid) vid = new DummyVideoProvider(0.0,10,w,h,backColour,true);
	AegiVideoFrame frame;
	frame.CopyFrom(vid->GetFrame(0,FORMAT_RGB32));

	// Try to get subtitles provider
	SubtitlesProvider *provider = NULL;
	try {
		provider = SubtitlesProviderFactoryManager::GetProvider();
	} 
	catch (...) {
		wxMessageBox(_T("Could not get any subtitles provider for the preview box. Make sure that you have a provider installed."),_T("No subtitles provider"),wxICON_ERROR);
	}

	// Provider OK
	if (provider) {
		// Generate subtitles
		AssFile *subs = new AssFile();
		subs->LoadDefault();
		int ver = 1;
		wxString outGroup;
		subs->InsertStyle((AssStyle *)style->Clone());
		subs->SetScriptInfo(_T("PlayResX"),wxString::Format(_T("%i"),w));
		subs->SetScriptInfo(_T("PlayResY"),wxString::Format(_T("%i"),h));
		subs->AddLine(_T("Dialogue: 0,0:00:00.00,0:00:05.00,Preview,,0000,0000,0000,,{\\q2}") + showText,_T("[Events]"),0,ver,&outGroup);

		// Apply subtitles
		try {
			provider->LoadSubtitles(subs);
			provider->DrawSubtitles(frame,0.1);
		}
		catch (...) {}
		delete provider;
	}

	// Convert frame to bitmap
	wxMemoryDC dc(*bmp);
	wxBitmap tempBmp(frame.GetImage());
	frame.Clear();
	dc.DrawBitmap(tempBmp,0,0);
	Refresh();
}


///////////////
// Event table
BEGIN_EVENT_TABLE(SubtitlesPreview,wxWindow)
	EVT_PAINT(SubtitlesPreview::OnPaint)
	EVT_SIZE(SubtitlesPreview::OnSize)
END_EVENT_TABLE()


///////////////
// Paint event
void SubtitlesPreview::OnPaint(wxPaintEvent &event) {
	wxPaintDC dc(this);
	if (!bmp) UpdateBitmap();
	if (bmp) dc.DrawBitmap(*bmp,0,0);
}


//////////////
// Size event
void SubtitlesPreview::OnSize(wxSizeEvent &event) {
	delete vid;
	vid = NULL;
	UpdateBitmap(event.GetSize().GetWidth(),event.GetSize().GetHeight());
}


//////////////
// Set colour
void SubtitlesPreview::SetColour(wxColour col) {
	backColour = col;
	delete vid;
	vid = NULL;
	UpdateBitmap();
}

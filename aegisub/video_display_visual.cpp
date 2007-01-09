// Copyright (c) 2005-2007, Rodrigo Braz Monteiro
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


#pragma once


//////////////
// Headers
#include <wx/wxprec.h>
#include "video_display_visual.h"
#include "video_display.h"
#include "video_provider.h"
#include "vfr.h"
#include "ass_file.h"
#include "ass_time.h"
#include "ass_dialogue.h"
#include "ass_override.h"
#include "ass_style.h"
#include "subs_grid.h"
#include "options.h"
#include "subs_edit_box.h"
#include "export_visible_lines.h"
#if USE_FEXTRACKER == 1
#include "../FexTrackerSource/FexTracker.h"
#include "../FexTrackerSource/FexTrackingFeature.h"
#include "../FexTrackerSource/FexMovement.h"
#endif


///////////////
// Constructor
VideoDisplayVisual::VideoDisplayVisual(VideoDisplay *par) {
	parent = par;
	backbuffer = NULL;
	curSelection = NULL;
	holding = false;
	mode = -1;
	SetMode(0);
}


//////////////
// Destructor
VideoDisplayVisual::~VideoDisplayVisual() {
	delete backbuffer;
}


////////////
// Set mode
void VideoDisplayVisual::SetMode(int _mode) {
	// Set mode
	mode = _mode;

	// Hide cursor
	if (mode == 0) {
		// Bleeeh! Hate this 'solution':
		#if __WXGTK__
		static char cursor_image[] = {0};
		wxCursor cursor(cursor_image, 8, 1, -1, -1, cursor_image);
		#else
		wxCursor cursor(wxCURSOR_BLANK);
		#endif // __WXGTK__
		parent->SetCursor(cursor);
	}

	// Show cursor
	else {
		parent->SetCursor(wxNullCursor);
	}
}


////////////////
// Draw overlay
void VideoDisplayVisual::DrawOverlay() {
	// Variables
	int x = mouse_x;
	int y = mouse_y;
	int w = parent->w;
	int h = parent->h;
	int frame_n = parent->frame_n;
	int sw,sh;
	parent->GetScriptSize(sw,sh);

	// Create backbuffer if needed
	bool needCreate = false;
	if (!backbuffer) needCreate = true;
	else if (backbuffer->GetWidth() != w || backbuffer->GetHeight() != h) {
		needCreate = true;
		delete backbuffer;
	}
	if (needCreate) backbuffer = new wxBitmap(w,h);

	// Prepare drawing
	wxMemoryDC dc;
	dc.SelectObject(*backbuffer);

	// Draw frame
	dc.DrawBitmap(parent->GetFrame(frame_n),0,0);

	// Draw the control points for FexTracker
	DrawTrackingOverlay(dc);

	// Draw pivot points of visible lines
	if (mode == 1) {
		int numRows = parent->grid->GetRows();
		int startMs = VFR_Output.GetTimeAtFrame(frame_n,true);
		int endMs = VFR_Output.GetTimeAtFrame(frame_n,false);
		AssDialogue *diag;
		dc.SetPen(wxPen(wxColour(255,0,0),1));

		// For each line
		for (int i=0;i<numRows;i++) {
			diag = parent->grid->GetDialogue(i);
			if (diag) {
				// Draw?
				bool draw = false;
				int dx = -1;
				int dy = -1;

				// Selected line
				if (diag == curSelection) {
					draw = true;
					dx = cur_x * w / sw;
					dy = cur_y * h / sh;
					dc.SetBrush(wxBrush(wxColour(255,255,255)));
				}

				// Line visible?
				else if (diag->Start.GetMS() <= startMs && diag->End.GetMS() >= endMs) {
					// Get position
					GetLinePosition(diag,dx,dy);
					dx = dx * w / sw;
					dy = dy * h / sh;

					// Mouse over?
					if (x >= dx-5 && x <= dx+5 && y >= dy-5 && y <= dy+5) dc.SetBrush(wxBrush(wxColour(255,255,255)));
					else dc.SetBrush(wxBrush(wxColour(255,255,0)));
					draw = true;
				}

				// Draw
				if (draw) {
					dc.DrawRectangle(dx-5,dy-5,11,11);
					dc.DrawLine(dx,dy-10,dx,dy+10);
					dc.DrawLine(dx-10,dy,dx+10,dy);
				}
			}
		}
	}

	// Current position info
	if (mode == 0 && x >= 0 && x < w && y >= 0 && y < h) {
		// Draw cross
		dc.SetPen(wxPen(wxColour(255,255,255),1));
		dc.SetLogicalFunction(wxINVERT);
		dc.DrawLine(0,y,w-1,y);
		dc.DrawLine(x,0,x,h-1);

		// Setup text
		wxFont font(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"));
		dc.SetFont(font);
		int tw,th;
		parent->GetTextExtent(mouseText,&tw,&th,NULL,NULL,&font);

		// Inversion
		bool left = x > w/2;
		bool bottom = y < h/2;

		// Text draw coords
		int dx = x,dy = y;
		if (left) dx -= tw + 4;
		else dx += 4;
		if (bottom) dy += 3;
		else dy -= th + 3;

		// Draw text
		dc.SetTextForeground(wxColour(64,64,64));
		dc.DrawText(mouseText,dx+1,dy-1);
		dc.DrawText(mouseText,dx+1,dy+1);
		dc.DrawText(mouseText,dx-1,dy-1);
		dc.DrawText(mouseText,dx-1,dy+1);
		dc.SetTextForeground(wxColour(255,255,255));
		dc.DrawText(mouseText,dx,dy);
	}

	// Blit to screen
	wxClientDC dcScreen(parent);
	//dcScreen.DrawBitmap(backbuffer,0,0);
	dcScreen.Blit(0,0,w,h,&dc,0,0);
}


////////////////////////
// Get position of line
void VideoDisplayVisual::GetLinePosition(AssDialogue *diag,int &x, int &y) {
	// No dialogue
	if (!diag) {
		x = -1;
		y = -1;
		return;
	}

	// Default values
	int margin[4];
	for (int i=0;i<4;i++) margin[i] = diag->Margin[i];
	int align = 2;

	// Get style
	AssStyle *style = parent->grid->ass->GetStyle(diag->Style);
	if (style) {
		align = style->alignment;
		for (int i=0;i<4;i++) {
			if (margin[i] == 0) margin[i] = style->Margin[i];
		}
	}

	// Script size
	int sw,sh;
	parent->GetScriptSize(sw,sh);

	// Process margins
	margin[3] = margin[2];
	margin[1] = sw - margin[1];
	margin[3] = sh - margin[3];

	// Position
	int posx = -1;
	int posy = -1;

	// Overrides processing
	diag->ParseASSTags();
	AssDialogueBlockOverride *override;
	AssOverrideTag *tag;
	size_t blockn = diag->Blocks.size();
	for (size_t i=0;i<blockn;i++) {
		override = AssDialogueBlock::GetAsOverride(diag->Blocks.at(i));
		if (override) {
			for (size_t j=0;j<override->Tags.size();j++) {
				tag = override->Tags.at(j);
				if ((tag->Name == _T("\\pos") || tag->Name == _("\\move")) && tag->Params.size() >= 2) {
					posx = tag->Params[0]->AsInt();
					posy = tag->Params[1]->AsInt();
				}
			}
		}
	}
	diag->ClearBlocks();

	// Got position
	if (posx != -1) {
		x = posx;
		y = posy;
		return;
	}

	// Alignment type
	int hor = (align - 1) % 3;
	int vert = (align - 1) / 3;

	// Calculate positions
	if (hor == 0) x = margin[0];
	else if (hor == 1) x = (margin[0] + margin[1])/2;
	else if (hor == 2) x = margin[1];
	if (vert == 0) y = margin[3];
	else if (vert == 1) y = (margin[2] + margin[3])/2;
	else if (vert == 2) y = margin[2];
}


//////////////////
// Draw Tracking Overlay
void VideoDisplayVisual::DrawTrackingOverlay( wxDC &dc )
{
#if USE_FEXTRACKER == 1
	int frame_n = parent->frame_n;
	VideoProvider *provider = parent->provider;
	if( parent->IsPlaying ) return;

	// Get line
	AssDialogue *curline = parent->grid->GetDialogue(parent->grid->editBox->linen);
	if( !curline ) return;

	int StartFrame = VFR_Output.GetFrameAtTime(curline->Start.GetMS(),true);
	int EndFrame = VFR_Output.GetFrameAtTime(curline->End.GetMS(),false);
	
	if( frame_n<StartFrame || frame_n>EndFrame ) return;

	int localframe = frame_n - StartFrame;

	if( curline->Tracker )
	{
		if( curline->Tracker->GetFrame() <= localframe ) return;

		dc.SetLogicalFunction(wxCOPY);

		for( int i=0;i<curline->Tracker->GetCount();i++ )
		{
			FexTrackingFeature* f = (*curline->Tracker)[i];
			if( f->StartTime > localframe ) continue;
			int llf = localframe - f->StartTime;
			if( f->Pos.size() <= llf ) continue;
			vec2 pt = f->Pos[llf];
			pt.x *= provider->GetZoom();
			pt.y *= provider->GetZoom();
			pt.x = int(pt.x);
			pt.y = int(pt.y);

			dc.SetPen(wxPen(wxColour(255*(1-f->Influence),255*f->Influence,0),1));

			dc.DrawLine( pt.x-2, pt.y, pt.x, pt.y );
			dc.DrawLine( pt.x, pt.y-2, pt.x, pt.y );
			dc.DrawLine( pt.x+1, pt.y, pt.x+3, pt.y );
			dc.DrawLine( pt.x, pt.y+1, pt.x, pt.y+3 );
		}
	}
	if( curline->Movement )
	{
		if( curline->Movement->Frames.size() <= localframe ) return;

		dc.SetPen(wxPen(wxColour(255,0,0),2));
		FexMovementFrame f = curline->Movement->Frames.lVal[localframe];
		f.Pos.x *= provider->GetZoom();
		f.Pos.y *= provider->GetZoom();
		f.Scale.x *= 30* provider->GetZoom();
		f.Scale.y *= 30* provider->GetZoom();

		FexMovementFrame f3 = f;
		dc.SetPen(wxPen(wxColour(0,0,255),1));
		int nBack = 8;
		while( --localframe>0 && nBack-- >0 )
		{
			FexMovementFrame f2 = curline->Movement->Frames.lVal[localframe];
			f2.Pos.x *= provider->GetZoom();
			f2.Pos.y *= provider->GetZoom();
			dc.DrawLine( f2.Pos.x, f2.Pos.y, f3.Pos.x, f3.Pos.y );
			f3 = f2;
		}

		dc.SetPen(wxPen(wxColour(255,0,0),2));
		dc.DrawLine( f.Pos.x-f.Scale.x, f.Pos.y, f.Pos.x+f.Scale.x+1, f.Pos.y );
		dc.DrawLine( f.Pos.x, f.Pos.y-f.Scale.y, f.Pos.x, f.Pos.y+f.Scale.y+1 );

		f3 = f;
		dc.SetPen(wxPen(wxColour(0,255,0),1));
		int nFront = 8;
		localframe = frame_n - StartFrame;
		while( ++localframe<curline->Movement->Frames.size() && nFront-- >0 )
		{
			FexMovementFrame f2 = curline->Movement->Frames.lVal[localframe];
			f2.Pos.x *= provider->GetZoom();
			f2.Pos.y *= provider->GetZoom();
			dc.DrawLine( f2.Pos.x, f2.Pos.y, f3.Pos.x, f3.Pos.y );
			f3 = f2;
		}
	}
#endif
}


///////////////
// Mouse event
void VideoDisplayVisual::OnMouseEvent (wxMouseEvent &event) {
	// Coords
	int x = event.GetX();
	int y = event.GetY();
	int w = parent->w;
	int h = parent->h;
	int sw,sh;
	parent->GetScriptSize(sw,sh);
	int frame_n = parent->frame_n;
	VideoProvider *provider = parent->provider;
	SubtitlesGrid *grid = parent->grid;
	bool hasOverlay = false;
	bool realTime = Options.AsBool(_T("Video Visual Realtime"));

	// FexTracker
	#if USE_FEXTRACKER == 1
	if( event.ButtonDown(wxMOUSE_BTN_LEFT) ) {
		parent->MouseDownX = x;
		parent->MouseDownY = y;
		parent->bTrackerEditing = 1;
	}
	if( event.ButtonUp(wxMOUSE_BTN_LEFT) ) parent->bTrackerEditing = 0;

	// Do tracker influence if needed
	if( parent->bTrackerEditing ) {
		AssDialogue *curline = parent->grid->GetDialogue(parent->grid->editBox->linen);
		int StartFrame, EndFrame, localframe;
		if( curline && (StartFrame = VFR_Output.GetFrameAtTime(curline->Start.GetMS(),true)) <= frame_n	&& (EndFrame = VFR_Output.GetFrameAtTime(curline->End.GetMS(),false)) >= frame_n ) {
			localframe = frame_n - StartFrame;
			if( parent->TrackerEdit!=0 && curline->Tracker && localframe < curline->Tracker->GetFrame() ) curline->Tracker->InfluenceFeatures( localframe, float(x)/provider->GetZoom(), float(y)/provider->GetZoom(), parent->TrackerEdit );
			if( parent->MovementEdit!=0 && curline->Movement && localframe < curline->Movement->Frames.size() )	{// no /provider->GetZoom() to improve precision
				if( parent->MovementEdit==1 ) {
					for( int i=0;i<curline->Movement->Frames.size();i++ ) {
						curline->Movement->Frames[i].Pos.x += float(x-parent->MouseDownX);
						curline->Movement->Frames[i].Pos.y += float(y-parent->MouseDownY);
					}
				}
				else if( parent->MovementEdit==2 ) {
					curline->Movement->Frames[localframe].Pos.x += float(x-parent->MouseDownX);
					curline->Movement->Frames[localframe].Pos.y += float(y-parent->MouseDownY);
				}
				else if( parent->MovementEdit==3 ) {
					for( int i=0;i<=localframe;i++ ) {
						curline->Movement->Frames[i].Pos.x += float(x-parent->MouseDownX);
						curline->Movement->Frames[i].Pos.y += float(y-parent->MouseDownY);
					}
				}
				else if( parent->MovementEdit==4 ) {
					for( int i=localframe;i<curline->Movement->Frames.size();i++ ) {
						curline->Movement->Frames[i].Pos.x += float(x-parent->MouseDownX);
						curline->Movement->Frames[i].Pos.y += float(y-parent->MouseDownY);
					}
				}
			}
			parent->MouseDownX = x;
			parent->MouseDownY = y;
		}
	}
	#endif

	// Text of current coords
	int vx = (sw * x + w/2) / w;
	int vy = (sh * y + h/2) / h;
	if (!event.ShiftDown()) mouseText = wxString::Format(_T("%i,%i"),vx,vy);
	else mouseText = wxString::Format(_T("%i,%i"),vx - sw,vy - sh);

	// Start dragging
	if (mode == 1 && event.LeftIsDown() && !holding) {
		// For each line
		int numRows = parent->grid->GetRows();
		int startMs = VFR_Output.GetTimeAtFrame(frame_n,true);
		int endMs = VFR_Output.GetTimeAtFrame(frame_n,false);
		AssDialogue *diag;
		AssDialogue *gotDiag = NULL;
		for (int i=0;i<numRows;i++) {
			diag = parent->grid->GetDialogue(i);
			if (diag) {
				// Line visible?
				if (diag->Start.GetMS() <= startMs && diag->End.GetMS() >= endMs) {
					// Get position
					int lineX,lineY;
					GetLinePosition(diag,lineX,lineY);
					lineX = lineX * w / sw;
					lineY = lineY * h / sh;

					// Mouse over?
					if (x >= lineX-5 && x <= lineX+5 && y >= lineY-5 && y <= lineY+5) {
						parent->grid->editBox->SetToLine(i,true);
						gotDiag = diag;
						orig_x = lineX;
						orig_y = lineY;
						break;
					}
				}
			}
		}

		// Got a line?
		if (gotDiag) {
			// Set dialogue
			curSelection = gotDiag;
			start_x = x;
			start_y = y;

			// Hold it
			holding = true;
			hold = 1;
			parent->CaptureMouse();
			hasOverlay = true;
		}
	}

	// Drag
	if (hold == 1) {
		cur_x = (x - start_x + orig_x) * sw / w;
		cur_y = (y - start_y + orig_y) * sh / h;
		if (realTime) {
			AssLimitToVisibleFilter::SetFrame(frame_n);
			grid->editBox->SetOverride(_T("\\pos"),wxString::Format(_T("(%i,%i)"),cur_x,cur_y),0);
			grid->editBox->CommitText(true);
			grid->CommitChanges(false,true);
		}
	}

	// End dragging
	if (holding && !event.LeftIsDown()) {
		// Disable limiting
		if (realTime) AssLimitToVisibleFilter::SetFrame(-1);

		// Finished dragging subtitles
		if (hold == 1) {
			grid->editBox->SetOverride(_T("\\pos"),wxString::Format(_T("(%i,%i)"),cur_x,cur_y),0);
			grid->editBox->CommitText();
			grid->ass->FlagAsModified();
			grid->CommitChanges(false,true);
			curSelection = NULL;
		}

		// Set flags
		hold = 0;
		holding = false;
		hasOverlay = true;
		parent->ReleaseMouse();
		parent->SetFocus();
	}

	// Double click
	if (mode == 0 && event.LeftDClick()) {
		grid->editBox->SetOverride(_T("\\pos"),wxString::Format(_T("(%i,%i)"),vx,vy),0);
		grid->editBox->CommitText();
		grid->ass->FlagAsModified();
		grid->CommitChanges(false,true);
		parent->SetFocus();
	}

	// Hover
	if (x != mouse_x || y != mouse_y) {
		// Set coords
		mouse_x = x;
		mouse_y = y;
		hasOverlay = true;
	}

	// Has something to draw
	if (hasOverlay) {
		DrawOverlay();
	}
}


/////////////
// Key event
void VideoDisplayVisual::OnKeyEvent(wxKeyEvent &event) {
	if (event.GetKeyCode() == 'A') SetMode(0);
	if (event.GetKeyCode() == 'S') SetMode(1);
}

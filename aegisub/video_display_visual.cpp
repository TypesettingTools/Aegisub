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
#include "utils.h"
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
	colour[0] = wxColour(27,60,114);
	colour[1] = wxColour(175,219,254);
	colour[2] = wxColour(255,255,255);
	colour[3] = wxColour(187,0,0);
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
	if (mode != 0) {
		int numRows = parent->grid->GetRows();
		int startMs = VFR_Output.GetTimeAtFrame(frame_n,true);
		int endMs = VFR_Output.GetTimeAtFrame(frame_n,false);
		AssDialogue *diag;

		// For each line
		for (int i=0;i<numRows;i++) {
			diag = parent->grid->GetDialogue(i);
			if (diag) {
				// Draw?
				bool draw = false;
				bool high = false;
				bool isCur = diag == curSelection;
				int dx = -1;
				int dy = -1;
				int orgx = -1;
				int orgy = -1;
				float rx = 0.0f;
				float ry = 0.0f;
				float rz = 0.0f;
				int deltax = 0;
				int deltay = 0;

				// Line visible?
				if (isCur || (diag->Start.GetMS() <= startMs && diag->End.GetMS() >= endMs)) {
					// Get position
					if (isCur && mode == 1) {
						dx = cur_x;
						dy = cur_y;
						high = true;
					}
					else GetLinePosition(diag,dx,dy,orgx,orgy);

					// Mouse over?
					if (mode == 1 && (x >= dx-8 && x <= dx+8 && y >= dy-8 && y <= dy+8)) {
						high = true;
					}

					// Highlight
					int brushCol = 1;
					if (high) brushCol = 2;
					dc.SetBrush(wxBrush(colour[brushCol]));
					dc.SetPen(wxPen(colour[0],1));

					// Set drawing coordinates
					int radius = (int) sqrt(double((dx-orgx)*(dx-orgx)+(dy-orgy)*(dy-orgy)));
					dx = dx * w / sw;
					dy = dy * h / sh;
					orgx = orgx * w / sw;
					orgy = orgy * h / sh;

					// Drag
					if (mode == 1) {
						dc.DrawRectangle(dx-8,dy-8,17,17);
						dc.DrawLine(dx,dy-16,dx,dy+16);
						dc.DrawLine(dx-16,dy,dx+16,dy);
					}

					// Rotate Z
					if (mode == 2) {
						// Calculate radius
						float arDistort = float(w) * float(sh) / float(h) / float(sw);
						int oRadiusX = radius * w / sw;
						int oRadiusY = radius * h / sh;
						if (radius < 50) radius = 50;
						int radiusX = radius * w / sw;
						int radiusY = radius * h / sh;

						// Get angle
						if (isCur && mode == 2) {
							rz = curAngle;
						}
						else GetLineRotation(diag,rx,ry,rz);

						// Get deltas
						deltax = int(cos(rz*3.1415926536/180.0)*radiusX);
						deltay = int(-sin(rz*3.1415926536/180.0)*radiusY);
						int odx = dx;
						int ody = dy;
						dx = orgx;
						dy = orgy;

						// Draw pivot
						dc.DrawCircle(dx,dy,7);
						dc.DrawLine(dx,dy-16,dx,dy+16);
						dc.DrawLine(dx-16,dy,dx+16,dy);

						// Draw the circle
						dc.SetBrush(*wxTRANSPARENT_BRUSH);
						dc.DrawEllipse(dx-radiusX-2,dy-radiusY-2,2*radiusX+4,2*radiusY+4);
						dc.DrawEllipse(dx-radiusX+2,dy-radiusY+2,2*radiusX-4,2*radiusY-4);

						// Draw line to mouse
						dc.DrawLine(dx,dy,mouse_x,mouse_y);

						// Draw the baseline
						dc.SetPen(wxPen(colour[3],2));
						dc.DrawLine(dx+deltax,dy+deltay,dx-deltax,dy-deltay);

						// Draw the connection line
						if (orgx != odx && orgy != ody) {
							double angle = atan2(double(dy*sh/h-ody*sh/h),double(odx*sw/w-dx*sw/w)) + rz*3.1415926536/180.0;
							int fx = dx+int(cos(angle)*oRadiusX);
							int fy = dy-int(sin(angle)*oRadiusY);
							dc.DrawLine(dx,dy,fx,fy);
							//dc.SetPen(wxPen(colour[0],1));
							int mdx = cos(rz*3.1415926536/180.0)*20;
							int mdy = -sin(rz*3.1415926536/180.0)*20;
							dc.DrawLine(fx-mdx,fy-mdy,fx+mdx,fy+mdy);
						}

						// Draw the rotation line
						dc.SetPen(wxPen(colour[0],1));
						dc.SetBrush(wxBrush(colour[brushCol]));
						dc.DrawCircle(dx+deltax,dy+deltay,4);
					}
				}
			}
		}
	}

	// Current position info
	if (mode == 0 && x >= 0 && x < w && y >= 0 && y < h) {
		// Draw cross
		dc.SetPen(wxPen(colour[2],1));
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
		dc.SetTextForeground(colour[2]);
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
	int orgx=0,orgy=0;
	GetLinePosition(diag,x,y,orgx,orgy);
}
void VideoDisplayVisual::GetLinePosition(AssDialogue *diag,int &x, int &y, int &orgx, int &orgy) {
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
	bool posSet = false;
	bool orgSet = false;
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

				// Position
				if ((tag->Name == _T("\\pos") || tag->Name == _("\\move")) && tag->Params.size() >= 2) {
					if (!posSet) {
						posx = tag->Params[0]->AsInt();
						posy = tag->Params[1]->AsInt();
						posSet = true;
					}
				}

				// Alignment
				else if ((tag->Name == _T("\\an") || tag->Name == _T("\\a")) && tag->Params.size() >= 1) {
					align = tag->Params[0]->AsInt();
					if (tag->Name == _T("\\a")) {
						switch(align) {
							case 1: align = 1; break;
							case 2: align = 2; break;
							case 3: align = 3; break;
							case 5: align = 7; break;
							case 6: align = 8; break;
							case 7: align = 9; break;
							case 9: align = 4; break;
							case 10: align = 5; break;
							case 11: align = 6; break;
							default: align = 2; break;
						}
					}
				}

				// Origin
				else if (!orgSet && tag->Name == _T("\\org") && tag->Params.size() >= 2) {
					orgx = tag->Params[0]->AsInt();
					orgy = tag->Params[1]->AsInt();
					orgSet = true;
				}
			}
		}
	}
	diag->ClearBlocks();

	// Got position
	if (posSet) {
		x = posx;
		y = posy;
		if (!orgSet) {
			orgx = x;
			orgy = y;
		}
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

	// No origin?
	if (!orgSet) {
		orgx = x;
		orgy = y;
	}
}


///////////////////////
// Get line's rotation
void VideoDisplayVisual::GetLineRotation(AssDialogue *diag,float &rx,float &ry,float &rz) {
	// Default values
	rx = ry = rz = 0.0f;

	// Prepare overrides
	diag->ParseASSTags();
	AssDialogueBlockOverride *override;
	AssOverrideTag *tag;
	size_t blockn = diag->Blocks.size();
	if (blockn == 0) {
		diag->ClearBlocks();
		return;
	}

	// Process override
	override = AssDialogueBlock::GetAsOverride(diag->Blocks.at(0));
	if (override) {
		for (size_t j=0;j<override->Tags.size();j++) {
			tag = override->Tags.at(j);
			if (tag->Name == _T("\\frx") && tag->Params.size() == 1) {
				rx = tag->Params[0]->AsFloat();
			}
			if (tag->Name == _T("\\fry") && tag->Params.size() == 1) {
				ry = tag->Params[0]->AsFloat();
			}
			if ((tag->Name == _T("\\frz") || tag->Name == _T("\fr")) && tag->Params.size() == 1) {
				rz = tag->Params[0]->AsFloat();
			}
		}
	}
	diag->ClearBlocks();
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

		dc.SetPen(wxPen(colour[0],2));
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

		dc.SetPen(wxPen(colour[0],2));
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
	int orgx = -1;
	int orgy = -1;
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
	if (mode != 0 && event.LeftIsDown() && !holding) {
		float rx,ry,rz;
		AssDialogue *gotDiag = NULL;

		// Drag
		if (mode == 1) {
			// For each line
			int numRows = parent->grid->GetRows();
			int startMs = VFR_Output.GetTimeAtFrame(frame_n,true);
			int endMs = VFR_Output.GetTimeAtFrame(frame_n,false);
			AssDialogue *diag;
			for (int i=0;i<numRows;i++) {
				diag = parent->grid->GetDialogue(i);
				if (diag) {
					// Line visible?
					if (diag->Start.GetMS() <= startMs && diag->End.GetMS() >= endMs) {
						// Get position
						int lineX,lineY;
						int torgx,torgy;
						GetLinePosition(diag,lineX,lineY,torgx,torgy);
						lineX = lineX * w / sw;
						lineY = lineY * h / sh;
						orgx = orgx * w / sw;
						orgy = orgy * h / sh;

						// Mouse over?
						if (x >= lineX-8 && x <= lineX+8 && y >= lineY-8 && y <= lineY+8) {
							parent->grid->editBox->SetToLine(i,true);
							gotDiag = diag;
							orig_x = lineX;
							orig_y = lineY;
							orgx = torgx;
							orgy = torgy;
							break;
						}
					}
				}
			}
		}

		// Rotate
		else {
			// Check if it's within range
			gotDiag = parent->grid->GetDialogue(parent->grid->editBox->linen);
			int f1 = VFR_Output.GetFrameAtTime(gotDiag->Start.GetMS(),true);
			int f2 = VFR_Output.GetFrameAtTime(gotDiag->End.GetMS(),false);

			// Invisible
			if (f1 > frame_n || f2 < frame_n) {
				gotDiag = NULL;
			}

			// OK
			else {
				GetLinePosition(gotDiag,orig_x,orig_y,orgx,orgy);
			}
		}

		// Got a line?
		if (gotDiag) {
			// Set dialogue
			curSelection = gotDiag;

			// Set coordinates
			if (mode == 1) {
				start_x = x;
				start_y = y;
			}

			// Rotate Z
			if (mode == 2) {
				lineOrgX = orgx;
				lineOrgY = orgy;
				startAngle = atan2(double(lineOrgY-y*sh/h),double(x*sw/w-lineOrgX)) * 180.0 / 3.1415926535897932;
				GetLineRotation(curSelection,rx,ry,rz);
				origAngle = rz;
			}

			// Hold it
			holding = true;
			hold = mode;
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

	// Rotate
	if (hold == 2) {
		// Find screen angle
		float screenAngle = atan2(double(lineOrgY-y*sh/h),double(x*sw/w-lineOrgX)) * 180.0 / 3.1415926535897932;

		// Update
		curAngle = screenAngle - startAngle + origAngle;
		if (curAngle < 0.0) curAngle += 360.0;
		if (realTime) {
			AssLimitToVisibleFilter::SetFrame(frame_n);
			grid->editBox->SetOverride(_T("\\frz"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curAngle)),0);
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
		}

		// Finished rotating subtitles
		if (hold == 2) {
			grid->editBox->SetOverride(_T("\\frz"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curAngle)),0);
			grid->editBox->CommitText();
			grid->ass->FlagAsModified();
			grid->CommitChanges(false,true);
		}

		// Set flags
		hold = 0;
		holding = false;
		hasOverlay = true;

		// Clean up
		curSelection = NULL;
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
	if (event.GetKeyCode() == 'D') SetMode(2);
}

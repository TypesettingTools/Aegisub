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


//////////////
// Headers
#include <wx/glcanvas.h>
#include <GL/glu.h>
#include <wx/wxprec.h>
#include "video_display_visual.h"
#include "video_display_fextracker.h"
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


///////////////
// Constructor
VideoDisplayVisual::VideoDisplayVisual(VideoDisplay *par) {
	parent = par;
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
	int frame_n = VideoContext::Get()->GetFrameN();
	int w,h;
	parent->GetClientSize(&w,&h);
	int sw,sh;
	VideoContext::Get()->GetScriptSize(sw,sh);
	int x = mouseX;
	int y = mouseY;
	int mx = mouseX * sw / w;
	int my = mouseY * sh / h;

	// Draw the control points for FexTracker
	glDisable(GL_TEXTURE_2D);
	parent->tracker->Render();

	// Draw lines
	if (mode != 0) {
		int numRows = VideoContext::Get()->grid->GetRows();
		AssDialogue *diag;
		AssDialogue *diagHigh = NULL;

		// Find where the highlight is
		if (mode == 1) {
			int dx,dy;
			for (int i=0;i<numRows;i++) {
				diag = VideoContext::Get()->grid->GetDialogue(i);
				if (diag) {
					if (VFR_Output.GetFrameAtTime(diag->Start.GetMS(),true) <= frame_n && VFR_Output.GetFrameAtTime(diag->End.GetMS(),false) >= frame_n) {
						GetLinePosition(diag,dx,dy);
						dx = dx * w / sw;
						dy = dy * h / sh;
						if (x >= dx-8 && x <= dx+8 && y >= dy-8 && y <= dy+8) {
							diagHigh = diag;
						}
					}
				}
			}
		}

		// For each line
		for (int i=0;i<numRows;i++) {
			diag = VideoContext::Get()->grid->GetDialogue(i);
			if (diag) {
				// Draw?
				bool draw = false;
				bool high = false;
				bool isCur = diag == curSelection;
				bool timeVisible = VFR_Output.GetFrameAtTime(diag->Start.GetMS(),true) <= frame_n && VFR_Output.GetFrameAtTime(diag->End.GetMS(),false) >= frame_n;
				bool show = timeVisible;
				if (mode != 1) {
					show = diag == VideoContext::Get()->grid->GetDialogue(VideoContext::Get()->grid->editBox->linen) && timeVisible;
				}

				// Variables
				int dx = -1;
				int dy = -1;
				int orgx = -1;
				int orgy = -1;
				float rx = 0.0f;
				float ry = 0.0f;
				float rz = 0.0f;
				float scalX = 100.0f;
				float scalY = 100.0f;
				int deltax = 0;
				int deltay = 0;

				// Line visible?
				if (show) {
					// Get position
					if (isCur && mode == 1) {
						dx = curX;
						dy = curY;
						high = true;
					}
					else GetLinePosition(diag,dx,dy,orgx,orgy);

					// Get scale
					if (isCur && mode == 4) {
						scalX = curScaleX;
						scalY = curScaleY;
					}
					else GetLineScale(diag,scalX,scalY);

					// Mouse over?
					if (diag == diagHigh) {
						high = true;
					}

					// Highlight
					int brushCol = 1;
					if (high) brushCol = 2;
					SetLineColour(colour[0]);
					SetFillColour(colour[brushCol],0.3f);

					// Set drawing coordinates
					int radius = (int) sqrt(double((dx-orgx)*(dx-orgx)+(dy-orgy)*(dy-orgy)));

					// Drag
					if (mode == 1) {
						SetFillColour(colour[brushCol],0.5f);
						DrawRectangle(dx-8,dy-8,dx+8,dy+8);
						SetLineColour(colour[2]);
						SetModeLine();
						DrawLine(dx,dy-16,dx,dy+16);
						DrawLine(dx-16,dy,dx+16,dy);
					}

					// Rotation
					if (mode == 2 || mode == 3) {
						// Pivot coordinates
						int odx = dx;
						int ody = dy;
						dx = orgx;
						dy = orgy;

						// Draw pivot
						DrawCircle(dx,dy,7);
						DrawLine(dx,dy-16,dx,dy+16);
						DrawLine(dx-16,dy,dx+16,dy);
							
						// Get angle
						GetLineRotation(diag,rx,ry,rz);
						if (isCur) {
							if (mode == 2) rz = curAngle;
							else {
								rx = curAngle;
								ry = curAngle2;
							}
						}

						// Rotate Z
						if (mode == 2) {
							// Transform
							glMatrixMode(GL_MODELVIEW);
							glPushMatrix();
							glLoadIdentity();
							glTranslatef(dx,dy,-1.0f);
							float matrix[16] = { 2500, 0, 0, 0, 0, 2500, 0, 0, 0, 0, 1, 1, 0, 0, 2500, 2500 };
							glMultMatrixf(matrix);
							glScalef(1.0f,1.0f,8.0f);
							glRotatef(ry,0.0f,-1.0f,0.0f);
							glRotatef(rx,-1.0f,0.0f,0.0f);
							glScalef(scalX/100.0f,scalY/100.0f,1.0f);

							// Calculate radii
							int oRadius = radius;
							if (radius < 50) radius = 50;

							// Draw the circle
							SetLineColour(colour[0]);
							SetFillColour(colour[1],0.3f);
							DrawRing(0,0,radius+4,radius-4);

							// Draw markers around circle
							int markers = 6;
							float markStart = -90.0f / markers;
							float markEnd = markStart+(180.0f/markers);
							for (int i=0;i<markers;i++) {
								float angle = i*(360.0f/markers);
								DrawRing(0,0,radius+30,radius+12,radius/radius,angle+markStart,angle+markEnd);
							}

							// Get deltas
							deltax = int(cos(rz*3.1415926536/180.0)*radius);
							deltay = int(-sin(rz*3.1415926536/180.0)*radius);

							// Draw the baseline
							SetLineColour(colour[3],1.0f,2);
							DrawLine(deltax,deltay,-deltax,-deltay);

							// Draw the connection line
							if (orgx != odx && orgy != ody) {
								//double angle = atan2(double(dy*sh/h-ody*sh/h),double(odx*sw/w-dx*sw/w)) + rz*3.1415926536/180.0;
								double angle = atan2(double(dy-ody),double(odx-dx)) + rz*3.1415926536/180.0;
								int fx = int(cos(angle)*oRadius);
								int fy = -int(sin(angle)*oRadius);
								DrawLine(0,0,fx,fy);
								int mdx = cos(rz*3.1415926536/180.0)*20;
								int mdy = -sin(rz*3.1415926536/180.0)*20;
								DrawLine(-mdx,-mdy,mdx,mdy);
							}

							// Draw the rotation line
							SetLineColour(colour[0],1.0f,1);
							SetFillColour(colour[brushCol],0.3f);
							DrawCircle(deltax,deltay,4);

							// Restore
							glPopMatrix();

							// Draw line to mouse
							SetLineColour(colour[0]);
							DrawLine(dx,dy,mx,my);
						}

						// Rotate XY
						if (mode == 3) {
							// Transform grid
							glMatrixMode(GL_MODELVIEW);
							glPushMatrix();
							glLoadIdentity();
							glTranslatef(dx,dy,0.0f);
							float matrix[16] = { 2500, 0, 0, 0, 0, 2500, 0, 0, 0, 0, 1, 1, 0, 0, 2500, 2500 };
							glMultMatrixf(matrix);
							glScalef(1.0f,1.0f,8.0f);
							if (ry != 0.0f) glRotatef(ry,0.0f,-1.0f,0.0f);
							if (rx != 0.0f) glRotatef(rx,-1.0f,0.0f,0.0f);
							if (rz != 0.0f) glRotatef(rz,0.0f,0.0f,-1.0f);

							// Draw grid
							glShadeModel(GL_SMOOTH);
							SetLineColour(colour[0],0.5f,1);
							SetModeLine();
							float r = colour[0].Red()/255.0f;
							float g = colour[0].Green()/255.0f;
							float b = colour[0].Blue()/255.0f;
							glBegin(GL_LINES);
							for (int i=0;i<11;i++) {
								float a = 1.0f - abs(i-5)*0.18f;
								int pos = 20*(i-5);
								glColor4f(r,g,b,0.0f);
								glVertex2i(pos,120);
								glColor4f(r,g,b,a);
								glVertex2i(pos,0);
								glVertex2i(pos,0);
								glColor4f(r,g,b,0.0f);
								glVertex2i(pos,-120);
								glVertex2i(120,pos);
								glColor4f(r,g,b,a);
								glVertex2i(0,pos);
								glVertex2i(0,pos);
								glColor4f(r,g,b,0.0f);
								glVertex2i(-120,pos);
							}
							glEnd();

							// Draw vectors
							SetLineColour(colour[3],1.0f,2);
							SetModeLine();
							glBegin(GL_LINES);
								glVertex3f(0.0f,0.0f,0.0f);
								glVertex3f(50.0f,0.0f,0.0f);
								glVertex3f(0.0f,0.0f,0.0f);
								glVertex3f(0.0f,-50.0f,0.0f);
								glVertex3f(0.0f,0.0f,0.0f);
								glVertex3f(0.0f,0.0f,-50.0f);
							glEnd();

							// Draw arrow tops
							glBegin(GL_TRIANGLE_FAN);
								glVertex3f(60.0f,0.0f,0.0f);
								glVertex3f(50.0f,-3.0f,-3.0f);
								glVertex3f(50.0f,3.0f,-3.0f);
								glVertex3f(50.0f,3.0f,3.0f);
								glVertex3f(50.0f,-3.0f,3.0f);
								glVertex3f(50.0f,-3.0f,-3.0f);
							glEnd();
							glBegin(GL_TRIANGLE_FAN);
								glVertex3f(0.0f,-60.0f,0.0f);
								glVertex3f(-3.0f,-50.0f,-3.0f);
								glVertex3f(3.0f,-50.0f,-3.0f);
								glVertex3f(3.0f,-50.0f,3.0f);
								glVertex3f(-3.0f,-50.0f,3.0f);
								glVertex3f(-3.0f,-50.0f,-3.0f);
							glEnd();
							glBegin(GL_TRIANGLE_FAN);
								glVertex3f(0.0f,0.0f,-60.0f);
								glVertex3f(-3.0f,-3.0f,-50.0f);
								glVertex3f(3.0f,-3.0f,-50.0f);
								glVertex3f(3.0f,3.0f,-50.0f);
								glVertex3f(-3.0f,3.0f,-50.0f);
								glVertex3f(-3.0f,-3.0f,-50.0f);
							glEnd();

							// Restore gl's state
							glPopMatrix();
							glShadeModel(GL_FLAT);
						}
					}

					// Scale
					if (mode == 4) {
						// Scale parameters
						int len = 160;
						int lenx = int(1.6 * scalX);
						int leny = int(1.6 * scalY);
						dx = MID(len/2+10,dx,sw-len/2-30);
						dy = MID(len/2+10,dy,sh-len/2-30);
						int drawX = dx + len/2 + 10;
						int drawY = dy + len/2 + 10;

						// Draw length markers
						SetLineColour(colour[3],1.0f,2);
						DrawLine(dx-lenx/2,drawY+10,dx+lenx/2,drawY+10);
						DrawLine(drawX+10,dy-leny/2,drawX+10,dy+leny/2);
						SetLineColour(colour[0],1.0f,1);
						SetFillColour(colour[brushCol],0.3f);
						DrawCircle(dx+lenx/2,drawY+10,4);
						DrawCircle(drawX+10,dy-leny/2,4);

						// Draw horizontal scale
						SetLineColour(colour[0],1.0f,1);
						DrawRectangle(dx-len/2,drawY,dx+len/2+1,drawY+5);
						SetLineColour(colour[0],1.0f,2);
						DrawLine(dx-len/2+1,drawY+5,dx-len/2+1,drawY+15);
						DrawLine(dx+len/2,drawY+5,dx+len/2,drawY+15);

						// Draw vertical scale
						SetLineColour(colour[0],1.0f,1);
						DrawRectangle(drawX,dy-len/2,drawX+5,dy+len/2+1);
						SetLineColour(colour[0],1.0f,2);
						DrawLine(drawX+5,dy-len/2+1,drawX+15,dy-len/2+1);
						DrawLine(drawX+5,dy+len/2,drawX+15,dy+len/2);
					}

					// Clip
					if (mode == 5) {
						int dx1,dx2,dy1,dy2;

						// Get position
						if (isCur) {
							dx1 = startX * sw / w;
							dy1 = startY * sh / h;
							dx2 = mx;
							dy2 = my;
						}
						else GetLineClip(diag,dx1,dy1,dx2,dy2);

						// Swap
						if (dx1 > dx2) IntSwap(dx1,dx2);
						if (dy1 > dy2) IntSwap(dy1,dy2);

						// Draw rectangle
						SetLineColour(colour[3]);
						SetFillColour(colour[3],0.0f);
						DrawRectangle(dx1,dy1,dx2,dy2);

						// Draw outside area
						SetLineColour(colour[3],0.0f);
						SetFillColour(colour[3],0.3f);
						glEnable(GL_BLEND);
						glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
						DrawRectangle(0,0,sw,dy1);
						DrawRectangle(0,dy2,sw,sh);
						DrawRectangle(0,dy1,dx1,dy2);
						DrawRectangle(dx2,dy1,sw,dy2);
						glDisable(GL_BLEND);

						// Draw circles
						SetLineColour(colour[0]);
						SetFillColour(colour[1],0.5);
						DrawCircle(dx1,dy1,4);
						DrawCircle(dx2,dy1,4);
						DrawCircle(dx2,dy2,4);
						DrawCircle(dx1,dy2,4);
					}
				}
			}
		}
	}

	// Current position info
	if (mode == 0 && x >= 0 && x < w && y >= 0 && y < h) {
		// Draw cross
		glEnable(GL_COLOR_LOGIC_OP);
		glLogicOp(GL_INVERT);
		glBegin(GL_LINES);
			glColor3f(1.0f,1.0f,1.0f);
			glVertex2f(0,my);
			glVertex2f(sw,my);
			glVertex2f(mx,0);
			glVertex2f(mx,sh);
		glEnd();
		glDisable(GL_COLOR_LOGIC_OP);

		//// Setup text
		//wxFont font(10,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,_T("Verdana"));
		//dc.SetFont(font);
		//int tw,th;
		//parent->GetTextExtent(mouseText,&tw,&th,NULL,NULL,&font);

		//// Inversion
		//bool left = x > w/2;
		//bool bottom = y < h/2;

		//// Text draw coords
		//int dx = x,dy = y;
		//if (left) dx -= tw + 4;
		//else dx += 4;
		//if (bottom) dy += 3;
		//else dy -= th + 3;

		//// Draw text
		//dc.SetTextForeground(wxColour(64,64,64));
		//dc.DrawText(mouseText,dx+1,dy-1);
		//dc.DrawText(mouseText,dx+1,dy+1);
		//dc.DrawText(mouseText,dx-1,dy-1);
		//dc.DrawText(mouseText,dx-1,dy+1);
		//dc.SetTextForeground(colour[2]);
		//dc.DrawText(mouseText,dx,dy);
	}
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
	AssStyle *style = VideoContext::Get()->grid->ass->GetStyle(diag->Style);
	if (style) {
		align = style->alignment;
		for (int i=0;i<4;i++) {
			if (margin[i] == 0) margin[i] = style->Margin[i];
		}
	}

	// Script size
	int sw,sh;
	VideoContext::Get()->GetScriptSize(sw,sh);

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


////////////////////
// Get line's scale
void VideoDisplayVisual::GetLineScale(AssDialogue *diag,float &scalX,float &scalY) {
	// Default values
	scalX = scalY = 100.0f;

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
			if (tag->Name == _T("\\fscx") && tag->Params.size() == 1) {
				scalX = tag->Params[0]->AsFloat();
			}
			if (tag->Name == _T("\\fscy") && tag->Params.size() == 1) {
				scalY = tag->Params[0]->AsFloat();
			}
		}
	}
	diag->ClearBlocks();
}


///////////////////
// Get line's clip
void VideoDisplayVisual::GetLineClip(AssDialogue *diag,int &x1,int &y1,int &x2,int &y2) {
	// Default values
	x1 = y1 = 0;
	int sw,sh;
	VideoContext::Get()->GetScriptSize(sw,sh);
	x2 = sw-1;
	y2 = sh-1;

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
			if (tag->Name == _T("\\clip") && tag->Params.size() == 4) {
				x1 = tag->Params[0]->AsInt();
				y1 = tag->Params[1]->AsInt();
				x2 = tag->Params[2]->AsInt();
				y2 = tag->Params[3]->AsInt();
			}
		}
	}
	diag->ClearBlocks();
}


///////////////
// Mouse event
void VideoDisplayVisual::OnMouseEvent (wxMouseEvent &event) {
	// Coords
	int x = event.GetX();
	int y = event.GetY();
	parent->ConvertMouseCoords(x,y);
	int w,h;
	parent->GetClientSize(&w,&h);
	int orgx = -1;
	int orgy = -1;
	int sw,sh;
	VideoContext::Get()->GetScriptSize(sw,sh);
	int frame_n = VideoContext::Get()->GetFrameN();
	SubtitlesGrid *grid = VideoContext::Get()->grid;
	bool hasOverlay = false;
	bool realTime = Options.AsBool(_T("Video Visual Realtime"));
	parent->tracker->OnMouseEvent(event);

	// Text of current coords
	int vx = (sw * x + w/2) / w;
	int vy = (sh * y + h/2) / h;
	if (!event.ShiftDown()) mouseText = wxString::Format(_T("%i,%i"),vx,vy);
	else mouseText = wxString::Format(_T("%i,%i"),vx - sw,vy - sh);

	// Start dragging
	if (mode != 0 && event.LeftIsDown() && !holding) {
		float rx,ry,rz,scalX,scalY;
		AssDialogue *gotDiag = NULL;

		// Drag
		if (mode == 1) {
			// For each line
			int numRows = VideoContext::Get()->grid->GetRows();
			int startMs = VFR_Output.GetTimeAtFrame(frame_n,true);
			int endMs = VFR_Output.GetTimeAtFrame(frame_n,false);
			AssDialogue *diag;

			// Don't uninvert this loop or selection will break
			for (int i=numRows;--i>=0;) {
				diag = VideoContext::Get()->grid->GetDialogue(i);
				if (diag) {
					// Line visible?
					int f1 = VFR_Output.GetFrameAtTime(diag->Start.GetMS(),true);
					int f2 = VFR_Output.GetFrameAtTime(diag->End.GetMS(),false);
					if (f1 <= frame_n && f2 >= frame_n) {
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
							VideoContext::Get()->grid->editBox->SetToLine(i,true);
							gotDiag = diag;
							origX = lineX;
							origY = lineY;
							orgx = torgx;
							orgy = torgy;
							break;
						}
					}
				}
			}
		}

		// Pick active line
		else {
			// Get active
			gotDiag = VideoContext::Get()->grid->GetDialogue(VideoContext::Get()->grid->editBox->linen);

			// Check if it's within range
			if (gotDiag) {
				int f1 = VFR_Output.GetFrameAtTime(gotDiag->Start.GetMS(),true);
				int f2 = VFR_Output.GetFrameAtTime(gotDiag->End.GetMS(),false);

				// Invisible
				if (f1 > frame_n || f2 < frame_n) {
					gotDiag = NULL;
				}

				// OK
				else {
					GetLinePosition(gotDiag,origX,origY,orgx,orgy);
				}
			}
		}

		// Got a line?
		if (gotDiag) {
			// Set dialogue
			curSelection = gotDiag;
			lineOrgX = orgx;
			lineOrgY = orgy;

			// Set coordinates
			if (mode == 1) {
				startX = x;
				startY = y;
				curSelection->StripTag(_T("\\pos"));
				curSelection->StripTag(_T("\\move"));
			}

			// Rotate Z
			if (mode == 2) {
				startAngle = atan2(double(lineOrgY-y*sh/h),double(x*sw/w-lineOrgX)) * 180.0 / 3.1415926535897932;
				GetLineRotation(curSelection,rx,ry,rz);
				origAngle = rz;
				curAngle = rz;
				curSelection->StripTag(_T("\\frz"));
				curSelection->StripTag(_T("\\fr"));
			}

			// Rotate XY
			if (mode == 3) {
				startAngle = (lineOrgY-y*sh/h)*2.0;
				startAngle2 = (x*sw/w-lineOrgX)*2.0;
				GetLineRotation(curSelection,rx,ry,rz);
				origAngle = rx;
				origAngle2 = ry;
				curAngle = rx;
				curAngle2 = ry;
				curSelection->StripTag(_T("\\frx"));
				curSelection->StripTag(_T("\\fry"));
			}

			// Scale
			if (mode == 4) {
				//startScaleX = x;
				//startScaleY = y;
				startX = x;
				startY = y;
				GetLineScale(curSelection,scalX,scalY);
				origScaleX = scalX;
				origScaleY = scalY;
				curScaleX = scalX;
				curScaleY = scalY;
				curSelection->StripTag(_T("\\fscx"));
				curSelection->StripTag(_T("\\fscy"));
			}

			// Clip
			if (mode == 5) {
				startX = x;
				startY = y;
				curSelection->StripTag(_T("\\clip"));
			}

			// Commit changes to edit box
			grid->editBox->TextEdit->SetTextTo(curSelection->Text);
			grid->editBox->CommitText(true);

			// Hold it
			holding = true;
			hold = mode;
			parent->CaptureMouse();
			hasOverlay = true;
		}
	}

	// Drag
	if (hold == 1) {
		curX = (x - startX + origX) * sw / w;
		curY = (y - startY + origY) * sh / h;
		if (realTime) {
			AssLimitToVisibleFilter::SetFrame(frame_n);
			grid->editBox->SetOverride(_T("\\pos"),wxString::Format(_T("(%i,%i)"),curX,curY),0,false);
			grid->editBox->CommitText(true);
			grid->CommitChanges(false,true);
		}
	}

	// Rotate Z
	else if (hold == 2) {
		// Find angle
		float screenAngle = atan2(double(lineOrgY-y*sh/h),double(x*sw/w-lineOrgX)) * 180.0 / 3.1415926535897932;
		curAngle = screenAngle - startAngle + origAngle;
		while (curAngle < 0.0f) curAngle += 360.0f;
		while (curAngle >= 360.0f) curAngle -= 360.0f;

		// Snap
		if (event.ShiftDown()) {
			curAngle = (float)((int)((curAngle+15.0f)/30.0f))*30.0f;
			if (curAngle == 360.0f) curAngle = 0.0f;
		}

		// Update
		if (realTime) {
			AssLimitToVisibleFilter::SetFrame(frame_n);
			wxString param = PrettyFloat(wxString::Format(_T("(%0.3f)"),curAngle));
			grid->editBox->SetOverride(_T("\\frz"),param,0,false);
			grid->editBox->CommitText(true);
			grid->CommitChanges(false,true);
		}
	}

	// Rotate XY
	else if (hold == 3) {
		// Find screen angles
		float screenAngle = (lineOrgY-y*sh/h)*2.0;
		float screenAngle2 = (x*sw/w-lineOrgX)*2.0;

		// Calculate
		curAngle = screenAngle - startAngle + origAngle;
		curAngle2 = screenAngle2 - startAngle2 + origAngle2;
		while (curAngle < 0.0) curAngle += 360.0;
		while (curAngle >= 360.0) curAngle -= 360.0;
		while (curAngle2 < 0.0) curAngle2 += 360.0;
		while (curAngle2 >= 360.0) curAngle2 -= 360.0;

		// Oh Snap
		if (event.ShiftDown()) {
			curAngle = (float)((int)((curAngle+15.0f)/30.0f))*30.0f;
			curAngle2 = (float)((int)((curAngle2+15.0f)/30.0f))*30.0f;
			if (curAngle == 360.0f) curAngle = 0.0f;
			if (curAngle2 == 360.0f) curAngle = 0.0f;
		}

		// Update
		if (realTime) {
			AssLimitToVisibleFilter::SetFrame(frame_n);
			grid->editBox->SetOverride(_T("\\frx"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curAngle)),0,false);
			grid->editBox->SetOverride(_T("\\fry"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curAngle2)),0,false);
			grid->editBox->CommitText(true);
			grid->CommitChanges(false,true);
		}
	}

	// Scale
	else if (hold == 4) {
		// Calculate
		curScaleX = (float(x - startX)/0.8f) + origScaleX;
		curScaleY = (float(startY - y)/0.8f) + origScaleY;
		if (curScaleX < 0.0f) curScaleX = 0.0f;
		if (curScaleY < 0.0f) curScaleY = 0.0f;

		// Snap
		if (event.ShiftDown()) {
			curScaleX = (float)((int)((curScaleX+12.5f)/25.0f))*25.0f;
			curScaleY = (float)((int)((curScaleY+12.5f)/25.0f))*25.0f;
		}

		// Update
		if (realTime) {
			AssLimitToVisibleFilter::SetFrame(frame_n);
			grid->editBox->SetOverride(_T("\\fscx"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curScaleX)),0,false);
			grid->editBox->SetOverride(_T("\\fscy"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curScaleY)),0,false);
			grid->editBox->CommitText(true);
			grid->CommitChanges(false,true);
		}
	}

	// Clip
	else if (hold == 5) {
		// Coordinates
		curX = startX * sw / w;
		curY = startY * sh / h;
		curX2 = x * sw / w;
		curY2 = y * sh / h;
		int temp;
		if (curX > curX2) {
			temp = curX;
			curX = curX2;
			curX2 = temp;
		}
		if (curY > curY2) {
			temp = curY;
			curY = curY2;
			curY2 = temp;
		}

		// Update
		if (realTime) {
			AssLimitToVisibleFilter::SetFrame(frame_n);
			grid->editBox->SetOverride(_T("\\clip"),wxString::Format(_T("(%i,%i,%i,%i)"),curX,curY,curX2,curY2),0,false);
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
			grid->editBox->SetOverride(_T("\\pos"),wxString::Format(_T("(%i,%i)"),curX,curY),0,false);
		}

		// Finished rotating Z
		else if (hold == 2) {
			grid->editBox->SetOverride(_T("\\frz"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curAngle)),0,false);
		}

		// Finished rotating XY
		else if (hold == 3) {
			grid->editBox->SetOverride(_T("\\frx"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curAngle)),0,false);
			grid->editBox->SetOverride(_T("\\fry"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curAngle2)),0,false);
		}

		// Finished scaling
		else if (hold == 4) {
			grid->editBox->SetOverride(_T("\\fscx"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curScaleX)),0,false);
			grid->editBox->SetOverride(_T("\\fscy"),PrettyFloat(wxString::Format(_T("(%0.3f)"),curScaleY)),0,false);
		}

		// Finished clipping
		else if (hold == 5) {
			grid->editBox->SetOverride(_T("\\clip"),wxString::Format(_T("(%i,%i,%i,%i)"),curX,curY,curX2,curY2),0,false);
		}

		// Commit
		grid->editBox->CommitText();
		grid->ass->FlagAsModified();
		grid->CommitChanges(false,true);

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
		grid->editBox->SetOverride(_T("\\pos"),wxString::Format(_T("(%i,%i)"),vx,vy),0,false);
		grid->editBox->CommitText();
		grid->ass->FlagAsModified();
		grid->CommitChanges(false,true);
		parent->SetFocus();
	}

	// Hover
	if (x != mouseX || y != mouseY) {
		// Set coords
		mouseX = x;
		mouseY = y;
		hasOverlay = true;
	}

	// Has something to draw
	if (hasOverlay) {
		//DrawOverlay();
		parent->Render();
	}
}


/////////////
// Key event
void VideoDisplayVisual::OnKeyEvent(wxKeyEvent &event) {
	if (event.GetKeyCode() == 'A') SetMode(0);
	if (event.GetKeyCode() == 'S') SetMode(1);
	if (event.GetKeyCode() == 'D') SetMode(2);
	if (event.GetKeyCode() == 'F') SetMode(3);
	if (event.GetKeyCode() == 'G') SetMode(4);
	if (event.GetKeyCode() == 'H') SetMode(5);
}

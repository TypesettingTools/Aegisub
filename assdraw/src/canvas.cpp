/*
* Copyright (c) 2007, ai-chan
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the ASSDraw3 Team nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY AI-CHAN ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL AI-CHAN BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
///////////////////////////////////////////////////////////////////////////////
// Name:        canvas.cpp
// Purpose:     implementations of ASSDraw main canvas class
// Author:      ai-chan
// Created:     08/26/06
// Copyright:   (c) ai-chan
// Licence:     3-clause BSD
///////////////////////////////////////////////////////////////////////////////

#include "assdraw.hpp"
#include "cmd.hpp"
#include "agg_gsv_text.h"
#include "agg_ellipse.h"
#include "agg_conv_dash.h"
#include "agg_trans_bilinear.h"
#include "agg_trans_perspective.h"

#include "agghelper.hpp"
#include <math.h>
#include <wx/image.h>
#include <wx/filename.h>

// ----------------------------------------------------------------------------
// the main drawing canvas: ASSDrawCanvas
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(ASSDrawCanvas, ASSDrawEngine)
    EVT_MOTION (ASSDrawCanvas::OnMouseMove)
    EVT_LEFT_UP(ASSDrawCanvas::OnMouseLeftUp)
    EVT_LEFT_DOWN(ASSDrawCanvas::OnMouseLeftDown)
    EVT_RIGHT_UP(ASSDrawCanvas::OnMouseRightUp)
    EVT_RIGHT_DOWN(ASSDrawCanvas::OnMouseRightDown)
    EVT_RIGHT_DCLICK(ASSDrawCanvas::OnMouseRightDClick)
	EVT_MOUSEWHEEL(ASSDrawCanvas::OnMouseWheel)
    EVT_KEY_DOWN(ASSDrawCanvas::CustomOnKeyDown)
    EVT_KEY_UP(ASSDrawCanvas::CustomOnKeyUp)
	EVT_MENU(MENU_DRC_LNTOBEZ, ASSDrawCanvas::OnSelect_ConvertLineToBezier)
	EVT_MENU(MENU_DRC_BEZTOLN, ASSDrawCanvas::OnSelect_ConvertBezierToLine)
	EVT_MENU(MENU_DRC_C1CONTBEZ, ASSDrawCanvas::OnSelect_C1ContinuityBezier)
	EVT_MENU(MENU_DRC_MOVE00, ASSDrawCanvas::OnSelect_Move00Here)
	EVT_MOUSE_CAPTURE_LOST(ASSDrawCanvas::CustomOnMouseCaptureLost)
END_EVENT_TABLE()

ASSDrawCanvas::ASSDrawCanvas(wxWindow *parent, ASSDrawFrame *frame, int extraflags)
 : ASSDrawEngine( parent, extraflags )
{
	m_frame = frame;
	preview_mode = false;
    lastDrag_left = NULL;
    lastDrag_right = NULL;
    dragAnchor_left = NULL;
    dragAnchor_right = NULL;
    newcommand = NULL;
    mousedownAt_point = NULL;
    pointedAt_point = NULL;
    draw_mode = MODE_ARR;

    //drag_mode = DRAG_DWG;
    dragOrigin = false;
	hilite_cmd = NULL;
	hilite_point = NULL;
	capturemouse_left = false;
	capturemouse_right = false;
	//was_preview_mode = false;
	bgimg.bgbmp = NULL;
	bgimg.bgimg = NULL;
	bgimg.alpha = 0.5;
	rectbound2upd = -1, rectbound2upd2 = -1;
	
	rgba_shape_normal = agg::rgba(0,0,1,0.5);
	rgba_outline = agg::rgba(0,0,0);
	rgba_guideline = agg::rgba(0.5,0.5,0.5);
	rgba_mainpoint = agg::rgba(1,0,0,0.75);
	rgba_controlpoint = agg::rgba(0,1,0,0.75);
	rgba_selectpoint = agg::rgba(0,0,1,0.75);
	rgba_origin = agg::rgba(0,0,0);
	rgba_ruler_h = agg::rgba(0,0,1);
	rgba_ruler_v = agg::rgba(1,0,0);

	wxFlexGridSizer* sizer = new wxFlexGridSizer(1, 1);
    sizer->AddGrowableRow(0);
    sizer->AddGrowableCol(0);
    sizer->Add( this, 0, wxGROW|wxGROW, 5);
    parent->SetSizer(sizer);

	// for background image loading
	::wxInitAllImageHandlers();
	bgimg.bgbmp = NULL;
	bgimg.bgimg = NULL;
	// drag image background file
	SetDropTarget(new ASSDrawFileDropTarget(this));
	
	hasStatusBar = m_frame->GetStatusBar() != NULL;

	// cursor = crosshair
	SetCursor( *wxCROSS_CURSOR );

	bgimg.alpha_dlg = new wxDialog(this, wxID_ANY, wxString(_T("Background image opacity")));
	bgimg.alpha_slider = new wxSlider(bgimg.alpha_dlg, TB_BGALPHA_SLIDER, 50, 0, 100, __DPDS__ , wxSL_LABELS);
	bgimg.alpha_slider->SetSize(280, bgimg.alpha_slider->GetSize().y);
	bgimg.alpha_dlg->Fit();
	bgimg.alpha_dlg->Show(false);
	bgimg.alpha_slider->Connect(wxEVT_SCROLL_LINEUP, wxScrollEventHandler(ASSDrawCanvas::OnAlphaSliderChanged), NULL, this);
	bgimg.alpha_slider->Connect(wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler(ASSDrawCanvas::OnAlphaSliderChanged), NULL, this);
	bgimg.alpha_slider->Connect(wxEVT_SCROLL_PAGEUP, wxScrollEventHandler(ASSDrawCanvas::OnAlphaSliderChanged), NULL, this);
	bgimg.alpha_slider->Connect(wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler(ASSDrawCanvas::OnAlphaSliderChanged), NULL, this);
	bgimg.alpha_slider->Connect(wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(ASSDrawCanvas::OnAlphaSliderChanged), NULL, this);
	bgimg.alpha_slider->Connect(wxEVT_SCROLL_CHANGED, wxScrollEventHandler(ASSDrawCanvas::OnAlphaSliderChanged), NULL, this);
	
	RefreshUndocmds();

}

// Destructor
ASSDrawCanvas::~ASSDrawCanvas()
{
	ASSDrawEngine::ResetEngine(false);
	if (pointsys) delete pointsys;
	if (bgimg.bgbmp) delete bgimg.bgbmp;
	if (bgimg.bgimg) delete bgimg.bgimg;
}

void ASSDrawCanvas::ParseASS(wxString str, bool addundo)
{
	if (addundo)
		AddUndo(_T("Modify drawing commands"));

	ASSDrawEngine::ParseASS(str);

	RefreshUndocmds();
}

void ASSDrawCanvas::ResetEngine(bool addM)
{
	ClearPointsSelection();
	SetHighlighted(NULL, NULL);
	SetPreviewMode(false);
	SetDrawMode(MODE_ARR);
	ASSDrawEngine::ResetEngine(addM);
	RefreshUndocmds();
}

void ASSDrawCanvas::SetPreviewMode(bool mode)
{
	//was_preview_mode = mode;
	preview_mode = mode;
	if (preview_mode)
	{
		if (mousedownAt_point != NULL)
		{
			mousedownAt_point->cmd_main->Init();
			mousedownAt_point = NULL;
		}

		if (pointedAt_point != NULL)
			pointedAt_point = NULL;

		SetHighlighted( NULL, NULL );

		RefreshDisplay();
	}
}

// (Re)draw canvas
void ASSDrawCanvas::RefreshDisplay() 
{ 
	ASSDrawEngine::RefreshDisplay();
	wxString asscmds = GenerateASS();
	if (oldasscmds != asscmds)
	{
		m_frame->UpdateASSCommandStringToSrcTxtCtrl(asscmds);
		oldasscmds = asscmds;
	}
}

void ASSDrawCanvas::SetDrawMode( MODE mode ) 
{ 
	draw_mode = mode; 

	if (!selected_points.empty())
		ClearPointsSelection();

	RefreshDisplay();

	if (IsTransformMode())
	{
		isshapetransformable = cmds.size() > 1;
		
		if (isshapetransformable)
		{

			// backup cmds
			backupcmds.free_all();
			for (DrawCmdList::iterator iterate = cmds.begin(); iterate != cmds.end(); iterate++)
			{
				DrawCmd* cmd = (*iterate);
				for (PointList::iterator iterate2 = cmd->controlpoints.begin(); iterate2 != cmd->controlpoints.end(); iterate2++)
				{
					wxPoint pp = (*iterate2)->ToWxPoint();
					backupcmds.move_to(pp.x, pp.y);
				}
				wxPoint pp = (*iterate)->m_point->ToWxPoint();
				backupcmds.move_to(pp.x, pp.y);			
			}
	
			// calculate bounding rectangle
			agg::trans_affine mtx;
			trans_path *rm, *rb;
			agg::conv_curve<trans_path> *rc;
			ConstructPathsAndCurves(mtx, rm, rb, rc);
			rasterizer.reset();
			rasterizer.add_path(*rc);
			delete rm, rb, rc;
		    int minx = rasterizer.min_x(), miny = rasterizer.min_y();
		    int maxx = rasterizer.max_x(), maxy = rasterizer.max_y();
		    
		    rectbound[0] = wxRealPoint(minx, miny);
		    rectbound[1] = wxRealPoint(maxx, miny);
		    rectbound[2] = wxRealPoint(maxx, maxy);
		    rectbound[3] = wxRealPoint(minx, maxy);
		    for (int i = 0; i < 4; i++)
		    	rectbound2[i] = rectbound[i];
	
		    rectbound2upd = -1;
		    rectbound2upd2 = -1;
		    
		    backupowner = NONE;
		    InitiateDraggingIfTransformMode();
		    
		    if (maxx - minx < 5 || maxy - miny < 5)
		    	isshapetransformable = false;
		}	    
	}
	
	RefreshUndocmds();
	m_frame->UpdateFrameUI();
	
}

void ASSDrawCanvas::SetDragMode( DRAGMODE mode )
{
	drag_mode = mode;
}

bool ASSDrawCanvas::IsTransformMode()
{ 
	return draw_mode == MODE_NUT_BILINEAR || draw_mode == MODE_SCALEROTATE; 
}

bool ASSDrawCanvas::CanZoom()
{
	return !IsTransformMode() || !drag_mode.drawing;
}

bool ASSDrawCanvas::CanMove()
{
	return !IsTransformMode() || dragAnchor_left == NULL;
}

// Do the dragging
void ASSDrawCanvas::OnMouseMove(wxMouseEvent &event)
{
	mouse_point = event.GetPosition();
	int xx, yy, wx, wy;
	xx = mouse_point.x; yy = mouse_point.y;
	pointsys->FromWxPoint ( mouse_point, wx, wy );
	if (event.Dragging())
	{
		if (IsTransformMode() && isshapetransformable && backupowner == LEFT)
		{
			// update bounding polygon
			if (rectbound2upd > -1)
			{
				if (!dragAnchor_left) dragAnchor_left = new wxPoint(mouse_point);
				wxPoint diff = mouse_point - *dragAnchor_left;
				wxRealPoint rdiff(diff.x, diff.y);
				switch(draw_mode)
				{
				case MODE_NUT_BILINEAR:
					if (rectbound2upd2 == -1) //only one vertex dragged
						rectbound2[rectbound2upd].x = xx, rectbound2[rectbound2upd].y = yy;
					else
					{
						rectbound2[rectbound2upd].x += diff.x, rectbound2[rectbound2upd].y += diff.y;
						rectbound2[rectbound2upd2].x += diff.x, rectbound2[rectbound2upd2].y += diff.y;
					}
					undodesc = _T("Bilinear transform");
					*dragAnchor_left = mouse_point;
					break;
				case MODE_SCALEROTATE:
					if (rectbound2upd2 == -1) //only one vertex dragged
					{
						int adjacent[2] = { (rectbound2upd + 3) % 4, (rectbound2upd + 1) % 4 };
						int opposite = (rectbound2upd + 2) % 4; 
						wxRealPoint newpoint = backup[rectbound2upd] + rdiff;
						//wxPoint newadjacent[2];
						double nx, ny;
						for (int i = 0; i < 2; i++)
						{
							bool isect = agg::calc_intersection(
								backup[opposite].x, backup[opposite].y, 
								backup[adjacent[i]].x, backup[adjacent[i]].y,
								newpoint.x, newpoint.y,
								backup[adjacent[i]].x + diff.x, backup[adjacent[i]].y + diff.y,
								&nx, &ny);
							if (isect && !(fabs(nx - backup[opposite].x) < 10 && fabs(ny - backup[opposite].y) < 10))
								rectbound2[adjacent[i]] = wxRealPoint(nx, ny);
						}
						GetThe4thPoint(backup[opposite].x, backup[opposite].y, 
							rectbound2[adjacent[0]].x, rectbound2[adjacent[0]].y,
							rectbound2[adjacent[1]].x, rectbound2[adjacent[1]].y, 
							&rectbound2[rectbound2upd].x, &rectbound2[rectbound2upd].y);
						if (event.ShiftDown()) // shift down = maintain aspect ratio (damn so complicated)
						{
							// first create the guide points, which are basically backup points reoriented based on mouse position
							wxRealPoint guide[4];
							guide[opposite] = backup[opposite];
							guide[adjacent[0]] = backup[adjacent[0]];
							guide[adjacent[1]] = backup[adjacent[1]];
							for (int i = 0; i < 2; i++)
							{
								if ((rectbound2[adjacent[i]].x < guide[opposite].x && guide[opposite].x < guide[adjacent[i]].x)
									|| (rectbound2[adjacent[i]].x > guide[opposite].x && guide[opposite].x > guide[adjacent[i]].x)
									|| (rectbound2[adjacent[i]].y < guide[opposite].y && guide[opposite].y < guide[adjacent[i]].y)
									|| (rectbound2[adjacent[i]].y > guide[opposite].y && guide[opposite].y > guide[adjacent[i]].y))
								{
									guide[adjacent[i]] = guide[opposite] - (guide[adjacent[i]] - guide[opposite]);
								}
							}
							guide[rectbound2upd] = guide[adjacent[0]] + (guide[adjacent[1]] - guide[opposite]);
							// now we determine which rescaled sides have larger enlargement/shrinkage ratio ..
							double ix[2], iy[2], dx[2], dy[2];
							for (int i = 0; i < 2; i++)
							{
								agg::calc_intersection(guide[opposite].x, guide[opposite].y, guide[rectbound2upd].x, guide[rectbound2upd].y,
									rectbound2[rectbound2upd].x, rectbound2[rectbound2upd].y,
									rectbound2[adjacent[i]].x, rectbound2[adjacent[i]].y, &ix[i], &iy[i]);
								dx[i] = ix[i] - guide[opposite].x;
								dy[i] = iy[i] - guide[opposite].y;
							}
							// .. and modify the other sides to follow the ratio
							for (int i = 0; i < 2; i++)
							{
								int j = (i + 1) % 2;
								if (fabs(dx[i]) > fabs(dx[j]) || fabs(dy[i]) > fabs(dy[j]))
								{
									rectbound2[rectbound2upd].x = ix[i];
									rectbound2[rectbound2upd].y = iy[i];
									GetThe4thPoint(rectbound2[adjacent[i]].x, rectbound2[adjacent[i]].y,
										guide[opposite].x, guide[opposite].y, ix[i], iy[i],
										&rectbound2[adjacent[j]].x, &rectbound2[adjacent[j]].y);
								}
							}
						}
					}
					else // an edge dragged (i.e. move 2 vertices in sync)
					{
						// it is guaranteed that rectbound2upd and rectbound2upd2 are in clockwise direction
						// from the way they are detected in OnMouseLeftDown()
						int toupd[2] = { rectbound2upd, rectbound2upd2 };
						int adjacent[2] = { (rectbound2upd2 + 2) % 4, (rectbound2upd2 + 1) % 4 };
						wxRealPoint vertexone = backup[toupd[0]] + rdiff, vertextwo = backup[toupd[1]] + rdiff;
						double nx, ny;
						for (int i = 0; i < 2; i++)
						{
							agg::calc_intersection(
								rectbound2[adjacent[i]].x, rectbound2[adjacent[i]].y,
								backup[toupd[i]].x, backup[toupd[i]].y,
								vertexone.x, vertexone.y,
								vertextwo.x, vertextwo.y,
								&nx, &ny);
							if (!(fabs(nx - backup[adjacent[i]].x) < 10 && fabs(ny - backup[adjacent[i]].y) < 10))
								rectbound2[toupd[i]].x = (int) nx, rectbound2[toupd[i]].y = (int) ny;
						}
					}
					UpdateTranformModeRectCenter();
					//*dragAnchor_left = mouse_point;
					undodesc = _T("Scale");
					break;
				}
			
				// update points
				UpdateNonUniformTransformation();
				RefreshDisplay();
			}
		}
		else if (draw_mode != MODE_DEL)
		{

			// point left-dragged
			if (mousedownAt_point != NULL && mousedownAt_point->isselected) 
			{
				if (!mousedownAt_point->IsAt( wx, wy ))
				{
					if (draw_mode == MODE_ARR) {
						int movex = wx - mousedownAt_point->x(), movey = wy - mousedownAt_point->y();
						PointSet::iterator iter = selected_points.begin();
						for (; iter != selected_points.end(); iter++)			
							(*iter)->setXY( (*iter)->x() + movex, (*iter)->y() + movey );
					}
					else
						mousedownAt_point->setXY( wx, wy );
	
					EnforceC1Continuity (mousedownAt_point->cmd_main, mousedownAt_point);
											
					RefreshDisplay();
					if (undodesc == _T(""))
					{
						if (mousedownAt_point->type == MP)
							undodesc = _T("Drag point");
						else
							undodesc = _T("Drag control point");
					}
				}
			} 
			// origin left-dragged
			else if (dragOrigin) 
			{
				if (wx != 0 || wy != 0)
				{
					wxPoint wxp = pointsys->ToWxPoint ( wx, wy );
					MovePoints(-wx, -wy);
					pointsys->originx = wxp.x;
					pointsys->originy = wxp.y;
					RefreshDisplay();
					undodesc = _T("Move origin");
				}          
			}
			else if (dragAnchor_left != NULL)
			{
				// handle left-dragging here
				if (lastDrag_left && dragAnchor_left != lastDrag_left)
			       delete lastDrag_left;
				lastDrag_left = new wxPoint(xx, yy);
				int ax = dragAnchor_left->x, ay = dragAnchor_left->y;
				int sx = lastDrag_left->x, sy = lastDrag_left->y;
				int lx, rx, ty, by;
				if (ax > sx) lx = sx, rx = ax;
				else lx = ax, rx = sx;
				if (ay > sy) ty = sy, by = ay;
				else ty = ay, by = sy;
				SelectPointsWithin( lx, rx, ty, by, GetSelectMode(event) );
				RefreshDisplay();
			}
		}

		// right-dragged
		if (dragAnchor_right != NULL)
		{
			if (draw_mode == MODE_SCALEROTATE)
			{
				if (backupowner == RIGHT)
				{
					double cx = rectcenter.x, cy = rectcenter.y; // center x,y
					double diameter = sqrt(pow(rectbound2[0].x - rectbound2[2].x, 2) + pow(rectbound2[0].y - rectbound2[2].y, 2));
					double radius = diameter / 2;
					double old_dx = dragAnchor_right->x - cx, old_dy = dragAnchor_right->y - cy;
					double new_dx = mouse_point.x - cx, new_dy = mouse_point.y - cy;
					double old_angle = atan2(old_dy, old_dx);
					double new_angle = atan2(new_dy, new_dx);
					double delta = new_angle - old_angle;
					for (int i = 0; i < 4; i++)
					{
						old_dx = backup[i].x - cx, old_dy = backup[i].y - cy;
						old_angle = atan2(old_dy, old_dx);
						new_angle = old_angle + delta;
						new_dx = radius * cos(new_angle), new_dy = radius * sin(new_angle);
						rectbound2[i].x = (int)(new_dx + cx), rectbound2[i].y = (int)(new_dy + cy);
					}
					UpdateNonUniformTransformation();
					RefreshDisplay();
					undodesc = _T("Rotate");
				}
			}
			else if (CanMove()) 
			{
				MoveCanvas(xx - dragAnchor_right->x, yy - dragAnchor_right->y);
				dragAnchor_right->x = xx;
				dragAnchor_right->y = yy;
				RefreshDisplay();
			}
		}		
	}    
	else if (!preview_mode)// not dragging and not preview mode
    {
		if (IsTransformMode())
		{
			int oldrectbound = rectbound2upd;
			rectbound2upd = -1;
			rectbound2upd2 = -1;
			for (int i = 0; i < 4; i++)
			{
				if (abs((int)rectbound2[i].x - mouse_point.x) <= pointsys->scale
					&& abs((int)rectbound2[i].y - mouse_point.y) <= pointsys->scale)
						rectbound2upd = i;
			}
			for (int i = 0; rectbound2upd == -1 && i < 4; i++)
			{
				int j = (i+1) % 4;
				wxRealPoint &pi = rectbound2[i], &pj = rectbound2[j];
				double dy = fabs(pj.y - pi.y);
				double dy3 = dy / 3;
				double dx = fabs(pj.x - pi.x);
				double dx3 = dx / 3;
				double ix, iy;
				bool intersect = false;
				if (dy > dx)
				{
					intersect = agg::calc_intersection(
						pi.x, pi.y, pj.x, pj.y,
	                    mouse_point.x - pointsys->scale, mouse_point.y,
						mouse_point.x + pointsys->scale, mouse_point.y, &ix, &iy);
					intersect &= fabs(mouse_point.x - ix) <= pointsys->scale;
					intersect &= (pj.y > pi.y? 
						pj.y - dy3 > iy && iy > pi.y + dy3:
						pj.y + dy3 < iy && iy < pi.y - dy3);
				}
				else
				{
					intersect = agg::calc_intersection(
						pi.x, pi.y, pj.x, pj.y,
	                    mouse_point.x, mouse_point.y - pointsys->scale,
						mouse_point.x, mouse_point.y + pointsys->scale, &ix, &iy);
					intersect &= fabs(mouse_point.y - iy) <= pointsys->scale;
					intersect &= (pj.x > pi.x? 
						pj.x - dx3 > ix && ix > pi.x + dx3:
						pj.x + dx3 < ix && ix < pi.x - dx3);
				}
				if (intersect)
				{
					rectbound2upd = i;
					rectbound2upd2 = j;
				}
			}
			if (rectbound2upd != -1 || oldrectbound != -1)
				RefreshDisplay();
		}
		else
		{
	         /* figure out if the mouse is pointing at a point of a command
	            we need not do this for dragging since this same block has set
	            the related variables before the user starts to hold the button
	            (well, before you can drag something you have to move the pointer
	            over that thing first, right?) and we don't want to mess with those 
	            when it's dragging
	         */
	         
			// check if mouse points on any control point first
			Point* last_pointedAt_point = pointedAt_point;
			ControlAt( wx, wy, pointedAt_point );
	
			// then check if mouse points on any m_points
			// override any control point set to pointedAt_point above
			DrawCmd* p = PointAt( wx, wy );
			if (p != NULL)
			{
				pointedAt_point = p->m_point;
			}
	         
			if (pointedAt_point != last_pointedAt_point)
			{
				if (pointedAt_point != NULL)
					SetHighlighted( pointedAt_point->cmd_main, pointedAt_point );
				else
					SetHighlighted( NULL, NULL );
				RefreshDisplay();
			}
		}
	} // not dragging and preview mode = ignore all mouse movements

	// we are not done yet?
    // oh yeah, we need to set the status bar just for fun
    if (hasStatusBar)
	{
        m_frame->SetStatusText( 
            wxString::Format( _T("%5d %5d"), (int)wx, (int)wy ), 0 );
        if (pointedAt_point == NULL || 
             (newcommand != NULL && !newcommand->initialized) )
           m_frame->SetStatusText( _T(""), 1 );
        else
           m_frame->SetStatusText( _T(" ") + pointedAt_point->cmd_main->ToString().Upper(), 1 );
	}

}

// End drag points
void ASSDrawCanvas::OnMouseLeftUp(wxMouseEvent& event)
{
	ProcessOnMouseLeftUp();
	event.Skip( true );
}

void ASSDrawCanvas::ProcessOnMouseLeftUp()
{
	if (!capturemouse_left) return;
	
	// draw mode
	if (newcommand != NULL) 
	{
		newcommand->Init();
		switch (newcommand->type)
		{
		case M:
			undodesc = _T("Add a new M"); break;
		case L:
			undodesc = _T("Add a new L"); break;
		case B:
			undodesc = _T("Add a new B"); break;
		}
		newcommand = NULL;
		// we need to manually refresh the GUI to draw the new control points
		RefreshDisplay();
	}
	else if ( draw_mode == MODE_DEL // if it's delete mode
				&& mousedownAt_point != NULL // and mouse down at a point
				&& mousedownAt_point == pointedAt_point ) // and released at the same point
	{
		// first take care of mouse readings
		pointedAt_point = NULL;
		Point *lastmousedownAt_point = mousedownAt_point;
		mousedownAt_point = NULL;

		// try deleting
		CMDTYPE t = lastmousedownAt_point->cmd_main->type;
		if ( DeleteCommand( lastmousedownAt_point->cmd_main ) )
		{
			ClearPointsSelection();
			SetHighlighted( NULL, NULL );
			RefreshDisplay();
			switch (t)
			{
			case M:
				undodesc = _T("Delete a M"); break;
			case L:
				undodesc = _T("Delete a L"); break;
			case B:
				undodesc = _T("Delete a B"); break;
			}
		}
		else
		{
			RefreshDisplay(); // redraw before showing the error box
			wxMessageDialog msgb(m_frame, 
			_T("You must delete that command/point last"), 
			_T("Error"), wxOK | wxICON_EXCLAMATION );
			msgb.ShowModal();
		}
	}
	else if ( lastDrag_left && dragAnchor_left ) // point selection
	{
		if (lastDrag_left && dragAnchor_left != lastDrag_left)
		    delete lastDrag_left;
	    delete dragAnchor_left;			
		lastDrag_left = NULL;
		dragAnchor_left = NULL;
	}
	
	if (dragOrigin)
	{
		dragOrigin = false;
		RefreshDisplay(); // clear the crosshair
	}

	rectbound2upd = -1;
	rectbound2upd2 = -1;
    backupowner = NONE;

	if (!undodesc.IsSameAs(_T("")))
	{
		AddUndo( undodesc );
		undodesc = _T("");
		RefreshUndocmds();
	}

	mousedownAt_point = NULL;

	if (HasCapture())
		ReleaseMouse();
	capturemouse_left = false;
	
	RefreshDisplay();
}

// Set up to drag points, if any
void ASSDrawCanvas::OnMouseLeftDown(wxMouseEvent& event)
{

    // no drawing in preview mode
	if (preview_mode /*|| was_preview_mode*/)
		return;

	wxPoint q = event.GetPosition();

	// wxPoint to Point
	int px, py;
	pointsys->FromWxPoint(q, px, py);
	
	// create new cmd if in draw mode / or delete point if del tool selected
	switch (draw_mode)
	{
	case MODE_M:
		newcommand = NewCmd(M, px, py);
		break;
	case MODE_L:
		newcommand = NewCmd(L, px, py);
		break;
	case MODE_B:
		newcommand = NewCmd(B, px, py);
		break;
	}

	// continue working on the new command (if any)
	// only if it is not mouse down on any control point
	if (newcommand != NULL)
	{
		if (pointedAt_point != NULL && pointedAt_point->type == CP)
		{
			// oops, user clicked on a control point so cancel new command
			// and let him drag the control point
			delete newcommand;
            newcommand = NULL;
		}
		else 
		{
			// first set to drag the new command no matter what
			mousedownAt_point = newcommand->m_point;
	      
			// if user drags from existing point, insert the new command
			// else append the new command
			if (pointedAt_point != NULL)
				InsertCmd( newcommand, pointedAt_point->cmd_main );
			else
			{
				if (cmds.empty() && newcommand->type != M)
					AppendCmd( M, px, py );
				newcommand = AppendCmd( newcommand );
			}

			pointedAt_point = mousedownAt_point;
	           
			//highlight it
			SetHighlighted( newcommand, newcommand->m_point );
		}                    
	}

	// we already calculated pointedAt_point in OnMouseMove() so just use it
	mousedownAt_point = pointedAt_point;
	SELECTMODE smode = GetSelectMode(event);
	if (mousedownAt_point && !mousedownAt_point->isselected)
	{
		if (smode == NEW)
		{
			ClearPointsSelection();
			mousedownAt_point->isselected = true;
			selected_points.insert(mousedownAt_point);
		}
		else
		{
			wxPoint wxp = mousedownAt_point->ToWxPoint();
			SelectPointsWithin(wxp.x, wxp.x, wxp.y, wxp.y, smode);
		}
	}
	else if (!mousedownAt_point && smode == NEW)
		ClearPointsSelection();

	if ( mousedownAt_point == NULL && px == 0 && py == 0 )
		dragOrigin = true;

	if ( mousedownAt_point == NULL && !dragOrigin )
	{
		dragAnchor_left = new wxPoint(q.x, q.y);
		lastDrag_left = dragAnchor_left;
	}

	if (InitiateDraggingIfTransformMode())
	    backupowner = LEFT;
	
	CaptureMouse();
	capturemouse_left = true;
	RefreshDisplay();

	event.Skip( true );
}

// End drag the canvas
void ASSDrawCanvas::OnMouseRightUp(wxMouseEvent& event)
{
	ProcessOnMouseRightUp();
	event.Skip( true );
}

void ASSDrawCanvas::ProcessOnMouseRightUp()
{
	if (!capturemouse_right) return;

    if (lastDrag_right && dragAnchor_right != lastDrag_right)
        delete lastDrag_right;
    if (dragAnchor_right)
        delete dragAnchor_right;
	dragAnchor_right = NULL;
	lastDrag_right = NULL;

	// don't crash the program
	if (HasCapture())
		ReleaseMouse();
	capturemouse_right = false;

	rectbound2upd = -1;
	rectbound2upd2 = -1;
    backupowner = NONE;

	if (!undodesc.IsSameAs(_T("")))
	{
		AddUndo( undodesc );
		undodesc = _T("");
		RefreshUndocmds();
	}

	RefreshDisplay();
	SetFocus();
}

// Set up to drag the canvas
void ASSDrawCanvas::OnMouseRightDown(wxMouseEvent &event)
{
	wxPoint q = event.GetPosition();
	dragAnchor_right = new wxPoint(q.x, q.y);
	lastDrag_right = dragAnchor_right;
	CaptureMouse();
	capturemouse_right = true;

	if (InitiateDraggingIfTransformMode())
	    backupowner = RIGHT;
//	was_preview_mode = preview_mode;
//	preview_mode = false;

	event.Skip( true );
}

// Reset to point-and-drag mode
void ASSDrawCanvas::OnMouseRightDClick(wxMouseEvent& event)
{
	wxMenu* menu = new wxMenu();

	if (pointedAt_point)
	{
		ProcessOnMouseLeftUp();
		ProcessOnMouseRightUp();
		dblclicked_point_right = pointedAt_point;
		pointedAt_point = NULL;
		wxMenuItem *cmdmenuitem = new wxMenuItem(menu, MENU_DUMMY, dblclicked_point_right->cmd_main->ToString());
		wxFont f = cmdmenuitem->GetFont();
		f.SetWeight(wxFONTWEIGHT_BOLD);
		cmdmenuitem->SetFont(f);
		menu->Append(cmdmenuitem); 
		menu->Enable(MENU_DUMMY, false);
		switch (dblclicked_point_right->cmd_main->type)
		{
			case L:
				menu->Append(MENU_DRC_LNTOBEZ, _T("Convert to Bezier curve (B command)"));
				break;
			case B:
				if (dblclicked_point_right->type != MP) break;
				menu->AppendCheckItem(MENU_DRC_BEZTOLN, _T("Convert to line (L command)"));
				if (dblclicked_point_right->cmd_next && dblclicked_point_right->cmd_next->type == B)
				{
					menu->AppendCheckItem(MENU_DRC_C1CONTBEZ, _T("Smooth connection"));
					if (static_cast<DrawCmd_B*>(dblclicked_point_right->cmd_main)->C1Cont)
						menu->Check(MENU_DRC_C1CONTBEZ, true);
				}
				break;
		}

	}
	else
	{
		menu->Append(MENU_DRC_MOVE00, _T("Move [0,0] here"));
		menu->AppendSeparator();
		menu->AppendRadioItem(MODE_ARR, _T("Mode: D&rag"));
		menu->AppendRadioItem(MODE_M, _T("Mode: Draw &M"));
		menu->AppendRadioItem(MODE_L, _T("Mode: Draw &L"));
		menu->AppendRadioItem(MODE_B, _T("Mode: Draw &B"));
		menu->AppendRadioItem(MODE_DEL, _T("Mode: &Delete"));
		menu->Check(GetDrawMode(), true);
	}

	if (menu->GetMenuItemCount() > 0) // only if there is menu item
	{
		SetHighlighted(NULL, NULL);
		mousedownAt_point = NULL;
		RefreshDisplay();
		PopupMenu(menu);
	}
	delete menu;

	event.Skip( true );
}

bool ASSDrawCanvas::InitiateDraggingIfTransformMode()
{
	if (IsTransformMode() && isshapetransformable && backupowner == NONE)
	{
		for (int i = 0; i < 4; i++)
			backup[i] = rectbound2[i];
		UpdateTranformModeRectCenter();
		return true;
	}
	else
		return false;
}

void ASSDrawCanvas::UpdateTranformModeRectCenter()
{
	double cx, cy;
	if (agg::calc_intersection(rectbound2[0].x, rectbound2[0].y, rectbound2[2].x, rectbound2[2].y,
		rectbound2[1].x, rectbound2[1].y, rectbound2[3].x, rectbound2[3].y, &cx, &cy))
	{ 
		rectcenter = wxRealPoint(cx, cy); 
	}	
}

bool ASSDrawCanvas::GetThe4thPoint(double ox, double oy, double a1x, double a1y, double a2x, double a2y, double *x, double *y)
{
	/*
	return agg::calc_intersection(a1x, a1y,
		a2x + a1x - ox,
		a2y + a1y - oy,
		a2x, a2y,
		a1x + a2x - ox,
		a1y + a2y - oy,
		x, y);	//*/
	
	*x = a1x + a2x - ox;
	*y = a1y + a2y - oy;
	return true;
}

// Mousewheel
void ASSDrawCanvas::OnMouseWheel(wxMouseEvent &event)
{
	double amount = event.GetWheelRotation() / event.GetWheelDelta();
	if (event.ControlDown()) amount /= 10.0;
	bgimg.new_center = wxRealPoint(mouse_point.x, mouse_point.y);
	ChangeZoomLevel( amount, mouse_point );
}

void ASSDrawCanvas::ChangeZoomLevelTo( double zoom, wxPoint bgzoomctr )
{
	ChangeZoomLevel(zoom - pointsys->scale, bgzoomctr);
}

void ASSDrawCanvas::ChangeZoomLevel( double amount, wxPoint bgzoomctr )
{
	double old_scale = pointsys->scale;
	/*
	switch (drag_mode)
	{
	case DRAG_BGIMG:
	    ChangeBackgroundZoomLevel(bgimg.scale + amount / 10.0,  wxRealPoint(bgzoomctr.x, bgzoomctr.y));
		break;

	case DRAG_BOTH:
		ChangeDrawingZoomLevel( amount );
	    ChangeBackgroundZoomLevel(bgimg.scale * pointsys->scale / old_scale, 
			wxRealPoint(pointsys->originx, pointsys->originy));
		break;

	case DRAG_DWG:
		ChangeDrawingZoomLevel( amount );
		break;
	}
	*/
	
	if (CanZoom() && drag_mode.drawing)
		ChangeDrawingZoomLevel( amount );

	if (CanZoom() && drag_mode.bgimg)
		if (drag_mode.drawing)
		    ChangeBackgroundZoomLevel(bgimg.scale * pointsys->scale / old_scale, wxRealPoint(pointsys->originx, pointsys->originy));
		else
		    ChangeBackgroundZoomLevel(bgimg.scale + amount / 10.0,  wxRealPoint(bgzoomctr.x, bgzoomctr.y));
	
	RefreshDisplay();
}

void ASSDrawCanvas::ChangeDrawingZoomLevel( double scrollamount )
{
	if (!CanZoom()) return;
    double zoom = pointsys->scale + scrollamount;
    if (zoom <= 50.0)
	{
		if (zoom < 1.0) zoom = 1.0;
		pointsys->scale = zoom;
	}
	
	m_frame->UpdateFrameUI();
}

void ASSDrawCanvas::ChangeBackgroundZoomLevel(double zoom, wxRealPoint newcenter)
{
	bgimg.new_scale = zoom;
	bgimg.new_center = newcenter;
	if (bgimg.new_scale < 0.01) bgimg.new_scale = 0.01;
	UpdateBackgroundImgScalePosition();	
}

void ASSDrawCanvas::MoveCanvasOriginTo(double originx, double originy)
{
	MoveCanvas(originx - pointsys->originx, originy - pointsys->originy);
}

void ASSDrawCanvas::MoveCanvas(double xamount, double yamount)
{
	/*
	if (drag_mode == DRAG_DWG || drag_mode == DRAG_BOTH)
		MoveCanvasDrawing(xamount, yamount);
	if (drag_mode == DRAG_BGIMG || drag_mode == DRAG_BOTH)
		MoveCanvasBackground(xamount, yamount);
	*/
	if (CanMove() && drag_mode.drawing)
		MoveCanvasDrawing(xamount, yamount);
	if (CanMove() && drag_mode.bgimg)
		MoveCanvasBackground(xamount, yamount);
}

void ASSDrawCanvas::MoveCanvasDrawing(double xamount, double yamount)
{
	if (!CanMove()) return;
	pointsys->originx += xamount;
	pointsys->originy += yamount;		
	if (IsTransformMode())
	{
	    for (int i = 0; i < 4; i++)
	    {
	    	rectbound[i].x += (int) xamount;
	    	rectbound[i].y += (int) yamount;
	    	rectbound2[i].x += (int) xamount;
	    	rectbound2[i].y += (int) yamount;
		}
		unsigned vertices = backupcmds.total_vertices();
		double x, y;
		for (int i = 0; i < vertices; i++)
		{
			backupcmds.vertex(i, &x, &y);
	        backupcmds.modify_vertex(i, x + xamount, y + yamount);
		}
	}
}

void ASSDrawCanvas::MoveCanvasBackground(double xamount, double yamount)
{
	//bgimg.new_center.x += xamount, bgimg.new_center.y += yamount;
	bgimg.new_disp.x += xamount;
	bgimg.new_disp.y += yamount;
	UpdateBackgroundImgScalePosition();	
}

void ASSDrawCanvas::OnSelect_ConvertLineToBezier(wxCommandEvent& WXUNUSED(event))
{
	if (!dblclicked_point_right) return;
	AddUndo( _T("Convert Line to Bezier") );
	DrawCmd_B *newB = new DrawCmd_B(dblclicked_point_right->x(), dblclicked_point_right->y(), 
		pointsys, dblclicked_point_right->cmd_main);
	InsertCmd ( newB, dblclicked_point_right->cmd_main );
	ClearPointsSelection();
	SetHighlighted(NULL, NULL);
	DeleteCommand(dblclicked_point_right->cmd_main);
	pointedAt_point = newB->m_point;
	newB->Init();
	RefreshDisplay();
	RefreshUndocmds();
}

void ASSDrawCanvas::OnSelect_ConvertBezierToLine(wxCommandEvent& WXUNUSED(event))
{
	if (!dblclicked_point_right) return;
	AddUndo( _T("Convert Bezier to Line") );
	DrawCmd_L *newL = new DrawCmd_L(dblclicked_point_right->x(), dblclicked_point_right->y(), pointsys, dblclicked_point_right->cmd_main);
	InsertCmd ( newL, dblclicked_point_right->cmd_main );
	ClearPointsSelection();
	SetHighlighted(NULL, NULL);
	DeleteCommand(dblclicked_point_right->cmd_main);
	pointedAt_point = newL->m_point;
	newL->Init();
	RefreshDisplay();
	RefreshUndocmds();
}

void ASSDrawCanvas::OnSelect_C1ContinuityBezier(wxCommandEvent& WXUNUSED(event))
{
	if (!dblclicked_point_right) return;
	DrawCmd_B *cmdb = static_cast<DrawCmd_B*>(dblclicked_point_right->cmd_main);
	AddUndo( cmdb->C1Cont? _T("Unset continuous"):_T("Set continuous") );
	cmdb->C1Cont = !cmdb->C1Cont;
	RefreshUndocmds();
}

void ASSDrawCanvas::OnSelect_Move00Here(wxCommandEvent& WXUNUSED(event))
{
	AddUndo( _T("Move origin") );
	int wx, wy;
	pointsys->FromWxPoint(mouse_point, wx, wy);
	wxPoint wxp = pointsys->ToWxPoint(wx, wy);
	pointsys->originx = wxp.x;
	pointsys->originy = wxp.y;
	MovePoints(-wx, -wy);
	RefreshDisplay();
	RefreshUndocmds();
}

void ASSDrawCanvas::ConnectSubsequentCmds (DrawCmd* cmd1, DrawCmd* cmd2)
{
	ASSDrawEngine::ConnectSubsequentCmds(cmd1, cmd2);
	if (cmd1 && cmd1->type == B)
	{
		DrawCmd_B *cmd1b = static_cast< DrawCmd_B* >(cmd1);
		cmd1b->C1Cont = false;
	}
}

void ASSDrawCanvas::EnforceC1Continuity (DrawCmd* cmd, Point* pnt)
{
	if (!cmd || !pnt) return;
	if (cmd->type != B && cmd->type != S) return;
	if (pnt->type == MP) return;
	Point *theotherpoint, *mainpoint;
	if (pnt->num == 1)
	{
		if (!cmd->prev || (cmd->prev->type != B && cmd->prev->type != S)) return;
		DrawCmd_B *prevb = static_cast< DrawCmd_B* >(cmd->prev);
		if (!prevb->C1Cont) return;
		PointList::iterator it = prevb->controlpoints.end();
		it--;
		theotherpoint = *it;
		mainpoint = prevb->m_point;
	}
	else if (pnt->num = cmd->controlpoints.size())
	{
		DrawCmd_B *thisb = static_cast< DrawCmd_B* >(cmd);
		if (!thisb->C1Cont) return;
		theotherpoint = *(cmd->m_point->cmd_next->controlpoints.begin());		
		mainpoint = thisb->m_point;
	}
	else
		return;
	theotherpoint->setXY( mainpoint->x() + mainpoint->x() - pnt->x(),
		mainpoint->y() + mainpoint->y() - pnt->y() );
}

// Undo/Redo system
void ASSDrawCanvas::AddUndo( wxString desc ) 
{
	PrepareUndoRedo(_undo, false, _T(""), desc);
	undos.push_back( _undo );
	// also empty redos
	redos.clear();
	m_frame->UpdateUndoRedoMenu();
}

bool ASSDrawCanvas::UndoOrRedo(bool isundo)
{
	std::list<UndoRedo>* main = (isundo? &undos:&redos);
	std::list<UndoRedo>* sub = (isundo? &redos:&undos);

	if (main->empty())
		return false;

	UndoRedo r = main->back();
	// push into sub
	UndoRedo nr(r);
	PrepareUndoRedo(nr, true, GenerateASS(), r.desc); 
	//PrepareUndoRedo(nr, false, _T(""), r.desc);
	sub->push_back( nr );
	// parse
	r.Export(this);
	// delete that
	std::list<UndoRedo>::iterator iter = main->end();
	iter--;
	main->erase(iter);

	// reset some values before refreshing
	mousedownAt_point = NULL;
	pointedAt_point = NULL;
	SetHighlighted ( NULL, NULL );
	ClearPointsSelection();
	RefreshDisplay();
	RefreshUndocmds();
	return true;
}

bool ASSDrawCanvas::Undo()
{
	return UndoOrRedo( true );
}

bool ASSDrawCanvas::Redo()
{
	return UndoOrRedo( false );
}

wxString ASSDrawCanvas::GetTopUndo()
{
	if (undos.empty())
		return _T("");
	else
		return undos.back().desc;
}

wxString ASSDrawCanvas::GetTopRedo()
{
	if (redos.empty())
		return _T("");
	else
		return redos.back().desc;
}

void ASSDrawCanvas::RefreshUndocmds()
{
	_undo.Import(this, true, GenerateASS());
}

void ASSDrawCanvas::PrepareUndoRedo(UndoRedo& ur, bool prestage, wxString cmds, wxString desc) 
{
	ur.Import(this, prestage, cmds);
	ur.desc = desc;	
}

// set command and point to highlight
void ASSDrawCanvas::SetHighlighted ( DrawCmd* cmd, Point* point )
{
     hilite_cmd = cmd;
     hilite_point = point;
}

int ASSDrawCanvas::SelectPointsWithin( int lx, int rx, int ty, int by, SELECTMODE smode )
{

	DrawCmdList::iterator iterate = cmds.begin();
	for (; iterate != cmds.end(); iterate++)
	{
		wxPoint wx = (*iterate)->m_point->ToWxPoint();

		if (wx.x >= lx && wx.x <= rx && wx.y >= ty && wx.y <= by)
			(*iterate)->m_point->isselected = (smode != DEL);
		else
	    	(*iterate)->m_point->isselected &= (smode != NEW);

		if ((*iterate)->m_point->isselected)
	    	selected_points.insert((*iterate)->m_point);
	    else
			selected_points.erase((*iterate)->m_point);

		PointList::iterator pnt_iterator = (*iterate)->controlpoints.begin();
		PointList::iterator end = (*iterate)->controlpoints.end();
		for (; pnt_iterator != end; pnt_iterator++)
		{
			wxPoint wx = (*pnt_iterator)->ToWxPoint();

			if (wx.x >= lx && wx.x <= rx && wx.y >= ty && wx.y <= by)
	    		(*pnt_iterator)->isselected = (smode != DEL);
		    else
	    		(*pnt_iterator)->isselected &= (smode != NEW);
			
			if ((*pnt_iterator)->isselected)
				selected_points.insert(*pnt_iterator);
			else
				selected_points.erase(*pnt_iterator);
		}
	}

	return selected_points.size();

}

void ASSDrawCanvas::ClearPointsSelection()
{
	if (!selected_points.empty())
	{
		PointSet::iterator piter = selected_points.begin();
		for (; piter != selected_points.end(); piter++)
		{
			(*piter)->isselected = false;
		}
		selected_points.clear();
	}
}

SELECTMODE ASSDrawCanvas::GetSelectMode(wxMouseEvent& event)
{
	SELECTMODE smode = NEW;
	bool CtrlDown = event.CmdDown();
	bool AltDown = event.AltDown();
	if (CtrlDown && !AltDown) smode = ADD;
	else if (!CtrlDown && AltDown) smode = DEL;
	return smode;
}

void ASSDrawCanvas::DoDraw( RendererBase& rbase, RendererPrimitives& rprim, RendererSolid& rsolid, agg::trans_affine& mtx )
{
	ASSDrawEngine::Draw_Clear( rbase );
	int ww, hh; GetClientSize(&ww, &hh);

	if (bgimg.bgbmp)
	{ 
		rasterizer.reset();
	    interpolator_type interpolator(bgimg.img_mtx);
	    PixelFormat::AGGType ipixfmt(bgimg.ibuf);
		span_gen_type spangen(ipixfmt, agg::rgba_pre(0, 0, 0), interpolator);
		agg::conv_transform< agg::path_storage > bg_border(bgimg.bg_path, bgimg.path_mtx);
		agg::conv_clip_polygon< agg::conv_transform< agg::path_storage > > bg_clip(bg_border);
		bg_clip.clip_box(0, 0, ww, hh);
		rasterizer.add_path(bg_clip);
		agg::render_scanlines_aa(rasterizer, scanline, rbase, bgimg.spanalloc, spangen);
	}

	ASSDrawEngine::Draw_Draw( rbase, rprim, rsolid, mtx, preview_mode? rgba_shape:rgba_shape_normal );
		
	if (!preview_mode)
	{
		// [0, 0]
		rasterizer.reset();
		agg::path_storage org_path;
		org_path.move_to(0, m_frame->sizes.origincross);
		org_path.line_to(0, -m_frame->sizes.origincross);
		org_path.move_to(m_frame->sizes.origincross, 0);
		org_path.line_to(-m_frame->sizes.origincross, 0);
		agg::conv_transform< agg::path_storage > org_path_t(org_path, mtx);
	    agg::conv_curve< agg::conv_transform< agg::path_storage > > crosshair(org_path_t);
		agg::conv_stroke< agg::conv_curve< agg::conv_transform< agg::path_storage > > > chstroke(crosshair);
		rasterizer.add_path(chstroke);
		rsolid.color(rgba_origin);
		render_scanlines(rsolid, false);

		if (IsTransformMode() && isshapetransformable)
		{
			if (draw_mode == MODE_SCALEROTATE)
			{
				// rotation centerpoint
				rasterizer.reset();
				double len = 10.0; //m_frame->sizes.origincross * pointsys->scale;
				org_path.free_all();
				org_path.move_to(rectcenter.x - len, rectcenter.y - len);
				org_path.line_to(rectcenter.x + len, rectcenter.y + len);
				org_path.move_to(rectcenter.x + len, rectcenter.y - len);
				org_path.line_to(rectcenter.x - len, rectcenter.y + len);
				agg::conv_stroke< agg::path_storage > cstroke(org_path);
				rasterizer.add_path(cstroke);
				agg::ellipse circ(rectcenter.x, rectcenter.y, len, len);
				agg::conv_stroke< agg::ellipse > c(circ);
				rasterizer.add_path(c);
				rsolid.color(rgba_origin);
				render_scanlines(rsolid, false);
			}

			rasterizer.reset();
			agg::path_storage org_path;
			org_path.move_to(rectbound2[0].x, rectbound2[0].y);
			org_path.line_to(rectbound2[1].x, rectbound2[1].y);
			org_path.line_to(rectbound2[2].x, rectbound2[2].y);
			org_path.line_to(rectbound2[3].x, rectbound2[3].y);
			org_path.line_to(rectbound2[0].x, rectbound2[0].y);
			agg::conv_stroke<agg::path_storage> chstroke(org_path);
			chstroke.width(1);
			rsolid.color(rgba_origin);
			rasterizer.add_path(chstroke);
			if (rectbound2upd != -1)
			{
				agg::ellipse circ(rectbound2[rectbound2upd].x, rectbound2[rectbound2upd].y, 
					pointsys->scale, pointsys->scale);
				agg::conv_contour< agg::ellipse > c(circ);
				rasterizer.add_path(c);
			}
			if (rectbound2upd2 != -1)
			{
				agg::ellipse circ(rectbound2[rectbound2upd2].x, rectbound2[rectbound2upd2].y, 
					pointsys->scale, pointsys->scale);
				agg::conv_contour< agg::ellipse > c(circ);
				rasterizer.add_path(c);
			}
			render_scanlines(rsolid, false);
		}
		else
		{
			// outlines
			agg::conv_stroke< agg::conv_transform< agg::path_storage > > bguidestroke(*rb_path);
			bguidestroke.width(1);
			rsolid.color(rgba_guideline);
			rasterizer.add_path(bguidestroke);
			render_scanlines(rsolid);
	
			agg::conv_stroke< agg::conv_curve< agg::conv_transform< agg::path_storage > > > stroke(*rm_curve);
			stroke.width(1);
			rsolid.color(rgba_outline);
			rasterizer.add_path(stroke);
			render_scanlines(rsolid);
	
			double diameter = pointsys->scale;
			double radius = diameter / 2.0;
			// hilite
			if (hilite_cmd && hilite_cmd->type != M)
			{
				rasterizer.reset();
				agg::path_storage h_path;
				AddDrawCmdToAGGPathStorage(hilite_cmd, h_path, HILITE);
				agg::conv_transform< agg::path_storage> h_path_trans(h_path, mtx);
			    agg::conv_curve< agg::conv_transform< agg::path_storage> > curve(h_path_trans);
				agg::conv_dash< agg::conv_curve< agg::conv_transform< agg::path_storage > > > d(curve);
				d.add_dash(10,5);
				agg::conv_stroke< agg::conv_dash< agg::conv_curve< agg::conv_transform< agg::path_storage > > > > stroke(d);
				stroke.width(3);
				rsolid.color(rgba_outline);
				rasterizer.add_path(stroke);
				render_scanlines(rsolid);
			}
			
			// m_point
			rasterizer.reset();
			DrawCmdList::iterator ci = cmds.begin();
			while (ci != cmds.end())
			{
				double lx = (*ci)->m_point->x() * pointsys->scale + pointsys->originx - radius;
				double ty = (*ci)->m_point->y() * pointsys->scale + pointsys->originy - radius;
				agg::path_storage sqp = agghelper::RectanglePath(lx, lx + diameter, ty, ty + diameter);
				agg::conv_contour< agg::path_storage > c(sqp);
				rasterizer.add_path(c);
				ci++;
			}
			render_scanlines_aa_solid(rbase, rgba_mainpoint);
	
			// control_points
			rasterizer.reset();
			ci = cmds.begin();
			while (ci != cmds.end())
			{
				PointList::iterator pi = (*ci)->controlpoints.begin();
				while (pi != (*ci)->controlpoints.end())
				{
					agg::ellipse circ((*pi)->x() * pointsys->scale + pointsys->originx, 
						(*pi)->y() * pointsys->scale + pointsys->originy, radius, radius);
					agg::conv_contour< agg::ellipse > c(circ);
					rasterizer.add_path(c);
					pi++;
				}
				ci++;
			}
			render_scanlines_aa_solid(rbase, rgba_controlpoint);
		
			// selection
			rasterizer.reset();
			PointSet::iterator si = selected_points.begin();
			while (si != selected_points.end())
			{
				agg::ellipse circ((*si)->x() * pointsys->scale + pointsys->originx, 
					(*si)->y() * pointsys->scale + pointsys->originy, radius + 3, radius + 3);
				agg::conv_stroke< agg::ellipse > s(circ);
				rasterizer.add_path(s);
				si++;
			}
			render_scanlines_aa_solid(rbase, rgba_selectpoint);
	
			// hover
			if (hilite_point)
			{
				rasterizer.reset();
				agg::ellipse circ(hilite_point->x() * pointsys->scale + pointsys->originx, 
					hilite_point->y() * pointsys->scale + pointsys->originy, radius + 3, radius + 3);
				agg::conv_stroke< agg::ellipse > s(circ);
				s.width(2);
				rasterizer.add_path(s);
				render_scanlines_aa_solid(rbase, rgba_selectpoint);
	
				rasterizer.reset();
				agg::gsv_text t;
				t.flip(true);
				t.size(8.0);
				wxPoint pxy = hilite_point->ToWxPoint(true);
				t.start_point(pxy.x + 5, pxy.y -5 );
				t.text(wxString::Format(_T("%d,%d"), hilite_point->x(), hilite_point->y()).mb_str(wxConvUTF8));
				agg::conv_stroke< agg::gsv_text > pt(t);
				pt.line_cap(agg::round_cap);
				pt.line_join(agg::round_join);
				pt.width(1.5);
				rasterizer.add_path(pt);
				rsolid.color(agg::rgba(0,0,0));
				render_scanlines(rsolid, false);
				
				rasterizer.reset();
				agg::path_storage sb_path;
				sb_path.move_to(pxy.x, 0);
				sb_path.line_to(pxy.x, hh);
				sb_path.move_to(0, pxy.y);
				sb_path.line_to(ww, pxy.y);
			    agg::conv_curve< agg::path_storage > curve(sb_path);
				agg::conv_dash< agg::conv_curve< agg::path_storage > > d(curve);
				d.add_dash(10,5);
				agg::conv_stroke< agg::conv_dash< agg::conv_curve< agg::path_storage > > > stroke(d);
				stroke.width(1);
				rsolid.color(agg::rgba(0,0,0,0.5));
				rasterizer.add_path(stroke);
				render_scanlines(rsolid);
			}
			
			// selection box
			if (lastDrag_left)
			{
				double x1 = lastDrag_left->x, y1 = lastDrag_left->y;
				double x2 = dragAnchor_left->x, y2 = dragAnchor_left->y;
				double lx, rx, ty, by;
				if (x1 < x2) lx = x1, rx = x2;
				else lx = x2, rx = x1;
				if (y1 < y2) ty = y1, by = y2;
				else ty = y2, by = y1;
				rasterizer.reset();
				agg::path_storage sb_path = agghelper::RectanglePath(lx, rx, ty, by);
			    agg::conv_curve< agg::path_storage > curve(sb_path);
				agg::conv_dash< agg::conv_curve< agg::path_storage > > d(curve);
				d.add_dash(10,5);
				agg::conv_stroke< agg::conv_dash< agg::conv_curve< agg::path_storage > > > stroke(d);
				stroke.width(1.0);
				rsolid.color(agg::rgba(0,0,0,0.5));
				rasterizer.add_path(stroke);
				render_scanlines(rsolid);
			}
		}		
	}
	
	// ruler
	int w, h;
	GetClientSize( &w, &h );
	double scale = pointsys->scale;
	double coeff = 9 / scale + 1;
	int numdist = (int) floor(coeff) * 5;
	{
		rsolid.color(rgba_ruler_h);
		rasterizer.reset();
		agg::path_storage rlr_path;
		double start = pointsys->originx;
		int t = - (int) floor(start / scale);
		double s = (start + t * scale);
		double collect = s;
		int len;

		for (; s <= w; s += scale)
		{
			bool longtick = t % numdist == 0;
			if (longtick)
			{
				len = 10;
				agg::gsv_text txt;
				txt.flip(true);
				txt.size(6.0);
				txt.start_point(s, 20);
				txt.text(wxString::Format(_T("%d"), t).mb_str(wxConvUTF8));
				agg::conv_stroke< agg::gsv_text > pt(txt);
				rasterizer.add_path(pt);
			}
			else
				len = 5;
			t++ ;
			collect += scale;
			if (collect > 5.0)
			{
				collect = 0.0;
				rlr_path.move_to(s, 0);
				rlr_path.line_to(s, len);
			}
		}
		agg::conv_stroke< agg::path_storage > rlr_stroke(rlr_path);
		rlr_stroke.width(1);
		rasterizer.add_path(rlr_stroke);
		render_scanlines(rsolid, false);
	}
	{
		rasterizer.reset();
		rsolid.color(rgba_ruler_v);
		agg::path_storage rlr_path;
		double start = pointsys->originy;
		int t = - (int) floor(start / scale);
		double s = (start + t * scale);
		double collect = 0;
		int len;

		for (; s <= h; s += scale)
		{
			bool longtick = t % numdist == 0;
			if (longtick)
			{
				len = 10;
				agg::gsv_text txt;
				txt.flip(true);
				txt.size(6.0);
				txt.start_point(12, s);
				txt.text(wxString::Format(_T("%d"), t).mb_str(wxConvUTF8));
				agg::conv_stroke< agg::gsv_text > pt(txt);
				rasterizer.add_path(pt);
			}
			else
				len = 5;
			t++ ;
			collect += scale;
			if (collect > 5.0)
			{
				collect = 0.0;
				rlr_path.move_to(0, s);
				rlr_path.line_to(len, s);
			}
		}
		agg::conv_stroke< agg::path_storage > rlr_stroke(rlr_path);
		rlr_stroke.width(1);
		rasterizer.add_path(rlr_stroke);
		render_scanlines(rsolid, false);
	}

}

void ASSDrawCanvas::ReceiveBackgroundImageFileDropEvent(const wxString& filename)
{
	const wxChar *shortfname = wxFileName::FileName(filename).GetFullName().c_str();
	m_frame->SetStatusText(wxString::Format(_T("Loading '%s' as canvas background ..."), shortfname), 1);
	wxImage img;
	img.LoadFile(filename);
	if (img.IsOk())
	{
		SetBackgroundImage(img, filename);
	}
	m_frame->SetStatusText(_T("Canvas background loaded"), 1);
}

void ASSDrawCanvas::RemoveBackgroundImage()
{
	if (bgimg.bgimg) delete bgimg.bgimg;
	bgimg.bgimg = NULL;
	if (bgimg.bgbmp) delete bgimg.bgbmp;
	bgimg.bgbmp = NULL;
	bgimg.bgimgfile = _T("");
	RefreshDisplay();
	drag_mode = DRAGMODE();
	bgimg.alpha_dlg->Show(false);
	m_frame->UpdateFrameUI();
}

void ASSDrawCanvas::SetBackgroundImage(const wxImage& img, wxString fname, bool ask4alpha)
{
	if (bgimg.bgimg) delete bgimg.bgimg;
	bgimg.bgimg = new wxImage(img);
	bgimg.bgimgfile = fname;
	PrepareBackgroundBitmap(bgimg.alpha);
    UpdateBackgroundImgScalePosition(true);
	RefreshDisplay();
	m_frame->UpdateFrameUI();
	if (ask4alpha && m_frame->behaviors.autoaskimgopac)
		AskUserForBackgroundAlpha();
}

void ASSDrawCanvas::AskUserForBackgroundAlpha()
{
	bgimg.alpha_slider->SetValue((int) (100 - bgimg.alpha * 100));
	bgimg.alpha_dlg->Show();
	SetFocus();
}
	
void ASSDrawCanvas::PrepareBackgroundBitmap(double alpha)
{
	if (alpha >= 0.0 && alpha <= 1.0) bgimg.alpha = alpha;
	if (bgimg.bgimg == NULL) return;
	if (bgimg.bgbmp) delete bgimg.bgbmp;
	bgimg.bgbmp = new wxBitmap(*bgimg.bgimg);
    PixelData data(*bgimg.bgbmp);
    wxAlphaPixelFormat::ChannelType* pd = (wxAlphaPixelFormat::ChannelType*) &data.GetPixels().Data();
    const int stride = data.GetRowStride();
    if (stride < 0)
        pd += (data.GetHeight() - 1) * stride;
    bgimg.ibuf.attach(pd, data.GetWidth(), data.GetHeight(), stride);
    
    // apply alpha
	rasterizer.reset();
	unsigned w = bgimg.bgbmp->GetWidth(), h = bgimg.bgbmp->GetHeight();
	bgimg.bg_path = agghelper::RectanglePath(0, w, 0, h);
    agg::conv_contour< agg::path_storage > cont(bgimg.bg_path);
	rasterizer.add_path(cont);
	PixelFormat::AGGType pxt(bgimg.ibuf);
	RendererBase rpxt(pxt);
	agg::render_scanlines_aa_solid(rasterizer, scanline, rpxt, agg::rgba(color_bg.r / 255.0, color_bg.g / 255.0, color_bg.b / 255.0, bgimg.alpha));
}

void ASSDrawCanvas::UpdateBackgroundImgScalePosition(bool firsttime)
{
	if (bgimg.bgbmp == NULL) return;
	// transform the enclosing polygon
	unsigned w = bgimg.bgbmp->GetWidth(), h = bgimg.bgbmp->GetHeight();
	bgimg.bg_path = agghelper::RectanglePath(0, w, 0, h);
    // linear interpolation on image buffer
    wxRealPoint center, disp;
    double scale;
    if (firsttime) // first time
    {
	    bgimg.img_mtx = agg::trans_affine();
	    scale = 1.0;
		center = wxRealPoint(0.0, 0.0);
	    disp = wxRealPoint(0.0, 0.0);
	    bgimg.path_mtx = bgimg.img_mtx;
	}
	else
	{
		wxRealPoint d_disp(bgimg.new_disp.x - bgimg.disp.x, bgimg.new_disp.y - bgimg.disp.y);
		scale = bgimg.new_scale;
		disp = bgimg.disp;
		center = bgimg.new_center;
		if (bgimg.scale == scale)
		{
		    bgimg.img_mtx.invert();
		    bgimg.img_mtx *= agg::trans_affine_translation(d_disp.x, d_disp.y);
			d_disp.x /= scale;
			d_disp.y /= scale;
		}
		else
		{
			d_disp.x /= scale;
			d_disp.y /= scale;
		    bgimg.img_mtx = agg::trans_affine();
			disp.x += (center.x - bgimg.center.x) * (1.0 - 1.0 / bgimg.scale);
			disp.y += (center.y - bgimg.center.y) * (1.0 - 1.0 / bgimg.scale);
		    bgimg.img_mtx *= agg::trans_affine_translation(-center.x + disp.x, -center.y + disp.y);
		    bgimg.img_mtx *= agg::trans_affine_scaling(scale);
		    bgimg.img_mtx *= agg::trans_affine_translation(center.x + d_disp.x, center.y + d_disp.y);
		}
	    bgimg.path_mtx = bgimg.img_mtx;
	    bgimg.img_mtx.invert();
	    disp.x += d_disp.x;
	    disp.y += d_disp.y;
	}
    //update
    bgimg.scale = scale;
    bgimg.center = center;
    bgimg.disp = disp;
    bgimg.new_scale = scale;
    bgimg.new_center = center;
    bgimg.new_disp = disp;
}

bool ASSDrawCanvas::GetBackgroundInfo(unsigned& w, unsigned& h, wxRealPoint& disp, double& scale)
{
	if (!HasBackgroundImage()) return false;
	w = bgimg.bgbmp->GetWidth(), h = bgimg.bgbmp->GetHeight();
	double t, l;
	agg::conv_transform<agg::path_storage, agg::trans_affine> trr(bgimg.bg_path, bgimg.path_mtx);
	trr.rewind(0);
	trr.vertex(&l, &t);
	disp = wxRealPoint(l, t);
	scale = bgimg.scale;
	return true;
}

void ASSDrawCanvas::UpdateNonUniformTransformation()
{
	double bound[8] = {
		rectbound2[0].x, rectbound2[0].y,
		rectbound2[1].x, rectbound2[1].y,
		rectbound2[2].x, rectbound2[2].y,
		rectbound2[3].x, rectbound2[3].y };
	agg::path_storage trans;
	unsigned vertices = backupcmds.total_vertices();
	double x, y;
	
	//if (draw_mode == MODE_NUT_BILINEAR)
	//{
		agg::trans_bilinear trans_b(rectbound[0].x, rectbound[0].y, rectbound[2].x, rectbound[2].y, bound);
		agg::conv_transform<agg::path_storage, agg::trans_bilinear> transb(backupcmds, trans_b);
		transb.rewind(0);
		for (int i = 0; i < vertices; i++)
		{
			transb.vertex(&x, &y);
			trans.move_to(x, y);
		}
	//}
	/*	else
	{
		agg::trans_perspective trans_p(rectbound[0].x, rectbound[0].y, rectbound[2].x, rectbound[2].y, bound);
		agg::conv_transform<agg::path_storage, agg::trans_perspective> transp(backupcmds, trans_p);
		transp.rewind(0);
		for (int i = 0; i < vertices; i++)
		{
			transp.vertex(&x, &y);
			trans.move_to(x, y);
		}
	}
	*/
	trans.rewind(0);
	for (DrawCmdList::iterator iterate = cmds.begin(); iterate != cmds.end(); iterate++)
	{
		DrawCmd* cmd = (*iterate);
		for (PointList::iterator iterate2 = cmd->controlpoints.begin(); iterate2 != cmd->controlpoints.end(); iterate2++)
		{
	        double x, y;
	        trans.vertex(&x, &y);
	        int wx, wy;
			pointsys->FromWxPoint ( wxPoint((int)x, (int)y), wx, wy );
			(*iterate2)->setXY(wx, wy);
		}
	    double x, y;
	    trans.vertex(&x, &y);
	    int wx, wy;
		pointsys->FromWxPoint ( wxPoint((int)x, (int)y), wx, wy );
		(*iterate)->m_point->setXY(wx, wy);
	}
	
}

void ASSDrawCanvas::CustomOnKeyDown(wxKeyEvent &event)
{
	int keycode = event.GetKeyCode();
	//m_frame->SetStatusText(wxString::Format(_T("Key: %d"), keycode));
	double scrollamount = (event.GetModifiers() == wxMOD_CMD? 10.0:1.0);	
    if (event.GetModifiers() == wxMOD_SHIFT)
	{
		MODE d_mode = GetDrawMode();
        if ((int) d_mode > (int) MODE_ARR && (int) d_mode < (int) MODE_SCALEROTATE)
		{
			mode_b4_shift = d_mode;
			SetDrawMode( MODE_ARR );
			m_frame->UpdateFrameUI();
		}
    }
    else
    {
		switch (keycode)
		{
		case WXK_PAGEUP:
			ChangeZoomLevel( 1.0 /scrollamount, wxPoint( (int) pointsys->originx, (int) pointsys->originy ) );
			RefreshDisplay();
			break;
		case WXK_PAGEDOWN:
			ChangeZoomLevel( - 1.0 /scrollamount, wxPoint( (int) pointsys->originx, (int) pointsys->originy ) );
			RefreshDisplay();
			break;
		case WXK_UP:
			MoveCanvas(0.0, -scrollamount);
			RefreshDisplay();
			break;
		case WXK_DOWN:
			MoveCanvas(0.0, scrollamount);
			RefreshDisplay();
			break;
		case WXK_LEFT:
			MoveCanvas(-scrollamount, 0.0);
			RefreshDisplay();
			break; 
		case WXK_RIGHT:
			MoveCanvas(scrollamount, 0.0);
			RefreshDisplay();
			break;
		case WXK_TAB:
			if (mousedownAt_point == NULL && !IsTransformMode() && cmds.size() > 0)
			{
				if (pointedAt_point == NULL)
				{
					Point *nearest = NULL;
					double dist = 0.0;
					DrawCmdList::iterator it = cmds.begin();
					while(it != cmds.end())
					{
						wxPoint point = (*it)->m_point->ToWxPoint();
						double distance = sqrt(pow(double(point.x - mouse_point.x), 2) + pow(double(point.y - mouse_point.y), 2));
						if (nearest == NULL || distance < dist)
						{
							nearest = (*it)->m_point;
							dist = distance;
						}
						PointList::iterator it2 = (*it)->controlpoints.begin();
						while (it2 != (*it)->controlpoints.end())
						{
							wxPoint point = (*it2)->ToWxPoint();
							double distance = sqrt(pow((double)point.x - mouse_point.x, 2) + pow((double)point.y - mouse_point.y, 2));
							if (nearest == NULL || distance < dist)
							{
								nearest = (*it2);
								dist = distance;
							}						
							it2++;	
						}
						it++;
					}
					if (nearest != NULL)
					{
						wxPoint point = nearest->ToWxPoint();
						WarpPointer(point.x, point.y);
					}
				}
				else
				{
					Point *warpto = NULL;
					if (pointedAt_point->type == MP)
					{
						if (pointedAt_point->cmd_next != NULL)
						{
							if (pointedAt_point->cmd_next->controlpoints.size() > 0)
								warpto = pointedAt_point->cmd_next->controlpoints.front();
							else
								warpto = pointedAt_point->cmd_next->m_point;
						}
					}
					else
					{
						PointList::iterator it = pointedAt_point->cmd_main->controlpoints.begin();
						while (*it != pointedAt_point) it++;
						it++;
						if (it == pointedAt_point->cmd_main->controlpoints.end())
							warpto = pointedAt_point->cmd_main->m_point;
						else
							warpto = *it;
					}
					if (warpto == NULL)
						warpto = cmds.front()->m_point;
					wxPoint point = warpto->ToWxPoint();
					WarpPointer(point.x, point.y);
				}
			}
			break;
		default:
	        event.Skip();
		}
	}
}

void ASSDrawCanvas::CustomOnKeyUp(wxKeyEvent &event)
{
	int keycode = event.GetKeyCode();
	if (event.GetModifiers() != wxMOD_SHIFT && (int) mode_b4_shift > (int) MODE_ARR)
	{
		SetDrawMode( mode_b4_shift );
		m_frame->UpdateFrameUI();
		mode_b4_shift = MODE_ARR;
	}
}

void ASSDrawCanvas::OnAlphaSliderChanged(wxScrollEvent &event)
{
	double pos = (double) event.GetPosition();
	PrepareBackgroundBitmap(1.0 - pos / 100.0);
	RefreshDisplay();
}

void ASSDrawCanvas::CustomOnMouseCaptureLost(wxMouseCaptureLostEvent &event)
{
	if (capturemouse_left)
		ProcessOnMouseLeftUp();

	if (capturemouse_right)
		ProcessOnMouseRightUp();
}

void UndoRedo::Import(ASSDrawCanvas *canvas, bool prestage, wxString cmds)
{
	if (prestage)
	{
		this->cmds = cmds;
		this->backupcmds.free_all();
		this->backupcmds.concat_path(canvas->backupcmds);
		for (int i = 0; i < 4; i++)
		{
			this->rectbound[i] = canvas->rectbound[i];
			this->rectbound2[i] = canvas->rectbound2[i];
			this->backup[i] = canvas->backup[i];
		}
		this->isshapetransformable = canvas->isshapetransformable;
	}
	else
	{
	    this->originx = canvas->pointsys->originx;
		this->originy = canvas->pointsys->originy;
	    this->scale = canvas->pointsys->scale;
	
		this->bgimgfile = canvas->bgimg.bgimgfile;
		this->bgdisp = canvas->bgimg.disp;
		this->bgcenter = canvas->bgimg.center;
		this->bgscale = canvas->bgimg.scale;
		this->bgalpha = canvas->bgimg.alpha;
		this->c1cont = canvas->PrepareC1ContData();
		this->draw_mode = canvas->draw_mode;
	}
}

void UndoRedo::Export(ASSDrawCanvas *canvas)
{
	canvas->pointsys->originx = this->originx;
	canvas->pointsys->originy = this->originy;
	canvas->pointsys->scale = this->scale;
	canvas->ParseASS( this->cmds );
	DrawCmdList::iterator it1 = canvas->cmds.begin();
	std::vector< bool >::iterator it2 = this->c1cont.begin();
	for(; it1 != canvas->cmds.end() && it2 != this->c1cont.end(); it1++, it2++)
		if (*it2 && (*it1)->type == B)
			static_cast<DrawCmd_B*>(*it1)->C1Cont = true;

	if (canvas->bgimg.bgimgfile != this->bgimgfile)
	{
		canvas->RemoveBackgroundImage();
		if (!this->bgimgfile.IsSameAs(_T("<clipboard>")) && ::wxFileExists(this->bgimgfile))
		{
			canvas->bgimg.alpha = this->bgalpha;
			canvas->ReceiveBackgroundImageFileDropEvent(this->bgimgfile);
		}
	}
	else
	{
		canvas->bgimg.new_scale = this->bgscale;
		canvas->bgimg.new_center = this->bgcenter;
		canvas->bgimg.new_disp = this->bgdisp;
		canvas->bgimg.alpha = this->bgalpha;
		canvas->UpdateBackgroundImgScalePosition();
	}	
	

	//canvas->SetDrawMode(this->draw_mode);
	canvas->draw_mode = this->draw_mode;
	if (canvas->IsTransformMode())
	{
		canvas->backupcmds.free_all();
		canvas->backupcmds.concat_path(this->backupcmds);
		for (int i = 0; i < 4; i++)
		{
			canvas->rectbound[i] = this->rectbound[i];
			canvas->rectbound2[i] = this->rectbound2[i];
			canvas->backup[i] = this->backup[i];
		}
		canvas->UpdateNonUniformTransformation();
		canvas->InitiateDraggingIfTransformMode();
		canvas->rectbound2upd = -1;
		canvas->rectbound2upd2 = -1;
		canvas->isshapetransformable = this->isshapetransformable;
	}
}

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
// Name:        engine.cpp
// Purpose:     ASSDraw drawing engine
// Author:      ai-chan
// Created:     08/26/06
// Copyright:   (c) ai-chan
// Licence:     3-clause BSD
///////////////////////////////////////////////////////////////////////////////

#include "engine.hpp" // the header
#include "cmd.hpp"    // we need the drawing command classes declaration
#include <wx/tokenzr.h> // we use string tokenizer
#include <vector> // ok, we use vector too
#include <stdio.h>
#include <algorithm>

      

// ----------------------------------------------------------------------------
// Point
// ----------------------------------------------------------------------------

// constructor
Point::Point ( int _x, int _y, PointSystem* ps, POINTTYPE t, DrawCmd* cmd, unsigned n ) 
{
	x_ = _x;
	y_ = _y;
	pointsys = ps;
	cmd_main = cmd; 
	cmd_next = NULL; 
	type = t;
	isselected = false;
	num = n;
}      

// setters
void Point::setXY( int _x, int _y)
{
	x_ = _x;
	y_ = _y;
}

// simply returns true if px and py are the coordinate values
bool Point::IsAt( int px, int py ) 
{
	return (x_ == px && y_ == py );
}
      
// convert this point to wxPoint using scale and originx, originy
wxPoint Point::ToWxPoint ( bool useorigin )
{
	if (useorigin)
		return pointsys->ToWxPoint( x_, y_ );
	else
		return *(new wxPoint(x_ * (int) pointsys->scale, y_ * (int) pointsys->scale ));
}
      
// check if wxpoint is nearby this point
bool Point::CheckWxPoint ( wxPoint wxpoint ) 
{
	wxPoint p = ToWxPoint();
	int cx, cy;
	pointsys->FromWxPoint( wxpoint, cx, cy );
	//delete &p;
	return (x_ == cx && y_ == cy );
}
      


// ----------------------------------------------------------------------------
// DrawCmd
// ----------------------------------------------------------------------------

// constructor
DrawCmd::DrawCmd ( int x, int y, PointSystem *ps, DrawCmd *pv ) 
{ 
	m_point = new Point ( x, y, ps, MP, this );
	m_point->cmd_main = this;
	prev = pv;
	dobreak = false;
	invisible = false;
}       

// destructor
DrawCmd::~DrawCmd ( )
{
	if (m_point)
		delete m_point;
	for (PointList::iterator iter_cpoint = controlpoints.begin();
			iter_cpoint != controlpoints.end(); iter_cpoint++)
		delete (*iter_cpoint);
} 



// ----------------------------------------------------------------------------
// ASSDrawEngine
// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(ASSDrawEngine, GUI::AGGWindow)
    EVT_PAINT  (ASSDrawEngine::OnPaint)
END_EVENT_TABLE()

// constructor
ASSDrawEngine::ASSDrawEngine( wxWindow *parent, int extraflags )
 : GUI::AGGWindow(parent, -1, wxDefaultPosition, wxDefaultSize,
        wxNO_FULL_REPAINT_ON_RESIZE | extraflags )
{
	pointsys = new PointSystem(1, 0, 0) ;
	refresh_called = false;
	fitviewpoint_hmargin = 10;
	fitviewpoint_vmargin = 10;
	setfitviewpoint = false;
	rgba_shape = agg::rgba(0,0,1);
	color_bg = PixelFormat::AGGType::color_type(255, 255, 255);
	drawcmdset = _T("m n l b s p c _"); //the spaces and underscore are in there for a reason, guess?
	ResetEngine();
}

ASSDrawEngine::~ASSDrawEngine()
{
    ResetEngine ( false );
    //delete pointsys;
}

// parse ASS draw commands; returns the number of parsed commands
int ASSDrawEngine::ParseASS ( wxString str )
{
	ResetEngine( false );
	str.Replace(_T("\t"), _T(""));
	str.Replace(_T("\r"), _T(""));
	str.Replace(_T("\n"), _T(""));
	str = str.Lower() + _T(" _ _");
	// we don't use regex because the pattern is too simple
	wxStringTokenizer tkz( str, _T(" ") );
	wxString currcmd(_T(""));
	std::vector<int> val;
	wxString token;
	long tmp_int;
	
	bool n_collected = false;
	DrawCmd_S *s_command = NULL;
	wxPoint tmp_n_pnt;
	
	while ( tkz.HasMoreTokens() )
	{
		token = tkz.GetNextToken();
		
		if ( drawcmdset.Find(token) > -1 )
		{
			bool done;
			
			do {
				done = true;
				
				// N
				if (currcmd.IsSameAs(_T("n")) && val.size() >= 2)
				{
					tmp_n_pnt.x = val[0], tmp_n_pnt.y = val[1];
					n_collected = true;
				}
				else if(n_collected)
				{
					AppendCmd ( L, tmp_n_pnt.x, tmp_n_pnt.y );
					n_collected = false;
				}
				
				if (s_command != NULL)
				{
					bool ends = true;
					if (currcmd.IsSameAs(_T("p"))&& val.size() >= 2)
					{
						s_command->m_point->type = CP;
						s_command->m_point->num = s_command->controlpoints.size() + 1;
						s_command->controlpoints.push_back(s_command->m_point);
						s_command->m_point = new Point(val[0], val[1], pointsys, MP, s_command);
						ends = false;
					}
					else if (currcmd.IsSameAs(_T("c")))
						s_command->closed = true;

					if (ends)
					{
						AppendCmd(s_command);
						s_command = NULL;	
					}
				}
				
				// M
				if (currcmd.IsSameAs(_T("m")) && val.size() >= 2)
					AppendCmd ( M, val[0], val[1] );
				
				// L
				if (currcmd.IsSameAs(_T("l")) && val.size() >= 2)
				{
					AppendCmd ( L, val[0], val[1] );
					val.erase(val.begin(), val.begin()+2);
					// L is greedy
					if (val.size() >= 2)
						done = false;
				}
				
				// B
				if (currcmd.IsSameAs(_T("b")) && val.size() >= 6)
				{
					AppendCmd ( new DrawCmd_B(val[4], val[5], val[0], val[1],
					val[2], val[3], pointsys, LastCmd()) );
					val.erase(val.begin(), val.begin()+6);
					// so is B
					if (val.size() >= 6)
						done = false;
				}
				
				// S
				if (currcmd.IsSameAs(_T("s")) && val.size() >= 6)
				{
					int num = (val.size() / 2) * 2;
					std::vector<int> val2;
					int i = 0;
					for (; i < num - 2; i++)
						val2.push_back(val[i]);

					s_command = new DrawCmd_S(val[num - 2], val[num - 1], val2, pointsys, LastCmd());
				}
				// more to come later
			} while (!done);
			
			val.clear();
			currcmd = token;
		}
		else if (token.ToLong( &tmp_int ))
		{
			val.push_back( (int) tmp_int );
		}	
	}
	
	return (int) cmds.size();
}

// generate ASS draw commands
wxString ASSDrawEngine::GenerateASS ( )
{
    wxString output = _T("");
    for (DrawCmdList::iterator iterate = cmds.begin(); iterate != cmds.end(); iterate++)
         output = output + (*iterate)->ToString() + _T(" ");
     return output;
}

// reset; delete all points and add a new M(0,0)
void ASSDrawEngine::ResetEngine()
{
     ResetEngine(true);
}

// reset; delete all points and add a new M(0,0) if addM == true
void ASSDrawEngine::ResetEngine( bool addM )
{
	for (DrawCmdList::iterator iterate = cmds.begin(); iterate != cmds.end(); iterate++)
		delete (*iterate);
	cmds.clear();
	if (addM) AppendCmd( M, 0, 0 );
}

// Create draw command of type 'type' and m_point (x, y), append to the
// list and return it
DrawCmd* ASSDrawEngine::AppendCmd ( CMDTYPE type, int x, int y )
{

	// use a variation of this method
	return AppendCmd( NewCmd( type, x, y ) );
}

// Append draw command
DrawCmd* ASSDrawEngine::AppendCmd ( DrawCmd* cmd )
{
	// no NULL command!
	if (cmd == NULL) return NULL;

	// set dependency of this command on the m_point of the last command
	if (!cmds.empty())
		ConnectSubsequentCmds( cmds.back(), cmd );
	else
	{
		// since this is the first command, if it's not an M make it into one
		if (cmd->type != M)
			cmd = NewCmd( M, cmd->m_point->x(), cmd->m_point->y() );
		ConnectSubsequentCmds( NULL, cmd );
	}
	// put it in the list
	cmds.push_back( cmd );

	return cmd;
}

// create draw command of type 'type' and m_point (x, y), insert to the
// list after the _cmd and return it
DrawCmd* ASSDrawEngine::InsertCmd ( CMDTYPE type, int x, int y, DrawCmd* _cmd )
{
     // prepare the new DrawCmd
     DrawCmd* c = NewCmd( type, x, y );

     // use a variation of this method
     InsertCmd( c, _cmd );

     return NULL;
}

// insert draw command cmd after _cmd
void ASSDrawEngine::InsertCmd ( DrawCmd* cmd, DrawCmd* _cmd )
{
     DrawCmdList::iterator iterate = cmds.begin();
     for (; iterate != cmds.end() && *iterate != _cmd; iterate++)
     {
         // do nothing
     }

     if (iterate == cmds.end())
     {
        AppendCmd( cmd );
     }
     else
     {
        iterate++;
        if (iterate != cmds.end())
        {
           ConnectSubsequentCmds( cmd, (*iterate) );
        }
        cmds.insert( iterate, cmd );
        ConnectSubsequentCmds( _cmd, cmd );
     }
}

DrawCmd* ASSDrawEngine::NewCmd ( CMDTYPE type, int x, int y )
{
     DrawCmd* c = NULL;

     switch (type)
     {
        case M:
             c = new DrawCmd_M(x, y, pointsys, LastCmd());
             break;
        case L:
             c = new DrawCmd_L(x, y, pointsys, LastCmd());
             break;
        case B:
             c = new DrawCmd_B(x, y, pointsys, LastCmd());
             break;
        case S:
             c = new DrawCmd_S(x, y, pointsys, LastCmd());
             break;
     }
     return c;
}

// returns the iterator for the list
DrawCmdList::iterator ASSDrawEngine::Iterator ( )
{
	return cmds.begin();
}

// returns the 'end' iterator for the list
DrawCmdList::iterator ASSDrawEngine::IteratorEnd ( )
{
	return cmds.end();
}

// returns the last command in the list
DrawCmd* ASSDrawEngine::LastCmd ()
{
	if (cmds.size() == 0)
		return NULL;
	else
		return cmds.back();
}

// move all points by relative amount of x, y coordinates
void ASSDrawEngine::MovePoints ( int x, int y )
{
     DrawCmdList::iterator iterate = cmds.begin();
     PointList::iterator iterate2;

     for (; iterate != cmds.end(); iterate++)
     {
         (*iterate)->m_point->setXY( (*iterate)->m_point->x() + x, (*iterate)->m_point->y() + y );
         for (iterate2 = (*iterate)->controlpoints.begin(); 
		 	iterate2 != (*iterate)->controlpoints.end(); iterate2++)
         {
			(*iterate2)->setXY( (*iterate2)->x() + x, (*iterate2)->y() + y );
         }
	}
}

// transform all points using the calculation:
//   | (m11)  (m12) | x | (x - mx) | + | nx |
//   | (m21)  (m22) |   | (y - my) |   | ny |
void ASSDrawEngine::Transform( float m11, float m12, float m21, float m22,
							float mx, float my, float nx, float ny )
{
    DrawCmdList::iterator iterate = cmds.begin();
    PointList::iterator iterate2;
	float x, y;
	for (; iterate != cmds.end(); iterate++)
	{
		x = ((float) (*iterate)->m_point->x()) - mx;
		y = ((float) (*iterate)->m_point->y()) - my;
		(*iterate)->m_point->setXY((int) (x * m11 + y * m12 + nx), (int) (x * m21 + y * m22 + ny) );
		for (iterate2 = (*iterate)->controlpoints.begin();
			 iterate2 != (*iterate)->controlpoints.end(); iterate2++)
		{
			x = ((float) (*iterate2)->x()) - mx;
			y = ((float) (*iterate2)->y()) - my;
			(*iterate2)->setXY((int) (x * m11 + y * m12 + nx), (int) (x * m21 + y * m22 + ny) );
		}
	}
}

// returns some DrawCmd if its m_point = (x, y)
DrawCmd* ASSDrawEngine::PointAt ( int x, int y )
{
     DrawCmd* c = NULL;
	 DrawCmdList::iterator iterate = cmds.begin();

     for (; iterate != cmds.end(); iterate++)
     {
         if ( (*iterate)->m_point->IsAt( x, y ) )
            c = (*iterate);
     }

     //delete &iterate;

     return c;
}

// returns some DrawCmd if one of its control point = (x, y)
// also set &point to refer to that control point
DrawCmd* ASSDrawEngine::ControlAt ( int x, int y, Point* &point )
{
	DrawCmd* c = NULL;
	point = NULL;
	DrawCmdList::iterator cmd_iterator = cmds.begin();
	PointList::iterator pnt_iterator;
	PointList::iterator end;

	for (; cmd_iterator != cmds.end(); cmd_iterator++)
	{
		pnt_iterator = (*cmd_iterator)->controlpoints.begin();
		end = (*cmd_iterator)->controlpoints.end();
		for (; pnt_iterator != end; pnt_iterator++)
		{
			if ( (*pnt_iterator)->IsAt( x, y ) )
			{
				c = (*cmd_iterator);
				point = (*pnt_iterator);
			}
		}
	}

	return c;
}

// attempts to delete a commmand, returns true|false if successful|fail
bool ASSDrawEngine::DeleteCommand ( DrawCmd* cmd )
{
	
	DrawCmdList::iterator iterate = cmds.begin();
	// can't delete the first command without deleting other commands first
	if ( cmd == (*iterate) && cmds.size() > 1) return false;
	
	DrawCmd* lastiter = NULL;
	
	for (; iterate != cmds.end(); iterate++)
	{
		if ( cmd == (*iterate) )
		{
			iterate++;
			DrawCmd* nxt = (iterate != cmds.end()? (*iterate):NULL);
			ConnectSubsequentCmds( lastiter, nxt );
			iterate--;
			cmds.erase( iterate );
			delete cmd;
			break;
		}
		else
			lastiter = (*iterate);
	}
	
	return true;
}

// set stuff to connect two drawing commands cmd1 and cmd2 such that
// cmd1 comes right before cmd2
void ASSDrawEngine::ConnectSubsequentCmds (DrawCmd* cmd1, DrawCmd* cmd2)
{
	if (cmd1 != NULL)
	{
		cmd1->m_point->cmd_next = cmd2;
	}
	
	if (cmd2 != NULL)
	{
		cmd2->prev = cmd1;
	}
}

void ASSDrawEngine::RefreshDisplay() 
{
	if (!refresh_called)
	{
		Refresh();
		refresh_called = true;
	}
}

void ASSDrawEngine::OnPaint(wxPaintEvent& event)
{
	draw();
	onPaint(event);
	if (setfitviewpoint)
	{
		FitToViewPoint( fitviewpoint_hmargin, fitviewpoint_vmargin );
		setfitviewpoint = false;
		RefreshDisplay();
	}
}

void ASSDrawEngine::draw()
{
	refresh_called = false;
	
	PixelFormat::AGGType pixf(rBuf);
	RendererBase rbase(pixf);
	RendererPrimitives rprim(rbase);
	RendererSolid rsolid(rbase);

	agg::trans_affine mtx;
	ConstructPathsAndCurves(mtx, rm_path, rb_path, rm_curve);

	rasterizer.reset();
	update_rendered_bound_coords(true);
	DoDraw(rbase, rprim, rsolid, mtx);
	
	delete rm_path, rb_path, rm_curve;
}

void ASSDrawEngine::ConstructPathsAndCurves(agg::trans_affine& mtx, 
	trans_path*& _rm_path, trans_path*& _rb_path, agg::conv_curve<trans_path>*& _rm_curve)
{
    mtx *= agg::trans_affine_scaling(pointsys->scale);
    mtx *= agg::trans_affine_translation(pointsys->originx, pointsys->originy);

	m_path.remove_all();
	b_path.remove_all();

	DrawCmdList::iterator ci = cmds.begin();
	while (ci != cmds.end())
	{
		AddDrawCmdToAGGPathStorage(*ci, m_path);
		AddDrawCmdToAGGPathStorage(*ci, b_path, CTRL_LN);
		ci++;
	}
	_rm_path = new trans_path(m_path, mtx);
	_rb_path = new trans_path(b_path, mtx);
    _rm_curve = new agg::conv_curve<trans_path>(*rm_path);
}

void ASSDrawEngine::DoDraw( RendererBase& rbase, RendererPrimitives& rprim, RendererSolid& rsolid, agg::trans_affine& mtx )
{
	Draw_Clear( rbase );
	Draw_Draw( rbase, rprim, rsolid, mtx, rgba_shape );
}

void ASSDrawEngine::Draw_Clear( RendererBase& rbase )
{
	rbase.clear(color_bg);
}

void ASSDrawEngine::Draw_Draw( RendererBase& rbase, RendererPrimitives& rprim, RendererSolid& rsolid, agg::trans_affine& mtx, agg::rgba color )
{
    agg::conv_contour< agg::conv_curve< agg::conv_transform< agg::path_storage > > > contour(*rm_curve);
	rasterizer.add_path(contour);
	render_scanlines_aa_solid(rbase, color);
}

void ASSDrawEngine::AddDrawCmdToAGGPathStorage(DrawCmd* cmd, agg::path_storage& path, DRAWCMDMODE mode)
{
	if (mode == HILITE && cmd->prev)
		path.move_to(cmd->prev->m_point->x(), cmd->prev->m_point->y());

	switch(cmd->type)
	{
	case M:
		path.move_to(cmd->m_point->x(),cmd->m_point->y());
		break;

	case B:
		if (cmd->initialized)
		{
			//path.move_to(cmd->prev->m_point->x(),cmd->prev->m_point->y());
			PointList::iterator iterate = cmd->controlpoints.begin();
			int x[2], y[2];
			x[0] = (*iterate)->x();
			y[0] = (*iterate)->y();
			iterate++;
			x[1] = (*iterate)->x();
			y[1] = (*iterate)->y();
			path.curve4(x[0], y[0], x[1], y[1], cmd->m_point->x(),cmd->m_point->y());
    		break;
		}

	case L:
		if (mode == CTRL_LN)
			path.move_to(cmd->m_point->x(),cmd->m_point->y());
		else
			path.line_to(cmd->m_point->x(),cmd->m_point->y());
		break;
		
	case S:
		unsigned np = cmd->controlpoints.size();
        agg::pod_array<double> m_polygon(np * 2);
        unsigned _pn = 0;
		PointList::iterator iterate = cmd->controlpoints.begin();
		while (iterate != cmd->controlpoints.end())
		{
	        m_polygon[_pn] = (*iterate)->x(); _pn++;
	        m_polygon[_pn] = (*iterate)->y(); _pn++;
	        iterate++;
		}
        //m_polygon[_pn++] = cmd->m_point->x();
        //m_polygon[_pn++] = cmd->m_point->y();
		//path.move_to(cmd->prev->m_point->x(),cmd->prev->m_point->y());
		if (mode == CTRL_LN)
		{
			_pn = 0;
			while (_pn < np * 2)
			{
				path.line_to((int) m_polygon[_pn],(int) m_polygon[_pn + 1]);
				_pn += 2;
			}
	        path.line_to(cmd->m_point->x(), cmd->m_point->y());
		}	
		else
		{	
			//path.line_to((int) m_polygon[0],(int) m_polygon[1]);
	        aggpolygon poly(&m_polygon[0], np, false, false);
	        agg::conv_bcspline<agg::simple_polygon_vertex_source>  bspline(poly);
	        bspline.interpolation_step(0.01);
	        agg::path_storage npath;
	        npath.join_path(bspline);
	        path.join_path(npath);
	        if (mode == HILITE)
	        	path.move_to((int) m_polygon[np * 2 - 2], (int) m_polygon[np * 2 - 1] );
	        path.line_to(cmd->m_point->x(), cmd->m_point->y());
		}
		break;
	}	

}

void ASSDrawEngine::render_scanlines_aa_solid(RendererBase& rbase, agg::rgba rgba, bool affectboundaries)
{
	agg::render_scanlines_aa_solid(rasterizer, scanline, rbase, rgba);
	if (affectboundaries) update_rendered_bound_coords();
}

void ASSDrawEngine::render_scanlines(RendererSolid& rsolid, bool affectboundaries)
{
	agg::render_scanlines(rasterizer, scanline, rsolid);
	if (affectboundaries) update_rendered_bound_coords();
}

void ASSDrawEngine::update_rendered_bound_coords(bool rendered_fresh)
{
    int min_x = rasterizer.min_x();
    int min_y = rasterizer.min_y();
    int max_x = rasterizer.max_x();
    int max_y = rasterizer.max_y();
	if (min_x < rendered_min_x || rendered_fresh) rendered_min_x = min_x;
	if (min_y < rendered_min_y || rendered_fresh) rendered_min_y = min_y;
	if (max_x > rendered_max_x || rendered_fresh) rendered_max_x = max_x;
	if (max_y > rendered_max_y || rendered_fresh) rendered_max_y = max_y;
}

void ASSDrawEngine::FitToViewPoint(int hmargin, int vmargin)
{
	wxSize v = GetClientSize();
	double wide = rendered_max_x - rendered_min_x;
	double high = rendered_max_y - rendered_min_y;
	double widthratio = (double) (v.x - hmargin * 2) / wide;
	double heightratio = (double) (v.y - vmargin * 2) / high;
	double ratio = (widthratio < heightratio? widthratio:heightratio);
	pointsys->scale = pointsys->scale * ratio;
	if (pointsys->scale < 0.01) pointsys->scale = 0.01;
	double new_min_x = pointsys->originx + (rendered_min_x - pointsys->originx) * ratio;
	double new_max_x = pointsys->originx + (rendered_max_x - pointsys->originx) * ratio;
	pointsys->originx += (v.x - new_max_x + new_min_x) / 2 - new_min_x;
	double new_min_y = pointsys->originy + (rendered_min_y - pointsys->originy) * ratio;
	double new_max_y = pointsys->originy + (rendered_max_y - pointsys->originy) * ratio;
	pointsys->originy += (v.y - new_max_y + new_min_y) / 2 - new_min_y;
	RefreshDisplay();
}

void ASSDrawEngine::SetFitToViewPointOnNextPaint(int hmargin, int vmargin)
{
	if (vmargin >= 0) fitviewpoint_vmargin = vmargin;
	if (hmargin >= 0) fitviewpoint_hmargin = hmargin;
	setfitviewpoint = true;
}

std::vector< bool > ASSDrawEngine::PrepareC1ContData()
{
	std::vector< bool > out;
	for (DrawCmdList::iterator it = cmds.begin(); it != cmds.end(); it++)
	{
		bool c1 = false;
		if ((*it)->type == B)
		{
			DrawCmd_B *cmdb = static_cast<DrawCmd_B *>(*it);
			c1 = cmdb->C1Cont;
		}
		out.push_back(c1);
	}
	return out;
}

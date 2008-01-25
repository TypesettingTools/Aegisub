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
// Name:        assdraw.hpp
// Purpose:     header file for ASSDraw main source file; also includes
//              declarations for all GUI elements (except wxRuler, which has
//              its own header file)
// Author:      ai-chan
// Created:     08/26/06
// Copyright:   (c) ai-chan
// Licence:     3-clause BSD
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "_common.hpp"

#include <vector>
#include <wx/wxprec.h>
#include <wx/aui/aui.h>
#include <wx/fileconf.h>
#include <wx/help.h>

#include "canvas.hpp" // the canvas
#include "dlgctrl.hpp" // custom dialogs & controls
#include "settings.hpp" // settings property grid
#include "library.hpp" // shape library

//#define BETAVERSION 2
#define VERSION _T("3.0 final")

// this header file declares the following classes
class ASSDrawApp;
class ASSDrawFrame;
class ASSDrawCanvas;

class ASSDrawApp : public wxApp 
{ 
public:      
    bool OnInit();
};

class ASSDrawFrame : public wxFrame
{
public:
    // constructor
    ASSDrawFrame(wxApp *app, const wxString& title, const wxPoint& pos, const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_FRAME_STYLE);
    virtual ~ASSDrawFrame();

    // event handlers (these functions should _not_ be virtual)
	// OnSelect_* for single items, OnChoose_* for many-choose-one items
	void OnSelect_Clear(wxCommandEvent& WXUNUSED(event)) { _Clear(); }
	void OnSelect_Preview(wxCommandEvent& WXUNUSED(event)) { _Preview(); }
	void OnSelect_Transform(wxCommandEvent& WXUNUSED(event)) { _Transform(); }
	void OnSelect_Library(wxCommandEvent& WXUNUSED(event)) { _ToggleLibrary(); }
	void OnSelect_Settings(wxCommandEvent& WXUNUSED(event)) { _ToggleSettings(); }
	void OnSelect_ResetPerspective(wxCommandEvent& WXUNUSED(event)) { _ResetPerspective(); }
	void OnSelect_Help(wxCommandEvent& WXUNUSED(event)) { _Help(); }
	void OnSelect_About(wxCommandEvent& WXUNUSED(event)) { _About(); }
	void OnSelect_Undo(wxCommandEvent& WXUNUSED(event)) { UndoOrRedo( true ); }
	void OnSelect_Redo(wxCommandEvent& WXUNUSED(event)) { UndoOrRedo( false ); }
	void OnSelect_Paste(wxCommandEvent& WXUNUSED(event)) { _Paste(); }
	void OnSelect_RemoveBG(wxCommandEvent& WXUNUSED(event)) { m_canvas->RemoveBackgroundImage(); }
	void OnSelect_AlphaBG(wxCommandEvent& WXUNUSED(event)) { m_canvas->AskUserForBackgroundAlpha(); }
	void OnChoose_Recenter(wxCommandEvent& event);
	void OnChoose_RecenterToBG(wxCommandEvent& event);
	void OnChoose_Mode(wxCommandEvent& event);
	void OnChoose_DragMode(wxCommandEvent& event);
	void OnZoomSliderChanged(wxScrollEvent &event);
	void OnToolRClick(wxCommandEvent& event);
	void OnChoose_TBarRClickMenu(wxCommandEvent& event);
	void OnSettingsChanged(wxCommandEvent& event);
	void OnClose(wxCloseEvent &event);

	void UpdateASSCommandStringToSrcTxtCtrl(wxString cmds);
	void UpdateASSCommandStringFromSrcTxtCtrl(wxString cmds);

	void UndoOrRedo(bool isundo);
	void UpdateUndoRedoMenu();

    void _Clear();
    void _Preview();
    void _Transform();
	void _ToggleLibrary();
	void _ToggleSettings();
    void _Help();
    void _About(unsigned timeout = 0);
    void _Paste();
    void _ResetPerspective();

	void UpdateFrameUI(unsigned level = 0);

    // the canvas
    wxApp *m_app;
    ASSDrawCanvas* m_canvas;
	wxAuiManager m_mgr;
	wxString default_perspective;
	ASSDrawSrcTxtCtrl* srctxtctrl;

	// config
	wxString configfile;
	wxFileConfig *config;
	
    // toolbars
    wxToolBar *drawtbar, *modetbar, *bgimgtbar;
	
	// zoom slider
	wxSlider* zoomslider;
	
	//library
	ASSDrawShapeLibrary *shapelib;
	typedef std::vector< ASSDrawEngine* > DrawEngineVec;
	DrawEngineVec libshapes;
	
	// menus
#if wxUSE_MENUS
    wxMenu *drawMenu;
    wxMenu *modeMenu;
    wxMenu *bgimgMenu;
    wxMenu *viewMenu;
    wxMenu *tbarMenu;
#endif

	// dialogs
	ASSDrawTransformDlg* transformdlg;	
	ASSDrawSettingsDialog* settingsdlg;
	
	// colors
	struct
	{
		wxColour canvas_bg;
		wxColour canvas_shape_normal;
		wxColour canvas_shape_preview;
		wxColour canvas_shape_outline;
		wxColour canvas_shape_guideline;
		wxColour canvas_shape_mainpoint;
		wxColour canvas_shape_controlpoint;
		wxColour canvas_shape_selectpoint;
		wxColour library_shape;
		wxColour library_libarea;
		wxColour origin, ruler_h, ruler_v;
	} colors;

	struct
	{
		long canvas_shape_normal;
		long canvas_shape_preview;
		long canvas_shape_outline;
		long canvas_shape_guideline;
		long canvas_shape_mainpoint;
		long canvas_shape_controlpoint;
		long canvas_shape_selectpoint;
	} alphas;

	struct
	{
		long origincross;
	} sizes;
	
	struct
	{
		bool capitalizecmds;
		bool autoaskimgopac;
		bool parse_spc;
		bool nosplashscreen;
		bool confirmquit;
	} behaviors;
	
	void LoadSettings();
	void SaveSettings();
	void InitializeDefaultSettings();
	void ApplySettings();
	static void wxColourToAggRGBA(wxColour &colour, agg::rgba &rgba);
	static void wxColourSetAlpha(wxColour &colour, long alpha);

protected:
	virtual void SetToolBars();
	virtual void SetMenus();
	virtual void SetPanes();
	DECLARE_EVENT_TABLE()
	
	wxHelpController helpcontroller;

};

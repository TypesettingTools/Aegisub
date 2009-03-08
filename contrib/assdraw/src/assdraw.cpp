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
// Name:        assdraw.cpp
// Purpose:     ASSDraw main source file
// Author:      ai-chan
// Created:     08/26/06
// Copyright:   (c) ai-chan
// Licence:     3-clause BSD
///////////////////////////////////////////////////////////////////////////////


#include "assdraw.hpp"
#include "enums.hpp"
#include "include_once.hpp"
#include <wx/clipbrd.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/dynlib.h>
#include <wx/stdpaths.h>

#if !defined(__WINDOWS__)
#include "xpm/res.h"
#endif

/////////////
// Libraries
#ifdef __VISUALC__
#pragma comment(lib, "Vfw32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "wsock32.lib")
#ifdef __WXDEBUG__
#pragma comment(lib, "wxmsw28ud_propgrid.lib")
#else
#pragma comment(lib, "wxmsw28u_propgrid.lib")
#endif
#endif


//DEFINE_EVENT_TYPE(wxEVT_SETTINGS_CHANGED)

// initialize wxWidget to accept our App class
IMPLEMENT_APP(ASSDrawApp)

BEGIN_EVENT_TABLE(ASSDrawFrame, wxFrame)
    EVT_TOOL(TB_CLEAR, ASSDrawFrame::OnSelect_Clear)
    EVT_TOOL(TB_PREVIEW, ASSDrawFrame::OnSelect_Preview)
    //EVT_TOOL(TB_EDITSRC, ASSDrawFrame::OnSelect_EditSrc)
    EVT_TOOL(TB_TRANSFORM, ASSDrawFrame::OnSelect_Transform)
    EVT_TOOL_RANGE(MODE_ARR, MODE_DEL, ASSDrawFrame::OnChoose_Mode)
    EVT_TOOL_RANGE(DRAG_DWG, DRAG_BOTH, ASSDrawFrame::OnChoose_DragMode)
    EVT_TOOL_RCLICKED(wxID_ANY, ASSDrawFrame::OnToolRClick)
    EVT_COMMAND(wxID_ANY, wxEVT_SETTINGS_CHANGED, ASSDrawFrame::OnSettingsChanged) 
    EVT_MENU_RANGE(MENU_TB_ALL, MENU_TB_BGIMG, ASSDrawFrame::OnChoose_TBarRClickMenu)
#if wxUSE_MENUS
    EVT_MENU(MENU_CLEAR, ASSDrawFrame::OnSelect_Clear)
    EVT_MENU(MENU_PREVIEW, ASSDrawFrame::OnSelect_Preview)
    EVT_MENU(MENU_TRANSFORM, ASSDrawFrame::OnSelect_Transform)
    EVT_MENU(MENU_LIBRARY, ASSDrawFrame::OnSelect_Library)
    EVT_MENU(MENU_SETTINGS, ASSDrawFrame::OnSelect_Settings)
    EVT_MENU(MENU_RESETPERSPECTIVE, ASSDrawFrame::OnSelect_ResetPerspective)
    EVT_MENU(MENU_HELP, ASSDrawFrame::OnSelect_Help)
    EVT_MENU(wxID_ABOUT, ASSDrawFrame::OnSelect_About)
    EVT_MENU(MENU_UNDO, ASSDrawFrame::OnSelect_Undo)
    EVT_MENU(MENU_REDO, ASSDrawFrame::OnSelect_Redo)
    EVT_MENU(MENU_PASTE, ASSDrawFrame::OnSelect_Paste)
    EVT_MENU(MENU_BGIMG_REMOVE, ASSDrawFrame::OnSelect_RemoveBG)
    EVT_MENU(MENU_BGIMG_ALPHA, ASSDrawFrame::OnSelect_AlphaBG)
    EVT_MENU_RANGE(MODE_ARR, MODE_NUT_BILINEAR, ASSDrawFrame::OnChoose_Mode)
    EVT_MENU_RANGE(DRAG_DWG, DRAG_BOTH, ASSDrawFrame::OnChoose_DragMode)
    EVT_MENU_RANGE(MENU_REPOS_TOPLEFT, MENU_REPOS_BOTRIGHT, ASSDrawFrame::OnChoose_Recenter)
    EVT_MENU_RANGE(MENU_REPOS_BGTOPLEFT, MENU_REPOS_BGBOTRIGHT, ASSDrawFrame::OnChoose_RecenterToBG)
#endif //wxUSE_MENUS
	EVT_CLOSE(ASSDrawFrame::OnClose)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// the application class: ASSDrawApp
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution _T("starts") here
bool ASSDrawApp::OnInit()
{
    // create the main application window
    ASSDrawFrame * assdrawframe = new ASSDrawFrame( this, TITLE, wxDefaultPosition, wxSize(640, 480) );
	SetTopWindow(assdrawframe);
    return TRUE;
}



// ----------------------------------------------------------------------------
// main frame: ASSDrawFrame
// ----------------------------------------------------------------------------

// constructor

ASSDrawFrame::ASSDrawFrame( wxApp *app, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
       : wxFrame(NULL, wxID_ANY, title, pos, size, style)
{
	m_app = app;
    m_mgr.SetManagedWindow(this);
    m_mgr.SetFlags(m_mgr.GetFlags() | wxAUI_MGR_ALLOW_ACTIVE_PANE);

    // set the frame icon
    SetIcon(wxICON(appico));
    
   	// Create status bar for the frame
	#if wxUSE_STATUSBAR
    CreateStatusBar(3);
    int statwidths[] = { 64, -1, 64 };
    GetStatusBar()->SetStatusWidths(3, statwidths);
    SetStatusBarPane(1);
	#endif // wxUSE_STATUSBAR
	
	InitializeDefaultSettings();
	
	// load config
	configfile = wxFileName(::wxGetCwd(), _T("ASSDraw3.cfg")).GetFullPath();
	if (!::wxFileExists(configfile))
		configfile = wxFileName(wxStandardPaths::Get().GetUserConfigDir(), _T("ASSDraw3.cfg")).GetFullPath();

	bool firsttime = !::wxFileExists(configfile);
	if (firsttime) wxFileOutputStream(configfile).Close();
	wxFileInputStream cfgf(configfile);
	config = new wxFileConfig(cfgf);

	// nullify transformdlg
	transformdlg = NULL;

	Maximize(true);
	Show(true); // to get the right client size, must call Show() first

	// config
	LoadSettings();

	// THE CANVAS
    m_canvas = new ASSDrawCanvas( this , this );

	// shapes library
	shapelib = new ASSDrawShapeLibrary(this, this);

	// source text ctrl
	srctxtctrl = new ASSDrawSrcTxtCtrl(this, this);

	// settings
	/*
	settingsdlg = NULL;
	wxString settingsdllfile = wxFileName(::wxGetCwd(), _T("settings.dll")).GetFullPath();

	if (::wxFileExists(settingsdllfile))
	{
		wxDynamicLibrary settingsdll(settingsdllfile);
		wxString symbol(_T("CreateASSDrawSettingsDialogInstance"));
		if (settingsdll.IsLoaded() && settingsdll.HasSymbol(symbol)) 
		{
			typedef ASSDrawSettingsDialog* (*FuncType)(wxWindow*,ASSDrawFrame*,int);
			FuncType func = (FuncType) settingsdll.GetSymbol(symbol);
			//wxDYNLIB_FUNCTION(ASSDrawSettingsDialog, CreateASSDrawSettingsDialogInstance, settingsdll)
			//ASSDrawSettingsDialog* test = func(NULL,this,971231);
			//test->Reparent(this);
			//test->Init();
			//SetTitle(settingsdllfile);
			//settingsdlg = func(NULL,this, 809131);
			//settingsdlg->Init();
			//m_mgr.AddPane(settingsdlg, wxAuiPaneInfo().Name(_T("settings")).Caption(_T("Settings")).Right().Layer(3).Position(0).CloseButton(true).BestSize(wxSize(240, 480)).MinSize(wxSize(200, 200)));
		}
	}
	*/
	
	settingsdlg = new ASSDrawSettingsDialog(this, this);
	settingsdlg->Init();
	
	SetMenus();
	SetToolBars();
	SetPanes();

	// config

	config->SetPath(_T("info"));
	wxString version;
	config->Read(_T("version"), &version);
	config->SetPath(_T(".."));

	default_perspective = m_mgr.SavePerspective(); // back up default perspective
	config->SetPath(_T("perspective"));
	wxString perspective;
	if (config->Read(_T("perspective"), &perspective) && version == VERSION) m_mgr.LoadPerspective(perspective, false);
	config->SetPath(_T(".."));

	config->SetPath(_T("library"));
	int n = 0;
	config->Read(_T("n"), &n);
	for (int i = 0; i < n; i++)
	{
		wxString libcmds;
		config->Read(wxString::Format(_T("%d"),i), &libcmds);
		shapelib->AddShapePreview(libcmds);		
	}
	config->SetPath(_T(".."));
	
    m_mgr.Update();
	m_canvas->SetFocus();
	m_canvas->Show();
	
	wxSize clientsize = m_canvas->GetClientSize();
	m_canvas->ChangeZoomLevelTo(DEFAULT_SCALE, wxPoint(clientsize.x / 2, clientsize.y / 2));
	m_canvas->MoveCanvasOriginTo(clientsize.x / 2, clientsize.y / 2);
	UpdateASSCommandStringToSrcTxtCtrl(m_canvas->GenerateASS());

	UpdateFrameUI();
	ApplySettings();

	#ifdef BETAVERSION
	wxDateTime expire(15, wxDateTime::Dec, 2007, 0, 0, 0);
	wxDateTime now = wxDateTime::Now();
	if (now.IsLaterThan(expire))
	{
	 	wxMessageDialog expired(this, _T("Thank you for trying ASSDraw3. This beta version has expired. Please visit http://malakith.net/aegisub/index.php?topic=912.0 to get the latest release. Visit now?"), _T("Beta version"), wxYES_NO | wxICON_INFORMATION);
	 	if (expired.ShowModal() == wxID_YES)
			::wxLaunchDefaultBrowser(wxString(_T("http://malakith.net/aegisub/index.php?topic=912.0")));
		Close();
	}
	SetTitle(wxString::Format(_T("%s beta %d (expires %s)"), TITLE, BETAVERSION, expire.FormatDate().c_str()));
	#endif

	if (firsttime)
		_About();
	else if (!behaviors.nosplashscreen)
		_About(3);

	helpcontroller.SetParentWindow(this);
	helpcontroller.Initialize(wxFileName(::wxGetCwd(), _T("ASSDraw3.chm")).GetFullPath());
}

void ASSDrawFrame::SetToolBars()
{
    drawtbar = new wxToolBar(this, wxID_ANY, __DPDS__ , wxTB_FLAT | wxTB_TEXT | wxTB_NODIVIDER | wxTB_HORIZONTAL);
	drawtbar->AddTool(TB_CLEAR, _T("Clear"), wxBITMAP(new_), wxNullBitmap, wxITEM_NORMAL, _T(""), TIPS_CLEAR);
    //tbar->AddTool(TB_EDITSRC, _T("Source"), wxBITMAP(src_), wxNullBitmap, wxITEM_NORMAL, _T(""), TIPS_EDITSRC);
    drawtbar->AddCheckTool(TB_PREVIEW, _T("Preview"), wxBITMAP(preview_), wxNullBitmap, _T(""), TIPS_PREVIEW);
    //drawtbar->AddTool(TB_TRANSFORM, _T("Transform"), wxBITMAP(rot_), wxNullBitmap, wxITEM_NORMAL, _T(""), TIPS_TRANSFORM);
	zoomslider = new wxSlider(drawtbar, TB_ZOOMSLIDER, 1000, 100, 5000, __DPDS__ );
	//zoomslider->SetSize(280, zoomslider->GetSize().y);
	zoomslider->Connect(wxEVT_SCROLL_LINEUP, wxScrollEventHandler(ASSDrawFrame::OnZoomSliderChanged), NULL, this);
	zoomslider->Connect(wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler(ASSDrawFrame::OnZoomSliderChanged), NULL, this);
	zoomslider->Connect(wxEVT_SCROLL_PAGEUP, wxScrollEventHandler(ASSDrawFrame::OnZoomSliderChanged), NULL, this);
	zoomslider->Connect(wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler(ASSDrawFrame::OnZoomSliderChanged), NULL, this);
	zoomslider->Connect(wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(ASSDrawFrame::OnZoomSliderChanged), NULL, this);
	zoomslider->Connect(wxEVT_SCROLL_CHANGED, wxScrollEventHandler(ASSDrawFrame::OnZoomSliderChanged), NULL, this);
	drawtbar->AddControl(zoomslider);
    drawtbar->Realize();

    m_mgr.AddPane(drawtbar, wxAuiPaneInfo().Name(_T("drawtbar")).Caption(TBNAME_DRAW).
                  ToolbarPane().Top().Position(0).Dockable(true).LeftDockable(false).RightDockable(false));

    modetbar = new wxToolBar(this, wxID_ANY, __DPDS__ , wxTB_FLAT | wxTB_TEXT | wxTB_NODIVIDER | wxTB_HORIZONTAL);
    modetbar->AddRadioTool(MODE_ARR, _T("Drag"), wxBITMAP(arr_), wxNullBitmap, _T(""), TIPS_ARR);
    modetbar->AddRadioTool(MODE_M, _T("Move"), wxBITMAP(m_), wxNullBitmap, _T(""), TIPS_M);
    //modetbar->AddRadioTool(MODE_N, _T("Move*"), wxBITMAP(n_), wxNullBitmap, _T(""), TIPS_N);
    modetbar->AddRadioTool(MODE_L, _T("Line"), wxBITMAP(l_), wxNullBitmap, _T(""), TIPS_L);
    modetbar->AddRadioTool(MODE_B, _T("Bezier"), wxBITMAP(b_), wxNullBitmap, _T(""), TIPS_B);
    //modetbar->AddRadioTool(MODE_S, _T("Spline"), wxBITMAP(s_), wxNullBitmap, _T(""), TIPS_S);
    //modetbar->AddRadioTool(MODE_P, _T("Extend"), wxBITMAP(p_), wxNullBitmap, _T(""), TIPS_P);
    //modetbar->AddRadioTool(MODE_C, _T("Close"), wxBITMAP(c_), wxNullBitmap, _T(""), TIPS_C);
    modetbar->AddRadioTool(MODE_DEL, _T("Delete"), wxBITMAP(del_), wxNullBitmap, _T(""), TIPS_DEL);
    modetbar->AddRadioTool(MODE_SCALEROTATE, _T("Scale/Rotate"), wxBITMAP(sc_rot_), wxNullBitmap, _T(""), TIPS_SCALEROTATE);
    modetbar->AddRadioTool(MODE_NUT_BILINEAR, _T("Bilinear"), wxBITMAP(nut_), wxNullBitmap, _T(""), TIPS_NUTB);
    //modetbar->AddRadioTool(MODE_NUT_PERSPECTIVE, _T("NUT:P"), wxBITMAP(arr_), wxNullBitmap, _T(""), _T(""));
    modetbar->Realize();

    m_mgr.AddPane(modetbar, wxAuiPaneInfo().Name(_T("modetbar")).Caption(TBNAME_MODE).
                  ToolbarPane().Top().Position(1).Dockable(true).LeftDockable(false).RightDockable(false));

    bgimgtbar = new wxToolBar(this, wxID_ANY, __DPDS__ , wxTB_FLAT | wxTB_TEXT | wxTB_NODIVIDER | wxTB_HORIZONTAL);
	bgimgtbar->SetToolBitmapSize(wxSize(24,15));
    bgimgtbar->AddCheckTool(DRAG_DWG, _T("Pan drawing"), wxBITMAP(pan_shp), wxNullBitmap, _T(""), TIPS_DWG);
    bgimgtbar->AddCheckTool(DRAG_BGIMG, _T("Pan background"), wxBITMAP(pan_bg), wxNullBitmap, _T(""), TIPS_BGIMG);
    //bgimgtbar->AddRadioTool(DRAG_BOTH, _T("Pan both"), wxBITMAP(pan_both), wxNullBitmap, _T(""), TIPS_BOTH);
    bgimgtbar->Realize();

    m_mgr.AddPane(bgimgtbar, wxAuiPaneInfo().Name(_T("bgimgtbar")).Caption(TBNAME_BGIMG).
                  ToolbarPane().Top().Position(2).Dockable(true).LeftDockable(false).RightDockable(false));
	
}

void ASSDrawFrame::SetMenus()
{
#if wxUSE_MENUS
	drawMenu = new wxMenu;
	drawMenu->Append(MENU_CLEAR, _T("&Clear\tCtrl+N"), TIPS_CLEAR);
	//drawMenu->Append(MENU_EDITSRC, _T("&Source"), TIPS_EDITSRC);
	drawMenu->Append(MENU_PREVIEW, _T("&Preview\tCtrl+P"), TIPS_PREVIEW, wxITEM_CHECK);
	drawMenu->Append(MENU_TRANSFORM, _T("&Transform"), TIPS_TRANSFORM);
	drawMenu->Append(MENU_PASTE, _T("&Paste\tCtrl+V"), TIPS_PASTE);
	drawMenu->AppendSeparator();
	drawMenu->Append(MENU_UNDO, _T("&Undo\tCtrl+Z"), TIPS_UNDO);
	drawMenu->Append(MENU_REDO, _T("&Redo\tCtrl+Y"), TIPS_REDO);
	drawMenu->Enable(MENU_UNDO, false);
	drawMenu->Enable(MENU_REDO, false);

	modeMenu = new wxMenu;
	modeMenu->Append(MODE_ARR, _T("D&rag\tF1"), TIPS_ARR, wxITEM_RADIO);
	modeMenu->Append(MODE_M, _T("Draw &M\tF2"), TIPS_M, wxITEM_RADIO);
	modeMenu->Append(MODE_L, _T("Draw &L\tF3"), TIPS_L, wxITEM_RADIO);
	modeMenu->Append(MODE_B, _T("Draw &B\tF4"), TIPS_B, wxITEM_RADIO);
	modeMenu->Append(MODE_DEL, _T("&Delete\tF5"), TIPS_DEL, wxITEM_RADIO);
	modeMenu->Append(MODE_SCALEROTATE, _T("&Scale/Rotate\tF6"), TIPS_NUTB, wxITEM_RADIO);
	modeMenu->Append(MODE_NUT_BILINEAR, _T("&Bilinear transformation\tF7"), TIPS_SCALEROTATE, wxITEM_RADIO);

	bgimgMenu = new wxMenu;
	bgimgMenu->Append(DRAG_DWG, _T("Pan/Zoom &Drawing\tShift+F1"), TIPS_DWG, wxITEM_CHECK);
	bgimgMenu->Append(DRAG_BGIMG, _T("Pan/Zoom Back&ground\tShift+F2"), TIPS_BGIMG, wxITEM_CHECK);
	bgimgMenu->AppendSeparator();
	bgimgMenu->Append(MENU_BGIMG_ALPHA, _T("Set background image opacity"), _T(""));
	wxMenu* reposbgMenu = new wxMenu;
	reposbgMenu->Append( MENU_REPOS_BGTOPLEFT, _T("Top left\tCtrl+Shift+7") );
	reposbgMenu->Append( MENU_REPOS_BGTOPRIGHT, _T("Top right\tCtrl+Shift+9") );
	reposbgMenu->Append( MENU_REPOS_BGCENTER, _T("&Center\tCtrl+Shift+5") );
	reposbgMenu->Append( MENU_REPOS_BGBOTLEFT, _T("Bottom left\tCtrl+Shift+1") );
	reposbgMenu->Append( MENU_REPOS_BGBOTRIGHT, _T("Bottom right\tCtrl+Shift+3") );
	bgimgMenu->Append(MENU_BGIMG_RECENTER, _T("Reposition [&0, 0]"), reposbgMenu);
	bgimgMenu->Append(MENU_BGIMG_REMOVE, _T("Remove background\tShift+Del"), _T(""));

	wxMenu* reposMenu = new wxMenu;
	reposMenu->Append( MENU_REPOS_TOPLEFT, _T("Top left\tCtrl+7") );
	reposMenu->Append( MENU_REPOS_TOPRIGHT, _T("Top right\tCtrl+9") );
	reposMenu->Append( MENU_REPOS_CENTER, _T("&Center\tCtrl+5") );
	reposMenu->Append( MENU_REPOS_BOTLEFT, _T("Bottom left\tCtrl+1") );
	reposMenu->Append( MENU_REPOS_BOTRIGHT, _T("Bottom right\tCtrl+3") );

	tbarMenu = new wxMenu;
	tbarMenu->AppendCheckItem(MENU_TB_DRAW, TBNAME_DRAW);
	tbarMenu->AppendCheckItem(MENU_TB_MODE, TBNAME_MODE);
	tbarMenu->AppendCheckItem(MENU_TB_BGIMG, TBNAME_BGIMG);
	tbarMenu->AppendSeparator();
	tbarMenu->Append(MENU_TB_ALL, _T("Show all"));
	tbarMenu->Append(MENU_TB_NONE, _T("Hide all"));
	tbarMenu->Append(MENU_TB_DOCK, _T("Dock all"));
	tbarMenu->Append(MENU_TB_UNDOCK, _T("Undock all"));

	viewMenu = new wxMenu;
	viewMenu->Append(MENU_LIBRARY, _T("&Library"), TIPS_LIBRARY, wxITEM_CHECK);
	if (settingsdlg)
		viewMenu->Append(MENU_SETTINGS, _T("&Settings"), _T(""), wxITEM_CHECK);
	viewMenu->Append(MENU_TBAR, _T("&Toolbars"), tbarMenu);
	viewMenu->Append(MENU_RECENTER, _T("Reposition [&0, 0]"), reposMenu);
	viewMenu->AppendSeparator();
	viewMenu->Append(MENU_RESETPERSPECTIVE, _T("&Reset workspace"));

	wxMenu* helpMenu = new wxMenu;
	helpMenu->Append(MENU_HELP, _T("&Manual"));
	helpMenu->Append(wxID_ABOUT, _T("&About"));

	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(drawMenu, _T("&Canvas"));
	menuBar->Append(modeMenu, _T("&Mode"));
	menuBar->Append(bgimgMenu, _T("&Background"));
	menuBar->Append(viewMenu, _T("&Workspace"));
	menuBar->Append(helpMenu, _T("&Help"));


	SetMenuBar(menuBar);
#endif // wxUSE_MENUS
}

void ASSDrawFrame::SetPanes()
{
	m_mgr.AddPane(shapelib, wxAuiPaneInfo().Name(_T("library")).Caption(_T("Shapes Library")).
                  Right().Layer(2).Position(0).CloseButton(true).BestSize(wxSize(120, 480)).MinSize(wxSize(100, 200)));

	m_mgr.AddPane(m_canvas, wxAuiPaneInfo().Name(_T("canvas")).CenterPane());

	m_mgr.AddPane(srctxtctrl, wxAuiPaneInfo().Name(_T("commands")).Caption(_T("Drawing commands")).
                  Bottom().Layer(1).CloseButton(false).BestSize(wxSize(320, 48)));
	
	if (settingsdlg)
		m_mgr.AddPane(settingsdlg, wxAuiPaneInfo().Name(_T("settings")).Caption(_T("Settings")).
                  Right().Layer(3).Position(0).CloseButton(true).BestSize(wxSize(240, 480)).MinSize(wxSize(200, 200)).Show(false));
}

ASSDrawFrame::~ASSDrawFrame()
{
	config->SetPath(_T("info"));
	config->Write(_T("assdraw3.exe"), wxStandardPaths::Get().GetExecutablePath());
	config->Write(_T("version"), VERSION);
	config->SetPath(_T(".."));

	SaveSettings();
	
	config->SetPath(_T("perspective"));
	config->Write(_T("perspective"), m_mgr.SavePerspective());
	config->SetPath(_T(".."));

	config->DeleteGroup(_T("library"));
	config->SetPath(_T("library"));
	typedef std::vector< ASSDrawShapePreview *> PrevVec;
	PrevVec shapes = shapelib->GetShapePreviews();
	int n = shapes.size();
	config->Write(_T("n"), n);
	for (int i = 0; i < n; i++)
		config->Write(wxString::Format(_T("%d"),i), shapes[i]->GenerateASS());
	config->SetPath(_T(".."));

	wxFileOutputStream cfgf(configfile);
	config->Save(cfgf);
	delete config;
	
	if (settingsdlg) settingsdlg->Destroy(); // we need this since wxPropertyGrid must be Clear()ed before deleting

	m_mgr.UnInit();
}

void ASSDrawFrame::_Clear()
{
	wxMessageDialog msgb(this, _T("Clear the canvas and create a new drawing?"), 
                  _T("Confirmation"), wxOK | wxCANCEL | wxICON_QUESTION );
	if (msgb.ShowModal() == wxID_OK)
	{
		m_canvas->RefreshUndocmds();
		m_canvas->AddUndo(_T("Clear canvas"));
		m_canvas->ResetEngine(true);
	    wxSize siz = m_canvas->GetClientSize();
		m_canvas->ChangeZoomLevelTo(DEFAULT_SCALE, wxPoint(siz.x / 2, siz.y / 2));
		m_canvas->MoveCanvasOriginTo(siz.x / 2, siz.y / 2);
		UpdateUndoRedoMenu();
		m_canvas->RefreshDisplay();
	}
}

void ASSDrawFrame::_Preview()
{
	m_canvas->SetPreviewMode( !m_canvas->IsPreviewMode() );
	UpdateFrameUI();
	m_canvas->RefreshDisplay();
}

void ASSDrawFrame::_ToggleLibrary()
{
	m_mgr.GetPane(shapelib).Show(!m_mgr.GetPane(shapelib).IsShown());
	m_mgr.Update();
	UpdateFrameUI();
}

void ASSDrawFrame::_ToggleSettings()
{
	if (settingsdlg == NULL) return;
	m_mgr.GetPane(settingsdlg).Show(!m_mgr.GetPane(settingsdlg).IsShown());
	m_mgr.Update();
	UpdateFrameUI();
}

void ASSDrawFrame::_ResetPerspective()
{
	m_mgr.LoadPerspective(default_perspective, false);
	UpdateFrameUI();
	m_mgr.Update();
	DRAGMODE bck = m_canvas->GetDragMode();
	if (m_canvas->HasBackgroundImage()) m_canvas->SetDragMode(DRAGMODE(true, true));
	wxSize clientsize = m_canvas->GetClientSize();
	m_canvas->ChangeZoomLevelTo(DEFAULT_SCALE, wxPoint(clientsize.x / 2, clientsize.y / 2));
	m_canvas->MoveCanvasOriginTo(clientsize.x / 2, clientsize.y / 2);
	m_canvas->SetDragMode(bck);
	UpdateFrameUI();
}

void ASSDrawFrame::_Transform()
{
	if (transformdlg == NULL)
		transformdlg = new ASSDrawTransformDlg( this );

	if (transformdlg->ShowModal() == wxID_OK)
	{
		m_canvas->Transform(
			transformdlg->xformvals.f1,
			transformdlg->xformvals.f2,
			transformdlg->xformvals.f3,
			transformdlg->xformvals.f4,
			transformdlg->xformvals.f5,
			transformdlg->xformvals.f6,
			transformdlg->xformvals.f7,
			transformdlg->xformvals.f8 );
		m_canvas->AddUndo(_T("Transform"));
		m_canvas->RefreshDisplay();
		UpdateUndoRedoMenu();
	}
}

void ASSDrawFrame::_Paste()
{
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported( wxDF_BITMAP ))
		{
			wxBitmapDataObject data;
			wxTheClipboard->GetData( data );
			m_canvas->SetBackgroundImage( data.GetBitmap().ConvertToImage() );
			//m_canvas->AskUserForBackgroundAlpha();
		}
		else if (wxTheClipboard->IsSupported( wxDF_TEXT ))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData( data );
			UpdateASSCommandStringToSrcTxtCtrl( data.GetText() );
		}
		wxTheClipboard->Close();
	}
}

void ASSDrawFrame::OnChoose_Recenter(wxCommandEvent& event)
{
	int x = 0, y = 0;
	wxSize f = m_canvas->GetClientSize();

	switch (event.GetId())
	{
	case MENU_REPOS_TOPLEFT: x = 0, y = 0; break;
	case MENU_REPOS_TOPRIGHT: x = f.x, y = 0; break;
	case MENU_REPOS_CENTER: x = f.x / 2, y = f.y / 2; break;
	case MENU_REPOS_BOTLEFT: x = 0, y = f.y; break;
	case MENU_REPOS_BOTRIGHT: x = f.x, y = f.y; break;
	}

	m_canvas->MoveCanvasOriginTo(x, y);
	m_canvas->RefreshDisplay();
}

void ASSDrawFrame::OnChoose_RecenterToBG(wxCommandEvent& event)
{
	unsigned w, h;
	wxRealPoint disp;
	double scale;
	if (m_canvas->GetBackgroundInfo(w, h, disp, scale))
	{
		int x = 0, y = 0;
		int lx = (int)disp.x, ty = (int)disp.y;
		int rx = lx + (int)(w * scale);
		int by = ty + (int)(h * scale);
		
		switch (event.GetId())
		{
		case MENU_REPOS_BGTOPLEFT: x = lx, y = ty; break;
		case MENU_REPOS_BGTOPRIGHT: x = rx, y = ty; break;
		case MENU_REPOS_BGCENTER: x = (rx + lx) / 2, y = (by + ty) / 2; break;
		case MENU_REPOS_BGBOTLEFT: x = lx, y = by; break;
		case MENU_REPOS_BGBOTRIGHT: x = rx, y = by; break;
		}
	
		m_canvas->MoveCanvasDrawing(x - m_canvas->GetOriginX(), y - m_canvas->GetOriginY());
		m_canvas->RefreshDisplay();
	}
}

void ASSDrawFrame::_Help()
{
	helpcontroller.DisplayContents();
}

void ASSDrawFrame::_About(unsigned timeout)
{
	ASSDrawAboutDlg *aboutdlg = new ASSDrawAboutDlg( this, timeout );
	aboutdlg->ShowModal();
	aboutdlg->Destroy();
}

void ASSDrawFrame::OnChoose_Mode(wxCommandEvent& event)
{
	m_canvas->SetDrawMode( (MODE) event.GetId() );
	UpdateFrameUI();
}

void ASSDrawFrame::OnChoose_DragMode(wxCommandEvent& event)
{
	DRAGMODE dm = m_canvas->GetDragMode();
	switch (event.GetId())
	{
		case DRAG_DWG: dm.drawing = !dm.drawing; break;
		case DRAG_BGIMG: dm.bgimg = !dm.bgimg; break;
	}
	m_canvas->SetDragMode( dm );
	UpdateFrameUI();
}

void ASSDrawFrame::OnZoomSliderChanged(wxScrollEvent &event)
{
	double zoom = (double) event.GetPosition() / 100.0;
	m_canvas->ChangeZoomLevelTo(zoom, wxPoint((int) m_canvas->GetOriginX(), (int) m_canvas->GetOriginY()));
}

void ASSDrawFrame::OnToolRClick(wxCommandEvent& event)
{
	int id = event.GetId();
	if (drawtbar->FindById(id) != NULL
		|| modetbar->FindById(id) != NULL
		|| bgimgtbar->FindById(id) != NULL)
	{
		PopupMenu(tbarMenu);
	}
}

void ASSDrawFrame::OnChoose_TBarRClickMenu(wxCommandEvent& event)
{
	int id = event.GetId();
	wxToolBar* tbar[3] = { drawtbar, modetbar, bgimgtbar };
	bool tb[3] = { false, false, false };
	bool show[2] = { false, true };
	bool dock[2] = { false, true };
	switch (id)
	{
	case MENU_TB_ALL: 
		tb[0] = true, tb[1] = true, tb[2] = true;
		show[0] = true, show[1] = true;
		break;
	case MENU_TB_NONE: 
		tb[0] = true, tb[1] = true, tb[2] = true;
		show[0] = true, show[1] = false;
		break;
	case MENU_TB_DOCK: 
		tb[0] = true, tb[1] = true, tb[2] = true;
		dock[0] = true, dock[1] = true;
		break;
	case MENU_TB_UNDOCK: 
		tb[0] = true, tb[1] = true, tb[2] = true;
		dock[0] = true, dock[1] = false;
		break;
	case MENU_TB_DRAW: 
		tb[0] = true;
		show[0] = true, show[1] = !m_mgr.GetPane(tbar[0]).IsShown();
		break;
	case MENU_TB_MODE: 
		tb[1] = true;
		show[0] = true, show[1] = !m_mgr.GetPane(tbar[1]).IsShown();
		break;
	case MENU_TB_BGIMG: 
		tb[2] = true;
		show[0] = true, show[1] = !m_mgr.GetPane(tbar[2]).IsShown();
		break;
	}
	for (int i = 0; i < 3; i++)
	{
		if (tb[i])
		{
			if (show[0])
				m_mgr.GetPane(tbar[i]).Show(show[1]);
			if (dock[0])
				if (dock[1])
					m_mgr.GetPane(tbar[i]).Dock();
				else
					m_mgr.GetPane(tbar[i]).Float();
		}
	}
	m_mgr.Update();
	UpdateFrameUI();
}

void ASSDrawFrame::UpdateASSCommandStringFromSrcTxtCtrl(wxString cmds)
{
	m_canvas->ParseASS(cmds, true);
	m_canvas->RefreshDisplay();
}

void ASSDrawFrame::UpdateASSCommandStringToSrcTxtCtrl(wxString cmd)
{
	if (behaviors.capitalizecmds)
		cmd.UpperCase();
	else
		cmd.LowerCase();
	srctxtctrl->ChangeValue(cmd);
	//srctxtctrl->AppendText(cmd);
}

void ASSDrawFrame::UndoOrRedo(bool isundo)
{
	if (isundo)
		m_canvas->Undo();
	else
		m_canvas->Redo();
	UpdateUndoRedoMenu();
	UpdateFrameUI();
}

void ASSDrawFrame::UpdateUndoRedoMenu()
{
	wxString nextUndo = m_canvas->GetTopUndo();
	if (nextUndo.IsSameAs(_T("")))
	{
		drawMenu->SetLabel(MENU_UNDO, _T("Undo\tCtrl+Z"));
		drawMenu->Enable(MENU_UNDO, false);
	}
	else
	{
		drawMenu->SetLabel(MENU_UNDO, wxString::Format(_T("Undo: %s\tCtrl+Z"), nextUndo.c_str()));
		drawMenu->Enable(MENU_UNDO, true);
	}
	wxString nextRedo = m_canvas->GetTopRedo();
	if (nextRedo.IsSameAs(_T("")))
	{
		drawMenu->SetLabel(MENU_REDO, _T("Redo\tCtrl+Y"));
		drawMenu->Enable(MENU_REDO, false);
	}
	else
	{
		drawMenu->SetLabel(MENU_REDO, wxString::Format(_T("Redo: %s\tCtrl+Y"), nextRedo.c_str()));
		drawMenu->Enable(MENU_REDO, true);
	}
}

void ASSDrawFrame::UpdateFrameUI(unsigned level)
{
	bool hasbg = m_canvas->HasBackgroundImage();
	int zoom = (int) ((m_canvas->GetScale() * 100.0)+0.5);
	switch (level)
	{
	case 0: // all
		drawtbar->ToggleTool(TB_PREVIEW, m_canvas->IsPreviewMode());
		modetbar->ToggleTool(m_canvas->GetDrawMode(), true);
		#if wxUSE_MENUS
		drawMenu->Check(MENU_PREVIEW, m_canvas->IsPreviewMode());
		modeMenu->Check(m_canvas->GetDrawMode(), true);
		#endif
	case 2: // bgimg & toolbars
		//bgimgtbar->ToggleTool(m_canvas->GetDragMode(), true);
		bgimgtbar->ToggleTool(DRAG_DWG, m_canvas->GetDragMode().drawing);
		bgimgtbar->ToggleTool(DRAG_BGIMG, m_canvas->GetDragMode().bgimg);
		bgimgtbar->EnableTool(DRAG_BGIMG, hasbg);
		//m_mgr.GetPane(bgimgtbar).Show(hasbg);
		m_mgr.Update();
		#if wxUSE_MENUS
		viewMenu->Check(MENU_LIBRARY, m_mgr.GetPane(shapelib).IsShown());
		if (settingsdlg)
			viewMenu->Check(MENU_SETTINGS, m_mgr.GetPane(settingsdlg).IsShown());
		//bgimgMenu->Check(m_canvas->GetDragMode(), true);
		bgimgMenu->Check(DRAG_DWG, m_canvas->GetDragMode().drawing);
		bgimgMenu->Check(DRAG_BGIMG, m_canvas->GetDragMode().bgimg);
		bgimgMenu->Enable(DRAG_BGIMG, hasbg);
		bgimgMenu->Enable(DRAG_BOTH, hasbg);
		bgimgMenu->Enable(MENU_BGIMG_ALPHA, hasbg);
		bgimgMenu->Enable(MENU_BGIMG_RECENTER, hasbg);
		bgimgMenu->Enable(MENU_BGIMG_REMOVE, hasbg);
		tbarMenu->Check(MENU_TB_DRAW, m_mgr.GetPane(drawtbar).IsShown());
		tbarMenu->Check(MENU_TB_MODE, m_mgr.GetPane(modetbar).IsShown());
		tbarMenu->Check(MENU_TB_BGIMG, m_mgr.GetPane(bgimgtbar).IsShown());
		#endif
	case 3:	// zoom slider
		zoomslider->SetValue(zoom);
		SetStatusText( wxString::Format(_T("%d%%"), zoom), 2 );
		zoomslider->Enable(m_canvas->GetDragMode().drawing && m_canvas->CanZoom());
	}
}

void ASSDrawFrame::OnClose(wxCloseEvent &event)
{
	if (event.CanVeto() && behaviors.confirmquit)
	{
		if (wxMessageDialog(this, _T("Do you want to close ASSDraw3 now?"), _T("Confirmation"), wxOK | wxCANCEL).ShowModal() == wxID_OK)
			Destroy();	
		else
			event.Veto();
	}
	else
		Destroy();	
}

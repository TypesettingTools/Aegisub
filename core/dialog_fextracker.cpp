// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include <wx/notebook.h>
#include "dialog_fextracker.h"
#include "../FexTrackerSource/FexTracker.h"


///////////////
// Constructor
DialogFexTracker::DialogFexTracker(wxWindow *parent, FexTrackerConfig *_cfg)
: wxDialog (parent,-1,_("Tracker configuration"),wxDefaultPosition)
{
	cfg = _cfg;
	cfg->FeatureNumber = 0;

	wxNotebook *MainNB = new wxNotebook(this,-1, wxDefaultPosition, wxSize(300,500), wxNO_BORDER );

	wxWindow *StdWnd = new wxPanel(MainNB,-1);
	wxWindow *AdvWnd = new wxPanel(MainNB,-1);

	FeatureNumber = new wxTextCtrl(StdWnd,-1,_T("250"));
	MinDistanceSquare = new wxTextCtrl(StdWnd,-1,_T("100"));
	SearchRange = new wxTextCtrl(StdWnd,-1,_T("15"));
	MaxResidue = new wxTextCtrl(StdWnd,-1,_T("10"));
	MaxIterations = new wxTextCtrl(StdWnd,-1,_T("10"));

	EdgeDetectSigma = new wxTextCtrl(AdvWnd,-1,_T("1.0"));
	WindowX = new wxTextCtrl(AdvWnd,-1,_T("3"));
	WindowY = new wxTextCtrl(AdvWnd,-1,_T("3"));
	MinDeterminant = new wxTextCtrl(AdvWnd,-1,_T("0.01"));
	MinDisplacement = new wxTextCtrl(AdvWnd,-1,_T("0.1"));

	wxSizer *Sizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText *Static;
	Static = new wxStaticText(StdWnd,-1,_("Number of points to track:"));
	Sizer->Add(Static,0,wxALIGN_LEFT,5);
	Sizer->Add(FeatureNumber,0,wxALIGN_LEFT,5);
	Static = new wxStaticText(StdWnd,-1,_("Minimal (sqared) distance between two points:  "));
	Sizer->Add(Static,0,wxALIGN_LEFT,5);
	Sizer->Add(MinDistanceSquare,0,wxALIGN_LEFT,5);
	Static = new wxStaticText(StdWnd,-1,_("Maximum feature movement:"));
	Sizer->Add(Static,0,wxALIGN_LEFT,5);
	Sizer->Add(SearchRange,0,wxALIGN_LEFT,5);
	Static = new wxStaticText(StdWnd,-1,_("Maximum feature appearance change:"));
	Sizer->Add(Static,0,wxALIGN_LEFT,5);
	Sizer->Add(MaxResidue,0,wxALIGN_LEFT,5);
	Static = new wxStaticText(StdWnd,-1,_("How much CPU per feature?"));
	Sizer->Add(Static,0,wxALIGN_LEFT,5);
	Sizer->Add(MaxIterations,0,wxALIGN_LEFT,5);

	wxSizer *SizerAdd = new wxBoxSizer(wxVERTICAL);
	Static = new wxStaticText(AdvWnd,-1,_("Edge detect filter size:"));
	SizerAdd->Add(Static,0,wxALIGN_LEFT,5);
	SizerAdd->Add(EdgeDetectSigma,0,wxALIGN_LEFT,5);
	Static = new wxStaticText(AdvWnd,-1,_("Feature comparison width:"));
	SizerAdd->Add(Static,0,wxALIGN_LEFT,5);
	SizerAdd->Add(WindowX,0,wxALIGN_LEFT,5);
	Static = new wxStaticText(AdvWnd,-1,_("Feature comparison height:"));
	SizerAdd->Add(Static,0,wxALIGN_LEFT,5);
	SizerAdd->Add(WindowY,0,wxALIGN_LEFT,5);
	Static = new wxStaticText(AdvWnd,-1,_("Minimal determinant:"));
	SizerAdd->Add(Static,0,wxALIGN_LEFT,5);
	SizerAdd->Add(MinDeterminant,0,wxALIGN_LEFT,5);
	Static = new wxStaticText(AdvWnd,-1,_("Minimal displacement per iteration:"));
	SizerAdd->Add(Static,0,wxALIGN_LEFT,5);
	SizerAdd->Add(MinDisplacement,0,wxALIGN_LEFT,5);

	StdWnd->SetSizer( Sizer );
	StdWnd->SetAutoLayout( 1 );
	MainNB->AddPage( StdWnd, _("Standard Settings") );

	AdvWnd->SetSizer( SizerAdd );
	AdvWnd->SetAutoLayout( 1 );
	MainNB->AddPage( AdvWnd, _("Advanced Settings") );

	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(MainNB,1,wxEXPAND|wxALL,5);
	MainSizer->AddSpacer(2);
	wxButton *but = new wxButton(this,BUTTON_START,_("Go!"));
	MainSizer->Add(but,0,wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER,5);

	MainSizer->SetSizeHints( this );
	SetSizer(MainSizer);
	SetAutoLayout(true);
	CenterOnParent();
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogFexTracker,wxDialog)
	EVT_BUTTON(BUTTON_START,DialogFexTracker::OnStart)
END_EVENT_TABLE()


////////////
// OnStart
void DialogFexTracker::OnStart (wxCommandEvent &event) {
	cfg->FeatureNumber = 0;

	swscanf( FeatureNumber->GetValue(), _T("%d"),  &cfg->FeatureNumber );
	swscanf( WindowX->GetValue(), _T("%d"),  &cfg->WindowX );
	swscanf( WindowY->GetValue(), _T("%d"),  &cfg->WindowY );
	swscanf( SearchRange->GetValue(), _T("%d"),  &cfg->SearchRange );
	swscanf( MaxIterations->GetValue(), _T("%d"),  &cfg->MaxIterations );

	swscanf( EdgeDetectSigma->GetValue(), _T("%f"), &cfg->EdgeDetectSigma );
	swscanf( MinDeterminant->GetValue(), _T("%f"), &cfg->MinDeterminant );
	swscanf( MinDisplacement->GetValue(), _T("%f"), &cfg->MinDisplacement );
	swscanf( MaxResidue->GetValue(), _T("%f"), &cfg->MaxResidue );
	swscanf( MinDistanceSquare->GetValue(), _T("%f"), &cfg->MinDistanceSquare );

	EndModal(0);
}


// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include <wx/notebook.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "dialog_fextracker.h"
#include "../FexTrackerSource/FexTracker.h"


///////////////
// Constructor
DialogFexTracker::DialogFexTracker(wxWindow *parent, FexTrackerConfig *_cfg)
: wxDialog (parent,-1,_("Tracker configuration"),wxDefaultPosition)
{
	cfg = _cfg;
	cfg->FeatureNumber = 0;

	FeatureNumber = new wxTextCtrl(this,-1,_T("250"));
	MinDistanceSquare = new wxTextCtrl(this,-1,_T("100"));
	SearchRange = new wxTextCtrl(this,-1,_T("15"));
	MaxResidue = new wxTextCtrl(this,-1,_T("10"));
	MaxIterations = new wxTextCtrl(this,-1,_T("10"));

	EdgeDetectSigma = new wxTextCtrl(this,-1,_T("1.0"));
	WindowX = new wxTextCtrl(this,-1,_T("3"));
	WindowY = new wxTextCtrl(this,-1,_T("3"));
	MinDeterminant = new wxTextCtrl(this,-1,_T("0.01"));
	MinDisplacement = new wxTextCtrl(this,-1,_T("0.1"));

	wxSizer *std_grid = new wxFlexGridSizer(2, 5, 10);
	wxSizer *adv_grid = new wxFlexGridSizer(2, 5, 10);

	std_grid->Add(new wxStaticText(this, -1, _("Number of points to track:")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	std_grid->Add(FeatureNumber, 1, wxALIGN_LEFT);
	std_grid->Add(new wxStaticText(this, -1, _("Minimal (squared) distance between two points:")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	std_grid->Add(MinDistanceSquare, 1, wxALIGN_LEFT);
	std_grid->Add(new wxStaticText(this,-1,_("Maximum feature movement:")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	std_grid->Add(SearchRange, 1, wxALIGN_LEFT);
	std_grid->Add(new wxStaticText(this,-1,_("Maximum feature appearance change:")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	std_grid->Add(MaxResidue, 1, wxALIGN_LEFT);
	std_grid->Add(new wxStaticText(this,-1,_("How much CPU per feature?")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	std_grid->Add(MaxIterations, 1, wxALIGN_LEFT);

	adv_grid->Add(new wxStaticText(this,-1,_("Edge detect filter size:")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	adv_grid->Add(EdgeDetectSigma, 1, wxALIGN_LEFT);
	adv_grid->Add(new wxStaticText(this,-1,_("Feature comparison width:")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	adv_grid->Add(WindowX, 1, wxALIGN_LEFT);
	adv_grid->Add(new wxStaticText(this,-1,_("Feature comparison height:")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	adv_grid->Add(WindowY, 1, wxALIGN_LEFT);
	adv_grid->Add(new wxStaticText(this,-1,_("Minimal determinant:")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	adv_grid->Add(MinDeterminant, 1, wxALIGN_LEFT);
	adv_grid->Add(new wxStaticText(this,-1,_("Minimal displacement per iteration:")), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	adv_grid->Add(MinDisplacement, 1, wxALIGN_LEFT);

	wxSizer *std_box = new wxStaticBoxSizer(new wxStaticBox(this, -1, _("Basic settings")), wxVERTICAL);
	std_box->Add(std_grid, 0, wxALL, 5);
	wxSizer *adv_box = new wxStaticBoxSizer(new wxStaticBox(this, -1, _("Additional settings")), wxVERTICAL);
	adv_box->Add(adv_grid, 0, wxALL, 5);

	wxStdDialogButtonSizer *buttons = new wxStdDialogButtonSizer();
	buttons->AddButton(new wxButton(this,wxID_OK,_("Start")));
	buttons->AddButton(new wxButton(this, wxID_CANCEL));
	buttons->Realize();

	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);

	MainSizer->Add(std_box, 0, wxALL|wxEXPAND, 5);
	MainSizer->Add(adv_box, 0, wxALL&~wxTOP|wxEXPAND, 5);
	MainSizer->Add(buttons, 0, wxALL&~wxTOP|wxEXPAND, 5);

	MainSizer->SetSizeHints( this );
	SetSizer(MainSizer);
	SetAutoLayout(true);
	CenterOnParent();
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogFexTracker,wxDialog)
	EVT_BUTTON(wxID_OK,DialogFexTracker::OnStart)
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


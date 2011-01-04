// Copyright (c) 2009, Amar Takhar <verm@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @file main.cpp
/// @brief Main loop
/// @ingroup base

#ifndef R_PRECOMP
#include <locale.h>

#include <wx/app.h>
#include <wx/window.h>
#include <wx/log.h>
#include <wx/cmdline.h>
#include <wx/wxchar.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/font.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/intl.h>
#endif

#include "main.h"
#include "upload.h"

/// @brief Init the reporter.
bool Reporter::OnInit()
{
//	if ( !wxApp::OnInit() )
//		return false;

	wxApp::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, _("Reporter"));


	static const wxCmdLineEntryDesc cmdLineDesc[] = {
		{ wxCMD_LINE_SWITCH, "c", "crash",      "Launch in crash mode.",	wxCMD_LINE_VAL_NONE, NULL },
		{ wxCMD_LINE_SWITCH, "r", "report",     "Launch in Report mode.",	wxCMD_LINE_VAL_NONE, NULL },
		{ wxCMD_LINE_SWITCH, "j", "json",		"Dump JSON file",			wxCMD_LINE_VAL_NONE, NULL },
		{ wxCMD_LINE_SWITCH, "h", "help",       "This help message",		wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
		{ wxCMD_LINE_NONE, NULL, NULL, NULL, wxCMD_LINE_VAL_NONE, NULL}
	};


	wxCmdLineParser parser(cmdLineDesc, argc, argv);

	parser.SetLogo("Aegisub Reporter version x.x");
	parser.SetCmdLine(argc, argv);
	switch ( parser.Parse() ) {
		case -1:
			return false;
			break; // Help
		case  0:
			break; // OK
		default:
			wxLogMessage(_T("Syntax error."));
			return false;
		break;
	}

	setlocale(LC_NUMERIC, "C");
 	setlocale(LC_CTYPE, "C");
	wxLocale *locale = new wxLocale();
	locale->Init(wxLANGUAGE_ENGLISH);
#ifdef __WINDOWS__
	wxStandardPathsBase &paths = wxStandardPaths::Get();
	locale->AddCatalogLookupPathPrefix(wxString::Format("%s/locale", paths.GetDataDir()));
	locale->AddCatalog(_T("reporter"));
#else
	locale->AddCatalog("reporter");
#endif
	locale->AddCatalog(_T("wxstd"));
	setlocale(LC_NUMERIC, "C");
	setlocale(LC_CTYPE, "C");


	mFrame *frame = new mFrame(_("Aegisub Reporter"));

	if (parser.Found("j")) {
		r->Save("report.json");
		std::cout << "Report saved to report.json" << std::endl;
		return false;
	}

	SetTopWindow(frame);

	r = new Report;
	frame->SetReport(r);

	return true;
}

/// Main frame.
/// @param window_title Window title.
mFrame::mFrame(const wxString &window_title)
		: wxFrame(NULL, wxID_ANY, window_title, wxDefaultPosition, wxSize(300,-1)) {

	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *msgSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(msgSizer, 0, wxALL, 5);

	wxStaticText *title = new wxStaticText(this, -1, _("Welcome to the Aegisub reporter!"));
	msgSizer->Add(title, 0, wxALL, 5);
	title->SetFont(wxFont(11, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

	wxStaticText *msg = new wxStaticText(this, -1, _("In order to better help us target development, and improve Aegisub we would like you to submit some information about your system and setup."));
	msg->Wrap(325);
	msgSizer->Add(msg, 1, wxALL, 5);

	wxStaticText *notice = new wxStaticText(this, -1, _("This information is completely anonymous, no personal information is sent along it is strictly used for targeting new features and the future direction of Aegisub."));
	msgSizer->Add(notice, 1, wxALL, 5);
	notice->SetFont(wxFont(11, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));
	notice->Wrap(325);
	msgSizer->Add(new wxButton(this, 42, "View Report"), 0, wxTOP, 5);

	wxStdDialogButtonSizer *stdButton = new wxStdDialogButtonSizer();
	stdButton->AddButton(new wxButton(this, wxID_OK, _("Submit")));
	stdButton->AddButton(new wxButton(this, wxID_CANCEL, _("Cancel")));
	stdButton->Realize();
	topSizer->Add(stdButton, 0, wxALL, 5);

	this->SetSizerAndFit(topSizer);
	msgSizer->Layout();

	// Is there a better way to do this?
	this->SetMaxSize(this->GetEffectiveMinSize());
	this->SetMinSize(this->GetEffectiveMinSize());

	this->Show(true);

}

/// @brief View report.
void mFrame::ReportView(wxCommandEvent& WXUNUSED(event)) {
    View View(this, r);
    View.ShowModal();
}

/// @brief Cancel reporter.
void mFrame::Cancel(wxCommandEvent& WXUNUSED(event)) {
	Close(true);
}

/// @brief Submit report
void mFrame::Submit(wxCommandEvent& WXUNUSED(event)) {
	Progress *progress = new Progress(this);
	Upload *upload = new Upload(progress);
	upload->Report("./test.json");
}


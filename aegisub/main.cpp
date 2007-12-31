// Copyright (c) 2005, Rodrigo Braz Monteiro
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


////////////
// Includes
#include <wx/wxprec.h>
#include <wx/config.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/mimetype.h>
#include <wx/utils.h>
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include "config.h"
#include "main.h"
#include "frame_main.h"
#include "options.h"
#include "hotkeys.h"
#include "dialog_associations.h"
#include "ass_file.h"
#include "audio_box.h"
#include "audio_display.h"
#include "export_framerate.h"
#include "ass_export_filter.h"
#include "ass_time.h"
#include "ass_dialogue.h"
#include "subs_grid.h"
#include "subtitle_format.h"
#include "video_context.h"
#include "standard_paths.h"
#ifdef WITH_AUTOMATION
#include "auto4_base.h"
#endif


///////////////////
// wxWidgets macro
IMPLEMENT_APP(AegisubApp)


#if 0
void StartupLog(const wxString &msg) {
	static wxStopWatch clock;
	wxString nmsg = wxString::Format(_T("Startup: %d - %s\n"), clock.Time(), msg.c_str());
#ifdef WIN32
	OutputDebugStringW(nmsg.wc_str());
#else
	printf(nmsg.mb_str(wxConvLocal));
#endif
}
#else
#define StartupLog(a)
#endif


///////////////////////////
// Initialization function
// -----------------------
// Gets called when application starts, creates MainFrame
bool AegisubApp::OnInit() {
	StartupLog(_T("Inside OnInit"));
	try {
		// Initialize randomizer
		StartupLog(_T("Initialize random generator"));
		srand(time(NULL));

		// locale for loading options
		StartupLog(_T("Set initial locale"));
		setlocale(LC_NUMERIC, "C");
		setlocale(LC_CTYPE, "C");

		// App name (yeah, this is a little weird to get rid of an odd warning)
#ifdef __WXMSW__
		SetAppName(_T("Aegisub"));
#else
#ifdef __WXMAC__
		SetAppName(_T("Aegisub"));
#else
		SetAppName(_T("aegisub"));
#endif
#endif

		// Crash handling
#ifndef _DEBUG
		StartupLog(_T("Install exception handler"));
		wxHandleFatalExceptions(true);
#endif

		// Set config file
		StartupLog(_T("Load configuration"));
		Options.SetFile(StandardPaths::DecodePath(_T("?data/config.dat")));
		Options.LoadDefaults();
		Options.Load();
		if (!Options.AsBool(_T("Local config"))) {
			Options.SetFile(StandardPaths::DecodePath(_T("?user/config.dat")));
			Options.Load();
			wxRemoveFile(StandardPaths::DecodePath(_T("?data/config.dat")));
		}
		StartupLog(_T("Store options back"));
		Options.Save();
		AssTime::UseMSPrecision = Options.AsBool(_T("Use nonstandard Milisecond Times"));

		// Set hotkeys file
		StartupLog(_T("Load hotkeys"));
		Hotkeys.SetFile(StandardPaths::DecodePath(_T("?user/hotkeys.dat")));
		Hotkeys.Load();

		StartupLog(_T("Initialize final locale"));
#ifdef __WINDOWS__
		// Set locale
		int lang = Options.AsInt(_T("Locale Code"));
		if (lang == -1) {
			lang = locale.PickLanguage();
			Options.SetInt(_T("Locale Code"),lang);
			Options.Save();
		}
		locale.Init(lang);
#else
		locale.Init(wxLANGUAGE_DEFAULT);
#endif

		// Set association
#ifndef _DEBUG
		StartupLog(_T("Install file type associations"));
		RegistryAssociate();
#endif

		// Load Automation scripts
#ifdef WITH_AUTOMATION
		StartupLog(_T("Load global Automation scripts"));
		global_scripts = new Automation4::AutoloadScriptManager(Options.AsText(_T("Automation Autoload Path")));
#endif

		// Load export filters
		StartupLog(_T("Prepare export filters"));
		AssExportFilterChain::PrepareFilters();

		// Get parameter subs
		StartupLog(_T("Parse command line"));
		wxArrayString subs;
		for (int i=1;i<argc;i++) {
			subs.Add(argv[i]);
		}

		// Open main frame
		StartupLog(_T("Create main window"));
		frame = new FrameMain(subs);
		SetTopWindow(frame);
	}

	catch (const wchar_t *err) {
		wxMessageBox(err,_T("Fatal error while initializing"));
		return false;
	}

	catch (...) {
		wxMessageBox(_T("Unhandled exception"),_T("Fatal error while initializing"));
		return false;
	}

	StartupLog(_T("Initialization complete"));
	return true;
}


////////
// Exit
int AegisubApp::OnExit() {
	SubtitleFormat::DestroyFormats();
	VideoContext::Clear();
	Options.Clear();
#ifdef WITH_AUTOMATION
	delete global_scripts;
#endif
	return wxApp::OnExit();
}


#ifndef _DEBUG
///////////////////////
// Unhandled exception
void AegisubApp::OnUnhandledException() {
	// Attempt to recover file
	wxFileName origfile(AssFile::top->filename);
	wxString path = Options.AsText(_T("Auto recovery path"));
	if (path.IsEmpty()) path = StandardPaths::DecodePath(_T("?user/"));
	wxFileName dstpath(path);
	if (!dstpath.IsAbsolute()) path = StandardPaths::DecodePath(_T("?user/")) + path;
	path += _T("/");
	dstpath.Assign(path);
	if (!dstpath.DirExists()) wxMkdir(path);
	wxString filename = StandardPaths::DecodePath(_T("?user/")) + origfile.GetName() + _T(".RECOVER.ass");
	AssFile::top->Save(filename,false,false);

	// Inform user of crash
	wxMessageBox(_T("Aegisub has encountered an unhandled exception error and will terminate now. The subtitles you were working on were saved to \"") + filename + _T("\", but they might be corrupt."), _T("Unhandled exception"), wxOK | wxICON_ERROR, NULL);
}


///////////////////
// Fatal exception
void AegisubApp::OnFatalException() {
	// Attempt to recover file
	wxFileName origfile(AssFile::top->filename);
	wxString path = Options.AsText(_T("Auto recovery path"));
	if (path.IsEmpty()) path = StandardPaths::DecodePath(_T("?user/"));
	wxFileName dstpath(path);
	if (!dstpath.IsAbsolute()) path = StandardPaths::DecodePath(_T("?user/")) + path;
	path += _T("/");
	dstpath.Assign(path);
	if (!dstpath.DirExists()) wxMkdir(path);
	wxString filename = path + origfile.GetName() + _T(".RECOVER.ass");
	AssFile::top->Save(filename,false,false);

	// Stack walk
#if wxUSE_STACKWALKER == 1
	StackWalker walker;
	walker.WalkFromException();
#endif

	// Inform user of crash
	wxMessageBox(_T("Aegisub has encountered a fatal error and will terminate now. The subtitles you were working on were saved to \"") + filename + _T("\", but they might be corrupt."), _T("Fatal error"), wxOK | wxICON_ERROR, NULL);
}
#endif


////////////////
// Stack walker
#if wxUSE_STACKWALKER == 1
void StackWalker::OnStackFrame(const wxStackFrame &frame) {
	wxString dst = wxString::Format(_T("%03i - 0x%08X: "),frame.GetLevel(),frame.GetAddress()) + frame.GetName() + _T(" on ") + frame.GetFileName() + wxString::Format(_T(":%i"),frame.GetLine());
	char temp[2048];
	if (file.is_open()) {
		strcpy(temp,dst.mb_str());
		file << temp << std::endl;
	}
	else wxLogMessage(dst);
}

StackWalker::StackWalker() {
	file.open(wxString(StandardPaths::DecodePath(_T("?user/crashlog.txt"))).mb_str(),std::ios::out | std::ios::app);
	if (file.is_open()) {
		file << std::endl << "Begining stack dump:\n";
	}
}

StackWalker::~StackWalker() {
	if (file.is_open()) {
		file << "End of stack dump.\n\n";
		file.close();
	}
}
#endif



//////////////////
// Call main loop
int AegisubApp::OnRun() {
	try {
		if (m_exitOnFrameDelete == Later) m_exitOnFrameDelete = Yes;
		return MainLoop();
	}

	catch (wxString err) {
		wxMessageBox(err, _T("Unhandled exception"), wxOK | wxICON_ERROR, NULL);
	}

	catch (wxChar *error) {
		wxMessageBox(error, _T("Unhandled exception"), wxOK | wxICON_ERROR, NULL);
	}

	catch (std::exception e) {
		wxMessageBox(wxString(_T("std::exception: ")) + wxString(e.what(),wxConvUTF8), _T("Unhandled exception"), wxOK | wxICON_ERROR, NULL);
	}

	catch (...) {
		wxMessageBox(_T("Program terminated in error."), _T("Unhandled exception"), wxOK | wxICON_ERROR, NULL);
	}

	ExitMainLoop();
	return 1;
}


/////////////////////////////////
// Registry program to filetypes
void AegisubApp::RegistryAssociate () {
#if defined(__WINDOWS__)
	// Command to open with this
	wxString command;
	wxStandardPaths stand;
	wxString fullPath = stand.GetExecutablePath();
	command = _T("\"") + fullPath + _T("\" \"%1\"");

	// Main program association
	wxRegKey *key = new wxRegKey(_T("HKEY_CURRENT_USER\\Software\\Classes\\Aegisub"));
	if (!key->Exists()) key->Create();
	key->SetValue(_T(""),_T("Aegisub Subtitle Script"));
	delete key;
	key = new wxRegKey(_T("HKEY_CURRENT_USER\\Software\\Classes\\Aegisub\\DefaultIcon"));
	if (!key->Exists()) key->Create();
	key->SetValue(_T(""),fullPath);
	delete key;
	key = new wxRegKey(_T("HKEY_CURRENT_USER\\Software\\Classes\\Aegisub\\Shell"));
	if (!key->Exists()) key->Create();
	key->SetValue(_T(""),_T("open"));
	delete key;
	key = new wxRegKey(_T("HKEY_CURRENT_USER\\Software\\Classes\\Aegisub\\Shell\\Open"));
	if (!key->Exists()) key->Create();
	key->SetValue(_T(""),_T("&Open with Aegisub"));
	delete key;
	key = new wxRegKey(_T("HKEY_CURRENT_USER\\Software\\Classes\\Aegisub\\Shell\\Open\\Command"));
	if (!key->Exists()) key->Create();
	key->SetValue(_T(""),command);
	delete key;

	// Check associations
	if (Options.AsBool(_T("Show associations"))) {
		bool gotAll = DialogAssociations::CheckAssociation(_T("ass")) && DialogAssociations::CheckAssociation(_T("ssa")) &&
					  DialogAssociations::CheckAssociation(_T("srt")) && DialogAssociations::CheckAssociation(_T("sub")) &&
					  DialogAssociations::CheckAssociation(_T("ttxt"));
		if (!gotAll) {
			DialogAssociations diag(NULL);
			diag.ShowModal();
			Options.SetBool(_T("Show associations"),false);
			Options.Save();
		}
	}
#elif defined(__APPLE__)
	// This is totally untested and pure guesswork
	// I don't know if it should be ".ass" or just "ass"
	wxFileName::MacRegisterDefaultTypeAndCreator(_T(".ass"), 0x41535341 /*ASSA*/, 0x41475355 /*AGSU*/);
	// Technically .ssa isn't ASSA but it makes it so much easier ;)
	wxFileName::MacRegisterDefaultTypeAndCreator(_T(".ssa"), 0x53534134 /*SSA4*/, 0x41475355 /*AGSU*/);
	// Not touching .srt yet, there might be some type already registered for it which we should probably use then
#else
	// Is there anything like this for other POSIX compatible systems?
#endif
}


////////////
// Open URL
void AegisubApp::OpenURL(wxString url) {
	wxLaunchDefaultBrowser(url, wxBROWSER_NEW_WINDOW);
}


////////////////
// Apple events
#ifdef __WXMAC__
void AegisubApp::MacOpenFile(const wxString &filename) {
	if (frame != NULL && !filename.empty()) {
		frame->LoadSubtitles(filename);
		wxFileName filepath(filename);
		Options.SetText(_T("Last open subtitles path"), filepath.GetPath());
	}
}
#endif


///////////////
// Event table
BEGIN_EVENT_TABLE(AegisubApp,wxApp)
	EVT_MOUSEWHEEL(AegisubApp::OnMouseWheel)
	EVT_KEY_DOWN(AegisubApp::OnKey)
END_EVENT_TABLE()


/////////////////////
// Mouse wheel moved
void AegisubApp::OnMouseWheel(wxMouseEvent &event) {
	wxPoint pt;
	wxWindow *target = wxFindWindowAtPointer(pt);
	if (target == frame->audioBox->audioDisplay || target == frame->SubsBox) {
		if (target->IsShownOnScreen()) target->AddPendingEvent(event);
		else event.Skip();
	}
	else event.Skip();
}


///////////////
// Key pressed
void AegisubApp::OnKey(wxKeyEvent &event) {
	//frame->audioBox->audioDisplay->AddPendingEvent(event);
	if (!event.GetSkipped()) {
		event.Skip();
	}
}

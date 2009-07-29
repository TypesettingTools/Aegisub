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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file main.cpp
/// @brief Main entry point, as well as crash handling
/// @ingroup main
///


////////////
// Includes
#include "config.h"

#include <wx/wxprec.h>
#include <wx/config.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/mimetype.h>
#include <wx/utils.h>
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include <wx/datetime.h>

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
#include "version.h"
#include "plugin_manager.h"
#include "charset_conv.h"


///////////////////
// wxWidgets macro
IMPLEMENT_APP(AegisubApp)


#ifdef WITH_STARTUPLOG
#define StartupLog(a) MessageBox(0, a, _T("Aegisub startup log"), 0)
#else
#define StartupLog(a)
#endif

#ifdef __VISUALC__
#define MS_VC_EXCEPTION 0x406d1388

typedef struct tagTHREADNAME_INFO {
	DWORD dwType; // must be 0x1000
	LPCSTR szName; // pointer to name (in same addr space)
	DWORD dwThreadID; // thread ID (-1 caller thread)
	DWORD dwFlags; // reserved for future use, most be zero
} THREADNAME_INFO;

void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName) {
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;
	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR *)&info);
	}
	__except (EXCEPTION_CONTINUE_EXECUTION) {}
}
#endif

///////////////////////////
// Initialization function
// -----------------------
// Gets called when application starts, creates MainFrame
bool AegisubApp::OnInit() {
#ifdef __VISUALC__
	SetThreadName((DWORD) -1,"AegiMain");
#endif

	StartupLog(_T("Inside OnInit"));
	frame = NULL;
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
		Options.LoadDefaults();
#ifdef __WXMSW__
		// Try loading configuration from the install dir if one exists there
		if (wxFileName::FileExists(StandardPaths::DecodePath(_T("?data/config.dat")))) {
			Options.SetFile(StandardPaths::DecodePath(_T("?data/config.dat")));
			Options.Load();

			if (Options.AsBool(_T("Local config"))) {
				// Local config, make ?user mean ?data so all user settings are placed in install dir
				StandardPaths::SetPathValue(_T("?user"), StandardPaths::DecodePath(_T("?data")));
			}
			else {
				// Not local config, we don't want that config.dat file here any more
				// It might be a leftover from a really old install
				wxRemoveFile(StandardPaths::DecodePath(_T("?data/config.dat")));
			}
		}
#endif
		// TODO: Check if we can write to config.dat and warn the user if we can't
		// If we had local config, ?user now means ?data so this will still be loaded from the correct location
		Options.SetFile(StandardPaths::DecodePath(_T("?user/config.dat")));
		Options.Load();

		StartupLog(_T("Store options back"));
		Options.SetInt(_T("Last Version"),GetSVNRevision());
		Options.LoadDefaults(false,true);	// Override options based on version number
		Options.Save();
		AssTime::UseMSPrecision = Options.AsBool(_T("Use nonstandard Milisecond Times"));

		// Set hotkeys file
		StartupLog(_T("Load hotkeys"));
		Hotkeys.SetFile(StandardPaths::DecodePath(_T("?user/hotkeys.dat")));
		Hotkeys.Load();

		StartupLog(_T("Initialize final locale"));

		// Set locale
		int lang = Options.AsInt(_T("Locale Code"));
		if (lang == -1) {
			lang = locale.PickLanguage();
			Options.SetInt(_T("Locale Code"),lang);
			Options.Save();
		}
		locale.Init(lang);

		// Load plugins
		plugins = new PluginManager();
		plugins->RegisterBuiltInPlugins();

		// Load Automation scripts
#ifdef WITH_AUTOMATION
		StartupLog(_T("Load global Automation scripts"));
		global_scripts = new Automation4::AutoloadScriptManager(Options.AsText(_T("Automation Autoload Path")));
#endif

		// Load export filters
		StartupLog(_T("Prepare export filters"));
		AssExportFilterChain::PrepareFilters();

		// Set association
#ifndef _DEBUG
		StartupLog(_T("Install file type associations"));
		if (!Options.AsBool(_T("Local config")))
			RegistryAssociate();
#endif

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
	delete plugins;
	Options.Clear();
#ifdef WITH_AUTOMATION
	delete global_scripts;
#endif
	return wxApp::OnExit();
}


#ifndef _DEBUG
/////////////////////////////////////////////
// Message displayed on unhandled exceptions
const static wxChar unhandled_exception_message[] = _T("Oops, Aegisub has crashed!\n\nI have tried to emergency-save a copy of your file, and a crash log file has been generated.\n\nYou can find the emergency-saved file in:\n%s\n\nIf you submit the crash log to the Aegisub team, we will investigate the problem and attempt to fix it. You can find the crashlog in:\n%s\n\nAegisub will now close.");
const static wxChar unhandled_exception_message_nocrashlog[] = _T("Oops, Aegisub has crashed!\n\nI have tried to emergency-save a copy of your file.\n\nYou can find the emergency-saved file in:\n%s\n\nAegisub will now close.");

///////////////////////
// Unhandled exception
void AegisubApp::OnUnhandledException() {
	// Attempt to recover file
	wxFileName origfile(AssFile::top->filename);
	wxString path = Options.AsText(_T("Auto recovery path"));
	if (path.IsEmpty()) path = StandardPaths::DecodePath(_T("?user/"));
	wxFileName dstpath(path);
	if (!dstpath.IsAbsolute()) path = StandardPaths::DecodePathMaybeRelative(path, _T("?user/"));
	path += _T("/");
	dstpath.Assign(path);
	if (!dstpath.DirExists()) wxMkdir(path);
	wxString filename = path + origfile.GetName() + _T(".RECOVER.ass");
	AssFile::top->Save(filename,false,false);

	// Inform user of crash
	wxMessageBox(wxString::Format(unhandled_exception_message, filename.c_str(), StandardPaths::DecodePath(_T("?user/crashlog.txt")).c_str()), _T("Unhandled exception"), wxOK | wxICON_ERROR, NULL);
}


///////////////////
// Fatal exception
void AegisubApp::OnFatalException() {
	// Attempt to recover file
	wxFileName origfile(AssFile::top->filename);
	wxString path = Options.AsText(_T("Auto recovery path"));
	if (path.IsEmpty()) path = StandardPaths::DecodePath(_T("?user/"));
	wxFileName dstpath(path);
	if (!dstpath.IsAbsolute()) path = StandardPaths::DecodePathMaybeRelative(path, _T("?user/"));
	path += _T("/");
	dstpath.Assign(path);
	if (!dstpath.DirExists()) wxMkdir(path);
	wxString filename = path + origfile.GetName() + _T(".RECOVER.ass");
	AssFile::top->Save(filename,false,false);

#if wxUSE_STACKWALKER == 1
	// Stack walk
	StackWalker walker(_T("Fatal exception"));
	walker.WalkFromException();

	// Inform user of crash
	wxMessageBox(wxString::Format(unhandled_exception_message, filename.c_str(), StandardPaths::DecodePath(_T("?user/crashlog.txt")).c_str()), _T("Fatal exception"), wxOK | wxICON_ERROR, NULL);
#else
	// Inform user of crash
	wxMessageBox(wxString::Format(unhandled_exception_message_nocrashlog, filename.c_str()), _T("Fatal exception"), wxOK | wxICON_ERROR, NULL);
#endif
}
#endif


////////////////
// Stack walker
#if wxUSE_STACKWALKER == 1
void StackWalker::OnStackFrame(const wxStackFrame &frame) {
	wxString dst = wxString::Format(_T("%03i - 0x%08X: "),frame.GetLevel(),frame.GetAddress()) + frame.GetName();
	if (frame.HasSourceLocation()) dst += _T(" on ") + frame.GetFileName() + wxString::Format(_T(":%i"),frame.GetLine());
	if (file.is_open()) {
		file << dst.mb_str() << std::endl;
	}
	else wxLogMessage(dst);
}

StackWalker::StackWalker(wxString cause) {
	file.open(wxString(StandardPaths::DecodePath(_T("?user/crashlog.txt"))).mb_str(),std::ios::out | std::ios::app);
	if (file.is_open()) {
		wxDateTime time = wxDateTime::Now();
		wxString timeStr = _T("---") + time.FormatISODate() + _T(" ") + time.FormatISOTime() + _T("------------------");
		formatLen = timeStr.Length();
		file << std::endl << timeStr.mb_str(csConvLocal);
		file << "\nVER - " << GetAegisubLongVersionString().mb_str(wxConvUTF8);
		file << "\nFTL - Begining stack dump for \"" << cause.mb_str(wxConvUTF8) <<"\":\n";
	}
}

StackWalker::~StackWalker() {
	if (file.is_open()) {
		char dashes[1024];
		int i = 0;
		for (i=0;i<formatLen;i++) dashes[i] = '-';
		dashes[i] = 0;
		file << "End of stack dump.\n";
		file << dashes;
		file << "\n";
		file.close();
	}
}
#endif



//////////////////
// Call main loop
int AegisubApp::OnRun() {
	wxString error;

	// Run program
	try {
		if (m_exitOnFrameDelete == Later) m_exitOnFrameDelete = Yes;
		return MainLoop();
	}

	// Catch errors
	catch (wxString &err) { error = err; }
	catch (wxChar *err) { error = err; }
	catch (std::exception &e) {	error = wxString(_T("std::exception: ")) + wxString(e.what(),wxConvUTF8); }
	catch (...) { error = _T("Program terminated in error."); }

	// Report errors
	if (!error.IsEmpty()) {
		std::ofstream file;
		file.open(wxString(StandardPaths::DecodePath(_T("?user/crashlog.txt"))).mb_str(),std::ios::out | std::ios::app);
		if (file.is_open()) {
			wxDateTime time = wxDateTime::Now();
			wxString timeStr = _T("---") + time.FormatISODate() + _T(" ") + time.FormatISOTime() + _T("------------------");
			file << std::endl << timeStr.mb_str(csConvLocal);
			file << "\nVER - " << GetAegisubLongVersionString().mb_str(wxConvUTF8);
			file << "\nEXC - Aegisub has crashed with unhandled exception \"" << error.mb_str(csConvLocal) <<"\".\n";
			int formatLen = timeStr.Length();
			char dashes[1024];
			int i = 0;
			for (i=0;i<formatLen;i++) dashes[i] = '-';
			dashes[i] = 0;
			file << dashes;
			file << "\n";
			file.close();
		}

		OnUnhandledException();
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
	if (frame && (target == frame->audioBox->audioDisplay || target == frame->SubsBox)) {
		if (target->IsShownOnScreen()) target->GetEventHandler()->ProcessEvent(event);
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


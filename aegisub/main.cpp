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
#include "auto4_base.h"


///////////////////
// wxWidgets macro
IMPLEMENT_APP(AegisubApp)


///////////////////////////
// Initialization function
// -----------------------
// Gets called when application starts, creates MainFrame
bool AegisubApp::OnInit() {
	try {
		// Initialize randomizer
		srand(time(NULL));

		// App name
		SetAppName(_T("Aegisub"));
		#ifndef _DEBUG
		wxHandleFatalExceptions(true);
		#endif

		// Set config file
		GetFullPath(argv[0]);
		GetFolderName();
		Options.SetFile(folderName + _T("/config.dat"));
		Options.Load();
		AssTime::UseMSPrecision = Options.AsBool(_T("Use nonstandard Milisecond Times"));

		// Set hotkeys file
		Hotkeys.SetFile(folderName + _T("/hotkeys.dat"));
		Hotkeys.Load();

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
		locale.Init(wxLocale::GetSystemLanguage());
#endif

		// Load Automation scripts
		global_scripts = new Automation4::AutoloadScriptManager(Options.AsText(_T("Automation Autoload Path")));

		// Load export filters
		AssExportFilterChain::PrepareFilters();

		// Set association
		RegistryAssociate();

		// Get parameter subs
		wxArrayString subs;
		for (int i=1;i<argc;i++) {
			subs.Add(argv[i]);
		}

		// Open main frame
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

	return true;
}


#ifndef _DEBUG
///////////////////////
// Unhandled exception
void AegisubApp::OnUnhandledException() {
	// Attempt to recover file
	wxFileName origfile(AssFile::top->filename);
	wxString path = Options.AsText(_T("Auto recovery path"));
	if (path.IsEmpty()) path = folderName;
	wxFileName dstpath(path);
	if (!dstpath.IsAbsolute()) path = AegisubApp::folderName + path;
	path += _T("/");
	dstpath.Assign(path);
	if (!dstpath.DirExists()) wxMkdir(path);
	wxString filename = folderName + origfile.GetName() + _T(".RECOVER.ass");
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
	if (path.IsEmpty()) path = folderName;
	wxFileName dstpath(path);
	if (!dstpath.IsAbsolute()) path = AegisubApp::folderName + path;
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
	file.open(wxString(AegisubApp::folderName + _T("/stack.txt")).mb_str(),std::ios::out | std::ios::app);
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
#ifdef __WINDOWS__
	// Command to open with this
	wxString command;
	command << _T("\"") << fullPath << _T("\" \"%1\"");

	// Main program association
	wxRegKey *key = new wxRegKey(_T("HKEY_LOCAL_MACHINE\\Software\\Classes\\Aegisub"));
	if (!key->Exists()) key->Create();
	key->SetValue(_T(""),_T("Aegisub Subtitle Script"));
	delete key;
	key = new wxRegKey(_T("HKEY_LOCAL_MACHINE\\Software\\Classes\\Aegisub\\DefaultIcon"));
	if (!key->Exists()) key->Create();
    key->SetValue(_T(""),fullPath);
	delete key;
	key = new wxRegKey(_T("HKEY_LOCAL_MACHINE\\Software\\Classes\\Aegisub\\Shell"));
	if (!key->Exists()) key->Create();
    key->SetValue(_T(""),_T("open"));
	delete key;
	key = new wxRegKey(_T("HKEY_LOCAL_MACHINE\\Software\\Classes\\Aegisub\\Shell\\Open"));
	if (!key->Exists()) key->Create();
    key->SetValue(_T(""),_T("&Open with Aegisub"));
	delete key;
	key = new wxRegKey(_T("HKEY_LOCAL_MACHINE\\Software\\Classes\\Aegisub\\Shell\\Open\\Command"));
	if (!key->Exists()) key->Create();
    key->SetValue(_T(""),command);
	delete key;

	// Check associations
	if (Options.AsBool(_T("Show associations"))) {
		bool gotAll = DialogAssociations::CheckAssociation(_T("ass")) && DialogAssociations::CheckAssociation(_T("ssa")) && DialogAssociations::CheckAssociation(_T("srt"));
		if (!gotAll) {
			DialogAssociations diag(NULL);
			diag.ShowModal();
			Options.SetBool(_T("Show associations"),false);
			Options.Save();
		}
	}
#endif
}


/////////////////////////////
// Gets and stores full path
void AegisubApp::GetFullPath(wxString arg) {
	if (wxIsAbsolutePath(arg)) {
		fullPath = arg;
		return;
	}

	// Is it a relative path?
	wxString currentDir(wxFileName::GetCwd());
	if (currentDir.Last() != wxFILE_SEP_PATH) currentDir += wxFILE_SEP_PATH;
	wxString str = currentDir + arg;
	if (wxFileExists(str)) {
		fullPath = str;
		return;
	}

    // OK, it's neither an absolute path nor a relative path.
    // Search PATH.
    wxPathList pathList;
    pathList.AddEnvList(_T("PATH"));
    str = pathList.FindAbsoluteValidPath(arg);
	if (!str.IsEmpty()) {
		fullPath = str;
		return;
	}

	fullPath = _T("");
	return;
}


///////////////////////////////////
// Gets folder name from full path
void AegisubApp::GetFolderName () {
#if defined(__WINDOWS__)
	folderName = _T("");
	wxFileName path(fullPath);
#elif defined(__APPLE__)
	wxFileName path;
	path.AssignHomeDir();
	path.AppendDir(_T("Library"));
	path.AppendDir(_T("Application Support"));
	if (!path.DirExists())
		path.Mkdir();
	path.AppendDir(_T("Aegisub"));
	if (!path.DirExists())
		path.Mkdir();
#else
	wxFileName path;
	path.AssignHomeDir();
	path.AppendDir(_T(".aegisub"));
	if (!path.DirExists())
		path.Mkdir();
#endif
	folderName += path.GetPath(wxPATH_GET_VOLUME);
	folderName += _T("/");
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


///////////
// Statics
wxString AegisubApp::fullPath;
wxString AegisubApp::folderName;


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
		target->AddPendingEvent(event);
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


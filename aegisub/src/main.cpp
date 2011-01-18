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

#ifndef AGI_PRE
#include <wx/config.h>
#include <wx/datetime.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/mimetype.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>
#endif

#include "include/aegisub/menu.h"
#include "command/command.h"
#include "command/icon.h"
#include "include/aegisub/toolbar.h"
#include "include/aegisub/hotkey.h"

#include "ass_dialogue.h"
#include "ass_export_filter.h"
#include "ass_file.h"
#include "ass_time.h"
#include "selection_controller.h"
#include "audio_box.h"
#ifdef WITH_AUTOMATION
#include "auto4_base.h"
#endif
#include "charset_conv.h"
#include "compat.h"
#include "export_framerate.h"
#include "frame_main.h"
#include "main.h"
#include "libresrc/libresrc.h"
#include "plugin_manager.h"
#include "standard_paths.h"
#include "subtitle_format.h"
#include "version.h"
#include "video_context.h"

#include <libaegisub/io.h>
#include <libaegisub/access.h>
#include <libaegisub/log.h>
#include <libaegisub/hotkey.h>



namespace config {
	agi::Options *opt;
	agi::MRUManager *mru;
}


///////////////////
// wxWidgets macro
IMPLEMENT_APP(AegisubApp)

static const wchar_t *LastStartupState = NULL;

#ifdef WITH_STARTUPLOG

/// DOCME
#define StartupLog(a) MessageBox(0, a, _T("Aegisub startup log"), 0)
#else
#define StartupLog(a) LastStartupState = a
#endif

#ifdef __VISUALC__

/// DOCME
#define MS_VC_EXCEPTION 0x406d1388


/// DOCME
typedef struct tagTHREADNAME_INFO {

	/// DOCME
	DWORD dwType; // must be 0x1000

	/// DOCME
	LPCSTR szName; // pointer to name (in same addr space)

	/// DOCME
	DWORD dwThreadID; // thread ID (-1 caller thread)

	/// DOCME
	DWORD dwFlags; // reserved for future use, most be zero

/// DOCME
} THREADNAME_INFO;


/// @brief DOCME
/// @param dwThreadID   
/// @param szThreadName 
///
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


/// @brief Handle wx assertions and redirct to the logging system.
/// @param file File name
/// @param line Line number
/// @param func Function name
/// @param cond Condition
/// @param msg  Message
static void wxAssertHandler(const wxString &file, int line, const wxString &func, const wxString &cond, const wxString &msg) {
	LOG_A("wx/assert") << file << ":" << line << ":" << func << "() " << cond << ": " << msg;
}


/// @brief Gets called when application starts.
/// @return bool
bool AegisubApp::OnInit() {
	// App name (yeah, this is a little weird to get rid of an odd warning)
#if defined(__WXMSW__) || defined(__WXMAC__)
	SetAppName(_T("Aegisub"));
#else
	SetAppName(_T("aegisub"));
#endif

	// logging.
	const std::string path_log(StandardPaths::DecodePath(_T("?user/log/")));
	wxFileName::Mkdir(path_log, 0777, wxPATH_MKDIR_FULL);
	agi::log::log = new agi::log::LogSink(path_log);


#ifdef _DEBUG
	emit_stdout = new agi::log::EmitSTDOUT();
	emit_stdout->Enable();
#endif

	// Init command manager
	cmd::cm = new cmd::CommandManager();

	// Init commands.
	cmd::init_command(cmd::cm);

	// Init hotkeys.
	const std::string conf_user_hotkey(StandardPaths::DecodePath(_T("?user/hotkey.json")));
	agi::hotkey::hotkey = new agi::hotkey::Hotkey(conf_user_hotkey, GET_DEFAULT_CONFIG(default_hotkey));

	// Init icons.
	icon::icon_init();

	// Generate menus.
	menu::menu = new menu::Menu();

	// Generate toolbars.
	toolbar::toolbar = new toolbar::Toolbar();

	// Install assertion handler
//	wxSetAssertHandler(wxAssertHandler);

	const std::string conf_mru(StandardPaths::DecodePath(_T("?user/mru.json")));
	config::mru = new agi::MRUManager(conf_mru, GET_DEFAULT_CONFIG(default_mru));

	// Set config file
	StartupLog(_T("Load configuration"));
	try {
		const std::string conf_user(StandardPaths::DecodePath(_T("?user/config.json")));
		config::opt = new agi::Options(conf_user, GET_DEFAULT_CONFIG(default_config));
	} catch (agi::Exception& e) {
		LOG_E("config/init") << "Caught exception: " << e.GetName() << " -> " << e.GetMessage();
	}

#ifdef __WXMSW__
	// Try loading configuration from the install dir if one exists there
	try {
		const std::string conf_local(StandardPaths::DecodePath(_T("?data/config.json")));
		std::ifstream* localConfig = agi::io::Open(conf_local);
		config::opt->ConfigNext(*localConfig);
		delete localConfig;

		if (OPT_GET("App/Local Config")->GetBool()) {
			// Local config, make ?user mean ?data so all user settings are placed in install dir
			StandardPaths::SetPathValue(_T("?user"), StandardPaths::DecodePath(_T("?data")));
		}
	} catch (agi::acs::AcsError const&) {
		// File doesn't exist or we can't read it
		// Might be worth displaying an error in the second case
	}
#endif
	try {
		config::opt->ConfigUser();
	}
	catch (agi::Exception const& err) {
		wxMessageBox(L"Configuration file is invalid. Error reported:\n" + lagi_wxString(err.GetMessage()), L"Error");
	}


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

		// Crash handling
#if !defined(_DEBUG) || defined(WITH_EXCEPTIONS)
		StartupLog(_T("Install exception handler"));
		wxHandleFatalExceptions(true);
#endif

		StartupLog(_T("Store options back"));
		OPT_SET("Version/Last Version")->SetInt(GetSVNRevision());
		AssTime::UseMSPrecision = OPT_GET("App/Nonstandard Milisecond Times")->GetBool();

		StartupLog(_T("Initialize final locale"));

		// Set locale
		int lang = OPT_GET("App/Locale")->GetInt();
		if (lang == -1) {
			lang = locale.PickLanguage();
			OPT_SET("App/Locale")->SetInt(lang);
		}
		locale.Init(lang);

		// Load plugins
		plugins = new PluginManager();
		plugins->RegisterBuiltInPlugins();

		// Load Automation scripts
#ifdef WITH_AUTOMATION
		StartupLog(_T("Load global Automation scripts"));
		global_scripts = new Automation4::AutoloadScriptManager(lagi_wxString(OPT_GET("Path/Automation/Autoload")->GetString()));
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
	catch (agi::Exception const& e) {
		wxMessageBox(e.GetMessage(),_T("Fatal error while initializing"));
		return false;
	}

	catch (...) {
		wxMessageBox(_T("Unhandled exception"),_T("Fatal error while initializing"));
		return false;
	}

	StartupLog(_T("Initialization complete"));
	return true;
}



/// @brief Exit 
/// @return 
///
int AegisubApp::OnExit() {
	SubtitleFormat::DestroyFormats();
	VideoContext::OnExit();
	delete plugins;
	delete config::opt;
	delete config::mru;
	delete agi::hotkey::hotkey;

#ifdef WITH_AUTOMATION
	delete global_scripts;
#endif
#ifdef _DEBUG
	delete emit_stdout;
#endif

	// Keep this last!
	delete agi::log::log;

	return wxApp::OnExit();
}


#if !defined(_DEBUG) || defined(WITH_EXCEPTIONS)
/// Message displayed when an exception has occurred.
const static wxString exception_message = _("Oops, Aegisub has crashed!\n\nAn attempt has been made to save a copy of your file to:\n\n%s\n\nAegisub will now close.");

static void UnhandledExeception(bool stackWalk) {
	if (AssFile::top) {
		// Current filename if any.
		wxFileName file(AssFile::top->filename);
		if (!file.HasName()) file.SetName(_T("untitled"));

		// Set path and create if it doesn't exist.
		file.SetPath(StandardPaths::DecodePath(_T("?user/recovered")));
		if (!file.DirExists()) file.Mkdir();

		// Save file
		wxString filename = wxString::Format("%s/%s_%s.ass", file.GetPath(), wxDateTime::Now().Format("%Y-%m-%d-%H%M%S"), file.GetName());
		AssFile::top->Save(filename,false,false);

#if wxUSE_STACKWALKER == 1
		if (stackWalk) {
			StackWalker walker(_T("Fatal exception"));
			walker.WalkFromException();
		}
#endif

		// Inform user of crash.
		wxMessageBox(wxString::Format(exception_message, filename.c_str()), _("Program error"), wxOK | wxICON_ERROR, NULL);
	}
	else if (LastStartupState) {
#if wxUSE_STACKWALKER == 1
		if (stackWalk) {
			StackWalker walker(_T("Fatal exception"));
			walker.WalkFromException();
		}
#endif
		wxMessageBox(wxString::Format("Aegisub has crashed while starting up!\n\nThe last startup step attempted was: %s.", LastStartupState), _("Program error"), wxOK | wxICON_ERROR);
	}
}

/// @brief Called during an unhandled exception
void AegisubApp::OnUnhandledException() {
	UnhandledExeception(false);
}


/// @brief Called during a fatal exception.
void AegisubApp::OnFatalException() {
	UnhandledExeception(true);
}
#endif


void AegisubApp::HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const {
#define SHOW_EXCEPTION(str) wxMessageBox(str, L"Exception in event handler", wxOK|wxICON_ERROR|wxSTAY_ON_TOP)
	try {
		wxApp::HandleEvent(handler, func, event);
	}
	catch (const agi::Exception &e) {
		SHOW_EXCEPTION(lagi_wxString(e.GetChainedMessage()));
	}
	catch (const std::exception &e) {
		SHOW_EXCEPTION(wxString(e.what(), wxConvUTF8));
	}
	catch (const wchar_t *e) {
		SHOW_EXCEPTION(wxString(e));
	}
	catch (const char *e) {
		SHOW_EXCEPTION(wxString(e, wxConvUTF8));
	}
	catch (const wxString &e) {
		SHOW_EXCEPTION(e);
	}
#undef SHOW_EXCEPTION
}



#if wxUSE_STACKWALKER == 1
/// @brief Called at the start of walking the stack.
/// @param cause cause of the crash.
///
StackWalker::StackWalker(wxString cause) {

	wxFileName report_dir("");
	report_dir.SetPath(StandardPaths::DecodePath(_T("?user/reporter")));
	if (!report_dir.DirExists()) report_dir.Mkdir();

	crash_text = new wxFile(StandardPaths::DecodePath("?user/crashlog.txt"), wxFile::write_append);
	crash_xml = new wxFile(StandardPaths::DecodePath("?user/reporter/crash.xml"), wxFile::write);

	if ((crash_text->IsOpened()) && (crash_xml->IsOpened())) {
		wxDateTime time = wxDateTime::Now();

		crash_text->Write(wxString::Format("--- %s ------------------\n", time.FormatISOCombined()));
		crash_text->Write(wxString::Format("VER - %s\n", GetAegisubLongVersionString()));
		crash_text->Write(wxString::Format("FTL - Begining stack dump for \"%s\":\n", cause));

		crash_xml->Write(                 "<crash>\n");
		crash_xml->Write(                 "  <info>\n");
		crash_xml->Write(wxString::Format("    <cause>%s</cause>\n", cause));
		crash_xml->Write(wxString::Format("    <time>%s</time>\n", time.FormatISOCombined()));
		crash_xml->Write(wxString::Format("    <version>%s</version>\n", GetAegisubLongVersionString()));
		crash_xml->Write(                 "  </info>\n");
		crash_xml->Write(                 "  <trace>\n");
	}
}


/// @brief Callback to format a single frame
/// @param frame frame to parse.
///
void StackWalker::OnStackFrame(const wxStackFrame &frame) {

	if ((crash_text->IsOpened()) && (crash_xml->IsOpened())) {

		wxString dst = wxString::Format("%03i - 0x%08X: ",frame.GetLevel(),frame.GetAddress()) + frame.GetName();
		if (frame.HasSourceLocation())
			dst = wxString::Format("%s on %s:%d", dst, frame.GetFileName(), frame.GetLine());

		crash_text->Write(wxString::Format("%s\n", dst));

		crash_xml->Write(wxString::Format("    <frame id='%i' loc='%X'>\n", frame.GetLevel(), frame.GetAddress()));
		crash_xml->Write(wxString::Format("      <name>%s</name>\n", frame.GetName()));
		if (frame.HasSourceLocation())
		crash_xml->Write(wxString::Format("      <file line='%d'>%s</file>>%s</name>\n", frame.GetLine(), frame.GetFileName()));
		crash_xml->Write(wxString::Format("      <module><![CDATA[%s]]></module>\n", frame.GetModule()));
		crash_xml->Write(                 "    </frame>\n");
	}
}


/// @brief Called at the end of walking the stack.
StackWalker::~StackWalker() {

	if ((crash_text->IsOpened()) && (crash_xml->IsOpened())) {

		crash_text->Write("End of stack dump.\n");
		crash_text->Write("----------------------------------------\n\n");

		crash_text->Close();

		crash_xml->Write("  </trace>\n");
		crash_xml->Write("</crash>\n");

		crash_xml->Close();
	}
}
#endif


/// @brief Call main loop 
/// @return 
///
int AegisubApp::OnRun() {
	wxString error;

	// Run program
	try {
		if (m_exitOnFrameDelete == Later) m_exitOnFrameDelete = Yes;
		return MainLoop();
	}

	// Catch errors
	catch (const wxString &err) { error = err; }
	catch (const wxChar *err) { error = err; }
	catch (const std::exception &e) { error = wxString(_T("std::exception: ")) + wxString(e.what(),wxConvUTF8); }
	catch (const agi::Exception &e) { error = "agi::exception: " + lagi_wxString(e.GetChainedMessage()); }
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

#if wxUSE_EXCEPTIONS
		OnUnhandledException();
#endif
	}

	ExitMainLoop();
	return 1;
}



/// @brief Open URL 
/// @param url 
///
void AegisubApp::OpenURL(wxString url) {
	wxLaunchDefaultBrowser(url, wxBROWSER_NEW_WINDOW);
}


////////////////
// Apple events
#ifdef __WXMAC__

/// @brief DOCME
/// @param filename 
///
void AegisubApp::MacOpenFile(const wxString &filename) {
	if (frame != NULL && !filename.empty()) {
		frame->LoadSubtitles(filename);
		wxFileName filepath(filename);
		OPT_SET("Path/Last/Subtitles")->SetString(STD_STR(filepath.GetPath()));
	}
}
#endif


///////////////
// Event table
BEGIN_EVENT_TABLE(AegisubApp,wxApp)
	EVT_MOUSEWHEEL(AegisubApp::OnMouseWheel)
END_EVENT_TABLE()



/// @brief Mouse wheel moved 
/// @param event 
///
void AegisubApp::OnMouseWheel(wxMouseEvent &event) {
	if (event.WasProcessed()) return;
	wxPoint pt;
	wxWindow *target = wxFindWindowAtPointer(pt);
	/*if (frame && (target == frame->audioBox->audioDisplay || target == frame->SubsGrid)) {
		if (target->IsShownOnScreen()) target->GetEventHandler()->ProcessEvent(event);
		else event.Skip();
	}
	else event.Skip();*/
}



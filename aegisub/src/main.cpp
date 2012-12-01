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

/// @file main.cpp
/// @brief Main entry point, as well as crash handling
/// @ingroup main
///

#include "config.h"

#include <sstream>

#include <wx/clipbrd.h>
#include <wx/config.h>
#include <wx/datetime.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>

#include "include/aegisub/menu.h"
#include "command/command.h"
#include "command/icon.h"
#include "include/aegisub/toolbar.h"
#include "include/aegisub/hotkey.h"

#include "ass_dialogue.h"
#include "ass_export_filter.h"
#include "ass_file.h"
#include "audio_box.h"
#include "auto4_base.h"
#include "charset_conv.h"
#include "compat.h"
#include "export_fixstyle.h"
#include "export_framerate.h"
#include "frame_main.h"
#include "main.h"
#include "libresrc/libresrc.h"
#include "plugin_manager.h"
#include "standard_paths.h"
#include "subtitle_format.h"
#include "version.h"
#include "video_context.h"
#include "utils.h"

#include <libaegisub/io.h>
#include <libaegisub/log.h>
#include <libaegisub/hotkey.h>
#include <libaegisub/scoped_ptr.h>

namespace config {
	agi::Options *opt = 0;
	agi::MRUManager *mru = 0;
}


///////////////////
// wxWidgets macro
IMPLEMENT_APP(AegisubApp)

static const char *LastStartupState = nullptr;

#ifdef WITH_STARTUPLOG
#define StartupLog(a) MessageBox(0, L ## a, L"Aegisub startup log", 0)
#else
#define StartupLog(a) LastStartupState = a
#endif

#ifdef __VISUALC__

/// DOCME
#define MS_VC_EXCEPTION 0x406d1388

/// Parameters for setting the thread name
struct THREADNAME_INFO {
	DWORD dwType;     ///< must be 0x1000
	LPCSTR szName;    ///< pointer to name (in same addr space)
	DWORD dwThreadID; ///< thread ID (-1 caller thread)
	DWORD dwFlags;    ///< reserved for future use, most be zero
};

/// Set the name of a thread in the visual studio debugger
/// @param dwThreadID Thread ID, or -1 for caller
/// @param szThreadName New name for the thread
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

void AegisubApp::OnAssertFailure(const wxChar *file, int line, const wxChar *func, const wxChar *cond, const wxChar *msg) {
	LOG_A("wx/assert") << file << ":" << line << ":" << func << "() " << cond << ": " << msg;
	wxApp::OnAssertFailure(file, line, func, cond, msg);
}

AegisubApp::AegisubApp() {
	// http://trac.wxwidgets.org/ticket/14302
	wxSetEnv("UBUNTU_MENUPROXY", "0");
}

/// @brief Gets called when application starts.
/// @return bool
bool AegisubApp::OnInit() {
	// App name (yeah, this is a little weird to get rid of an odd warning)
#if defined(__WXMSW__) || defined(__WXMAC__)
	SetAppName("Aegisub");
#else
	SetAppName("aegisub");
#endif

	Bind(EVT_CALL_THUNK, [](wxThreadEvent &evt) {
		evt.GetPayload<std::function<void()>>()();
	});

	// logging.
	agi::log::log = new agi::log::LogSink;

#ifdef _DEBUG
	agi::log::log->Subscribe(new agi::log::EmitSTDOUT());
#endif

	// Set config file
	StartupLog("Load local configuration");
#ifdef __WXMSW__
	// Try loading configuration from the install dir if one exists there
	try {
		std::string conf_local(STD_STR(StandardPaths::DecodePath("?data/config.json")));
		agi::scoped_ptr<std::istream> localConfig(agi::io::Open(conf_local));
		config::opt = new agi::Options(conf_local, GET_DEFAULT_CONFIG(default_config));

		// Local config, make ?user mean ?data so all user settings are placed in install dir
		StandardPaths::SetPathValue("?user", StandardPaths::DecodePath("?data"));
		StandardPaths::SetPathValue("?local", StandardPaths::DecodePath("?data"));
	} catch (agi::FileNotAccessibleError const&) {
		// File doesn't exist or we can't read it
		// Might be worth displaying an error in the second case
	}
#endif

	StartupLog("Create log writer");
	wxString path_log = StandardPaths::DecodePath("?user/log/");
	wxFileName::Mkdir(path_log, 0777, wxPATH_MKDIR_FULL);
	agi::log::log->Subscribe(new agi::log::JsonEmitter(STD_STR(path_log), agi::log::log));
	CleanCache(path_log, "*.json", 10, 100);

	StartupLog("Load user configuration");
	try {
		if (!config::opt)
			config::opt = new agi::Options(STD_STR(StandardPaths::DecodePath("?user/config.json")), GET_DEFAULT_CONFIG(default_config));
		std::istringstream stream(GET_DEFAULT_CONFIG(default_config_platform));
		config::opt->ConfigNext(stream);
	} catch (agi::Exception& e) {
		LOG_E("config/init") << "Caught exception: " << e.GetName() << " -> " << e.GetMessage();
	}

	try {
		config::opt->ConfigUser();
	}
	catch (agi::Exception const& err) {
		wxMessageBox("Configuration file is invalid. Error reported:\n" + lagi_wxString(err.GetMessage()), "Error");
	}

	// Init commands.
	cmd::init_builtin_commands();

	// Init hotkeys
	hotkey::init();

	// Init icons.
	icon::icon_init();

	StartupLog("Load MRU");
	config::mru = new agi::MRUManager(STD_STR(StandardPaths::DecodePath("?user/mru.json")), GET_DEFAULT_CONFIG(default_mru), config::opt);

#ifdef __VISUALC__
	SetThreadName((DWORD) -1,"AegiMain");
#endif

	StartupLog("Inside OnInit");
	frame = nullptr;
	try {
		// Initialize randomizer
		StartupLog("Initialize random generator");
		srand(time(nullptr));

		// locale for loading options
		StartupLog("Set initial locale");
		setlocale(LC_NUMERIC, "C");
		setlocale(LC_CTYPE, "C");

		// Crash handling
#if (!defined(_DEBUG) || defined(WITH_EXCEPTIONS)) && (wxUSE_ON_FATAL_EXCEPTION+0)
		StartupLog("Install exception handler");
		wxHandleFatalExceptions(true);
#endif

		StartupLog("Store options back");
		OPT_SET("Version/Last Version")->SetInt(GetSVNRevision());

		StartupLog("Initialize final locale");

		// Set locale
		wxString lang = lagi_wxString(OPT_GET("App/Language")->GetString());
		if (!lang) {
			lang = locale.PickLanguage();
			OPT_SET("App/Language")->SetString(STD_STR(lang));
		}
		locale.Init(lang);

		// Load plugins
		plugins = new PluginManager();
		plugins->RegisterBuiltInPlugins();

		// Load Automation scripts
		StartupLog("Load global Automation scripts");
		global_scripts = new Automation4::AutoloadScriptManager(lagi_wxString(OPT_GET("Path/Automation/Autoload")->GetString()));

		// Load export filters
		StartupLog("Register export filters");
		AssExportFilterChain::Register(new AssFixStylesFilter);
		AssExportFilterChain::Register(new AssTransformFramerateFilter);

		// Get parameter subs
		StartupLog("Parse command line");
		wxArrayString subs;
		for (int i = 1; i < argc; ++i)
			subs.push_back(argv[i]);

		// Open main frame
		StartupLog("Create main window");
		frame = new FrameMain(subs);
		SetTopWindow(frame);
	}

	catch (const char *err) {
		wxMessageBox(err,"Fatal error while initializing");
		return false;
	}
	catch (agi::Exception const& e) {
		wxMessageBox(e.GetMessage(),"Fatal error while initializing");
		return false;
	}

#ifndef _DEBUG
	catch (...) {
		wxMessageBox("Unhandled exception","Fatal error while initializing");
		return false;
	}
#endif

	StartupLog("Clean old autosave files");
	wxString autosave_path = StandardPaths::DecodePath(lagi_wxString(OPT_GET("Path/Auto/Save")->GetString()));
	CleanCache(autosave_path, "*.AUTOSAVE.ass", 100, 1000);

	StartupLog("Initialization complete");
	return true;
}

int AegisubApp::OnExit() {
	if (frame)
		delete frame;

	SubtitleFormat::DestroyFormats();
	VideoContext::OnExit();
	delete plugins;
	delete config::opt;
	delete config::mru;
	hotkey::clear();
	cmd::clear();

	delete global_scripts;

	AssExportFilterChain::Clear();

	// Keep this last!
	delete agi::log::log;

	return wxApp::OnExit();
}

/// Message displayed when an exception has occurred.
const static wxString exception_message = _("Oops, Aegisub has crashed!\n\nAn attempt has been made to save a copy of your file to:\n\n%s\n\nAegisub will now close.");

static void UnhandledExeception(bool stackWalk) {
#if (!defined(_DEBUG) || defined(WITH_EXCEPTIONS)) && (wxUSE_ON_FATAL_EXCEPTION+0)
	if (AssFile::top) {
		// Current filename if any.
		wxFileName file(AssFile::top->filename);
		if (!file.HasName()) file.SetName("untitled");

		// Set path and create if it doesn't exist.
		file.SetPath(StandardPaths::DecodePath("?user/recovered"));
		if (!file.DirExists()) file.Mkdir();

		// Save file
		wxString filename = wxString::Format("%s/%s.%s.ass", file.GetPath(), file.GetName(), wxDateTime::Now().Format("%Y-%m-%d-%H-%M-%S"));
		AssFile::top->Save(filename,false,false);

#if wxUSE_STACKWALKER == 1
		if (stackWalk) {
			StackWalker walker("Fatal exception");
			walker.WalkFromException();
		}
#endif

		// Inform user of crash.
		wxMessageBox(wxString::Format(exception_message, filename), _("Program error"), wxOK | wxICON_ERROR | wxCENTER, nullptr);
	}
	else if (LastStartupState) {
#if wxUSE_STACKWALKER == 1
		if (stackWalk) {
			StackWalker walker("Fatal exception");
			walker.WalkFromException();
		}
#endif
		wxMessageBox(wxString::Format("Aegisub has crashed while starting up!\n\nThe last startup step attempted was: %s.", LastStartupState), _("Program error"), wxOK | wxICON_ERROR | wxCENTER);
	}
#endif
}

/// @brief Called during an unhandled exception
void AegisubApp::OnUnhandledException() {
	UnhandledExeception(false);
}

/// @brief Called during a fatal exception.
void AegisubApp::OnFatalException() {
	UnhandledExeception(true);
}

void AegisubApp::HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const {
#define SHOW_EXCEPTION(str) wxMessageBox(str, "Exception in event handler", wxOK | wxICON_ERROR | wxCENTER | wxSTAY_ON_TOP)
	try {
		wxApp::HandleEvent(handler, func, event);
	}
	catch (const agi::Exception &e) {
		SHOW_EXCEPTION(lagi_wxString(e.GetChainedMessage()));
	}
	catch (const std::exception &e) {
		SHOW_EXCEPTION(wxString(e.what(), wxConvUTF8));
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
	report_dir.SetPath(StandardPaths::DecodePath("?user/reporter"));
	if (!report_dir.DirExists()) report_dir.Mkdir();

	crash_text = new wxFile(StandardPaths::DecodePath("?user/crashlog.txt"), wxFile::write_append);
	crash_xml = new wxFile(StandardPaths::DecodePath("?user/reporter/crash.xml"), wxFile::write);

	if ((crash_text->IsOpened()) && (crash_xml->IsOpened())) {
		wxDateTime time = wxDateTime::Now();

		crash_text->Write(wxString::Format("--- %s ------------------\n", time.FormatISOCombined()));
		crash_text->Write(wxString::Format("VER - %s\n", GetAegisubLongVersionString()));
		crash_text->Write(wxString::Format("FTL - Beginning stack dump for \"%s\":\n", cause));

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

		wxString dst = wxString::Format("%03u - %p: ", (unsigned)frame.GetLevel(),frame.GetAddress()) + frame.GetName();
		if (frame.HasSourceLocation())
			dst = wxString::Format("%s on %s:%u", dst, frame.GetFileName(), (unsigned)frame.GetLine());

		crash_text->Write(wxString::Format("%s\n", dst));

		crash_xml->Write(wxString::Format("    <frame id='%u' loc='%p'>\n", (int)frame.GetLevel(), frame.GetAddress()));
		crash_xml->Write(wxString::Format("      <name>%s</name>\n", frame.GetName()));
		if (frame.HasSourceLocation())
			crash_xml->Write(wxString::Format("      <file line='%u'>%s</file>\n", (unsigned)frame.GetLine(), frame.GetFileName()));
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

int AegisubApp::OnRun() {
	wxString error;

	// Run program
	try {
		if (m_exitOnFrameDelete == Later) m_exitOnFrameDelete = Yes;
		return MainLoop();
	}

	// Catch errors
	catch (const wxString &err) { error = err; }
	catch (const char *err) { error = err; }
	catch (const std::exception &e) { error = wxString("std::exception: ") + wxString(e.what(),wxConvUTF8); }
	catch (const agi::Exception &e) { error = "agi::exception: " + lagi_wxString(e.GetChainedMessage()); }
	catch (...) { error = "Program terminated in error."; }

	// Report errors
	if (!error.IsEmpty()) {
		std::ofstream file;
		file.open(wxString(StandardPaths::DecodePath("?user/crashlog.txt")).mb_str(),std::ios::out | std::ios::app);
		if (file.is_open()) {
			wxDateTime time = wxDateTime::Now();
			wxString timeStr = "---" + time.FormatISODate() + " " + time.FormatISOTime() + "------------------";
			file << std::endl << timeStr.mb_str(csConvLocal);
			file << "\nVER - " << GetAegisubLongVersionString();
			file << "\nEXC - Aegisub has crashed with unhandled exception \"" << error.mb_str(csConvLocal) <<"\".\n";
			file << wxString('-', timeStr.size());
			file << "\n";
			file.close();
		}

		OnUnhandledException();
	}

	ExitMainLoop();
	return 1;
}

#ifdef __WXMAC__
void AegisubApp::MacOpenFile(const wxString &filename) {
	if (frame != nullptr && !filename.empty()) {
		frame->LoadSubtitles(filename);
		wxFileName filepath(filename);
		OPT_SET("Path/Last/Subtitles")->SetString(STD_STR(filepath.GetPath()));
	}
}
#endif

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

#include "command/command.h"
#include "include/aegisub/hotkey.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "auto4_base.h"
#include "compat.h"
#include "export_fixstyle.h"
#include "export_framerate.h"
#include "frame_main.h"
#include "include/aegisub/context.h"
#include "main.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "plugin_manager.h"
#include "subs_controller.h"
#include "subtitle_format.h"
#include "version.h"
#include "video_context.h"
#include "utils.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/fs.h>
#include <libaegisub/hotkey.h>
#include <libaegisub/io.h>
#include <libaegisub/log.h>
#include <libaegisub/path.h>
#include <libaegisub/util.h>

#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <sstream>

#include <wx/config.h>
#include <wx/msgdlg.h>
#include <wx/stackwalk.h>
#include <wx/utils.h>

namespace config {
	agi::Options *opt = nullptr;
	agi::MRUManager *mru = nullptr;
	agi::Path *path = nullptr;
}

IMPLEMENT_APP(AegisubApp)

static const char *LastStartupState = nullptr;

#ifdef WITH_STARTUPLOG
#define StartupLog(a) MessageBox(0, L ## a, L"Aegisub startup log", 0)
#else
#define StartupLog(a) LastStartupState = a
#endif

void AegisubApp::OnAssertFailure(const wxChar *file, int line, const wxChar *func, const wxChar *cond, const wxChar *msg) {
	LOG_A("wx/assert") << file << ":" << line << ":" << func << "() " << cond << ": " << msg;
	wxApp::OnAssertFailure(file, line, func, cond, msg);
}

AegisubApp::AegisubApp() {
	// http://trac.wxwidgets.org/ticket/14302
	wxSetEnv("UBUNTU_MENUPROXY", "0");
}

wxDEFINE_EVENT(EVT_CALL_THUNK, wxThreadEvent);

/// @brief Gets called when application starts.
/// @return bool
bool AegisubApp::OnInit() {
	// App name (yeah, this is a little weird to get rid of an odd warning)
#if defined(__WXMSW__) || defined(__WXMAC__)
	SetAppName("Aegisub");
#else
	SetAppName("aegisub");
#endif

	// The logger isn't created on demand on background threads, so force it to
	// be created now
	(void)wxLog::GetActiveTarget();

	// Set the global locale to the utf-8 version of the current locale
	std::locale::global(boost::locale::generator().generate(""));

#ifndef __APPLE__
	// Boost.FileSystem always uses UTF-8 for paths on OS X (since paths
	// actually are required to be UTF-8 strings rather than just opaque binary
	// blobs like on Linux), so there's no need to imbue the new locale and in
	// fact it actively breaks things for unknown reasons when launching the
	// app from Finder (but not from the command line).
	boost::filesystem::path::imbue(std::locale());
#endif

	// Pointless `this` capture required due to http://gcc.gnu.org/bugzilla/show_bug.cgi?id=51494
	agi::dispatch::Init([this](agi::dispatch::Thunk f) {
		auto evt = new wxThreadEvent(EVT_CALL_THUNK);
		evt->SetPayload(f);
		wxTheApp->QueueEvent(evt);
	});

	wxTheApp->Bind(EVT_CALL_THUNK, [](wxThreadEvent &evt) {
		evt.GetPayload<std::function<void()>>()();
	});

	config::path = new agi::Path;

	agi::log::log = new agi::log::LogSink;
#ifdef _DEBUG
	agi::log::log->Subscribe(agi::util::make_unique<agi::log::EmitSTDOUT>());
#endif

	// Set config file
	StartupLog("Load local configuration");
#ifdef __WXMSW__
	// Try loading configuration from the install dir if one exists there
	try {
		auto conf_local(config::path->Decode("?data/config.json"));
		std::unique_ptr<std::istream> localConfig(agi::io::Open(conf_local));
		config::opt = new agi::Options(conf_local, GET_DEFAULT_CONFIG(default_config));

		// Local config, make ?user mean ?data so all user settings are placed in install dir
		config::path->SetToken("?user", config::path->Decode("?data"));
		config::path->SetToken("?local", config::path->Decode("?data"));
	} catch (agi::fs::FileSystemError const&) {
		// File doesn't exist or we can't read it
		// Might be worth displaying an error in the second case
	}
#endif

	StartupLog("Create log writer");
	auto path_log = config::path->Decode("?user/log/");
	agi::fs::CreateDirectory(path_log);
	agi::log::log->Subscribe(agi::util::make_unique<agi::log::JsonEmitter>(path_log));
	CleanCache(path_log, "*.json", 10, 100);

	StartupLog("Load user configuration");
	try {
		if (!config::opt)
			config::opt = new agi::Options(config::path->Decode("?user/config.json"), GET_DEFAULT_CONFIG(default_config));
		std::istringstream stream(GET_DEFAULT_CONFIG(default_config_platform));
		config::opt->ConfigNext(stream);
	} catch (agi::Exception& e) {
		LOG_E("config/init") << "Caught exception: " << e.GetName() << " -> " << e.GetMessage();
	}

	try {
		config::opt->ConfigUser();
	}
	catch (agi::Exception const& err) {
		wxMessageBox("Configuration file is invalid. Error reported:\n" + to_wx(err.GetMessage()), "Error");
	}

	// Init commands.
	cmd::init_builtin_commands();

	// Init hotkeys
	hotkey::init();

	StartupLog("Load MRU");
	config::mru = new agi::MRUManager(config::path->Decode("?user/mru.json"), GET_DEFAULT_CONFIG(default_mru), config::opt);

	agi::util::SetThreadName("AegiMain");

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
		wxString lang = to_wx(OPT_GET("App/Language")->GetString());
		if (!lang) {
			lang = locale.PickLanguage();
			OPT_SET("App/Language")->SetString(from_wx(lang));
		}
		locale.Init(lang);

		// Load plugins
		RegisterBuiltInPlugins();

		// Load Automation scripts
		StartupLog("Load global Automation scripts");
		global_scripts = new Automation4::AutoloadScriptManager(OPT_GET("Path/Automation/Autoload")->GetString());

		// Load export filters
		StartupLog("Register export filters");
		AssExportFilterChain::Register(agi::util::make_unique<AssFixStylesFilter>());
		AssExportFilterChain::Register(agi::util::make_unique<AssTransformFramerateFilter>());

		// Open main frame
		StartupLog("Create main window");
		frame = new FrameMain;
		SetTopWindow(frame);

		// Get parameter subs
		StartupLog("Parse command line");
		wxArrayString subs;
		for (int i = 1; i < argc; ++i)
			subs.push_back(argv[i]);
		if (!subs.empty())
			frame->LoadList(subs);
	}

	catch (const char *err) {
		wxMessageBox(err,"Fatal error while initializing");
		return false;
	}
	catch (wxString const& err) {
		wxMessageBox(err, "Fatal error while initializing");
		return false;
	}
	catch (agi::Exception const& e) {
		wxMessageBox(to_wx(e.GetMessage()),"Fatal error while initializing");
		return false;
	}

#ifndef _DEBUG
	catch (...) {
		wxMessageBox("Unhandled exception","Fatal error while initializing");
		return false;
	}
#endif

	StartupLog("Clean old autosave files");
	CleanCache(config::path->Decode(OPT_GET("Path/Auto/Save")->GetString()), "*.AUTOSAVE.ass", 100, 1000);

	StartupLog("Initialization complete");
	return true;
}

int AegisubApp::OnExit() {
	if (frame)
		delete frame;

	if (wxTheClipboard->Open()) {
		wxTheClipboard->Flush();
		wxTheClipboard->Close();
	}

	SubtitleFormat::DestroyFormats();
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

#if wxUSE_STACKWALKER == 1
class StackWalker: public wxStackWalker {
	boost::filesystem::ofstream fp;

public:
	StackWalker(std::string const& cause);
	~StackWalker();
	void OnStackFrame(wxStackFrame const& frame) override;
};

/// @brief Called at the start of walking the stack.
/// @param cause cause of the crash.
StackWalker::StackWalker(std::string const& cause)
: fp(config::path->Decode("?user/crashlog.txt"), std::ios::app)
{
	if (!fp.good()) return;

	fp << agi::util::strftime("--- %y-%m-%d %H:%M:%S ------------------\n");
	fp << boost::format("VER - %s\n") % GetAegisubLongVersionString();
	fp << boost::format("FTL - Beginning stack dump for \"%s\": \n") % cause;
}

/// @brief Callback to format a single frame
/// @param frame frame to parse.
void StackWalker::OnStackFrame(wxStackFrame const& frame) {
	if (!fp.good()) return;

	fp << boost::format("%03u - %p: %s") % frame.GetLevel() % frame.GetAddress() % frame.GetName().utf8_str().data();
	if (frame.HasSourceLocation())
		fp << boost::format(" on %s:%u") % frame.GetFileName().utf8_str().data() % frame.GetLine();

	fp << "\n";
}

/// @brief Called at the end of walking the stack.
StackWalker::~StackWalker() {
	if (!fp.good()) return;

	fp << "End of stack dump.\n";
	fp << "----------------------------------------\n\n";
}
#endif

/// Message displayed when an exception has occurred.
const static wxString exception_message = _("Oops, Aegisub has crashed!\n\nAn attempt has been made to save a copy of your file to:\n\n%s\n\nAegisub will now close.");

static void UnhandledExeception(bool stackWalk, agi::Context *c) {
#if (!defined(_DEBUG) || defined(WITH_EXCEPTIONS)) && (wxUSE_ON_FATAL_EXCEPTION+0)
	if (c->ass && c->subsController) {
		auto path = config::path->Decode("?user/recovered");
		agi::fs::CreateDirectory(path);

		auto filename = c->subsController->Filename().stem();
		filename.replace_extension(str(boost::format("%s.ass") % agi::util::strftime("%Y-%m-%d-%H-%M-%S")));
		path /= filename;
		c->subsController->Save(path);

#if wxUSE_STACKWALKER == 1
		if (stackWalk) {
			StackWalker walker("Fatal exception");
			walker.WalkFromException();
		}
#endif

		// Inform user of crash.
		wxMessageBox(wxString::Format(exception_message, path.wstring()), _("Program error"), wxOK | wxICON_ERROR | wxCENTER, nullptr);
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

void AegisubApp::OnUnhandledException() {
	UnhandledExeception(false, frame ? frame->context.get() : nullptr);
}

void AegisubApp::OnFatalException() {
	UnhandledExeception(true, frame ? frame->context.get() : nullptr);
}

#define SHOW_EXCEPTION(str) \
	wxMessageBox(wxString::Format(_("An unexpected error has occurred. Please save your work and restart Aegisub.\n\nError Message: %s"), str), \
				"Exception in event handler", wxOK | wxICON_ERROR | wxCENTER | wxSTAY_ON_TOP)
void AegisubApp::HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const {
	try {
		wxApp::HandleEvent(handler, func, event);
	}
	catch (...) {
		const_cast<AegisubApp *>(this)->OnExceptionInMainLoop();
	}
}

bool AegisubApp::OnExceptionInMainLoop() {
	try {
		throw;
	}
	catch (const agi::Exception &e) {
		SHOW_EXCEPTION(to_wx(e.GetChainedMessage()));
	}
	catch (const std::exception &e) {
		SHOW_EXCEPTION(to_wx(e.what()));
	}
	catch (const char *e) {
		SHOW_EXCEPTION(to_wx(e));
	}
	catch (const wxString &e) {
		SHOW_EXCEPTION(e);
	}
	catch (...) {
		SHOW_EXCEPTION("Unknown error");
	}
	return true;
}

#undef SHOW_EXCEPTION

int AegisubApp::OnRun() {
	std::string error;

	try {
		if (m_exitOnFrameDelete == Later) m_exitOnFrameDelete = Yes;
		return MainLoop();
	}
	catch (const wxString &err) { error = from_wx(err); }
	catch (const char *err) { error = err; }
	catch (const std::exception &e) { error = std::string("std::exception: ") + e.what(); }
	catch (const agi::Exception &e) { error = "agi::exception: " + e.GetChainedMessage(); }
	catch (...) { error = "Program terminated in error."; }

	// Report errors
	if (!error.empty()) {
		boost::filesystem::ofstream file(config::path->Decode("?user/crashlog.txt"), std::ios::app);
		if (file.is_open()) {
			file << agi::util::strftime("--- %y-%m-%d %H:%M:%S ------------------\n");
			file << boost::format("VER - %s\n") % GetAegisubLongVersionString();
			file << boost::format("EXC - Aegisub has crashed with unhandled exception \"%s\".\n") % error;
			file << "----------------------------------------\n\n";
			file.close();
		}

		OnUnhandledException();
	}

	ExitMainLoop();
	return 1;
}

void AegisubApp::MacOpenFile(const wxString &filename) {
	if (frame && !filename.empty())
		frame->context->subsController->Load(agi::fs::path(filename));
}

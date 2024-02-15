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

#include "main.h"

#include "command/command.h"
#include "include/aegisub/hotkey.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "auto4_base.h"
#include "auto4_lua_factory.h"
#include "cli.h"
#include "compat.h"
#include "crash_writer.h"
#include "dialogs.h"
#include "export_fixstyle.h"
#include "export_framerate.h"
#include "format.h"
#include "frame_main.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "project.h"
#include "selection_controller.h"
#include "subs_controller.h"
#include "subtitles_provider_libass.h"
#include "utils.h"
#include "value_event.h"
#include "version.h"
#include "xdg_desktop_portal_utils.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/format_path.h>
#include <libaegisub/fs.h>
#include <libaegisub/io.h>
#include <libaegisub/log.h>
#include <libaegisub/path.h>
#include <libaegisub/split.h>
#include <libaegisub/util.h>

#include <boost/filesystem/operations.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <wx/clipbrd.h>
#include <wx/msgdlg.h>
#include <wx/stackwalk.h>
#include <wx/utils.h>

namespace config {
	agi::Options *opt = nullptr;
	agi::MRUManager *mru = nullptr;
	agi::Path *path = nullptr;
	Automation4::AutoloadScriptManager *global_scripts;

	bool hasGui = false;
	bool hasInitializedWx = false;
	bool loadGlobalAutomation = false;
	std::map<std::string, std::vector<std::string>> choice_indices;
	std::list<std::pair<int, std::string>> dialog_responses;
	std::list<std::vector<agi::fs::path>> file_responses;
}

wxIMPLEMENT_APP_NO_MAIN(AegisubApp);

static const char *LastStartupState = nullptr;

#ifdef WITH_STARTUPLOG
#define StartupLog(a) MessageBox(0, L ## a, L"Aegisub startup log", 0)
#else
#define StartupLog(a) LastStartupState = a
#endif

void AegisubApp::OnAssertFailure(const wxChar *file, int line, const wxChar *func, const wxChar *cond, const wxChar *msg) {
	LOG_A("wx/assert") << wxString(file) << ":" << line << ":" << wxString(func) << "() " << wxString(cond) << ": " << wxString(msg);
	wxApp::OnAssertFailure(file, line, func, cond, msg);
}

AegisubApp::AegisubApp() {
	// http://trac.wxwidgets.org/ticket/14302
	wxSetEnv("UBUNTU_MENUPROXY", "0");

	// Fallback to X11 if wxGTK implementation is build without Wayland EGL support
	// Fix https://github.com/TypesettingTools/Aegisub/issues/233
	#if defined(__WXGTK__) && !wxUSE_GLCANVAS_EGL && !wxHAS_EGL
		wxString xdg_session_type = wxGetenv("XDG_SESSION_TYPE");
		wxString wayland_display  = wxGetenv("WAYLAND_DISPLAY");
		wxString gdk_backend  = wxGetenv("GDK_BACKEND");	// Do not override GDK_BACKEND if it's already manually set

		if ((xdg_session_type == "wayland" || wayland_display.Contains("wayland")) && gdk_backend != "x11") {
			printf("Warning: Running on Wayland, but wxWidgets is not compiled with EGL support.");
			if (gdk_backend.empty()) {
				printf(" Falling back to X11.");
				wxSetEnv("GDK_BACKEND", "x11");
			} else {
				printf(" Set GDK_BACKEND=x11 to run Aegisub under X11.");
			}
			printf("\n");
		}
	#endif

}

namespace {
wxDEFINE_EVENT(EVT_CALL_THUNK, ValueEvent<agi::dispatch::Thunk>);
}

/// Message displayed when an exception has occurred.
static wxString exception_message = "Oops, Aegisub has crashed!\n\nAn attempt has been made to save a copy of your file to:\n\n%s\n\nAegisub will now close.";

/// @brief Non-GUI-specific initialization, shared between the GUI startup
/// path and CLI mode. @a app is null when running without a wxApp.
bool AegisubInitialize(AegisubApp *app, std::function<void(std::string, std::string)> showError) {
	config::path = new agi::Path;

	agi::log::log = new agi::log::LogSink;
#ifdef _DEBUG
	agi::log::log->Subscribe(std::make_unique<agi::log::EmitSTDOUT>());
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
	crash_writer::Initialize(config::path->Decode("?user"));

	if (config::hasGui) {
		StartupLog("Create log writer");
		auto path_log = config::path->Decode("?user/log/");
		agi::fs::CreateDirectory(path_log);
		agi::log::log->Subscribe(std::make_unique<agi::log::JsonEmitter>(path_log));
		CleanCache(path_log, "*.json", 10, 100);
	}

	StartupLog("Load user configuration");
	try {
		if (!config::opt)
			config::opt = new agi::Options(config::path->Decode("?user/config.json"), GET_DEFAULT_CONFIG(default_config));
	} catch (agi::Exception& e) {
		LOG_E("config/init") << "Caught exception: " << e.GetMessage();
	}

	try {
		config::opt->ConfigUser();
	}
	catch (agi::Exception const& err) {
		wxMessageBox(fmt_tl("Configuration file is invalid. Error reported:\n%s", err.GetMessage()), _("Error"));
	}

#ifdef _WIN32
	StartupLog("Load installer configuration");
	if (OPT_GET("App/First Start")->GetBool()) {
		try {
			auto installer_config = agi::io::Open(config::path->Decode("?data/installer_config.json"));
			config::opt->ConfigNext(*installer_config.get());
		} catch (agi::fs::FileSystemError const&) {
			// Not an error obviously as the user may not have used the installer
		}
	}
#endif

	// Init commands.
	cmd::init_builtin_commands();

	// Init hotkeys
	hotkey::init();

	StartupLog("Load MRU");
	config::mru = new agi::MRUManager(config::hasGui ? config::path->Decode("?user/mru.json") : "", GET_DEFAULT_CONFIG(default_mru), config::opt);

	agi::util::SetThreadName("AegiMain");

	StartupLog("Inside OnInit");
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

		if (app) {
			auto lang = OPT_GET("App/Language")->GetString();
			if (lang.empty() || (lang != "en_US" && !app->locale.HasLanguage(lang))) {
				lang = app->locale.PickLanguage();
				OPT_SET("App/Language")->SetString(lang);
			}
			app->locale.Init(lang);
		}

#ifdef __APPLE__
		// When run from an app bundle, LC_CTYPE defaults to "C", which breaks on
		// anything involving unicode and in some cases number formatting.
		// The right thing to do here would be to query CoreFoundation for the user's
		// locale and add .UTF-8 to that, but :effort:
		setlocale(LC_CTYPE, "en_US.UTF-8");
#endif

		exception_message = _("Oops, Aegisub has crashed!\n\nAn attempt has been made to save a copy of your file to:\n\n%s\n\nAegisub will now close.");

		agi::xdp_utils::Initialize();

		// Load plugins
		Automation4::ScriptFactory::Register(std::make_unique<Automation4::LuaScriptFactory>());
		libass::CacheFonts();

		// Load Automation scripts
		if (config::loadGlobalAutomation) {
			StartupLog("Load global Automation scripts");
			config::global_scripts = new Automation4::AutoloadScriptManager(OPT_GET("Path/Automation/Autoload")->GetString());

			// Load export filters
			StartupLog("Register export filters");
			AssExportFilterChain::Register(std::make_unique<AssFixStylesFilter>());
			AssExportFilterChain::Register(std::make_unique<AssTransformFramerateFilter>());
		}
	}
	catch (agi::Exception const& e) {
		showError(e.GetMessage(), "Fatal error while initializing");
		return false;
	}
	catch (std::exception const& e) {
		showError(e.what(), "Fatal error while initializing");
		return false;
	}
#ifndef _DEBUG
	catch (...) {
		showError("Fatal error while initializing", "Unhandled exception");
		return false;
	}
#endif
	return true;
}

std::unique_ptr<Automation4::Script> find_script(const std::string& file)
{
	auto absolute = agi::fs::path(file);
	auto relative = boost::filesystem::current_path() / file;

	agi::fs::path script;

	if (agi::fs::FileExists(absolute)) {
		script = absolute;
	} else if (agi::fs::FileExists(relative)) {
		script = relative;
	} else {
		auto autodirs = OPT_GET("Path/Automation/Autoload")->GetString();

		for (auto tok : agi::Split(autodirs, '|')) {
			auto dirname = config::path->Decode(agi::str(tok));
			if (!agi::fs::DirectoryExists(dirname)) continue;

			auto scriptname = dirname / file;
			if (agi::fs::FileExists(scriptname)) {
				script = scriptname;
			}
		}
	}

	if (script.empty()) {
		throw agi::InvalidInputException("Could not find script file: " + file);
	}

	return Automation4::ScriptFactory::CreateFromFile(script, true, false);
}

/// @brief Gets called when application starts.
/// @return int
int main(int argc, char *argv[]) {
	wxDISABLE_DEBUG_SUPPORT();

	boost::program_options::options_description cmdline("Options");
	boost::program_options::options_description flags("Options");
	boost::program_options::positional_options_description posdesc;

	cmdline.add_options()
		("in-file", boost::program_options::value<std::string>(), "input file")
		("out-file", boost::program_options::value<std::string>(), "output file")
		("macro", boost::program_options::value<std::string>(), "macro to run")
	;

	flags.add_options()
		("help", "produce help message")
		("cli", "run in CLI mode, without a GUI window. Enables the other options")
		("video", boost::program_options::value<std::string>(), "video to load")
		("timecodes", boost::program_options::value<std::string>(), "timecodes to load")
		("keyframes", boost::program_options::value<std::string>(), "keyframes to load")
		("automation", boost::program_options::value<std::vector<std::string>>(), "an automation script to run")
		("active-line", boost::program_options::value<int>()->default_value(-1), "the active line")
		("selected-lines", boost::program_options::value<std::string>()->default_value(""), "the selected lines")
		("dialog", boost::program_options::value<std::vector<std::string>>(), "response to a dialog, in JSON")
		("file", boost::program_options::value<std::vector<std::string>>(), "filename to supply to an open/save call")
	;

	//TODO figure this out properly

	cmdline.add(flags);
	posdesc.add("in-file", 1);
	posdesc.add("out-file", 1);
	posdesc.add("macro", 1);
	boost::program_options::variables_map vm;
	boost::program_options::store(
		boost::program_options::command_line_parser(argc, argv).
		options(cmdline).positional(posdesc).run(), vm);
	boost::program_options::notify(vm);

	bool cli = vm.count("cli");
	config::hasGui = !cli;

	if (vm.count("help") || (cli && !vm.count("macro"))) {
		if (!vm.count("help"))
			std::cout << "Too few arguments." << std::endl;
		std::cout << argv[0] << " [options] <input file> <output file> <macro>" << std::endl;
		std::cout << flags << std::endl;
		return 0;
	}

	agi::util::InitLocale();

	if (cli) {
		// TODO force everything onto one thread or figure something else out here
		agi::dispatch::Init([](agi::dispatch::Thunk f) {
			f();
		});

		if (!AegisubInitialize(nullptr, [&](std::string msg, std::string title) { std::cerr << title << ": " << msg << "\n"; })) {
			return -1;
		}

		agi::Context context;

		LOG_D("main") << "Loading subtitles...";
		context.project->LoadSubtitles(std::filesystem::absolute(vm["in-file"].as<std::string>()), "", false);

		if (vm.count("video")) {
			LOG_D("main") << "Loading video...";
			context.project->LoadVideo(std::filesystem::absolute(vm["video"].as<std::string>()));
		}

		if (vm.count("timecodes")) {
			LOG_D("main") << "Loading timecodes...";
			context.project->LoadTimecodes(std::filesystem::absolute(vm["timecodes"].as<std::string>()));
		}

		if (vm.count("keyframes")) {
			LOG_D("main") << "Loading keyframes...";
			context.project->LoadKeyframes(std::filesystem::absolute(vm["keyframes"].as<std::string>()));
		}

		auto active_index = vm["active-line"].as<int>();
		AssDialogue* active_line = nullptr;

		auto selected_indices = parse_range(vm["selected-lines"].as<std::string>());
		Selection selected_lines;

		int i = 0;
		for (auto& line : context.ass->Events) {
			if (i == active_index) {
				active_line = &line;
			}

			if (selected_indices.empty() || selected_indices.count(i)) {
				selected_lines.insert(&line);
				if (active_line == nullptr) {
					// assign first line in selection as a fallback
					active_line = &line;
				}
			}
			i++;
		}

		if (active_line == nullptr) {
			// selection was empty
			active_line = &context.ass->Events.front();
			selected_lines.insert(active_line);
		}

		context.selectionController->SetSelectionAndActive(
			std::move(selected_lines), active_line);

		if (vm.count("dialog"))
			config::dialog_responses = parse_dialog_responses(vm["dialog"].as<std::vector<std::string>>());

		if (vm.count("file"))
			config::file_responses = parse_file_responses(vm["file"].as<std::vector<std::string>>());

		// cache cwd in case automation changes it
		auto cwd = std::filesystem::current_path();

		std::vector<std::unique_ptr<Automation4::Script>> scripts;
		if (vm.count("automation")) {
			for (auto& s : vm["automation"].as<std::vector<std::string>>()) {
				LOG_D("main") << "Loading " << s;
				auto script = find_script(s);
				if (!script) {
					return 1;
				}
				scripts.emplace_back(std::move(script));
			}
		}

		auto macro = vm["macro"].as<std::string>();

		cmd::Command *cmd = nullptr;

		// Allow calling automation scripts by their display name
		for (auto const& script : scripts) {
			for (auto const& c: script->GetMacros()) {
				if (c->StrMenu(&context) == to_wx(macro)) {
					cmd = c;
				}
			}
		}

		// If we don't find one, try the command name instead
		if (!cmd) {
			try {
				cmd = cmd::get(macro);
			} catch (cmd::CommandNotFound const&) {
				std::cerr << "Command not found: " << macro << std::endl;
				LOG_E("main") << "Command not found: " << macro;
				return 1;
			}
		}

		if (!cmd->Validate(&context)) {
			LOG_E("main") << "Skipping automation because validation function returned false";
			return 1;
		}

		LOG_D("main") << "Calling " << cmd->name();
		(*cmd)(&context);

		// restore cwd for saving
		std::filesystem::current_path(cwd);
		context.subsController->Save(std::filesystem::absolute(vm["out-file"].as<std::string>()));

		if (config::hasInitializedWx) {
			wxUninitialize();
		}
		return 0;
	} else {
		config::loadGlobalAutomation = true;
		config::hasInitializedWx = true;
		return wxEntry(argc, argv);
	}
}


/// @brief wx's initialization function. Called from main() to initialize the GUI-specific stuff and then call Initialize()
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

	// Pointless `this` capture required due to http://gcc.gnu.org/bugzilla/show_bug.cgi?id=51494
	agi::dispatch::Init([this](agi::dispatch::Thunk f) {
		auto evt = new ValueEvent<agi::dispatch::Thunk>(EVT_CALL_THUNK, -1, std::move(f));
		wxTheApp->QueueEvent(evt);
	});

	wxTheApp->Bind(EVT_CALL_THUNK, [this](ValueEvent<agi::dispatch::Thunk>& evt) {
		try {
			evt.Get()();
		}
		catch (...) {
			OnExceptionInMainLoop();
		}
	});

	if (!::AegisubInitialize(this, [](std::string msg, std::string title) { wxMessageBox(to_wx(msg), to_wx(title)); })) {
		return false;
	}

	try {
		StartupLog("Install PNG handler");
		wxImage::AddHandler(new wxPNGHandler);

		// Open main frame
		StartupLog("Create main window");
		NewProjectContext();

		// Version checker
		StartupLog("Possibly perform automatic updates check");
		if (OPT_GET("App/First Start")->GetBool()) {
			OPT_SET("App/First Start")->SetBool(false);
#ifdef WITH_UPDATE_CHECKER
			int result = wxMessageBox(_("Do you want Aegisub to check for updates whenever it starts? You can still do it manually via the Help menu."),_("Check for updates?"), wxYES_NO | wxCENTER);
			OPT_SET("App/Auto/Check For Updates")->SetBool(result == wxYES);
			try {
				config::opt->Flush();
			}
			catch (agi::fs::FileSystemError const& e) {
				wxMessageBox(to_wx(e.GetMessage()), _("Error saving config file"), wxOK | wxICON_ERROR | wxCENTER);
			}
#endif
		}

#ifdef WITH_UPDATE_CHECKER
		PerformVersionCheck(false);
#endif

		// Get parameter subs
		StartupLog("Parse command line");
		auto const& args = argv.GetArguments();
		if (args.size() > 1)
			OpenFiles(wxArrayStringsAdapter(args.size() - 1, &args[1]));
	}
	catch (agi::Exception const& e) {
		wxMessageBox(to_wx(e.GetMessage()), _("Fatal error while initializing"));
		return false;
	}
	catch (std::exception const& e) {
		wxMessageBox(to_wx(e.what()), _("Fatal error while initializing"));
		return false;
	}
#ifndef _DEBUG
	catch (...) {
		wxMessageBox(_("Unhandled exception"), _("Fatal error while initializing"));
		return false;
	}
#endif

	StartupLog("Clean old autosave files");
	CleanCache(config::path->Decode(OPT_GET("Path/Auto/Save")->GetString()), "*.AUTOSAVE.ass", 100, 1000);

	StartupLog("Initialization complete");
	return true;
}

int AegisubApp::OnExit() {
	for (auto frame : frames)
		delete frame;
	frames.clear();

	if (wxTheClipboard->Open()) {
		wxTheClipboard->Flush();
		wxTheClipboard->Close();
	}

	delete config::opt;
	delete config::mru;
	hotkey::clear();
	cmd::clear();

	delete config::global_scripts;

	AssExportFilterChain::Clear();

	agi::xdp_utils::Cleanup();

	// Keep this last!
	delete agi::log::log;
	crash_writer::Cleanup();

	return wxApp::OnExit();
}

agi::Context& AegisubApp::NewProjectContext() {
	auto frame = new FrameMain;
	frame->Bind(wxEVT_DESTROY, [=, this](wxWindowDestroyEvent& evt) {
		if (evt.GetWindow() != frame) {
			evt.Skip();
			return;
		}

		frames.erase(remove(begin(frames), end(frames), frame), end(frames));
		if (frames.empty()) {
			ExitMainLoop();
		}
	});
	frames.push_back(frame);
	return *frame->context;
}

void AegisubApp::CloseAll() {
	for (auto frame : frames) {
		if (!frame->Close())
			break;
	}
}

void AegisubApp::UnhandledException([[maybe_unused]] bool stackWalk) {
#if (!defined(_DEBUG) || defined(WITH_EXCEPTIONS)) && (wxUSE_ON_FATAL_EXCEPTION+0)
	bool any = false;
	agi::fs::path path;
	for (auto& frame : frames) {
		auto c = frame->context.get();
		if (!c || !c->ass || !c->subsController) continue;

		path = config::path->Decode("?user/recovered");
		agi::fs::CreateDirectory(path);

		auto filename = c->subsController->Filename().stem();
		filename.replace_extension(agi::format("%s.ass", agi::util::strftime("%Y-%m-%d-%H-%M-%S")));
		path /= filename;
		c->subsController->Save(path);

		any = true;
	}

	if (stackWalk)
		crash_writer::Write();

	if (any) {
		// Inform user of crash.
		wxMessageBox(agi::wxformat(exception_message, path), _("Program error"), wxOK | wxICON_ERROR | wxCENTER, nullptr);
	}
	else if (LastStartupState) {
		wxMessageBox(fmt_tl("Aegisub has crashed while starting up!\n\nThe last startup step attempted was: %s.", LastStartupState), _("Program error"), wxOK | wxICON_ERROR | wxCENTER);
	}
#endif
}

void AegisubApp::OnUnhandledException() {
	UnhandledException(false);
}

void AegisubApp::OnFatalException() {
	UnhandledException(true);
}

#define SHOW_EXCEPTION(str) \
	wxMessageBox(fmt_tl("An unexpected error has occurred. Please save your work and restart Aegisub.\n\nError Message: %s", str), \
				_("Exception in event handler"), wxOK | wxICON_ERROR | wxCENTER | wxSTAY_ON_TOP)
bool AegisubApp::OnExceptionInMainLoop() {
	try {
		throw;
	}
	catch (const agi::Exception &e) {
		SHOW_EXCEPTION(to_wx(e.GetMessage()));
	}
	catch (const std::exception &e) {
		SHOW_EXCEPTION(to_wx(e.what()));
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
		return MainLoop();
	}
	catch (const std::exception &e) { error = std::string("std::exception: ") + e.what(); }
	catch (const agi::Exception &e) { error = "agi::exception: " + e.GetMessage(); }
	catch (...) { error = "Program terminated in error."; }

	// Report errors
	if (!error.empty()) {
		crash_writer::Write(error);
		OnUnhandledException();
	}

	ExitMainLoop();
	return 1;
}

void AegisubApp::MacOpenFiles(wxArrayString const& filenames) {
	OpenFiles(filenames);
}

void AegisubApp::OpenFiles(wxArrayStringsAdapter filenames) {
	std::vector<agi::fs::path> files;
	for (size_t i = 0; i < filenames.GetCount(); ++i)
		files.push_back(from_wx(filenames[i]));
	if (!files.empty())
		frames[0]->context->project->LoadList(files);
}

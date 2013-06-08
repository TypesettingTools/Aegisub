// Copyright (c) 2006, Niels Martin Hansen
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

/// @file auto4_base.h
/// @see auto4_base.cpp
/// @ingroup scripting
///

#pragma once

#include <libaegisub/background_runner.h>
#include <libaegisub/exception.h>
#include <libaegisub/fs_fwd.h>
#include <libaegisub/signal.h>

#include "ass_export_filter.h"
#include "compat.h"

#include <boost/filesystem/path.hpp>
#include <deque>
#include <memory>
#include <vector>

#include <wx/dialog.h>

class AssFile;
class AssStyle;
class DialogProgress;
class SubtitleFormat;
class wxWindow;
class wxDialog;

namespace agi { struct Context; }
namespace cmd { class Command; }

namespace Automation4 {
	DEFINE_BASE_EXCEPTION_NOINNER(AutomationError, agi::Exception)
	DEFINE_SIMPLE_EXCEPTION_NOINNER(ScriptLoadError, AutomationError, "automation/load/generic")
	DEFINE_SIMPLE_EXCEPTION_NOINNER(MacroRunError, AutomationError, "automation/macro/generic")

	// Calculate the extents of a text string given a style
	bool CalculateTextExtents(AssStyle *style, std::string const& text, double &width, double &height, double &descent, double &extlead);

	class ScriptDialog;

	class ExportFilter : public AssExportFilter {
		std::unique_ptr<ScriptDialog> config_dialog;

		/// subclasses should implement this, producing a new ScriptDialog
		virtual ScriptDialog* GenerateConfigDialog(wxWindow *parent, agi::Context *c) = 0;

	protected:
		std::string GetScriptSettingsIdentifier();

	public:
		ExportFilter(std::string const& name, std::string const& description, int priority);

		wxWindow* GetConfigDialogWindow(wxWindow *parent, agi::Context *c);
		void LoadSettings(bool is_default, agi::Context *c);

		// Subclasses must implement ProcessSubs from AssExportFilter
	};

	/// A "dialog" which actually generates a non-top-level window that is then
	/// either inserted into a dialog or into the export filter configuration
	/// panel
	class ScriptDialog {
	public:
		virtual ~ScriptDialog() { }

		/// Create a window with the given parent
		virtual wxWindow *CreateWindow(wxWindow *parent) = 0;

		/// Serialize the values of the controls in this dialog to a string
		/// suitable for storage in the subtitle script
		virtual std::string Serialise() { return ""; }

		/// Restore the values of the controls in this dialog from a string
		/// stored in the subtitle script
		virtual void Unserialise(std::string const& serialised) { }
	};

	class ProgressSink;

	class BackgroundScriptRunner {
		std::unique_ptr<DialogProgress> impl;

	public:
		wxWindow *GetParentWindow() const;
		std::string GetTitle() const;

		void Run(std::function<void(ProgressSink*)> task);

		BackgroundScriptRunner(wxWindow *parent, std::string const& title);
		~BackgroundScriptRunner();
	};

	/// A wrapper around agi::ProgressSink which adds the ability to open
	/// dialogs on the GUI thread
	class ProgressSink : public agi::ProgressSink {
		agi::ProgressSink *impl;
		BackgroundScriptRunner *bsr;
		int trace_level;
	public:
		void SetIndeterminate() { impl->SetIndeterminate(); }
		void SetTitle(std::string const& title) { impl->SetTitle(title); }
		void SetMessage(std::string const& msg) { impl->SetMessage(msg); }
		void SetProgress(int64_t cur, int64_t max) { impl->SetProgress(cur, max); }
		void Log(std::string const& str) { impl->Log(str); }
		bool IsCancelled() { return impl->IsCancelled(); }

		/// Show the passed dialog on the GUI thread, blocking the calling
		/// thread until it closes
		void ShowDialog(ScriptDialog *config_dialog);
		int ShowDialog(wxDialog *dialog);
		wxWindow *GetParentWindow() const { return bsr->GetParentWindow(); }

		/// Get the current automation trace level
		int GetTraceLevel() const { return trace_level; }

		ProgressSink(agi::ProgressSink *impl, BackgroundScriptRunner *bsr);
	};

	class Script {
		agi::fs::path filename;

	protected:
		/// The automation include path, consisting of the user-specified paths
		/// along with the script's path
		std::vector<agi::fs::path> include_path;
		Script(agi::fs::path const& filename);

	public:
		virtual ~Script() { }

		/// Reload this script
		virtual void Reload() = 0;

		/// The script's file name with path
		agi::fs::path GetFilename() const { return filename; }
		/// The script's file name without path
		agi::fs::path GetPrettyFilename() const { return filename.filename(); }
		/// The script's name. Not required to be unique.
		virtual std::string GetName() const=0;
		/// A short description of the script
		virtual std::string GetDescription() const=0;
		/// The author of the script
		virtual std::string GetAuthor() const=0;
		/// A version string that should not be used for anything but display
		virtual std::string GetVersion() const=0;
		/// Did the script load correctly?
		virtual bool GetLoadedState() const=0;

		/// Get a list of commands provided by this script
		virtual std::vector<cmd::Command*> GetMacros() const=0;
		/// Get a list of export filters provided by this script
		virtual std::vector<ExportFilter*> GetFilters() const=0;
		/// Get a list of subtitle formats provided by this script
		virtual std::vector<SubtitleFormat*> GetFormats() const=0;
	};

	/// A manager of loaded automation scripts
	class ScriptManager {
	protected:
		std::vector<std::unique_ptr<Script>> scripts;
		std::vector<cmd::Command*> macros;

		agi::signal::Signal<> ScriptsChanged;

	public:
		/// Deletes all scripts managed
		virtual ~ScriptManager();
		/// Add a script to the manager.
		void Add(std::unique_ptr<Script>&& script);
		/// Remove a script from the manager, and delete the Script object.
		void Remove(Script *script);
		/// Deletes all scripts managed
		void RemoveAll();
		/// Reload all scripts managed
		virtual void Reload() = 0;
		/// Reload a single managed script
		virtual void Reload(Script *script);

		/// Get all managed scripts (both loaded and invalid)
		const std::vector<std::unique_ptr<Script>>& GetScripts() const { return scripts; }

		const std::vector<cmd::Command*>& GetMacros();
		// No need to have getters for the other kinds of features, I think.
		// They automatically register themselves in the relevant places.

		DEFINE_SIGNAL_ADDERS(ScriptsChanged, AddScriptChangeListener)
	};

	/// Manager for scripts specified by a subtitle file
	class LocalScriptManager : public ScriptManager {
		std::deque<agi::signal::Connection> slots;
		agi::Context *context;

		void OnSubtitlesSave();
	public:
		LocalScriptManager(agi::Context *context);
		void Reload();
	};

	/// Manager for scripts in the autoload directory
	class AutoloadScriptManager : public ScriptManager {
		std::string path;
	public:
		AutoloadScriptManager(std::string const& path);
		void Reload();
	};

	/// Both a base class for script factories and a manager of registered
	/// script factories
	class ScriptFactory {
		std::string engine_name;
		std::string filename_pattern;

		/// Load a file, or return nullptr if the file is not in a supported
		/// format. If the file is in a supported format but is invalid, a
		/// script should be returned which returns false from IsLoaded and
		/// an appropriate error message from GetDescription.
		///
		/// This is private as it should only ever be called through
		/// CreateFromFile
		virtual std::unique_ptr<Script> Produce(agi::fs::path const& filename) const = 0;

		static std::vector<std::unique_ptr<ScriptFactory>>& Factories();

	protected:
		ScriptFactory(std::string const& engine_name, std::string const& filename_pattern);

	public:
		virtual ~ScriptFactory() { }

		/// Name of this automation engine
		const std::string& GetEngineName() const { return engine_name; }
		/// Extension which this engine supports
		const std::string& GetFilenamePattern() const { return filename_pattern; }

		/// Register an automation engine.
		static void Register(std::unique_ptr<ScriptFactory>&& factory);

		/// Get the full wildcard string for all loaded engines
		static std::string GetWildcardStr();

		/// Load a script from a file
		/// @param filename Script to load
		/// @param complain_about_unrecognised Should an error be displayed for files that aren't automation scripts?
		/// @param create_unknown Create a placeholder rather than returning nullptr if no script engine supports the file
		static std::unique_ptr<Script> CreateFromFile(agi::fs::path const& filename, bool complain_about_unrecognised, bool create_unknown=true);

		static const std::vector<std::unique_ptr<ScriptFactory>>& GetFactories();
	};

	/// A script which represents a file not recognized by any registered
	/// automation engines
	class UnknownScript : public Script {
	public:
		UnknownScript(agi::fs::path const& filename) : Script(filename) { }

		void Reload() { }

		std::string GetName() const { return GetFilename().stem().string(); }
		std::string GetDescription() const { return from_wx(_("File was not recognized as a script")); }
		std::string GetAuthor() const { return ""; }
		std::string GetVersion() const { return ""; }
		bool GetLoadedState() const { return false; }

		std::vector<cmd::Command*> GetMacros() const { return std::vector<cmd::Command*>(); }
		std::vector<ExportFilter*> GetFilters() const { return std::vector<ExportFilter*>(); }
		std::vector<SubtitleFormat*> GetFormats() const { return std::vector<SubtitleFormat*>(); }
	};
}

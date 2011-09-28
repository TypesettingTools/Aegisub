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
//
// $Id$

/// @file auto4_base.h
/// @see auto4_base.cpp
/// @ingroup scripting
///


#pragma once


#ifndef AGI_PRE
#include <vector>

#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/gauge.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/timer.h>
#endif

#include <libaegisub/background_runner.h>
#include <libaegisub/exception.h>
#include <libaegisub/scoped_ptr.h>
#include <libaegisub/signal.h>

#include "ass_export_filter.h"
#include "subtitle_format.h"


class AssFile;
class AssStyle;
class DialogProgress;
class SubtitleFormat;
class wxWindow;
class wxDialog;
class wxStopWatch;
class wxPathList;

namespace agi { struct Context; }


DECLARE_EVENT_TYPE(wxEVT_AUTOMATION_SCRIPT_COMPLETED, -1)



/// DOCME
namespace Automation4 {
	DEFINE_BASE_EXCEPTION_NOINNER(AutomationError, agi::Exception)
	DEFINE_SIMPLE_EXCEPTION_NOINNER(ScriptLoadError, AutomationError, "automation/load/generic")
	DEFINE_SIMPLE_EXCEPTION_NOINNER(MacroRunError, AutomationError, "automation/macro/generic")

	// Calculate the extents of a text string given a style
	bool CalculateTextExtents(AssStyle *style, wxString &text, double &width, double &height, double &descent, double &extlead);



	/// DOCME
	enum ScriptFeatureClass {

		/// DOCME
		SCRIPTFEATURE_MACRO = 0,

		/// DOCME
		SCRIPTFEATURE_FILTER,

		/// DOCME
		SCRIPTFEATURE_SUBFORMAT,


		/// DOCME
		SCRIPTFEATURE_MAX // must be last
	};


	// A Feature describes a function provided by a Script.
	// There are several distinct classes of features.
	class FeatureMacro;
	class FeatureFilter;
	class FeatureSubtitleFormat;

	/// DOCME
	/// @class Feature
	/// @brief DOCME
	///
	/// DOCME
	class Feature {
	private:

		/// DOCME
		ScriptFeatureClass featureclass;

		/// DOCME
		wxString name;

	protected:
		Feature(ScriptFeatureClass _featureclass, const wxString &_name);

	public:

		/// @brief DOCME
		///
		virtual ~Feature() { }

		ScriptFeatureClass GetClass() const;
		FeatureMacro* AsMacro();
		FeatureFilter* AsFilter();
		FeatureSubtitleFormat* AsSubFormat();

		virtual const wxString& GetName() const;
	};



	/// DOCME
	/// @class FeatureMacro
	/// @brief DOCME
	///
	/// DOCME
	class FeatureMacro : public virtual Feature {
	private:

		/// DOCME
		wxString description;

	protected:
		FeatureMacro(const wxString &_name, const wxString &_description);

	public:

		/// @brief DOCME
		///
		virtual ~FeatureMacro() { }

		const wxString& GetDescription() const;

		virtual bool Validate(AssFile *subs, const std::vector<int> &selected, int active) = 0;
		virtual void Process(AssFile *subs, std::vector<int> &selected, int active, wxWindow * const progress_parent) = 0;
	};


	class ScriptConfigDialog;

	/// DOCME
	/// @class FeatureFilter
	/// @brief DOCME
	///
	/// DOCME
	class FeatureFilter : public virtual Feature, public AssExportFilter {
	private:

		/// DOCME
		ScriptConfigDialog *config_dialog;

	protected:
		FeatureFilter(const wxString &_name, const wxString &_description, int _priority);

		// Subclasses should probably implement AssExportFilter::Init

		virtual ScriptConfigDialog* GenerateConfigDialog(wxWindow *parent) = 0; // subclasses should implement this, producing a new ScriptConfigDialog

		wxString GetScriptSettingsIdentifier();

	public:
		virtual ~FeatureFilter();

		wxWindow* GetConfigDialogWindow(wxWindow *parent);
		void LoadSettings(bool IsDefault);

		// Subclasses must implement ProcessSubs from AssExportFilter
	};



	/// DOCME
	/// @class FeatureSubtitleFormat
	/// @brief DOCME
	///
	/// DOCME
	class FeatureSubtitleFormat : public virtual Feature, public SubtitleFormat {
	private:

		/// DOCME
		wxString extension;

	protected:
		FeatureSubtitleFormat(const wxString &_name, const wxString &_extension);

	public:

		/// @brief DOCME
		/// @return 
		///
		virtual ~FeatureSubtitleFormat() { }

		const wxString& GetExtension() const;

		// Default implementations of these are provided, that just checks extension,
		// but subclasses can provide more elaborate implementations, or go back to
		// the "return false" implementation, in case of reader-only or writer-only.
		virtual bool CanWriteFile(wxString filename);
		virtual bool CanReadFile(wxString filename);

		// Subclasses should implement ReadFile and/or WriteFile here
	};



	/// DOCME
	/// @class ScriptConfigDialog
	/// @brief DOCME
	///
	/// DOCME
	class ScriptConfigDialog {
	private:

		/// DOCME
		wxWindow *win;

	protected:
		virtual wxWindow* CreateWindow(wxWindow *parent) = 0;

	public:

		/// @brief DOCME
		///
		ScriptConfigDialog() : win(0) { }

		/// @brief DOCME
		///
		virtual ~ScriptConfigDialog() { }
		wxWindow* GetWindow(wxWindow *parent);
		void DeleteWindow();
		virtual void ReadBack() = 0;

		virtual wxString Serialise();

		/// @brief DOCME
		/// @param serialised 
		///
		virtual void Unserialise(const wxString &serialised) { }
	};

	class ProgressSink;

	class BackgroundScriptRunner {
		agi::scoped_ptr<DialogProgress> impl;

		void OnConfigDialog(wxThreadEvent &evt);
	public:

		void QueueEvent(wxEvent *evt);

		void Run(std::tr1::function<void(ProgressSink*)> task);

		BackgroundScriptRunner(wxWindow *parent, wxString const& title);
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
		void SetProgress(int cur, int max) { impl->SetProgress(cur, max); }
		void Log(std::string const& str) { impl->Log(str); }
		bool IsCancelled() { return impl->IsCancelled(); }

		/// Show the passed dialog on the GUI thread, blocking the calling
		/// thread until it closes
		void ShowConfigDialog(ScriptConfigDialog *config_dialog);

		/// Get the current automation trace level
		int GetTraceLevel() const { return trace_level; }

		ProgressSink(agi::ProgressSink *impl, BackgroundScriptRunner *bsr);
	};

	class Script {
		wxString filename;

	protected:
		/// The automation include path, consisting of the user-specified paths
		/// along with the script's path
		wxPathList include_path;
		Script(wxString const& filename);

	public:
		virtual ~Script() { }

		/// Reload this script
		virtual void Reload() = 0;

		/// The script's file name with path
		wxString GetFilename() const { return filename; }
		/// The script's file name without path
		wxString GetPrettyFilename() const { return wxFileName(filename).GetFullName(); }
		/// The script's name. Not required to be unique.
		virtual wxString GetName() const=0;
		/// A short description of the script
		virtual wxString GetDescription() const=0;
		/// The author of the script
		virtual wxString GetAuthor() const=0;
		/// A version string that should not be used for anything but display
		virtual wxString GetVersion() const=0;
		/// Did the script load correctly?
		virtual bool GetLoadedState() const=0;

		/// Get a list of features provided by this script
		virtual std::vector<Feature*> GetFeatures() const=0;
	};

	/// DOCME
	/// @class ScriptManager
	/// @brief DOCME
	///
	/// DOCME
	class ScriptManager {
	private:

		/// DOCME
		std::vector<Script*> scripts;


		/// DOCME
		std::vector<FeatureMacro*> macros;

	public:
		ScriptManager();
		virtual ~ScriptManager();		// Deletes all scripts managed
		void Add(Script *script);		// Add a script to the manager. The ScriptManager takes owvership of the script and will automatically delete it.
		void Remove(Script *script);	// Remove a script from the manager, and delete the Script object.
		void RemoveAll();				// Deletes all scripts managed
		virtual void Reload() = 0;

		const std::vector<Script*>& GetScripts() const;

		const std::vector<FeatureMacro*>& GetMacros();
		// No need to have getters for the other kinds of features, I think.
		// They automatically register themselves in the relevant places.
	};

	/// Manager for scripts specified by a subtitle file
	class LocalScriptManager : public ScriptManager {
		std::list<agi::signal::Connection> slots;
		agi::Context *context;

		void OnSubtitlesSave();
	public:
		LocalScriptManager(agi::Context *context);
		void Reload();
	};

	/// DOCME
	/// @class AutoloadScriptManager
	/// @brief DOCME
	///
	/// DOCME
	class AutoloadScriptManager : public ScriptManager {
	private:

		/// DOCME
		wxString path;
	public:
		AutoloadScriptManager(const wxString &_path);
		void Reload();
	};

	/// Both a base class for script factories and a manager of registered
	/// script factories
	class ScriptFactory {
		/// Vector of loaded script engines
		static std::vector<ScriptFactory*> *factories;

		wxString engine_name;
		wxString filename_pattern;

		/// Load a file, or return NULL if the file is not in a supported
		/// format. If the file is in a supported format but is invalid, a
		/// script should be returned which returns false from IsLoaded and
		/// an appropriate error message from GetDescription.
		///
		/// This is private as it should only ever be called through
		/// CreateFromFile
		virtual Script* Produce(wxString const& filename) const = 0;

	protected:
		ScriptFactory(wxString engine_name, wxString filename_pattern);
		virtual ~ScriptFactory() { }

	public:
		/// Name of this automation engine
		const wxString& GetEngineName() const { return engine_name; }
		/// Extension which this engine supports
		const wxString& GetFilenamePattern() const { return filename_pattern; }

		/// Register an automation engine. Calling code retains ownership of pointer
		static void Register(ScriptFactory *factory);
		/// Unregister and delete an automation engine
		static void Unregister(ScriptFactory *factory);
		/// Is there an automation engine registered which can open the file?
		static bool CanHandleScriptFormat(wxString const& filename);

		/// Get the full wildcard string for all loaded engines
		static wxString GetWildcardStr();

		/// Load a script from a file
		/// @param filename Script to load
		/// @param log_errors Should load errors be displayed?
		/// @return Always returns a valid Script, even if no engine could load the file
		static Script* CreateFromFile(wxString const& filename, bool log_errors);

		static const std::vector<ScriptFactory*>& GetFactories();
	};

	/// A script which represents a file not recognized by any registered
	/// automation engines
	class UnknownScript : public Script {
	public:
		UnknownScript(wxString const& filename);

		void Reload() { }

		wxString GetName() const { return wxFileName(GetFilename()).GetName(); }
		wxString GetDescription() const { return _("File was not recognized as a script"); }
		wxString GetAuthor() const { return ""; }
		wxString GetVersion() const { return ""; }
		bool GetLoadedState() const { return false; }

		std::vector<Feature*> GetFeatures() const { return std::vector<Feature*>(); }
	};
};

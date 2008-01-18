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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:jiifurusu@gmail.com
//

#pragma once

#ifndef _AUTO4_BASE_H
#define _AUTO4_BASE_H

#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/gauge.h>
#include <wx/timer.h>
#include <wx/stattext.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <vector>

#include "ass_export_filter.h"
#include "subtitle_format.h"

class AssFile;
class AssStyle;
class wxWindow;
class wxDialog;
class wxStopWatch;
class wxPathList;


DECLARE_EVENT_TYPE(wxEVT_AUTOMATION_SCRIPT_COMPLETED, -1)


namespace Automation4 {

	// Calculate the extents of a text string given a style
	bool CalculateTextExtents(AssStyle *style, wxString &text, double &width, double &height, double &descent, double &extlead);


	// The class of a Feature...
	enum ScriptFeatureClass {
		SCRIPTFEATURE_MACRO = 0,
		SCRIPTFEATURE_FILTER,
		SCRIPTFEATURE_SUBFORMAT,

		SCRIPTFEATURE_MAX // must be last
	};


	// A Feature describes a function provided by a Script.
	// There are several distinct classes of features.
	class FeatureMacro;
	class FeatureFilter;
	class FeatureSubtitleFormat;
	class Feature {
	private:
		ScriptFeatureClass featureclass;
		wxString name;

	protected:
		Feature(ScriptFeatureClass _featureclass, const wxString &_name);

	public:
		virtual ~Feature() { }

		ScriptFeatureClass GetClass() const;
		FeatureMacro* AsMacro();
		FeatureFilter* AsFilter();
		FeatureSubtitleFormat* AsSubFormat();

		virtual const wxString& GetName() const;
	};


	// The Macro feature; adds a menu item that runs script code
	class FeatureMacro : public virtual Feature {
	private:
		wxString description;

	protected:
		FeatureMacro(const wxString &_name, const wxString &_description);

	public:
		virtual ~FeatureMacro() { }

		const wxString& GetDescription() const;

		virtual bool Validate(AssFile *subs, const std::vector<int> &selected, int active) = 0;
		virtual void Process(AssFile *subs, std::vector<int> &selected, int active, wxWindow * const progress_parent) = 0;
	};


	class ScriptConfigDialog;
	// The Export Filter feature; adds a new export filter
	class FeatureFilter : public virtual Feature, public AssExportFilter {
	private:
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


	// The Subtitle Format feature; adds new subtitle format readers/writers
	class FeatureSubtitleFormat : public virtual Feature, public SubtitleFormat {
	private:
		wxString extension;

	protected:
		FeatureSubtitleFormat(const wxString &_name, const wxString &_extension);

	public:
		virtual ~FeatureSubtitleFormat() { }

		const wxString& GetExtension() const;

		// Default implementations of these are provided, that just checks extension,
		// but subclasses can provide more elaborate implementations, or go back to
		// the "return false" implementation, in case of reader-only or writer-only.
		virtual bool CanWriteFile(wxString filename);
		virtual bool CanReadFile(wxString filename);

		// Subclasses should implement ReadFile and/or WriteFile here
	};


	// Base class for script-provided config dialogs
	class ScriptConfigDialog {
	private:
		wxWindow *win;

	protected:
		virtual wxWindow* CreateWindow(wxWindow *parent) = 0;

	public:
		ScriptConfigDialog() : win(0) { }
		virtual ~ScriptConfigDialog() { }
		wxWindow* GetWindow(wxWindow *parent);
		void DeleteWindow();
		virtual void ReadBack() = 0;

		virtual wxString Serialise();
		virtual void Unserialise(const wxString &serialised) { }
	};


	// Config dialog event class and related stuff (wx </3)
	extern const wxEventType EVT_SHOW_CONFIG_DIALOG_t;

	class ShowConfigDialogEvent : public wxCommandEvent {
	public:
		ShowConfigDialogEvent(const wxEventType &event = EVT_SHOW_CONFIG_DIALOG_t)
			: wxCommandEvent(event)
			, config_dialog(0)
			, sync_sema(0) { };

		virtual wxEvent *Clone() const { return new ShowConfigDialogEvent(*this); }

		ScriptConfigDialog *config_dialog;
		// Synchronisation for config dialog events:
		// You don't want the script asynchronically continue executing while the dialog
		// is displaying, so a synchronisation mechanism is used.
		// The poster of the event should supply a semaphore object with an initial count
		// of zero. After posting the event, the poster should wait for the semaphore.
		// When the dialog is finished, the semaphore is posted, and the poster can
		// continue.
		// The poster is responsible for cleaning up the semaphore.
		wxSemaphore *sync_sema;
	};

	typedef void (wxEvtHandler::*ShowConfigDialogEventFunction)(ShowConfigDialogEvent&);

#define EVT_SHOW_CONFIG_DIALOG(fn) DECLARE_EVENT_TABLE_ENTRY( EVT_SHOW_CONFIG_DIALOG_t, -1, -1, (wxObjectEventFunction)(wxEventFunction)(ShowConfigDialogEventFunction)&fn, (wxObject*)0 ),


	// Base class for progress reporting/other output
	class ProgressSink : public wxDialog {
	private:
		wxBoxSizer *sizer;
		wxGauge *progress_display;
		wxButton *cancel_button;
		wxStaticText *title_display;
		wxStaticText *task_display;
		wxTextCtrl *debug_output;

		volatile bool debug_visible;
		volatile bool data_updated;

		float progress;
		wxString task;
		wxString title;
		wxString pending_debug_output;
		wxMutex data_mutex;

		wxTimer *update_timer;

		void OnCancel(wxCommandEvent &evt);
		void OnInit(wxInitDialogEvent &evt);
		void OnIdle(wxIdleEvent &evt);
		void OnConfigDialog(ShowConfigDialogEvent &evt);

		void DoUpdateDisplay();

	protected:
		volatile bool cancelled;
		int trace_level;

		ProgressSink(wxWindow *parent);
		virtual ~ProgressSink();

	public:
		void SetProgress(float _progress);
		void SetTask(const wxString &_task);
		void SetTitle(const wxString &_title);
		void AddDebugOutput(const wxString &msg);

		volatile bool has_inited;
		volatile bool script_finished;

		DECLARE_EVENT_TABLE()
	};


	// Base class for Scripts
	class Script {
	private:
		wxString filename;

	protected:
		wxString name;
		wxString description;
		wxString author;
		wxString version;
		bool loaded; // is the script properly loaded?

		wxPathList include_path;

		std::vector<Feature*> features;

		Script(const wxString &_filename);

	public:
		virtual ~Script();

		virtual void Reload() = 0;

		const wxString& GetFilename() const;
		wxString GetPrettyFilename() const;
		const wxString& GetName() const;
		const wxString& GetDescription() const;
		const wxString& GetAuthor() const;
		const wxString& GetVersion() const;
		bool GetLoadedState() const;

		std::vector<Feature*>& GetFeatures();
	};


	// Manages loaded scripts; for whatever reason, multiple managers might be instantiated. In truth, this is more
	// like a macro manager at the moment, since Export Filter and Subtitle Format are already managed by other
	// classes.
	class ScriptManager {
	private:
		std::vector<Script*> scripts;

		std::vector<FeatureMacro*> macros;

	public:
		ScriptManager();
		virtual ~ScriptManager();		// Deletes all scripts managed
		void Add(Script *script);		// Add a script to the manager. The ScriptManager takes owvership of the script and will automatically delete it.
		void Remove(Script *script);	// Remove a script from the manager, and delete the Script object.
		void RemoveAll();				// Deletes all scripts managed

		const std::vector<Script*>& GetScripts() const;

		const std::vector<FeatureMacro*>& GetMacros();
		// No need to have getters for the other kinds of features, I think.
		// They automatically register themselves in the relevant places.
	};


	// Scans a directory for scripts and attempts to load all of them
	class AutoloadScriptManager : public ScriptManager {
	private:
		wxString path;
	public:
		AutoloadScriptManager(const wxString &_path);
		void Reload();
	};


	// Script factory; each scripting engine should create exactly one instance of this object and register it.
	// This is used to create Script objects from a file.
	class ScriptFactory {
	private:
		static std::vector<ScriptFactory*> *factories;
	protected:
		ScriptFactory() { }
		virtual ~ScriptFactory() { }
		wxString engine_name;
		wxString filename_pattern;
	public:
		virtual Script* Produce(const wxString &filename) const = 0;
		const wxString& GetEngineName() const;
		const wxString& GetFilenamePattern() const;

		static void Register(ScriptFactory *factory);
		static void Unregister(ScriptFactory *factory);
		static Script* CreateFromFile(const wxString &filename, bool log_errors);
		static bool CanHandleScriptFormat(const wxString &filename);
		static const std::vector<ScriptFactory*>& GetFactories();
	};

	// Dummy class for scripts that could not be loaded by the ScriptFactory
	class UnknownScript : public Script {
	public:
		UnknownScript(const wxString &filename);
		void Reload() { };
	};

};

#endif

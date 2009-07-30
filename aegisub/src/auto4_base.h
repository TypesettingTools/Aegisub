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

#ifndef _AUTO4_BASE_H

/// DOCME
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



/// DOCME
namespace Automation4 {

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


	// Config dialog event class and related stuff (wx </3)
	extern const wxEventType EVT_SHOW_CONFIG_DIALOG_t;


	/// DOCME
	/// @class ShowConfigDialogEvent
	/// @brief DOCME
	///
	/// DOCME
	class ShowConfigDialogEvent : public wxCommandEvent {
	public:

		/// @brief DOCME
		/// @param event
		/// @return 
		///
		ShowConfigDialogEvent(const wxEventType &event = EVT_SHOW_CONFIG_DIALOG_t)
			: wxCommandEvent(event)
			, config_dialog(0)
			, sync_sema(0) { };


		/// @brief DOCME
		/// @return 
		///
		virtual wxEvent *Clone() const { return new ShowConfigDialogEvent(*this); }


		/// DOCME
		ScriptConfigDialog *config_dialog;

		/// DOCME
		wxSemaphore *sync_sema;
	};


	/// DOCME
	typedef void (wxEvtHandler::*ShowConfigDialogEventFunction)(ShowConfigDialogEvent&);


/// DOCME
#define EVT_SHOW_CONFIG_DIALOG(fn) DECLARE_EVENT_TABLE_ENTRY( EVT_SHOW_CONFIG_DIALOG_t, -1, -1, (wxObjectEventFunction)(wxEventFunction)(ShowConfigDialogEventFunction)&fn, (wxObject*)0 ),



	/// DOCME
	/// @class ProgressSink
	/// @brief DOCME
	///
	/// DOCME
	class ProgressSink : public wxDialog {
	private:

		/// DOCME
		wxBoxSizer *sizer;

		/// DOCME
		wxGauge *progress_display;

		/// DOCME
		wxButton *cancel_button;

		/// DOCME
		wxStaticText *title_display;

		/// DOCME
		wxStaticText *task_display;

		/// DOCME
		wxTextCtrl *debug_output;


		/// DOCME
		volatile bool debug_visible;

		/// DOCME
		volatile bool data_updated;


		/// DOCME
		float progress;

		/// DOCME
		wxString task;

		/// DOCME
		wxString title;

		/// DOCME
		wxString pending_debug_output;

		/// DOCME
		wxMutex data_mutex;


		/// DOCME
		wxTimer *update_timer;

		void OnCancel(wxCommandEvent &evt);
		void OnInit(wxInitDialogEvent &evt);
		void OnIdle(wxIdleEvent &evt);
		void OnConfigDialog(ShowConfigDialogEvent &evt);

		void DoUpdateDisplay();

	protected:

		/// DOCME
		volatile bool cancelled;

		/// DOCME
		int trace_level;

		ProgressSink(wxWindow *parent);
		virtual ~ProgressSink();

	public:
		void SetProgress(float _progress);
		void SetTask(const wxString &_task);
		void SetTitle(const wxString &_title);
		void AddDebugOutput(const wxString &msg);


		/// DOCME
		volatile bool has_inited;

		/// DOCME
		volatile bool script_finished;

		DECLARE_EVENT_TABLE()
	};



	/// DOCME
	/// @class Script
	/// @brief DOCME
	///
	/// DOCME
	class Script {
	private:

		/// DOCME
		wxString filename;

	protected:

		/// DOCME
		wxString name;

		/// DOCME
		wxString description;

		/// DOCME
		wxString author;

		/// DOCME
		wxString version;

		/// DOCME
		bool loaded; // is the script properly loaded?


		/// DOCME
		wxPathList include_path;


		/// DOCME
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

		const std::vector<Script*>& GetScripts() const;

		const std::vector<FeatureMacro*>& GetMacros();
		// No need to have getters for the other kinds of features, I think.
		// They automatically register themselves in the relevant places.
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



	/// DOCME
	/// @class ScriptFactory
	/// @brief DOCME
	///
	/// DOCME
	class ScriptFactory {
	private:

		/// DOCME
		static std::vector<ScriptFactory*> *factories;
	protected:

		/// @brief DOCME
		///
		ScriptFactory() { }

		/// @brief DOCME
		///
		virtual ~ScriptFactory() { }

		/// DOCME
		wxString engine_name;

		/// DOCME
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


	/// DOCME
	/// @class UnknownScript
	/// @brief DOCME
	///
	/// DOCME
	class UnknownScript : public Script {
	public:
		UnknownScript(const wxString &filename);

		/// @brief DOCME
		///
		void Reload() { };
	};

};

#endif



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

/// @file auto4_base.cpp
/// @brief Baseclasses for Automation 4 scripting framework
/// @ingroup scripting
///


#include "config.h"

#ifdef WITH_AUTOMATION

#ifndef AGI_PRE
#ifdef __WINDOWS__
#include <tchar.h>
#include <windows.h>
#endif

#include <wx/button.h>
#include <wx/dcmemory.h>
#include <wx/dialog.h>
#include <wx/dir.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/gauge.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#endif

#ifndef __WINDOWS__
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include "ass_file.h"
#include "ass_style.h"
#include "auto4_base.h"
#include "compat.h"
#include "main.h"
#include "options.h"
#include "standard_paths.h"
#include "string_codec.h"



/// DOCME
namespace Automation4 {


	/// @brief DOCME
	/// @param style   
	/// @param text    
	/// @param width   
	/// @param height  
	/// @param descent 
	/// @param extlead 
	/// @return 
	///
	bool CalculateTextExtents(AssStyle *style, wxString &text, double &width, double &height, double &descent, double &extlead)
	{
		width = height = descent = extlead = 0;

		double fontsize = style->fontsize * 64;
		double spacing = style->spacing * 64;

#ifdef WIN32
		// This is almost copypasta from TextSub
		HDC thedc = CreateCompatibleDC(0);
		if (!thedc) return false;
		SetMapMode(thedc, MM_TEXT);

		LOGFONTW lf;
		ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = (LONG)fontsize;
		lf.lfWeight = style->bold ? FW_BOLD : FW_NORMAL;
		lf.lfItalic = style->italic;
		lf.lfUnderline = style->underline;
		lf.lfStrikeOut = style->strikeout;
		lf.lfCharSet = style->encoding;
		lf.lfOutPrecision = OUT_TT_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = ANTIALIASED_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
		_tcsncpy(lf.lfFaceName, style->font.c_str(), 32);

		HFONT thefont = CreateFontIndirect(&lf);
		if (!thefont) return false;
		SelectObject(thedc, thefont);
		
		SIZE sz;
		size_t thetextlen = text.length();
		const TCHAR *thetext = text.wc_str();
		if (spacing != 0 ) {
			width = 0;
			for (unsigned int i = 0; i < thetextlen; i++) {
				GetTextExtentPoint32(thedc, &thetext[i], 1, &sz);
				width += sz.cx + spacing;
				height = sz.cy;
			}
		} else {
			GetTextExtentPoint32(thedc, thetext, (int)thetextlen, &sz);
			width = sz.cx;
			height = sz.cy;
		}

		// HACKISH FIX! This seems to work, but why? It shouldn't be needed?!?
		//fontsize = style->fontsize;
		//width = (int)(width * fontsize/height + 0.5);
		//height = (int)(fontsize + 0.5);

		TEXTMETRIC tm;
		GetTextMetrics(thedc, &tm);
		descent = tm.tmDescent;
		extlead= tm.tmExternalLeading;

		DeleteObject(thedc);
		DeleteObject(thefont);

#else // not WIN32
		wxMemoryDC thedc;

		// fix fontsize to be 72 DPI
		//fontsize = -FT_MulDiv((int)(fontsize+0.5), 72, thedc.GetPPI().y);

		// now try to get a font!
		// use the font list to get some caching... (chance is the script will need the same font very often)
		// USING wxTheFontList SEEMS TO CAUSE BAD LEAKS!
		//wxFont *thefont = wxTheFontList->FindOrCreateFont(
		wxFont thefont(
			(int)fontsize,
			wxFONTFAMILY_DEFAULT,
			style->italic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
			style->bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
			style->underline,
			style->font,
			wxFONTENCODING_SYSTEM); // FIXME! make sure to get the right encoding here, make some translation table between windows and wx encodings
		thedc.SetFont(thefont);

		if (spacing) {
			// If there's inter-character spacing, kerning info must not be used, so calculate width per character
			// NOTE: Is kerning actually done either way?!
			for (unsigned int i = 0; i < text.length(); i++) {
				int a, b, c, d;
				thedc.GetTextExtent(text[i], &a, &b, &c, &d);
				double scaling = fontsize / (double)(b>0?b:1); // semi-workaround for missing OS/2 table data for scaling
				width += (a + spacing)*scaling;
				height = b > height ? b*scaling : height;
				descent = c > descent ? c*scaling : descent;
				extlead = d > extlead ? d*scaling : extlead;
			}
		} else {
			// If the inter-character spacing should be zero, kerning info can (and must) be used, so calculate everything in one go
			wxCoord lwidth, lheight, ldescent, lextlead;
			thedc.GetTextExtent(text, &lwidth, &lheight, &ldescent, &lextlead);
			double scaling = fontsize / (double)(lheight>0?lheight:1); // semi-workaround for missing OS/2 table data for scaling
			width = lwidth*scaling; height = lheight*scaling; descent = ldescent*scaling; extlead = lextlead*scaling;
		}
#endif

		// Compensate for scaling
		width = style->scalex / 100 * width / 64;
		height = style->scaley / 100 * height / 64;
		descent = style->scaley / 100 * descent / 64;
		extlead = style->scaley / 100 * extlead / 64;

		return true;
	}


	// Feature


	/// @brief DOCME
	/// @param _featureclass 
	/// @param _name         
	///
	Feature::Feature(ScriptFeatureClass _featureclass, const wxString &_name)
		: featureclass(_featureclass)
		, name(_name)
	{
		// nothing to do
	}


	/// @brief DOCME
	/// @return 
	///
	ScriptFeatureClass Feature::GetClass() const
	{
		return featureclass;
	}


	/// @brief DOCME
	/// @return 
	///
	FeatureMacro* Feature::AsMacro()
	{
		if (featureclass == SCRIPTFEATURE_MACRO)
			// For VS, remember to enable building with RTTI, otherwise dynamic_cast<> won't work
			return dynamic_cast<FeatureMacro*>(this);
		return 0;
	}


	/// @brief DOCME
	/// @return 
	///
	FeatureFilter* Feature::AsFilter()
	{
		if (featureclass == SCRIPTFEATURE_FILTER)
			return dynamic_cast<FeatureFilter*>(this);
		return 0;
	}


	/// @brief DOCME
	/// @return 
	///
	FeatureSubtitleFormat* Feature::AsSubFormat()
	{
		if (featureclass == SCRIPTFEATURE_SUBFORMAT)
			return dynamic_cast<FeatureSubtitleFormat*>(this);
		return 0;
	}


	/// @brief DOCME
	/// @return 
	///
	const wxString& Feature::GetName() const
	{
		return name;
	}


	// FeatureMacro


	/// @brief DOCME
	/// @param _name        
	/// @param _description 
	///
	FeatureMacro::FeatureMacro(const wxString &_name, const wxString &_description)
		: Feature(SCRIPTFEATURE_MACRO, _name)
		, description(_description)
	{
		// nothing to do
	}


	/// @brief DOCME
	/// @return 
	///
	const wxString& FeatureMacro::GetDescription() const
	{
		return description;
	}


	// FeatureFilter


	/// @brief DOCME
	/// @param _name        
	/// @param _description 
	/// @param _priority    
	///
	FeatureFilter::FeatureFilter(const wxString &_name, const wxString &_description, int _priority)
		: Feature(SCRIPTFEATURE_FILTER, _name)
		, AssExportFilter()
		, config_dialog(0)
	{
		description = _description; // from AssExportFilter
		Register(_name, _priority);
	}


	/// @brief DOCME
	///
	FeatureFilter::~FeatureFilter()
	{
		Unregister();
	}


	/// @brief DOCME
	/// @return 
	///
	wxString FeatureFilter::GetScriptSettingsIdentifier()
	{
		return inline_string_encode(wxString::Format(_T("Automation Settings %s"), GetName().c_str()));
	}


	/// @brief DOCME
	/// @param parent 
	/// @return 
	///
	wxWindow* FeatureFilter::GetConfigDialogWindow(wxWindow *parent) {
		if (config_dialog) {
			delete config_dialog;
			config_dialog = 0;
		}
		if ((config_dialog = GenerateConfigDialog(parent)) != NULL) {
			wxString val = AssFile::top->GetScriptInfo(GetScriptSettingsIdentifier());
			if (!val.IsEmpty()) {
				config_dialog->Unserialise(val);
			}
			return config_dialog->GetWindow(parent);
		} else {
			return 0;
		}
	}


	/// @brief DOCME
	/// @param IsDefault 
	///
	void FeatureFilter::LoadSettings(bool IsDefault) {
		if (config_dialog) {
			config_dialog->ReadBack();

			wxString val = config_dialog->Serialise();
			if (!val.IsEmpty()) {
				AssFile::top->SetScriptInfo(GetScriptSettingsIdentifier(), val);
			}
		}
	}


	// FeatureSubtitleFormat


	/// @brief DOCME
	/// @param _name      
	/// @param _extension 
	///
	FeatureSubtitleFormat::FeatureSubtitleFormat(const wxString &_name, const wxString &_extension)
		: Feature(SCRIPTFEATURE_SUBFORMAT, _name)
		, extension(_extension)
	{
		// nothing to do
	}


	/// @brief DOCME
	/// @return 
	///
	const wxString& FeatureSubtitleFormat::GetExtension() const
	{
		return extension;
	}


	/// @brief DOCME
	/// @param filename 
	/// @return 
	///
	bool FeatureSubtitleFormat::CanWriteFile(wxString filename)
	{
		return !filename.Right(extension.Length()).CmpNoCase(extension);
	}


	/// @brief DOCME
	/// @param filename 
	/// @return 
	///
	bool FeatureSubtitleFormat::CanReadFile(wxString filename)
	{
		return !filename.Right(extension.Length()).CmpNoCase(extension);
	}


	// ShowConfigDialogEvent


	/// DOCME
	const wxEventType EVT_SHOW_CONFIG_DIALOG_t = wxNewEventType();


	// ScriptConfigDialog


	/// @brief DOCME
	/// @param parent 
	/// @return 
	///
	wxWindow* ScriptConfigDialog::GetWindow(wxWindow *parent)
	{
		if (win) return win;
		return win = CreateWindow(parent);
	}


	/// @brief DOCME
	///
	void ScriptConfigDialog::DeleteWindow()
	{
		if (win) delete win;
		win = 0;
	}


	/// @brief DOCME
	/// @return 
	///
	wxString ScriptConfigDialog::Serialise()
	{
		return _T("");
	}


	// ProgressSink


	/// @brief DOCME
	/// @param parent 
	///
	ProgressSink::ProgressSink(wxWindow *parent)
		: wxDialog(parent, -1, _T("Automation"), wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED)
		, debug_visible(false)
		, data_updated(false)
		, cancelled(false)
		, has_inited(false)
		, script_finished(false)
	{
		// make the controls
		progress_display = new wxGauge(this, -1, 1000, wxDefaultPosition, wxSize(300, 20));
		title_display = new wxStaticText(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE|wxST_NO_AUTORESIZE);
		task_display = new wxStaticText(this, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE|wxST_NO_AUTORESIZE);
		cancel_button = new wxButton(this, wxID_CANCEL);
		debug_output = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(300, 120), wxTE_MULTILINE|wxTE_READONLY);

		// put it in a sizer
		sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(title_display, 0, wxEXPAND | wxALL, 5);
		sizer->Add(progress_display, 0, wxALL&~wxTOP, 5);
		sizer->Add(task_display, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
		sizer->Add(cancel_button, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 5);
		sizer->Add(debug_output, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
		sizer->Show(debug_output, false);

		// make the title a slightly larger font
		wxFont title_font = title_display->GetFont();
		int fontsize = title_font.GetPointSize();
		title_font.SetPointSize(fontsize + fontsize/4 + fontsize/8);
		title_font.SetWeight(wxFONTWEIGHT_BOLD);
		title_display->SetFont(title_font);

		// Set up a timer to regularly update the status
		// It doesn't need an event handler attached, as just a the timer in itself
		// will ensure that the idle event is fired
		update_timer = new wxTimer();
		update_timer->Start(50, false);

		sizer->SetSizeHints(this);
		SetSizer(sizer);
		Center();

		// Init trace level
		trace_level = OPT_GET("Automation/Trace Level")->GetInt();
	}


	/// @brief DOCME
	///
	ProgressSink::~ProgressSink()
	{
		delete update_timer;
	}


	/// @brief DOCME
	/// @param evt 
	///
	void ProgressSink::OnIdle(wxIdleEvent &evt)
	{
		// The big glossy "update display" event
		DoUpdateDisplay();

		if (script_finished) {
			if (!debug_visible) {
				EndModal(0);
			} else {
				cancel_button->Enable(true);
				cancel_button->SetLabel(_("Close"));
				SetProgress(100.0);
				SetTask(_("Script completed"));
			}
		}
	}


	/// @brief DOCME
	/// @return 
	///
	void ProgressSink::DoUpdateDisplay()
	{
		// If debug output isn't handled before the test for script_finished later,
		// there might actually be some debug output but the debug_visible flag won't
		// be set before the dialog closes itself.
		wxMutexLocker lock(data_mutex);
		if (!data_updated) return;
		if (!pending_debug_output.IsEmpty()) {
			if (!debug_visible) {
				sizer->Show(debug_output, true);
				Layout();
				sizer->Fit(this);

				debug_visible = true;
			}

			*debug_output << pending_debug_output;
			debug_output->SetInsertionPointEnd();

			pending_debug_output = _T("");
		}

		progress_display->SetValue((int)(progress*10));
		task_display->SetLabel(task);
		title_display->SetLabel(title);
		data_updated = false;
	}


	/// @brief DOCME
	/// @param _progress 
	///
	void ProgressSink::SetProgress(float _progress)
	{
		wxMutexLocker lock(data_mutex);
		progress = _progress;
		data_updated = true;
	}


	/// @brief DOCME
	/// @param _task 
	///
	void ProgressSink::SetTask(const wxString &_task)
	{
		wxMutexLocker lock(data_mutex);
		task = _task;
		data_updated = true;
	}


	/// @brief DOCME
	/// @param _title 
	///
	void ProgressSink::SetTitle(const wxString &_title)
	{
		wxMutexLocker lock(data_mutex);
		title = _title;
		data_updated = true;
	}


	/// @brief DOCME
	/// @param msg 
	///
	void ProgressSink::AddDebugOutput(const wxString &msg)
	{
		wxMutexLocker lock(data_mutex);
		pending_debug_output << msg;
		data_updated = true;
	}

	BEGIN_EVENT_TABLE(ProgressSink, wxWindow)
		EVT_INIT_DIALOG(ProgressSink::OnInit)
		EVT_BUTTON(wxID_CANCEL, ProgressSink::OnCancel)
		EVT_IDLE(ProgressSink::OnIdle)
		EVT_SHOW_CONFIG_DIALOG(ProgressSink::OnConfigDialog)
	END_EVENT_TABLE()


	/// @brief DOCME
	/// @param evt 
	///
	void ProgressSink::OnInit(wxInitDialogEvent &evt)
	{
		has_inited = true;
	}


	/// @brief DOCME
	/// @param evt 
	///
	void ProgressSink::OnCancel(wxCommandEvent &evt)
	{
		if (!script_finished) {
			cancelled = true;
			cancel_button->Enable(false);
		} else {
			EndModal(0);
		}
	}


	/// @brief DOCME
	/// @param evt 
	///
	void ProgressSink::OnConfigDialog(ShowConfigDialogEvent &evt)
	{
		// assume we're in the GUI thread here

		DoUpdateDisplay();

		if (evt.config_dialog) {
			wxDialog *w = new wxDialog(this, -1, title); // container dialog box
			wxBoxSizer *s = new wxBoxSizer(wxHORIZONTAL); // sizer for putting contents in
			wxWindow *ww = evt.config_dialog->GetWindow(w); // get/generate actual dialog contents
			s->Add(ww, 0, wxALL, 5); // add contents to dialog
			w->SetSizerAndFit(s);
			w->CenterOnParent();
			w->ShowModal();
			evt.config_dialog->ReadBack();
			evt.config_dialog->DeleteWindow();
			delete w;
		} else {
			wxMessageBox(_T("Uh... no config dialog?"));
		}

		// See note in auto4_base.h
		if (evt.sync_sema) {
			evt.sync_sema->Post();
		}
	}


	// Script


	/// @brief DOCME
	/// @param _filename 
	///
	Script::Script(const wxString &_filename)
		: filename(_filename)
		, name(_T(""))
		, description(_T(""))
		, author(_T(""))
		, version(_T(""))
		, loaded(false)
	{
		// copied from auto3
		include_path.clear();
		include_path.EnsureFileAccessible(filename);
		wxStringTokenizer toker(lagi_wxString(OPT_GET("Path/Automation/Include")->GetString()), _T("|"), wxTOKEN_STRTOK);
		while (toker.HasMoreTokens()) {
			// todo? make some error reporting here
			wxFileName path(StandardPaths::DecodePath(toker.GetNextToken()));
			if (!path.IsOk()) continue;
			if (path.IsRelative()) continue;
			if (!path.DirExists()) continue;
			include_path.Add(path.GetLongPath());
		}
	}


	/// @brief DOCME
	///
	Script::~Script()
	{
		for (std::vector<Feature*>::iterator f = features.begin(); f != features.end(); ++f) {
			delete *f;
		}
	}


	/// @brief DOCME
	/// @return 
	///
	wxString Script::GetPrettyFilename() const
	{
		wxFileName fn(filename);
		return fn.GetFullName();
	}


	/// @brief DOCME
	/// @return 
	///
	const wxString& Script::GetFilename() const
	{
		return filename;
	}


	/// @brief DOCME
	/// @return 
	///
	const wxString& Script::GetName() const
	{
		return name;
	}


	/// @brief DOCME
	/// @return 
	///
	const wxString& Script::GetDescription() const
	{
		return description;
	}


	/// @brief DOCME
	/// @return 
	///
	const wxString& Script::GetAuthor() const
	{
		return author;
	}


	/// @brief DOCME
	/// @return 
	///
	const wxString& Script::GetVersion() const
	{
		return version;
	}


	/// @brief DOCME
	/// @return 
	///
	bool Script::GetLoadedState() const
	{
		return loaded;
	}


	/// @brief DOCME
	/// @return 
	///
	std::vector<Feature*>& Script::GetFeatures()
	{
		return features;
	}


	// ScriptManager


	/// @brief DOCME
	///
	ScriptManager::ScriptManager()
	{
		// do nothing...?
	}


	/// @brief DOCME
	///
	ScriptManager::~ScriptManager()
	{
		RemoveAll();
	}


	/// @brief DOCME
	/// @param script 
	/// @return 
	///
	void ScriptManager::Add(Script *script)
	{
		for (std::vector<Script*>::iterator i = scripts.begin(); i != scripts.end(); ++i) {
			if (script == *i) return;
		}
		scripts.push_back(script);
	}


	/// @brief DOCME
	/// @param script 
	/// @return 
	///
	void ScriptManager::Remove(Script *script)
	{
		for (std::vector<Script*>::iterator i = scripts.begin(); i != scripts.end(); ++i) {
			if (script == *i) {
				delete *i;
				scripts.erase(i);
				return;
			}
		}
	}


	/// @brief DOCME
	///
	void ScriptManager::RemoveAll()
	{
		for (std::vector<Script*>::iterator i = scripts.begin(); i != scripts.end(); ++i) {
			delete *i;
		}
		scripts.clear();
	}


	/// @brief DOCME
	/// @return 
	///
	const std::vector<Script*>& ScriptManager::GetScripts() const
	{
		return scripts;
	}


	/// @brief DOCME
	/// @return 
	///
	const std::vector<FeatureMacro*>& ScriptManager::GetMacros()
	{
		macros.clear();
		for (std::vector<Script*>::iterator i = scripts.begin(); i != scripts.end(); ++i) {
			std::vector<Feature*> &sfs = (*i)->GetFeatures();
			for (std::vector<Feature*>::iterator j = sfs.begin(); j != sfs.end(); ++j) {
				FeatureMacro *m = dynamic_cast<FeatureMacro*>(*j);
				if (!m) continue;
				macros.push_back(m);
			}
		}
		return macros;
	}


	// AutoloadScriptManager


	/// @brief DOCME
	/// @param _path 
	///
	AutoloadScriptManager::AutoloadScriptManager(const wxString &_path)
		: path(_path)
	{
		Reload();
	}


	/// @brief DOCME
	///
	void AutoloadScriptManager::Reload()
	{
		RemoveAll();

		int error_count = 0;

		wxStringTokenizer tok(path, _T("|"), wxTOKEN_STRTOK);
		while (tok.HasMoreTokens()) {
			wxDir dir;
			wxString dirname = StandardPaths::DecodePath(tok.GetNextToken());
			if (!dir.Exists(dirname)) {
				//wxLogWarning(_T("A directory was specified in the Automation autoload path, but it doesn't exist: %s"), dirname.c_str());
				continue;
			}
			if (!dir.Open(dirname)) {
				//wxLogWarning(_T("Failed to open a directory in the Automation autoload path: %s"), dirname.c_str());
				continue;
			}

			wxString fn;
			wxFileName script_path(dirname + _T("/"), _T(""));
			bool more = dir.GetFirst(&fn, wxEmptyString, wxDIR_FILES);
			while (more) {
				script_path.SetName(fn);
				try {
					wxString fullpath = script_path.GetFullPath();
					if (ScriptFactory::CanHandleScriptFormat(fullpath)) {
						Script *s = ScriptFactory::CreateFromFile(fullpath, true);
						Add(s);
						if (!s->GetLoadedState()) error_count++;
					}
				}
				catch (const wchar_t *e) {
					error_count++;
					wxLogError(_T("Error loading Automation script: %s\n%s"), fn.c_str(), e);
				}
				catch (...) {
					error_count++;
					wxLogError(_T("Error loading Automation script: %s\nUnknown error."), fn.c_str());
				}
				more = dir.GetNext(&fn);
			}
		}
		if (error_count > 0) {
			wxLogWarning(_T("One or more scripts placed in the Automation autoload directory failed to load\nPlease review the errors above, correct them and use the Reload Autoload dir button in Automation Manager to attempt loading the scripts again."));
		}
	}



	// ScriptFactory


	/// DOCME
	std::vector<ScriptFactory*> *ScriptFactory::factories = 0;


	/// @brief DOCME
	/// @return 
	///
	const wxString& ScriptFactory::GetEngineName() const
	{
		return engine_name;
	}


	/// @brief DOCME
	/// @return 
	///
	const wxString& ScriptFactory::GetFilenamePattern() const
	{
		return filename_pattern;
	}


	/// @brief DOCME
	/// @param factory 
	///
	void ScriptFactory::Register(ScriptFactory *factory)
	{
		if (!factories)
			factories = new std::vector<ScriptFactory*>();

		for (std::vector<ScriptFactory*>::iterator i = factories->begin(); i != factories->end(); ++i) {
			if (*i == factory) {
				throw _T("Automation 4: Attempt to register the same script factory multiple times. This should never happen.");
			}
		}
		factories->push_back(factory);
	}


	/// @brief DOCME
	/// @param factory 
	/// @return 
	///
	void ScriptFactory::Unregister(ScriptFactory *factory)
	{
		if (!factories)
			factories = new std::vector<ScriptFactory*>();

		for (std::vector<ScriptFactory*>::iterator i = factories->begin(); i != factories->end(); ++i) {
			if (*i == factory) {
				factories->erase(i);
				if (factories->empty()) delete factories;
				return;
			}
		}
	}


	/// @brief DOCME
	/// @param filename   
	/// @param log_errors 
	/// @return 
	///
	Script* ScriptFactory::CreateFromFile(const wxString &filename, bool log_errors)
	{
		if (!factories)
			factories = new std::vector<ScriptFactory*>();

		for (std::vector<ScriptFactory*>::iterator i = factories->begin(); i != factories->end(); ++i) {
			try {
				Script *s = (*i)->Produce(filename);
				if (s) {
					if (!s->GetLoadedState() && log_errors) {
						wxLogError(_("An Automation script failed to load. File name: '%s', error reported:"), filename.c_str());
						wxLogError(s->GetDescription());
					}
					return s;
				}
			}
			catch (Script *e) {
				// This was the wrong script factory, but it throwing a Script object means it did know what to do about the file
				// Use this script object
				return e;
			}
		}
		if (log_errors) {
			wxLogWarning(_("The file was not recognised as an Automation script: %s"), filename.c_str());
		}
		return new UnknownScript(filename);
	}


	/// @brief DOCME
	/// @param filename 
	/// @return 
	///
	bool ScriptFactory::CanHandleScriptFormat(const wxString &filename)
	{
		// Just make this always return true to bitch about unknown script formats in autoload

		if (!factories)
			factories = new std::vector<ScriptFactory*>();

		for (std::vector<ScriptFactory*>::iterator i = factories->begin(); i != factories->end(); ++i) {
			wxString pattern = (*i)->GetFilenamePattern();
			if (filename.Matches(pattern)) return true;
		}

		return false;
	}


	/// @brief DOCME
	/// @return 
	///
	const std::vector<ScriptFactory*>& ScriptFactory::GetFactories()
	{
		if (!factories)
			factories = new std::vector<ScriptFactory*>();

		return *factories;
	}


	// UnknownScript


	/// @brief DOCME
	/// @param filename 
	///
	UnknownScript::UnknownScript(const wxString &filename)
		: Script(filename)
	{
		wxFileName fn(filename);
		name = fn.GetName();
		description = _("File was not recognized as a script");
		loaded = false;
	}

};

#endif // WITH_AUTOMATION



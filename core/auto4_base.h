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
// Contact: mailto:zeratul@cellosoft.com
//

#pragma once

#ifndef AUTO4_CORE_H
#define AUTO4_CORE_H

#include <wx/string.h>
#include <vector>

namespace Automation4 {

	enum MacroMenu {
		MACROMENU_NONE = 0,
		MACROMENU_ALL = 0,
		MACROMENU_EDIT,
		MACROMENU_VIDEO,
		MACROMENU_AUDIO,
		MACROMENU_TOOLS,
		MACROMENU_RIGHT,

		MACROMENU_MAX // must be last
	};
	enum ScriptFeatureClass {
		SCRIPTFEATURE_MACRO = 0,
		SCRIPTFEATURE_FILTER,
		SCRIPTFEATURE_READER,
		SCRIPTFEATURE_WRITER,

		SCRIPTFEATURE_MAX // must be last
	};

	class FeatureMacro;
	class FeatureFilter;
	class FeatureReader;
	class FeatureWriter;
	class Feature {
	private:
		ScriptFeatureClass featureclass;
		wxString name;

	protected:
		Feature(ScriptFeatureClass _featureclass, wxString &_name);

	public:
		ScriptFeatureClass GetClass() const;
		const FeatureMacro* AsMacro() const;
		const FeatureFilter* AsFilter() const;
		const FeatureReader* AsReader() const;
		const FeatureWriter* AsWriter() const;

		const wxString& GetName() const;
	};

	class FeatureMacro : public Feature {
	private:
		wxString description;
		MacroMenu menu;

	protected:
		FeatureMacro(wxString &_name, wxString &_description, MacroMenu _menu);

	public:
		const wxString& GetDescription() const;
		MacroMenu GetMenu() const;

		virtual bool Validate(/* TODO: this needs arguments */) = 0;
		virtual void Process(/* TODO: this needs arguments */) = 0;
	};

	class FeatureFilter : public Feature { // TODO: also inherit from export filter class?
	private:
		wxString description;
		int priority;

	protected:
		FeatureFilter(wxString &_name, wxString &_description, int _priority);

	public:
		const wxString& GetDescription() const;
		int GetPriority() const;

		virtual void GetOptionsWindow(/* TODO: this needs arguments */) = 0; // TODO: return type
		virtual void Process(/* TODO: this needs arguments */) = 0;
	};

	class FeatureReader : public Feature { // TODO: also inherit from SubtitleFormat class?
	private:
		wxString extension;
		bool is_text_format;

	protected:
		FeatureReader(wxString &_name, wxString &_extension, bool _is_text_format);

	public:
		const wxString& GetExtension() const;
		bool GetIsTextFormat() const;

		virtual void Process(/* TODO: this needs arguments */) = 0; // TODO: return type
	};

	class FeatureWriter : public Feature { // TODO: also inherit from SubtitleFormat class?
	private:
		wxString extension;
		bool is_text_format;

	protected:
		FeatureWriter(wxString &_name, wxString &_extension, bool _is_text_format);

	public:
		const wxString& GetExtension() const;
		bool GetIsTextFormat() const;

		virtual void Process(/* TODO: this needs arguments */) = 0; // TODO: return type
	};

	class Script {
	private:
		wxString filename;

	protected:
		wxString name;
		wxString description;
		wxString author;
		wxString version;

		std::vector<Feature*> features;

		Script(wxString &_filename);

	public:
		virtual void Reload() = 0;

		const wxString& GetFilename() const;
		const wxString& GetName() const;
		const wxString& GetDescription() const;
		const wxString& GetAuthor() const;
		const wxString& GetVersion() const;

		const std::vector<Feature*> GetFeatures() const;
	};

	class ScriptManager {
	private:
		/*struct ScriptRecord {
			Script *script;
			int priority;
		};*/
		typedef Script* ScriptRecord;
		std::vector<ScriptRecord> scripts;

		std::vector<Feature*> macros[MACROMENU_MAX]; // array of vectors...

	public:
		ScriptManager();
		~ScriptManager();
		void Add(Script *script);
		void Remove(Script *script);

		const std::vector<Feature*>& GetMacros(MacroMenu menu) const;
	};

};

#endif

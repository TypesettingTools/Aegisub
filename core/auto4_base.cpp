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

#include "auto4_base.h"

namespace Automation4 {


	// Feature

	Feature::Feature(ScriptFeatureClass _featureclass, wxString &_name)
		: featureclass(_featureclass), name(_name)
	{
		// nothing to do
	}

	ScriptFeatureClass Feature::GetClass() const
	{
		return featureclass;
	}

	const FeatureMacro* Feature::AsMacro() const
	{
		if (featureclass == SCRIPTFEATURE_MACRO)
			return static_cast<const FeatureMacro*>(this);
		return 0;
	}

	const FeatureFilter* Feature::AsFilter() const
	{
		if (featureclass == SCRIPTFEATURE_FILTER)
			return static_cast<const FeatureFilter*>(this);
		return 0;
	}

	const FeatureReader* Feature::AsReader() const
	{
		if (featureclass == SCRIPTFEATURE_READER)
			return static_cast<const FeatureReader*>(this);
		return 0;
	}

	const FeatureWriter* Feature::AsWriter() const
	{
		if (featureclass == SCRIPTFEATURE_WRITER)
			return static_cast<const FeatureWriter*>(this);
		return 0;
	}

	const wxString& Feature::GetName() const
	{
		return name;
	}


	// FeatureMacro

	FeatureMacro::FeatureMacro(wxString &_name, wxString &_description, MacroMenu _menu)
		: Feature(SCRIPTFEATURE_MACRO, _name), description(_description), menu(_menu)
	{
		// nothing to do
	}

	const wxString& FeatureMacro::GetDescription() const
	{
		return description;
	}

	MacroMenu FeatureMacro::GetMenu() const
	{
		return menu;
	}


	// FeatureFilter

	FeatureFilter::FeatureFilter(wxString &_name, wxString &_description, int _priority)
		: Feature(SCRIPTFEATURE_FILTER, _name), description(_description), priority(_priority)
	{
		// nothing to do
	}

	const wxString& FeatureFilter::GetDescription() const
	{
		return description;
	}

	int FeatureFilter::GetPriority() const
	{
		return priority;
	}


	// FeatureReader

	FeatureReader::FeatureReader(wxString &_name, wxString &_extension, bool _is_text_format)
		: Feature(SCRIPTFEATURE_READER, _name), extension(_extension), is_text_format(_is_text_format)
	{
		// nothing to do
	}

	const wxString& FeatureReader::GetExtension() const
	{
		return extension;
	}

	bool FeatureReader::GetIsTextFormat() const
	{
		return is_text_format;
	}


	// FeatureWriter

	FeatureWriter::FeatureWriter(wxString &_name, wxString &_extension, bool _is_text_format)
		: Feature(SCRIPTFEATURE_WRITER, _name), extension(_extension), is_text_format(_is_text_format)
	{
		// nothing to do
	}

	const wxString& FeatureWriter::GetExtension() const
	{
		return extension;
	}

	bool FeatureWriter::GetIsTextFormat() const
	{
		return is_text_format;
	}


	// Script

	Script::Script(wxString &_filename)
		: filename(_filename), name(_T("")), description(_T("")), author(_T("")), version(_T(""))
	{
		// nothing to do..?
	}

	const wxString& Script::GetFilename() const
	{
		return filename;
	}

	const wxString& Script::GetName() const
	{
		return name;
	}

	const wxString& Script::GetDescription() const
	{
		return description;
	}

	const wxString& Script::GetAuthor() const
	{
		return author;
	}

	const wxString& Script::GetVersion() const
	{
		return version;
	}

	const std::vector<Feature*> Script::GetFeatures() const
	{
		return features;
	}


	// ScriptManager

	ScriptManager::ScriptManager()
	{
		// do nothing...?
	}

	ScriptManager::~ScriptManager()
	{
		// do nothing...?
	}

	void ScriptManager::Add(Script *script)
	{
		// TODO
	}

	void ScriptManager::Remove(Script *script)
	{
		// TODO
	}

	const std::vector<Feature*>& ScriptManager::GetMacros(MacroMenu menu) const
	{
		return macros[menu];
	}

};

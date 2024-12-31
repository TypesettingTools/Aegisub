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

#include <libaegisub/fs.h>
#include <map>
#include <string>
#include <vector>

class AssExportFilter;
namespace agi { struct Context; }
class wxSizer;
class wxWindow;

class AssExporter {
	typedef std::vector<AssExportFilter*>::const_iterator filter_iterator;

	/// Sizers for configuration panels
	std::map<std::string, wxSizer*> Sizers;

	/// Filters which will be applied to the subtitles
	std::vector<AssExportFilter*> filters;

	/// Input context
	agi::Context *c;

	/// Have the config windows been created, or should filters simply use
	/// their default settings
	bool is_default = true;

public:
	AssExporter(agi::Context *c);

	/// Get the names of all registered export filters
	std::vector<std::string> GetAllFilterNames() const;

	/// Add the named filter to the list of filters to be run
	/// @throws std::string if filter is not found
	void AddFilter(std::string const& name);

	/// Apply selected export filters and save with the given charset
	/// @param file Target filename
	/// @param charset Target charset
	/// @param parent_window Parent window the filters should use when opening dialogs
	void Export(agi::fs::path const& file, const char *charset, wxWindow *parent_window= nullptr);

	/// Add configuration panels for all registered filters to the target sizer
	/// @param parent Parent window for controls
	/// @param target_sizer Sizer to add configuration panels to
	void DrawSettings(wxWindow *parent, wxSizer *target_sizer);

	/// Get the sizer created by DrawSettings for a specific filter
	wxSizer *GetSettingsSizer(std::string const& name);

	/// Get the description of the named export filter
	/// @throws std::string if filter is not found
	std::string const& GetDescription(std::string const& name) const;
};

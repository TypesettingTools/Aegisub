// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @file toolbar.h
/// @brief Dynamic toolbar generator.
/// @ingroup menu toolbar

#ifndef AGI_PRE
#include <map>

#include <wx/toolbar.h>
#endif

#include <libaegisub/cajun/elements.h>
#include <libaegisub/exception.h>

#pragma once

namespace toolbar {
DEFINE_BASE_EXCEPTION_NOINNER(ToolbarError, agi::Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(ToolbarJsonValueArray, ToolbarError, "toolbar/value/array")
DEFINE_SIMPLE_EXCEPTION_NOINNER(ToolbarInvalidName, ToolbarError, "toolbar/invalid")

class Toolbar;
extern Toolbar *toolbar;

class Toolbar {
public:
	Toolbar();
	~Toolbar();
	void GetToolbar(std::string name, wxToolBar *toolbar);

private:
	typedef std::map<std::string, wxToolBar*> TbMap;
	typedef std::pair<std::string, wxToolBar*> TbPair;

	TbMap map;

	enum ToolbarTypes {
		Standard = 1,
		Spacer = 100
	};

	void BuildToolbar(wxToolBar *toolbar, const json::Array& array);

};

} // namespace toolbar

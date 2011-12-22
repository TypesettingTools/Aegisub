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
//
// $Id$

/// @file ass_entry.h
/// @see ass_entry.cpp
/// @ingroup subs_storage
///


#pragma once

#ifndef AGI_PRE
#include <wx/string.h>
#endif

#include <libaegisub/exception.h>

class AssDialogue;
class AssStyle;
class AssAttachment;

enum ASS_EntryType {
	ENTRY_BASE,
	ENTRY_DIALOGUE,
	ENTRY_STYLE,
	ENTRY_ATTACHMENT
};

/// @see aegisub.h
namespace Aegisub {

	/// DOCME
	/// @class InvalidMarginIdError
	/// @brief DOCME
	///
	/// DOCME
	class InvalidMarginIdError : public agi::InternalError {
	public:
		InvalidMarginIdError() : InternalError("Invalid margin id", 0) { }
		const char *GetName() const { return "internal_error/invalid_margin_id"; }
	};
}

/// DOCME
/// @class AssEntry
/// @brief DOCME
///
/// DOCME
class AssEntry {
	/// Raw data, exactly the same line that appears on the .ass (note that this will be in ass even if source wasn't)
	wxString data;

public:
	/// Group it belongs to, e.g. "[Events]"
	wxString group;

	AssEntry(wxString const& data = "") : data(data) { }
	virtual ~AssEntry() { }

	/// Create a copy of this entry
	virtual AssEntry *Clone() const;

	/// Get this entry's fully-derived type
	virtual ASS_EntryType GetType() const { return ENTRY_BASE; }

	/// @brief Get this line's raw entry data in ASS format
	virtual const wxString GetEntryData() const { return data; }

	/// @brief Set this line's raw entry data
	/// @param newData New raw entry data
	virtual void SetEntryData(wxString const& newData) { data = newData; }

	/// Get this line in SSA format
	virtual wxString GetSSAText() const;
};

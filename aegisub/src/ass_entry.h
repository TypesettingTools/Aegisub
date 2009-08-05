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


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/string.h>
#include "include/aegisub/exception.h"


//////////////
// Prototypes
class AssDialogue;
class AssStyle;
class AssAttachment;



/// DOCME
enum ASS_EntryType {

	/// DOCME
	ENTRY_BASE,

	/// DOCME
	ENTRY_DIALOGUE,

	/// DOCME
	ENTRY_STYLE,

	/// DOCME
	ENTRY_ATTACHMENT
};



/// @see aegisub.h
namespace Aegisub {

	/// DOCME
	/// @class InvalidMarginIdError
	/// @brief DOCME
	///
	/// DOCME
	class InvalidMarginIdError : public InternalError {
	public:

		/// @brief DOCME
		/// @return 
		///
		InvalidMarginIdError() : InternalError(_T("Invalid margin id"), 0) { }

		/// @brief DOCME
		/// @return 
		///
		const wxChar *GetName() { return _T("internal_error/invalid_margin_id"); }
	};
};



/// DOCME
/// @class AssEntry
/// @brief DOCME
///
/// DOCME
class AssEntry {
private:

	/// DOCME
	wxString data;		// Raw data, exactly the same line that appears on the .ass (note that this will be in ass even if source wasn't)

	/// DOCME
	int StartMS;		// This is only stored for sorting issues, in order to keep non-dialogue lines aligned

public:

	/// DOCME
	bool Valid;			// Flags as valid or not

	/// DOCME
	wxString group;		// Group it belongs to, e.g. "[Events]"

	AssEntry();
	AssEntry(wxString data);
	virtual ~AssEntry();

	virtual AssEntry *Clone() const;


	/// @brief DOCME
	/// @return 
	///
	virtual int GetStartMS() const { return StartMS; }

	/// @brief DOCME
	/// @return 
	///
	virtual int GetEndMS() const { return StartMS; }

	/// @brief DOCME
	/// @param newStart 
	///
	virtual void SetStartMS(const int newStart) { StartMS = newStart; }

	/// @brief DOCME
	/// @param newEnd 
	/// @return 
	///
	virtual void SetEndMS(const int newEnd) { /* do nothing */ (void)newEnd; }


	/// @brief DOCME
	/// @return 
	///
	virtual ASS_EntryType GetType() { return ENTRY_BASE; }

	/// @brief DOCME
	/// @return 
	///
	virtual const wxString GetEntryData() { return data; }

	/// @brief DOCME
	/// @param newData 
	///
	virtual void SetEntryData(wxString newData) { if (newData.IsEmpty()) data.Clear(); else data = newData; }

	virtual wxString GetSSAText();
	static AssDialogue *GetAsDialogue(AssEntry *base);	// Returns an entry base as a dialogue if it is valid, null otherwise
	static AssStyle *GetAsStyle(AssEntry *base);		// Returns an entry base as a style if it is valid, null otherwise
	static AssAttachment *GetAsAttachment(AssEntry *base);// Returns an entry base as an attachment if it is valid, null otherwise
};

// This operator is for sorting
bool operator < (const AssEntry &t1, const AssEntry &t2);



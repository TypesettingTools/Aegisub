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

/// @file ass_entry.cpp
/// @brief Superclass for different kinds of lines in subtitles
/// @ingroup subs_storage
///


///////////
// Headers
#include "config.h"

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_entry.h"
#include "ass_style.h"


/// @brief Constructs AssEntry  AssEntry //////////////////////
///
AssEntry::AssEntry() {
	Valid = true;
}


/// @brief DOCME
/// @param _data 
///
AssEntry::AssEntry(wxString _data) {
	data = _data;
	Valid = true;
}



/// @brief Destructor for AssEntry 
///
AssEntry::~AssEntry() {
}



/// @brief Comparison for STL Sort 
/// @param t1 
/// @param t2 
/// @return 
///
bool operator < (const AssEntry &t1, const AssEntry &t2) {
	return (t1.GetStartMS() < t2.GetStartMS());
}



/// @brief Returns an entry as dialogue if possible, else, returns NULL 
/// @param base 
/// @return 
///
AssDialogue *AssEntry::GetAsDialogue(AssEntry *base) {
	if (!base) return NULL;
	if (base->GetType() == ENTRY_DIALOGUE) {
		return static_cast<AssDialogue*> (base);
	}
	return NULL;
}



/// @brief Returns an entry as style if possible, else, returns NULL 
/// @param base 
/// @return 
///
AssStyle *AssEntry::GetAsStyle(AssEntry *base) {
	if (!base) return NULL;
	if (base->GetType() == ENTRY_STYLE) {
		return static_cast<AssStyle*> (base);
	}
	return NULL;
}



/// @brief Returns an entry as attachment if possible, else, returns NULL 
/// @param base 
/// @return 
///
AssAttachment *AssEntry::GetAsAttachment(AssEntry *base) {
	if (!base) return NULL;
	if (base->GetType() == ENTRY_ATTACHMENT) {
		return static_cast<AssAttachment*> (base);
	}
	return NULL;
}



/// @brief Get SSA conversion 
/// @return 
///
wxString AssEntry::GetSSAText() {
	// Special cases
	if (data.Lower() == _T("[v4+ styles]")) return wxString(_T("[V4 Styles]"));
	if (data.Lower() == _T("scripttype: v4.00+")) return wxString(_T("ScriptType: v4.00"));
	if (data.Lower().Left(7) == _T("format:")) {
		if (group.Lower() == _T("[events]")) return wxString(_T("Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text"));
		if (group.Lower() == _T("[v4+ styles]")) return wxString(_T("Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, TertiaryColour, BackColour, Bold, Italic, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, AlphaLevel, Encoding"));
	}
	return GetEntryData();
}



/// @brief Clone 
///
AssEntry *AssEntry::Clone() const {
	// Create clone
	AssEntry *final = new AssEntry();

	// Copy data
	final->data = data;
	final->group = group;
	final->StartMS = StartMS;
	final->Valid = Valid;

	// Return
	return final;
}



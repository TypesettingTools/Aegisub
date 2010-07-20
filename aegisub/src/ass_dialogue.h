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

/// @file ass_dialogue.h
/// @see ass_dialogue.cpp
/// @ingroup subs_storage
///

#ifndef AGI_PRE
#include <vector>
#endif

#include "ass_entry.h"
#include "ass_time.h"

enum ASS_BlockType {
	BLOCK_BASE,
	BLOCK_PLAIN,
	BLOCK_OVERRIDE,
	BLOCK_DRAWING
};

class AssOverrideParameter;
class AssOverrideTag;

/// DOCME
/// @class AssDialogueBlock

/// @brief AssDialogue Blocks
///
/// A block is each group in the text field of an AssDialogue
/// @verbatim
///  Yes, I {\i1}am{\i0} here.
///
/// Gets split in five blocks:
///  "Yes, I " (Plain)
///  "\\i1"     (Override)
///  "am"      (Plain)
///  "\\i0"     (Override)
///  " here."  (Plain)
///
/// Also note how {}s are discarded.
/// Override blocks are further divided in AssOverrideTag's.
///
/// The GetText() method generates a new value for the "text" field from
/// the other fields in the specific class, and returns the new value.
/// @endverbatim
class AssDialogueBlock {
public:
	/// DOCME
	wxString text;

	/// DOCME
	AssDialogue *parent;

	AssDialogueBlock() { }
	virtual ~AssDialogueBlock() { }

	virtual ASS_BlockType GetType() = 0;

	/// @brief DOCME
	/// @return 
	///
	virtual wxString GetText() { return text; }
};



/// DOCME
/// @class AssDialogueBlockPlain

/// @brief DOCME
///
/// DOCME
class AssDialogueBlockPlain : public AssDialogueBlock {
public:
	ASS_BlockType GetType() { return BLOCK_PLAIN; }
	AssDialogueBlockPlain() { }
};



/// DOCME
/// @class AssDialogueBlockDrawing
/// @brief DOCME
///
/// DOCME
class AssDialogueBlockDrawing : public AssDialogueBlock {
public:
	/// DOCME
	int Scale;

	ASS_BlockType GetType() { return BLOCK_DRAWING; }
	AssDialogueBlockDrawing() { }
	void TransformCoords(int trans_x,int trans_y,double mult_x,double mult_y);
};



/// DOCME
/// @class AssDialogueBlockOverride
/// @brief DOCME
///
/// DOCME
class AssDialogueBlockOverride : public AssDialogueBlock {
public:
	AssDialogueBlockOverride() { }
	~AssDialogueBlockOverride();

	/// DOCME
	std::vector<AssOverrideTag*> Tags;


	/// @brief DOCME
	/// @return 
	///
	ASS_BlockType GetType() { return BLOCK_OVERRIDE; }
	wxString GetText();
	void ParseTags();		// Parses tags
	void AddTag(wxString const& tag);

	/// Type of callback function passed to ProcessParameters
	typedef void (*ProcessParametersCallback)(wxString,int,AssOverrideParameter*,void *);
	/// @brief Process parameters via callback 
	/// @param callback The callback function to call per tag paramer
	/// @param userData User data to pass to callback function
	void ProcessParameters(ProcessParametersCallback callback,void *userData);
};



/// DOCME
/// @class AssDialogue
/// @brief DOCME
///
/// DOCME
class AssDialogue : public AssEntry {
	wxString GetData(bool ssa) const;
public:

	/// Contains information about each block of text
	std::vector<AssDialogueBlock*> Blocks;

	/// Is this a comment line?
	bool Comment;
	/// Layer number
	int Layer;
	/// Margins: 0 = Left, 1 = Right, 2 = Top (Vertical), 3 = Bottom
	int Margin[4];
	/// Starting time
	AssTime Start;
	/// Ending time
	AssTime End;
	/// Style name
	wxString Style;
	/// Actor name
	wxString Actor;
	/// Effect name
	wxString Effect;
	/// Raw text data
	wxString Text;

	ASS_EntryType GetType() const { return ENTRY_DIALOGUE; }

	/// @brief Parse raw ASS data into everything else
	/// @param data ASS line
	/// @param version ASS version to try first (4, 4+, ASS2)
	/// @return Did it successfully parse?
	bool Parse(wxString data,int version=1);
	/// Parse text as ASS to generate block information
	void ParseASSTags();
	/// Parse text as SRT to generate block information
	void ParseSRTTags();
	/// Clear all blocks, ALWAYS call this after you're done processing tags
	void ClearBlocks();

	/// @brief Process parameters via callback 
	/// @param callback The callback function to call per tag parameter
	/// @param userData User data to pass to callback function
	void ProcessParameters(AssDialogueBlockOverride::ProcessParametersCallback callback,void *userData=NULL);
	/// Convert ASS tags to SRT tags
	void ConvertTagsToSRT();
	/// Strip all ASS tags from the text
	void StripTags();
	/// Strip a specific ASS tag from the text
	void StripTag(wxString tagName);
	/// Get text without tags
	wxString GetStrippedText() const;

	/// If blocks have been parsed, update the text from their current value
	void UpdateText();
	const wxString GetEntryData() const;
	/// Do nothing
	void SetEntryData(wxString) { }

	template<int which>
	void SetMarginString(const wxString value) { SetMarginString(value, which);}
	/// @brief Set a margin 
	/// @param value New value of the margin
	/// @param which 0 = left, 1 = right, 2 = vertical/top, 3 = bottom
	void SetMarginString(const wxString value,int which);
	/// @brief Get a margin
	/// @param which 0 = left, 1 = right, 2 = vertical/top, 3 = bottom
	/// @param pad Pad the number to four digits
	wxString GetMarginString(int which,bool pad=true) const;
	/// Get the line as SSA rather than ASS
	wxString GetSSAText() const;
	/// Does this line collide with the passed line?
	bool CollidesWith(AssDialogue *target);

	AssEntry *Clone() const;

	AssDialogue();
	AssDialogue(AssDialogue const&);
	AssDialogue(wxString data,int version=1);
	~AssDialogue();
};

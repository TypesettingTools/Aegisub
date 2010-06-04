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


////////////
// Includes

#ifndef AGI_PRE
#include <vector>
#endif

#include "ass_entry.h"
#include "ass_time.h"


//////////////
// Prototypes
class AssOverrideParameter;
class AssOverrideTag;
class AssDialogueBlockPlain;
class AssDialogueBlockOverride;
class AssDialogueBlockDrawing;



/// DOCME
enum ASS_BlockType {

	/// DOCME
	BLOCK_BASE,

	/// DOCME
	BLOCK_PLAIN,

	/// DOCME
	BLOCK_OVERRIDE,

	/// DOCME
	BLOCK_DRAWING
};


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

	AssDialogueBlock();
	virtual ~AssDialogueBlock();

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

	/// @brief DOCME
	/// @return 
	///
	ASS_BlockType GetType() { return BLOCK_PLAIN; }
	AssDialogueBlockPlain();
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


	/// @brief DOCME
	/// @return 
	///
	ASS_BlockType GetType() { return BLOCK_DRAWING; }
	AssDialogueBlockDrawing();
	void TransformCoords(int trans_x,int trans_y,double mult_x,double mult_y);
};



/// DOCME
/// @class AssDialogueBlockOverride
/// @brief DOCME
///
/// DOCME
class AssDialogueBlockOverride : public AssDialogueBlock {
public:
	AssDialogueBlockOverride();
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
private:
	wxString MakeData();

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


	/// @brief DOCME
	/// @return 
	///
	ASS_EntryType GetType() { return ENTRY_DIALOGUE; }

	bool Parse(wxString data,int version=1);	// Parses raw ASS data into everything else
	void ParseASSTags();			// Parses text to generate block information (doesn't update data)
	void ParseSRTTags();			// Converts tags to ass format and calls ParseASSTags+UpdateData
	void ClearBlocks();				// Clear all blocks, ALWAYS call this after you're done processing tags

	/// @brief Process parameters via callback 
	/// @param callback The callback function to call per tag paramer
	/// @param userData User data to pass to callback function
	void ProcessParameters(AssDialogueBlockOverride::ProcessParametersCallback callback,void *userData=NULL);	// Callback to process parameters
	void ConvertTagsToSRT();		// Converts tags to SRT format
	void StripTags();				// Strips all tags from the text
	void StripTag(wxString tagName);// Strips a specific tag from the text
	wxString GetStrippedText() const; // Gets text without tags

	void UpdateData();				// Updates raw data from current values + text
	void UpdateText();				// Generates text from the override tags
	const wxString GetEntryData();
	void SetEntryData(wxString newData);
	void Clear();					// Wipes all data


	void SetMarginString(const wxString value,int which);	// Set string to a margin value (0 = left, 1 = right, 2 = vertical/top, 3 = bottom)
	wxString GetMarginString(int which,bool pad=true);		// Returns the string of a margin value (0 = left, 1 = right, 2 = vertical/top, 3 = bottom)
	wxString GetSSAText();
	bool CollidesWith(AssDialogue *target);					// Checks if two lines collide

	AssEntry *Clone() const;

	AssDialogue();
	AssDialogue(wxString data,int version=1);
	~AssDialogue();
};

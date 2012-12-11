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

/// @file ass_dialogue.h
/// @see ass_dialogue.cpp
/// @ingroup subs_storage
///

#include "ass_entry.h"
#include "ass_override.h"
#include "ass_time.h"

#include <libaegisub/exception.h>

#include <boost/flyweight.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>

enum AssBlockType {
	BLOCK_BASE,
	BLOCK_PLAIN,
	BLOCK_OVERRIDE,
	BLOCK_DRAWING
};

std::size_t hash_value(wxString const& s);

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
/// Override blocks are further divided in AssOverrideTags.
///
/// The GetText() method generates a new value for the "text" field from
/// the other fields in the specific class, and returns the new value.
/// @endverbatim
class AssDialogueBlock {
protected:
	/// Text of this block
	wxString text;
public:
	AssDialogueBlock(wxString const& text) : text(text) { }
	virtual ~AssDialogueBlock() { }

	virtual AssBlockType GetType() const = 0;
	virtual wxString GetText() { return text; }
};

class AssDialogueBlockPlain : public AssDialogueBlock {
public:
	AssBlockType GetType() const override { return BLOCK_PLAIN; }
	AssDialogueBlockPlain(wxString const& text = wxString()) : AssDialogueBlock(text) { }
};

class AssDialogueBlockDrawing : public AssDialogueBlock {
public:
	int Scale;

	AssBlockType GetType() const override { return BLOCK_DRAWING; }
	AssDialogueBlockDrawing(wxString const& text, int scale) : AssDialogueBlock(text), Scale(scale) { }
	void TransformCoords(int trans_x,int trans_y,double mult_x,double mult_y);
};

class AssDialogueBlockOverride : public AssDialogueBlock {
public:
	AssDialogueBlockOverride(wxString const& text = wxString()) : AssDialogueBlock(text) { }

	std::vector<AssOverrideTag> Tags;

	AssBlockType GetType() const override { return BLOCK_OVERRIDE; }
	wxString GetText() override;
	void ParseTags();
	void AddTag(wxString const& tag);

	/// Type of callback function passed to ProcessParameters
	typedef void (*ProcessParametersCallback)(wxString const&, AssOverrideParameter *, void *);
	/// @brief Process parameters via callback
	/// @param callback The callback function to call per tag parameter
	/// @param userData User data to pass to callback function
	void ProcessParameters(ProcessParametersCallback callback, void *userData);
};

class AssDialogue : public AssEntry {
	wxString GetData(bool ssa) const;
public:
	/// Is this a comment line?
	bool Comment;
	/// Layer number
	int Layer;
	/// Margins: 0 = Left, 1 = Right, 2 = Top (Vertical)
	int Margin[3];
	/// Starting time
	AssTime Start;
	/// Ending time
	AssTime End;
	/// Style name
	boost::flyweight<wxString> Style;
	/// Actor name
	boost::flyweight<wxString> Actor;
	/// Effect name
	boost::flyweight<wxString> Effect;
	/// Raw text data
	boost::flyweight<wxString> Text;

	AssEntryGroup Group() const override { return ENTRY_DIALOGUE; }

	/// @brief Parse raw ASS data into everything else
	/// @param data ASS line
	/// @return Did it successfully parse?
	bool Parse(wxString const& data);

	/// Parse text as ASS and return block information
	std::auto_ptr<boost::ptr_vector<AssDialogueBlock>> ParseTags() const;

	/// Strip all ASS tags from the text
	void StripTags();
	/// Strip a specific ASS tag from the text
	/// Get text without tags
	wxString GetStrippedText() const;

	/// Update the text of the line from parsed blocks
	void UpdateText(boost::ptr_vector<AssDialogueBlock>& blocks);
	const wxString GetEntryData() const override;

	template<int which>
	void SetMarginString(wxString const& value) { SetMarginString(value, which);}
	/// @brief Set a margin
	/// @param value New value of the margin
	/// @param which 0 = left, 1 = right, 2 = vertical
	void SetMarginString(wxString const& value, int which);
	/// @brief Get a margin
	/// @param which 0 = left, 1 = right, 2 = vertical
	wxString GetMarginString(int which) const;
	/// Get the line as SSA rather than ASS
	wxString GetSSAText() const override;
	/// Does this line collide with the passed line?
	bool CollidesWith(const AssDialogue *target) const;

	AssEntry *Clone() const;

	AssDialogue();
	AssDialogue(AssDialogue const&);
	AssDialogue(wxString const& data);
	~AssDialogue();
};

class InvalidMarginIdError : public agi::InternalError {
public:
	InvalidMarginIdError() : InternalError("Invalid margin id", 0) { }
	const char *GetName() const { return "internal_error/invalid_margin_id"; }
};

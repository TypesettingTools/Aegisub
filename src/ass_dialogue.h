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

#include <array>
#include <boost/flyweight.hpp>
#include <vector>

enum class AssBlockType {
	PLAIN,
	COMMENT,
	OVERRIDE,
	DRAWING
};

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
	std::string text;
public:
	AssDialogueBlock(std::string text) : text(std::move(text)) { }
	virtual ~AssDialogueBlock() { }

	virtual AssBlockType GetType() const = 0;
	virtual std::string GetText() { return text; }
};

class AssDialogueBlockPlain final : public AssDialogueBlock {
public:
	using AssDialogueBlock::text;
	AssBlockType GetType() const override { return AssBlockType::PLAIN; }
	AssDialogueBlockPlain(std::string const& text = std::string()) : AssDialogueBlock(text) { }
};

class AssDialogueBlockComment final : public AssDialogueBlock {
public:
	AssBlockType GetType() const override { return AssBlockType::COMMENT; }
	AssDialogueBlockComment(std::string const& text = std::string()) : AssDialogueBlock("{" + text + "}") { }
};

class AssDialogueBlockDrawing final : public AssDialogueBlock {
public:
	using AssDialogueBlock::text;
	int Scale;

	AssBlockType GetType() const override { return AssBlockType::DRAWING; }
	AssDialogueBlockDrawing(std::string const& text, int scale) : AssDialogueBlock(text), Scale(scale) { }
};

class AssDialogueBlockOverride final : public AssDialogueBlock {
public:
	AssDialogueBlockOverride(std::string const& text = std::string()) : AssDialogueBlock(text) { }

	std::vector<AssOverrideTag> Tags;

	AssBlockType GetType() const override { return AssBlockType::OVERRIDE; }
	std::string GetText() override;
	void ParseTags();
	void AddTag(std::string const& tag);

	/// Type of callback function passed to ProcessParameters
	typedef void (*ProcessParametersCallback)(std::string const&, AssOverrideParameter *, void *);
	/// @brief Process parameters via callback
	/// @param callback The callback function to call per tag parameter
	/// @param userData User data to pass to callback function
	void ProcessParameters(ProcessParametersCallback callback, void *userData);
};

struct AssDialogueBase {
	/// Unique ID of this line. Copies of the line for Undo/Redo purposes
	/// preserve the unique ID, so that the equivalent lines can be found in
	/// the different versions of the file.
	int Id;

	int Row = -1;

	/// Is this a comment line?
	bool Comment = false;
	/// Layer number
	int Layer = 0;
	/// Margins: 0 = Left, 1 = Right, 2 = Top (Vertical)
	std::array<int, 3> Margin = {{0, 0, 0}};
	/// Starting time
	AssTime Start = 0;
	/// Ending time
	AssTime End = 5000;
	/// Style name
	boost::flyweight<std::string> Style{ "Default" };
	/// Actor name
	boost::flyweight<std::string> Actor;
	/// Effect name
	boost::flyweight<std::string> Effect;
	/// Raw text data
	boost::flyweight<std::string> Text;
};

class AssDialogue final : public AssEntry, public AssDialogueBase, public AssEntryListHook {
	std::string GetData(bool ssa) const;

	/// @brief Parse raw ASS data into everything else
	/// @param data ASS line
	void Parse(std::string const& data);
public:
	AssEntryGroup Group() const override { return AssEntryGroup::DIALOGUE; }

	/// Parse text as ASS and return block information
	std::vector<std::unique_ptr<AssDialogueBlock>> ParseTags() const;

	/// Strip all ASS tags from the text
	void StripTags();
	/// Strip a specific ASS tag from the text
	/// Get text without tags
	std::string GetStrippedText() const;

	/// Update the text of the line from parsed blocks
	void UpdateText(std::vector<std::unique_ptr<AssDialogueBlock>>& blocks);
	std::string GetEntryData() const { return GetData(false); }

	/// Get the line as SSA rather than ASS
	std::string GetSSAText() const { return GetData(true); }
	/// Does this line collide with the passed line?
	bool CollidesWith(const AssDialogue *target) const;

	AssDialogue();
	AssDialogue(AssDialogue const&);
	AssDialogue(AssDialogueBase const&);
	AssDialogue(std::string const& data);
	~AssDialogue();
};

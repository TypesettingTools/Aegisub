// Copyright (c) 2010, Niels Martin Hansen
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

#include <libaegisub/signal.h>

#include <set>
#include <vector>

class AssDialogue;
typedef std::set<AssDialogue *> Selection;

namespace agi { struct Context; }

class SelectionController {
	agi::signal::Signal<AssDialogue *> AnnounceActiveLineChanged;
	agi::signal::Signal<> AnnounceSelectedSetChanged;

	agi::Context *context;

	Selection selection; ///< Currently selected lines
	AssDialogue *active_line = nullptr; ///< The currently active line or 0 if none

public:
	SelectionController(agi::Context *context);

	/// @brief Change the active line
	/// @param new_line Subtitle line to become the new active line
	///
	/// The active line may be changed to nullptr, in which case there is no longer an
	/// active line.
	///
	/// Calling this method should only cause a change notification to be sent if
	/// the active line was actually changed.
	///
	/// This method must not affect the selected set.
	void SetActiveLine(AssDialogue *new_line);

	/// @brief Obtain the active line
	/// @return The active line or nullptr if there is none
	AssDialogue *GetActiveLine() const { return active_line;  }

	/// @brief Change the selected set
	/// @param new_selection The set of subtitle lines to become the new selected set
	///
	/// Implementations must either completely change the selected set to the new
	/// set provided, or not change the selected set at all. Partial changes are
	/// not allowed.
	///
	/// If no change happens to the selected set, whether because it was refused or
	/// because the new set was identical to the old set, no change notification may
	/// be sent.
	void SetSelectedSet(Selection new_selection);

	/// @brief Obtain the selected set
	/// @return The selected set
	Selection const& GetSelectedSet() const { return selection; }

	/// Get the selection sorted by row number
	std::vector<AssDialogue *> GetSortedSelection() const;

	/// @brief Set both the selected set and active line
	/// @param new_line Subtitle line to become the new active line
	/// @param new_selection The set of subtitle lines to become the new selected set
	///
	/// This sets both the active line and selected set before announcing the
	/// change to either of them, and is guaranteed to announce the active line
	/// change before the selection change.
	void SetSelectionAndActive(Selection new_selection, AssDialogue *new_line);

	/// @brief Change the active line to the next in sequence
	///
	/// If there is no logical next line in sequence, no change happens. This should
	/// also reset the selected set to consist of exactly the active line, if the
	/// active line was changed.
	void NextLine();

	/// @brief Change the active line to the previous in sequence
	///
	/// If there is no logical previous line in sequence, no change happens. This
	/// should also reset the selected set to consist of exactly the active line, if
	/// the active line was changed.
	void PrevLine();

	DEFINE_SIGNAL_ADDERS(AnnounceSelectedSetChanged, AddSelectionListener)
	DEFINE_SIGNAL_ADDERS(AnnounceActiveLineChanged, AddActiveLineListener)
};

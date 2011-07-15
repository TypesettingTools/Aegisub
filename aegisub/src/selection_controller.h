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
//
// $Id$

/// @file selection_controller.h
/// @ingroup controllers
/// @brief Interface declaration for the SubtitleSelectionController

#pragma once

#ifndef AGI_PRE
#include <set>
#endif

/// @class SelectionListener
/// @brief Abstract interface for classes wanting to subtitle selection change notifications
template <typename ItemDataType>
class SelectionListener {
public:
	typedef std::set<ItemDataType*> Selection;

	/// Virtual destructor for safety
	virtual ~SelectionListener() { }

	/// @brief Called when the active line changes
	/// @param new_line The line that is now the active line
	///
	/// In case new_line is 0, the active line was changed to none.
	virtual void OnActiveLineChanged(ItemDataType *new_line) = 0;

	/// @brief Called when the selected set changes
	/// @param lines_added   Lines added to the selection
	/// @param lines_removed Lines removed from the selection
	///
	/// The two sets must not intersect.
	virtual void OnSelectedSetChanged(const Selection &lines_added, const Selection &lines_removed) = 0;
};


/// @class SelectionController
/// @brief Abstract interface for selection controllers
///
/// Two concepts are managed by implementations of this interface: The concept of the
/// active line, and the concept of the set of selected lines. There is one or zero
/// active lines, the active line is the base for subtitle manipulation in the GUI.
/// The set of selected lines may contain any number of subtitle lines, and those
/// lines are the primary target of subtitle manipulation. In other words, the active
/// line controls what values the user is presented to modify, and the selected set
/// controls what lines are actually modified when the user performs modifications.
/// In most cases, the active line will be a member of the selected set. It will be
/// the responsibility of manipulators to affect the appropriate lines.
///
/// There is only intended to be one instance of a class implementing this interface
/// per editing session, but there may be many different implementations of it.
/// The primary implementation would be the subtitle grid in the main GUI, allowing
/// the user to actively manipulate the active and selected line sets, but other
/// potential implementations are in a test driver and in a non-interactive scenario.
///
/// Objects implementing the SelectionListener interface can subscribe to
/// changes in the active line and the selected set.
template <typename ItemDataType>
class SelectionController {
public:
	typedef std::set<ItemDataType*> Selection;

	/// Virtual destructor for safety
	virtual ~SelectionController() { }

	/// @brief Change the active line
	/// @param new_line Subtitle line to become the new active line
	///
	/// The active line may be changed to NULL, in which case there is no longer an
	/// active line.
	///
	/// Calling this method should only cause a change notification to be sent if
	/// the active line was actually changed.
	///
	/// This method must not affect the selected set.
	virtual void SetActiveLine(ItemDataType *new_line) = 0;
	
	/// @brief Obtain the active line
	/// @return The active line or NULL if there is none
	virtual ItemDataType * GetActiveLine() const = 0;

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
	virtual void SetSelectedSet(const Selection &new_selection) = 0;

	/// @brief Obtain the selected set
	/// @param[out] selection Filled with the selected set on return
	virtual void GetSelectedSet(Selection &selection) const = 0;

	/// @brief Obtain the selected set
	/// @return The selected set
	virtual Selection GetSelectedSet() const = 0;

	/// @brief Change the active line to the next in sequence
	///
	/// If there is no logical next line in sequence, no change happens. This should
	/// also reset the selected set to consist of exactly the active line, if the
	/// active line was changed.
	virtual void NextLine() = 0;

	/// @brief Change the active line to the previous in sequence
	///
	/// If there is no logical previous line in sequence, no change happens. This
	/// should also reset the selected set to consist of exactly the active line, if
	/// the active line was changed.
	virtual void PrevLine() = 0;

	/// @brief Subscribe an object to receive change notifications
	/// @param listener Object to subscribe to change notifications
	virtual void AddSelectionListener(SelectionListener<ItemDataType> *listener) = 0;

	/// @brief Unsubscribe an object from change notifications
	/// @param listener Object to unsubscribe from change notifications
	virtual void RemoveSelectionListener(SelectionListener<ItemDataType> *listener) = 0;
};


/// @class BaseSelectionController
/// @brief Base-implementation of SelectionController
///
/// This class implements adding and removing listeners for selection change
/// notifications, and provides protected functions to announce selection changes.
///
/// This class should be derived from for most real-world uses, but might not
/// be desirable in some special cases such as test drivers.
template <typename ItemDataType>
class BaseSelectionController : public SelectionController<ItemDataType> {
public:
	typedef typename SelectionController<ItemDataType>::Selection Selection;
private:
	typedef std::set<SelectionListener<ItemDataType> *> SelectionListenerSet;
	SelectionListenerSet listeners;

protected:
	/// Call OnActiveLineChanged on all listeners
	void AnnounceActiveLineChanged(ItemDataType *new_line)
	{
		for (typename SelectionListenerSet::iterator listener = listeners.begin(); listener != listeners.end(); ++listener)
		{
			(*listener)->OnActiveLineChanged(new_line);
		}
	}

	/// Call OnSelectedSetChangedon all listeners
	void AnnounceSelectedSetChanged(const Selection &lines_added, const Selection &lines_removed)
	{
		for (typename SelectionListenerSet::iterator listener = listeners.begin(); listener != listeners.end(); ++listener)
		{
			(*listener)->OnSelectedSetChanged(lines_added, lines_removed);
		}
	}

public:
	virtual ~BaseSelectionController() { }

	virtual void AddSelectionListener(SelectionListener<ItemDataType> *listener)
	{
		listeners.insert(listener);
	}

	virtual void RemoveSelectionListener(SelectionListener<ItemDataType> *listener)
	{
		listeners.erase(listener);
	}
};

/// Do-nothing selection controller, can be considered to always operate on an empty subtitle file
template <typename ItemDataType>
class DummySelectionController : public SelectionController<ItemDataType> {
public:
	typedef typename SelectionController<ItemDataType>::Selection Selection;
	virtual ~DummySelectionController() { }
	virtual void SetActiveLine(ItemDataType *new_line) { }
	virtual ItemDataType * GetActiveLine() const { return 0; }
	virtual void SetSelectedSet(const Selection &new_selection) { }
	virtual void GetSelectedSet(Selection &selection) const { }
	virtual void NextLine() { }
	virtual void PrevLine() { }
	virtual void AddSelectionListener(SelectionListener<ItemDataType> *listener) { }
	virtual void RemoveSelectionListener(SelectionListener<ItemDataType> *listener) { }
};

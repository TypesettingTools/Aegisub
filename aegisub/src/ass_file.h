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

/// @file ass_file.h
/// @see ass_file.cpp
/// @ingroup subs_storage
///


#pragma once

#ifndef AGI_PRE
#include <fstream>
#include <list>
#include <set>
#include <vector>

#include <wx/arrstr.h>
#endif

#include <libaegisub/signal.h>

class AssDialogue;
class AssStyle;
class AssAttachment;
class AssEntry;

typedef std::list<AssEntry*>::iterator entryIter;

/// DOCME
/// @class AssFile
/// @brief DOCME
///
/// DOCME
class AssFile {
	std::list<AssFile> UndoStack;
	std::list<AssFile> RedoStack;
	wxString undoDescription;
	/// Revision counter for undo coalescing and modified state tracking
	int commitId;
	/// Last saved version of this file
	int savedCommitId;
	/// Last autosaved version of this file
	int autosavedCommitId;

	/// A set of changes has been committed to the file (AssFile::CommitType)
	agi::signal::Signal<int> AnnounceCommit;
	/// A new file has been opened (filename)
	agi::signal::Signal<wxString> FileOpen;
	/// The file is about to be saved
	/// This signal is intended for adding metadata such as video filename,
	/// frame number, etc. Ideally this would all be done immediately rather
	/// than waiting for a save, but that causes (more) issues with undo
	agi::signal::Signal<> FileSave;

public:
	/// The lines in the file
	std::list<AssEntry*> Line;
	/// The filename of this file, if any
	wxString filename;
	/// Is the file loaded?
	bool loaded;

	AssFile();
	AssFile(const AssFile &from);
	AssFile& operator=(AssFile from);
	~AssFile();

	/// Does the file have unsaved changes?
	bool IsModified() const {return commitId != savedCommitId; };
	/// Clear the file
	void Clear();

	/// @brief Load default file
	/// @param defline Add a blank line to the file
	void LoadDefault(bool defline=true);
	/// Add a style to the file
	void InsertStyle(AssStyle *style);
	/// Add an attachment to the file
	void InsertAttachment(AssAttachment *attach);
	/// Attach a file to the ass file
	void InsertAttachment(wxString filename);
	/// Add a dialogue line to the file
	void InsertDialogue(AssDialogue *diag);
	/// Get the names of all of the styles available
	wxArrayString GetStyles() const;
	/// @brief Get a style by name
	/// @param name Style name
	/// @return Pointer to style or NULL
	AssStyle *GetStyle(wxString const& name);

	void swap(AssFile &) throw();

	/// @brief Load from a file
	/// @param file File name
	/// @param charset Character set of file or empty to autodetect
	void Load(const wxString &file, wxString const& charset="");

	/// @brief Save to a file
	/// @param file Path to save to
	/// @param setfilename Should the filename be changed to the passed path?
	/// @param addToRecent Should the file be added to the MRU list?
	/// @param encoding Encoding to use, or empty to let the writer decide (which usually means "App/Save Charset")
	void Save(wxString file,bool setfilename=false,bool addToRecent=true,const wxString encoding="");

	/// @brief Autosave the file if there have been any chances since the last autosave
	/// @return File name used or empty if no save was performed
	wxString AutoSave();

	/// @brief Save to a memory buffer. Used for subtitle providers which support it
	/// @param[out] dst Destination vector
	void SaveMemory(std::vector<char> &dst);
	/// Add file name to the MRU list
	void AddToRecent(wxString const& file) const;
	/// Can the file be saved in its current format?
	bool CanSave() const;

	/// @brief Get the script resolution
	/// @param[out] w Width
	/// @param[in] h Height
	void GetResolution(int &w,int &h) const;
	/// Get the value in a [Script Info] key as int, or 0 if it is not present
	int GetScriptInfoAsInt(wxString const& key) const;
	/// Get the value in a [Script Info] key as string.
	wxString GetScriptInfo(wxString key) const;
	/// Set the value of a [Script Info] key. Adds it if it doesn't exist.
	void SetScriptInfo(wxString const& key, wxString const& value);
	// Add a ";" comment in the [Script Info] section
	void AddComment(wxString comment);

	/// Type of changes made in a commit
	enum CommitType {
		/// Potentially the entire file has been changed; any saved information
		/// should be discarded. Note that the active line and selected set
		/// should not be touched in handlers for this, as they may not have
		/// been updated yet
		/// Note that it is intentional that this cannot be combined with
		/// other commit types
		COMMIT_NEW         = 0,
		/// The order of lines in the file has changed
		COMMIT_ORDER       = 0x1,
		/// The script info section has changed in some way
		COMMIT_SCRIPTINFO  = 0x2,
		/// The styles have changed in some way
		COMMIT_STYLES      = 0x4,
		/// The attachments have changed in some way
		COMMIT_ATTACHMENT  = 0x8,
		/// Dialogue lines have been added or removed
		/// Note that if the active dialogue line was removed, the active line
		/// should be updated BEFORE committing
		COMMIT_DIAG_ADDREM = 0x10,
		/// The metadata fields of existing dialogue lines have changed
		COMMIT_DIAG_META   = 0x20,
		/// The start and/or end times of existing dialogue lines have changed
		COMMIT_DIAG_TIME   = 0x40,
		/// The text of existing dialogue lines have changed
		COMMIT_DIAG_TEXT   = 0x80,
		COMMIT_DIAG_FULL   = COMMIT_DIAG_META | COMMIT_DIAG_TIME | COMMIT_DIAG_TEXT
	};

	DEFINE_SIGNAL_ADDERS(AnnounceCommit, AddCommitListener)
	DEFINE_SIGNAL_ADDERS(FileOpen, AddFileOpenListener)
	DEFINE_SIGNAL_ADDERS(FileSave, AddFileSaveListener)

	/// @brief Flag the file as modified and push a copy onto the undo stack
	/// @param desc        Undo description
	/// @param type        Type of changes made to the file in this commit
	/// @param commitId    Commit to amend rather than pushing a new commit
	/// @param single_line Line which was changed, if only one line was
	/// @return Unique identifier for the new undo group
	int Commit(wxString const& desc, int type, int commitId = -1, AssEntry *single_line = 0);
	/// @brief Undo the last set of changes to the file
	void Undo();
	/// @brief Redo the last undone changes
	void Redo();
	/// Check if undo stack is empty
	bool IsUndoStackEmpty() const { return UndoStack.size() <= 1; };
	/// Check if redo stack is empty
	bool IsRedoStackEmpty() const { return RedoStack.empty(); };
	/// Get the description of the first undoable change
	wxString GetUndoDescription() const;
	/// Get the description of the first redoable change
	wxString GetRedoDescription() const;

	/// Current script file. It is "above" the stack.
	static AssFile *top;

	/// Comparison function for use when sorting
	typedef bool (*CompFunc)(const AssDialogue* lft, const AssDialogue* rgt);

	/// Compare based on start time
	static bool CompStart(const AssDialogue* lft, const AssDialogue* rgt);
	/// Compare based on end time
	static bool CompEnd(const AssDialogue* lft, const AssDialogue* rgt);
	/// Compare based on style name
	static bool CompStyle(const AssDialogue* lft, const AssDialogue* rgt);
	/// Compare based on actor name
	static bool CompActor(const AssDialogue* lft, const AssDialogue* rgt);
	/// Compare based on effect
	static bool CompEffect(const AssDialogue* lft, const AssDialogue* rgt);
	/// Compare based on layer
	static bool CompLayer(const AssDialogue* lft, const AssDialogue* rgt);

	/// @brief Sort the dialogue lines in this file
	/// @param comp Comparison function to use. Defaults to sorting by start time.
	/// @param limit If non-empty, only lines in this set are sorted
	void Sort(CompFunc comp = CompStart, std::set<AssDialogue*> const& limit = std::set<AssDialogue*>());
	/// @brief Sort the dialogue lines in the given list
	/// @param comp Comparison function to use. Defaults to sorting by start time.
	/// @param limit If non-empty, only lines in this set are sorted
	static void Sort(std::list<AssEntry*>& lst, CompFunc comp = CompStart, std::set<AssDialogue*> const& limit = std::set<AssDialogue*>());
	/// @brief Sort the dialogue lines in the given list
	/// @param comp Comparison function to use. Defaults to sorting by start time.
	static void Sort(std::list<AssDialogue*>& lst, CompFunc comp = CompStart);
};

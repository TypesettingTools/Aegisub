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
#include <vector>

#include <wx/arrstr.h>
#endif

class FrameRate;
class AssDialogue;
class AssStyle;
class AssAttachment;
class AssDialogueBlock;
class AssDialogueBlockOverride;
class AssDialogueBlockPlain;
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
	/// Get the names of all of the styles available
	wxArrayString GetStyles();
	/// @brief Get a style by name
	/// @param name Style name
	/// @return Pointer to style or NULL
	AssStyle *GetStyle(wxString name);

	void swap(AssFile &) throw();

	/// @brief Load from a file
	/// @param file File name
	/// @param charset Character set of file or empty to autodetect
	/// @param addToRecent Should the file be added to the MRU list?
	void Load(const wxString &file,wxString charset="",bool addToRecent=true);
	/// @brief Save to a file
	/// @param file Path to save to
	/// @param setfilename Should the filename be changed to the passed path?
	/// @param addToRecent Should the file be added to the MRU list?
	/// @param encoding Encoding to use, or empty to let the writer decide (which usually means "App/Save Charset")
	void Save(wxString file,bool setfilename=false,bool addToRecent=true,const wxString encoding=_T(""));
	/// @brief Save to a memory buffer. Used for subtitle providers which support it
	/// @param[out] dst Destination vector
	void SaveMemory(std::vector<char> &dst,const wxString encoding=_T(""));
	/// @brief Saves exported copy, with effects applied
	/// @param file Path to save to; file name is never set to this
	void Export(wxString file);
	/// Add file name to the MRU list
	void AddToRecent(wxString file);
	/// Can the file be saved in its current format?
	bool CanSave();
	/// @brief Get the list of wildcards supported
	/// @param mode 0 = open, 1 = save, 2 = export
	static wxString GetWildcardList(int mode);

	/// @brief Get the script resolution
	/// @param[out] w Width
	/// @param[in] h Height
	void GetResolution(int &w,int &h);
	/// Get the value in a [Script Info] key as int.
	int GetScriptInfoAsInt(const wxString key);
	/// Get the value in a [Script Info] key as string.
	wxString GetScriptInfo(const wxString key);
	/// Set the value of a [Script Info] key. Adds it if it doesn't exist.
	void SetScriptInfo(const wxString key,const wxString value);
	// Add a ";" comment in the [Script Info] section
	void AddComment(const wxString comment);
	/// @brief Add a line to the file
	/// @param data Full text of ASS line
	/// @param group Section of the file to add the line to
	/// @param[out] version ASS version the line was parsed as
	/// @param[out] outGroup Group it was actually added to; attachments do something strange here
	void AddLine(wxString data,wxString group,int &version,wxString *outGroup=NULL);

	/// @brief Flag the file as modified and push a copy onto the undo stack
	/// @param desc     Undo description
	/// @param commitId Commit to amend rather than pushing a new commit
	/// @return Unique identifier for the new undo group
	int Commit(wxString desc, int commitId = -1);
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

	/// @brief Compare based on start time
	static bool CompStart(const AssDialogue* lft, const AssDialogue* rgt);
	/// @brief Compare based on end time
	static bool CompEnd(const AssDialogue* lft, const AssDialogue* rgt);
	/// @brief Compare based on end time
	static bool CompStyle(const AssDialogue* lft, const AssDialogue* rgt);

	/// @brief Sort the dialogue lines in this file
	/// @param comp Comparison function to use. Defaults to sorting by start time.
	void Sort(CompFunc comp = CompStart);
	/// @brief Sort the dialogue lines in the given list
	/// @param comp Comparison function to use. Defaults to sorting by start time.
	static void Sort(std::list<AssEntry*>& lst, CompFunc comp = CompStart);
	/// @brief Sort the dialogue lines in the given list
	/// @param comp Comparison function to use. Defaults to sorting by start time.
	static void Sort(std::list<AssDialogue*>& lst, CompFunc comp = CompStart);
};

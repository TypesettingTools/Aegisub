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
private:

	/// DOCME
	bool Modified;


	/// DOCME
	static std::list<AssFile*> UndoStack;

	/// DOCME
	static std::list<AssFile*> RedoStack;

	/// DOCME
	static bool StackModified;
	static void StackClear();

public:

	/// DOCME
	std::list<AssEntry*> Line;


	/// DOCME
	wxString filename;

	/// DOCME
	wxString undodescription;

	/// DOCME
	bool loaded;

	AssFile();
	AssFile(AssFile &from);
	~AssFile();

	bool IsModified();									// Returns if file has unmodified changes
	void FlagAsModified(wxString desc);					// Flag file as being modified, will automatically put a copy on stack
	void Clear();										// Wipes file
	void CompressForStack(bool compress);				// Compress/decompress for storage on stack
	void LoadDefault(bool defline=true);				// Loads default file. Pass false to prevent it from adding a default line too
	void InsertStyle(AssStyle *style);					// Inserts a style to file
	void InsertAttachment(AssAttachment *attach);		// Inserts an attachment
	void InsertAttachment(wxString filename);			// Inserts a file as an attachment
	wxArrayString GetStyles();							// Gets a list of all styles available
	AssStyle *GetStyle(wxString name);					// Gets style by its name

	//wxString GetString();								// Returns the whole file as a single string
	void Load(wxString file,wxString charset=_T(""),bool addToRecent=true);	// Load from a file
	void Save(wxString file,bool setfilename=false,bool addToRecent=true,const wxString encoding=_T(""));	// Save to a file. Pass true to second argument if this isn't a copy
	void SaveMemory(std::vector<char> &dst,const wxString encoding=_T(""));	// Save to a memory string
	void Export(wxString file);							// Saves exported copy, with effects applied
	void AddToRecent(wxString file);                    // Adds file name to list of recently opened files
	bool CanSave();										// Returns true if the file can be saved in its current format
	static wxString GetWildcardList(int mode);			// Returns the list of wildcards supported (0 = open, 1 = save, 2 = export)

	void GetResolution(int &w,int &h);								// Get resolution
	int GetScriptInfoAsInt(const wxString key);						// Returns the value in a [Script Info] key as int.
	wxString GetScriptInfo(const wxString key);						// Returns the value in a [Script Info] key as string.
	void SetScriptInfo(const wxString key,const wxString value);	// Sets the value of a [Script Info] key. Adds it if it doesn't exist.
	void AddComment(const wxString comment);						// Adds a ";" comment under [Script Info].
	void AddLine(wxString data,wxString group,int &version,wxString *outGroup=NULL);

	static void StackPop();					// Pop subs from stack and sets 'top' to it
	static void StackRedo();				// Redoes action on stack
	static void StackPush(wxString desc);	// Puts a copy of 'top' on the stack
	static void StackReset();				// Resets stack. Do this before loading new subtitles.
	static bool IsUndoStackEmpty();			// Checks if undo stack is empty
	static bool IsRedoStackEmpty();			// Checks if undo stack is empty
	static wxString GetUndoDescription();	// Gets field undodescription from back of UndoStack
	static wxString GetRedoDescription();	// Gets field undodescription from back of RedoStack

	/// DOCME
	static bool Popping;					// Flags the stack as popping. You must unset this after popping

	/// DOCME
	static AssFile *top;					// Current script file. It is "above" the stack.

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

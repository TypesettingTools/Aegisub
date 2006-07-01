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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


#pragma once


///////////
// Headers
#include <wx/wxprec.h>
#include <fstream>
#include <list>


//////////////
// Prototypes
class FrameRate;
class AssDialogue;
class AssStyle;
class AssDialogueBlock;
class AssDialogueBlockOverride;
class AssDialogueBlockPlain;
class AssEntry;


//////////////////////////////////////
// Class to store the actual ass file
class AssFile {
private:
	bool Modified;

	// Stack operations
	static std::list<AssFile*> UndoStack;
	static std::list<AssFile*> RedoStack;
	static bool StackModified;
	static void StackClear();

public:
	std::list<AssEntry*> Line;

	wxString filename;
	bool loaded;

	AssFile();
	AssFile(AssFile &from);
	~AssFile();

	bool IsModified();									// Returns if file has unmodified changes
	void FlagAsModified();								// Flag file as being modified, will automatically put a copy on stack
	void Clear();										// Wipes file
	void CompressForStack(bool compress);				// Compress/decompress for storage on stack
	void LoadDefault(bool noline=true);					// Loads default file. Pass true to prevent it from adding a default line too
	void InsertStyle(AssStyle *style);					// Inserts a style to file
	wxArrayString GetStyles();							// Gets a list of all styles available
	AssStyle *GetStyle(wxString name);					// Gets style by its name

	wxString GetString();
	void Load(wxString file,wxString charset=_T(""));	// Load from a file
	void Save(wxString file,bool setfilename=false,bool addToRecent=true,const wxString encoding=_T(""));	// Save to a file. Pass true to second argument if this isn't a copy
	void Export(wxString file);							// Saves exported copy, with effects applied
	void AddToRecent(wxString file);					// Adds file name to list of recently opened files
	bool CanSave();										// Return true if the file can be saved in its current format

	int GetScriptInfoAsInt(const wxString key);
	wxString GetScriptInfo(const wxString key);						// Returns the value in a [Script Info] key.
	void SetScriptInfo(const wxString key,const wxString value);	// Sets the value of a [Script Info] key. Adds it if it doesn't exist.
	void AddComment(const wxString comment);						// Adds a ";" comment under [Script Info].
	int AddLine(wxString data,wxString group,int lasttime,bool &IsSSA,wxString *outGroup=NULL);

	static void StackPop();		// Pop subs from stack and sets 'top' to it
	static void StackRedo();	// Redoes action on stack
	static void StackPush();	// Puts a copy of 'top' on the stack
	static void StackReset();	// Resets stack. Do this before loading new subtitles.
	static bool IsUndoStackEmpty();	// Checks if undo stack is empty
	static bool IsRedoStackEmpty();	// Checks if undo stack is empty
	static bool Popping;		// Flags the stack as popping. You must unset this after popping
	static AssFile *top;		// Current script file. It is "above" the stack.
};


////////////
// Typedefs
typedef std::list<AssEntry*>::iterator entryIter;


//////////////////////////////////////////////////////
// Hack to get STL sort to work on a list of pointers
template <typename T>
class LessByPointedToValue : std::binary_function<T const *, T const *, bool> {
public:
	bool operator()(T const * x, T const * y) const {
		return std::less<T>()(*x, *y);
	}
};

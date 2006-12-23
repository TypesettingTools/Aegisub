// Copyright (c) 2005, Ghassan Nassar
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


#include "setup.h"
#if USE_ASPELL == 1

#pragma once
#include <wx/wxprec.h>

extern "C" {
#include <aspell.h>
}
#include "ass_dialogue.h"
#include "ass_file.h"
#include "subs_grid.h"

// Z:\SVN\Aspell\lib\libaspell-15-dll.lib 

#ifndef DIALOG_SPELL_CHECK_H
#define DIALOG_SPELL_CHECK_H

class wxArrayInt;

class DialogSpellCheck : public wxDialog {

public:
	DialogSpellCheck (wxWindow *parent, AssFile *_subs, wxArrayInt* selList, SubtitlesGrid *_grid);
	~DialogSpellCheck(void);
private:

	////////////
	// Privates 
	


	/////
	//UI
	wxSizer *MainSizer;
	wxTextCtrl *Textbox;
	wxListBox *SuggestionsList;
	wxButton *AddtoCustomDic; //
	wxButton *AcceptSuggestion; //
	wxButton *SkipError; //
	wxButton *Exit; // 
	wxButton *Options; //

	SubtitlesGrid *grid;
	wxArrayInt lnList;

	// Dealing with Subs
	AssFile *subs;
	AssDialogue * current_line; //depreciated (sorta?)
	
	// Aspell:
	AspellToken token;
	AspellConfig * aspellConfig;
	AspellSpeller* aspellSpeller;
    AspellDocumentChecker* aspellChecker;
	bool pDictionaryChanged;
	wxArrayString lIgnore;
	wxArrayString sList;
	wxString strReplacementText;
	wxString misspelled_word;

	//Internals
	int start;
	int end;
	int nDiff;
	int curLineNumber;
	size_t curBlockNumber;
	bool nLine;
	wxString current_block;
	
	
	//
	//Functions
	//
	void Stage0();
	void Stage1();
	void Stage2();
	void Stage3();

	bool IncLines();						/* Incremenet lines */
	void LineSetUp();						/* First Run Operation Deals With User Selection */
    bool LineComputation();					/* Returns Block Text To Edit */
	bool SetDefaultOptions();				 /* Sets MY Aspell Default Options. TODO Make user able to set optinos */
	bool SetOptions(wxString Option,wxString Value);	 /* Sets Options Generic Function */
	void InitSpellCheck();					/* First Run Operation Deals With Variable Set up */
	bool Uninit();							/*Future Usage */
	void LineEnd();							/*Run At End Of Lines, Cleans up.  */
	bool LineBegin();						/*Run At Line Begining. Sets up variables  */
	bool Precheck();						/*Not Really Pre anymore, it does the work.  */
	void Populate(wxString misspelling);	/* Populate Gui. */
	void UILock(bool enable);				/* Lock Interface  */
	void ComputeAction(int action);			/* Send Enum, Do Work. Does most of the On* calls.  */
	int  AddWordToDictionary(wxString strWord);		 /*Adds word to dictionary */
	wxArrayString GetSuggestions(wxString strMisspelledWord);	/*Gets suggestions for misspelled word  */
	wxArrayString GetWordListAsArray();		/*Same as above... legacy code marked to be removed.   */
	void BlockStore();						/* Save Changes.  */
	

	/////
	//Events 
	void OnClose (wxCommandEvent &event);
	void OnAccept (wxCommandEvent &event);
	void OnOptions (wxCommandEvent &event);
	void OnExit (wxCommandEvent &event);
	void OnSkip (wxCommandEvent &event);
	void OnAdd (wxCommandEvent &event);
	void OnListDoubleClick (wxCommandEvent &event);
	void OnTextChange(wxCommandEvent &event);
	
	DECLARE_EVENT_TABLE()


	enum actioncodes {
		ACTION_ACCEPT = 1,
		ACTION_SKIP,
		ACTION_ADD,
		ACTION_EXIT,
		ACTION_USER_CUSTOM,

	};



};

///////
// IDs
enum {
	BUTTON_EXIT = 100,
	BUTTON_ACCEPT,
	BUTTON_SKIP,
	BUTTON_ADDCUSTOM,
	BUTTON_PREFERENCES,
	LIST_SUGGESTIONS,
	TEXTBOX_LINE
};

#endif

#endif

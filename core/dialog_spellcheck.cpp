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
#include "aspell_wrap.h"
#include "main.h"
#include "dialog_spellcheck.h"
#include "aspell_wrap.h"
#include <wx\file.h>
#include <wx\filename.h>
#include <wx\dir.h>


DialogSpellCheck::DialogSpellCheck (wxWindow *parent, AssFile *_subs, wxArrayInt* selList, SubtitlesGrid *_grid)
		: wxDialog (parent,-1,_T("Document Spell Checker"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE,_T("DialogSpellCheck"))
{
	// Suggestions
	wxSizer *SuggestionBox = new wxStaticBoxSizer(wxVERTICAL,this,_T("ErrorBox"));
	Textbox = new wxTextCtrl(this, TEXTBOX_LINE, _T(""), wxDefaultPosition, wxSize(185,55), wxTE_RICH2 | wxTE_WORDWRAP, wxDefaultValidator, _T("Displayed Line"));
	SuggestionsList = new wxListBox(this, LIST_SUGGESTIONS, wxDefaultPosition, wxSize(185,95), 0, NULL, wxLB_SINGLE | wxLB_SORT, wxDefaultValidator, _T("Suggestions"));
	SuggestionBox->Add(Textbox,0,wxRIGHT,5);
	SuggestionBox->AddSpacer(5);
	SuggestionBox->Add(SuggestionsList,1,wxEXPAND | wxRIGHT | wxALIGN_RIGHT,5);

	//// RButtons styles list
	wxSizer *RButtons = new wxStaticBoxSizer(wxVERTICAL, this, _T("Extra Commands"));
	SkipError = new wxButton(this, BUTTON_SKIP, _T("Skip this error"), wxDefaultPosition, wxSize(80,25), 0, wxDefaultValidator, _T("Skip This Error"));
	AddtoCustomDic = new wxButton(this, BUTTON_ADDCUSTOM, _T("Add to Dict."), wxDefaultPosition, wxSize(80,25), 0, wxDefaultValidator, _T("Add To Custom Dictionary"));
	Options = new wxButton(this, BUTTON_PREFERENCES, _T("Options"), wxDefaultPosition, wxSize(80,25), 0, wxDefaultValidator, _T("Preferences"));
	
	RButtons->AddSpacer(20);
	RButtons->Add(SkipError,0, wxALL | wxALIGN_CENTER_VERTICAL,0);
	RButtons->AddSpacer(20);
	RButtons->Add(AddtoCustomDic,0, wxALL | wxALIGN_CENTER_VERTICAL,0);
	RButtons->AddSpacer(20);
	RButtons->Add(Options,0, wxALL | wxALIGN_CENTER_VERTICAL,0);
	RButtons->AddSpacer(20);
	
	////Lower Buttons
	wxSizer *LButtons = new wxBoxSizer(wxHORIZONTAL);
	AcceptSuggestion = new wxButton(this, BUTTON_ACCEPT, _T("Accept Suggestion"), wxDefaultPosition, wxSize(100,20), 0, wxDefaultValidator, _T("Accept"));
	Exit = new wxButton(this, BUTTON_EXIT, _T("Exit"), wxDefaultPosition, wxSize(100,20), 0, wxDefaultValidator, _T("Exit"));
	
	LButtons->AddSpacer(30);
	LButtons->Add(AcceptSuggestion,0, wxALL | wxALIGN_CENTER_HORIZONTAL ,0);
	LButtons->AddSpacer(30);
	LButtons->Add(Exit,0, wxALL | wxALIGN_CENTER_HORIZONTAL,0);
	LButtons->AddSpacer(30);

	//// TOP General layout
	wxSizer *MBox = new wxBoxSizer(wxHORIZONTAL);
	MBox->Add(SuggestionBox,0,wxRIGHT,5);
	MBox->Add(RButtons,0,wxLEFT,5);

	//wxButton *CloseButton = new wxButton(this, wxID_CLOSE, _T(""), wxDefaultPosition, wxSize(100,25), 0, wxDefaultValidator, _T("Close"));
	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(MBox ,0,wxEXPAND | wxLEFT | wxRIGHT | wxTOP,5);
	LButtons->AddSpacer(5);
	MainSizer->Add(LButtons,0,wxEXPAND | wxLEFT | wxRIGHT |wxTOP ,5);
	//MainSizer->Add(CloseButton,0,wxBOTTOM | wxALIGN_CENTER,5);

	//// Set sizer
	SetSizer(MainSizer);
	MainSizer->SetSizeHints(this);

	subs = _subs;
	grid = _grid;
	lnList = *selList;

	aspellConfig = NULL;
    aspellSpeller = NULL;
    aspellChecker = NULL;
	
	InitSpellCheck();
	

}


DialogSpellCheck::~DialogSpellCheck(void)
{
  if (pDictionaryChanged)	{
	  if (wxYES == ::wxMessageBox(_T("Would you like to save any of your changes to your personal dictionary?"), _T("Save Changes"), wxYES_NO | wxICON_QUESTION))
		aspell_speller_save_all_word_lists(aspellSpeller);
	}

  if (aspellChecker != NULL)
    delete_aspell_document_checker(aspellChecker);

  if (aspellConfig != NULL)
    delete_aspell_config(aspellConfig);

  if (aspellSpeller != NULL)
    delete_aspell_speller(aspellSpeller);

  Aspell.Unload();

}



bool DialogSpellCheck::SetOptions(wxString Option,wxString Value){

	if (aspellConfig == NULL)
		aspellConfig =  new_aspell_config();

	if (aspellConfig == NULL)
		return FALSE;

	aspell_config_replace(aspellConfig, Option.mb_str(wxConvUTF8), Value.mb_str(wxConvUTF8));
	return true;
}


bool DialogSpellCheck::SetDefaultOptions(){

	wxString defaultDataDir = AegisubApp::folderName;
	defaultDataDir.Append(_T("data\\"));
	wxString defaultDictDir = AegisubApp::folderName;
	defaultDictDir.Append(_T("dict\\"));

  if (wxDir::Exists(defaultDataDir)) //&& wxDir::Exists(defaultDictDir))
  {
    
	SetOptions(wxString(_T("encoding")), _T("utf-8"));
    SetOptions(wxString(_T("lang")), wxString(_T("en-US")));   //1* 
    SetOptions(wxString(_T("data-dir")), defaultDataDir);
    SetOptions(wxString(_T("dict-dir")), defaultDataDir); //defaultDictDir
	SetOptions(wxString(_T("local-data-dir")), defaultDataDir);
  }

	wxString l2Checka = defaultDataDir;
	l2Checka.Append( _T("\\en.dat"));	
	wxString l2Checkb = defaultDataDir;  // Set to the same place for now. It's regularily in two folders. 
	l2Checkb.Append(_T("\\en_phonet.dat"));

  if (!wxFile::Exists(l2Checka) || !wxFile::Exists(l2Checkb))
  {
 	  wxMessageBox(_T("The Files: \"en.dat\" and \"en_phonet.dat\", in \".\\data\" are necessary for proper execution. \nProcess will continue, but with **unpredicable** execution. \nFix: Reinstall Dictionaries."), _T("Warning"),wxICON_EXCLAMATION | wxOK | wxSTAY_ON_TOP); 
	  // 2
  
  }
  
  if (aspell_config_error(aspellConfig) != 0) {                         
  	wxString A(aspell_config_error_message(aspellConfig),wxConvUTF8);
    wxMessageBox(wxString::Format(_T("Error: %s\n"), A) ,  _T("Warning"),wxICON_ERROR | wxOK | wxSTAY_ON_TOP);
    return false;                                                       
  }

  return true;
}



bool DialogSpellCheck::Uninit(){
  Aspell.Unload(); // Here for when I get it dynamically linked. 
  return true;  // this is useless for now, but not in the future. 
}

void DialogSpellCheck::UILock(bool enable){
	if (enable)
	{
		SkipError->Enable(false);
		AcceptSuggestion->Enable(false);
		AddtoCustomDic->Enable(false);
		SuggestionsList->Enable(false);
		Textbox->Enable(false);
	}
	else{
		SkipError->Enable(true);
		AcceptSuggestion->Enable(true);
		AddtoCustomDic->Enable(true);
		SuggestionsList->Enable(true);
		Textbox->Enable(true);
	}
}

void DialogSpellCheck::LineSetUp(){ 
		
	bool alt = false;
    if (lnList.GetCount() > 0)
		if (lnList.Item(0) == -1)
				alt = true;
	
	if ((lnList.GetCount() == 0) || (alt == true)) {
			start = 0;
			end = grid->GetRows() - 1;
			curLineNumber = 0;
			current_line = grid->GetDialogue(curLineNumber);
			return;
	}
	else {
		curLineNumber = lnList.Item(0);
		lnList.RemoveAt(0,1);
		current_line = grid->GetDialogue(curLineNumber);
		start = -1;
		end = -1;
		return;
	}
	// something went drastically wrong... 
	return;
		
}


bool DialogSpellCheck::IncLines(){ // Used for lines 1+, line 0 is handled by LineSetUp
//if (a) return false; // used as an aux check for end of line.  
	
	if ((start == -1) && (end == -1)){ 
		if (lnList.GetCount() > 0) {
		curLineNumber = lnList.Item(0);
		lnList.RemoveAt(0,1);	
		current_line = grid->GetDialogue(curLineNumber);
		return true;
		}
		else 
			return false;
	}
	else if (start == 0){
		if (curLineNumber < end){
			curLineNumber++;
			current_line = grid->GetDialogue(curLineNumber);
			return true;
		}
		else
			return false;
	}
	return false; // shouldnt every really happen but vs stops yelling about it.  
}


void DialogSpellCheck::Stage0(){ // Stage 0, used intially.  
	LineSetUp();
	if (!LineComputation())
		return;
	LineBegin();
	Stage2();
}


void DialogSpellCheck::Stage1(){ // Stage 1, used after stage 0 is initiated.  
	if (!LineComputation()){
		if (!IncLines()){
			UILock(true);//init lines.
			return;
		}
	}
	LineBegin();
	Stage2();
}

void DialogSpellCheck::Stage2(){ // Stage 2, rather useless to be honest but, it makes things clear. 
		Precheck();  // Repeat here till line is checked. 
}

void DialogSpellCheck::Stage3(){ // Stage 3: Kill things that need to be killed find next line and continue. 
		LineEnd();
		Stage1();
}



bool DialogSpellCheck::Precheck()
{
  /* Using Document Checker and Settings previously set, find errors in the line. Also check if the word was told to be ignored.*/
  if ((token = aspell_document_checker_next_misspelling(aspellChecker), token.len != 0)){
		token.offset += nDiff;
		const wxString misspelling = current_block.Mid(token.offset, token.len);
		misspelled_word = misspelling;
		if (lIgnore.Index(misspelling) != wxNOT_FOUND) // implement button to actually DO this. 
			return true;
		Populate(misspelling);
		UILock(false);
  }
  else {
	  Stage3();
	  return false;
  }
  return true;
}

void DialogSpellCheck::Populate(wxString misspelling){
	Textbox->Clear();
	Textbox->AppendText(current_block);
	int c_start = current_block.Find(misspelling);
	Textbox->SetStyle(c_start, c_start + misspelling.Length(), wxTextAttr(*wxRED, wxNullColour, wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, _T(""), wxFONTENCODING_DEFAULT) ));

	SuggestionsList->Clear();
	SuggestionsList->InsertItems(GetSuggestions(misspelling),0);
}

void DialogSpellCheck::LineEnd(){
  nLine = false;
  nDiff = 0;
  token.len = 0;
  token.offset = 0;
  delete_aspell_document_checker(aspellChecker);
  aspellChecker = NULL;
  UILock(true);
}

bool DialogSpellCheck::LineBegin(){
  if (aspellSpeller == NULL)
    return false;
  AspellCanHaveError* ret = new_aspell_document_checker(aspellSpeller);
  if (aspell_error(ret) != 0) {
    wxString A(aspell_error_message(ret),wxConvUTF8);
	wxMessageBox(wxString::Format(_T("Error: %s\n"), A));
    return false;
  }
  aspellChecker = to_aspell_document_checker(ret);
  aspell_document_checker_process(aspellChecker , current_block.mb_str(wxConvUTF8), -1);
  return true;
}


wxArrayString DialogSpellCheck::GetSuggestions(wxString strMisspelledWord)
{
  const char * suggest;
  const wxString * misspelling = &strMisspelledWord;
  wxArrayString wxStringList;
  const AspellWordList * suggestions = aspell_speller_suggest(aspellSpeller, misspelling->mb_str(wxConvUTF8), misspelling->Length());
  AspellStringEnumeration * elements = aspell_word_list_elements(suggestions);
  wxStringList.IsEmpty();
  while ( (suggest = aspell_string_enumeration_next(elements)) != NULL ){
	wxString StringADD(suggest,wxConvUTF8);   
    wxStringList.Add(StringADD);
  }
  delete_aspell_string_enumeration(elements);
  return wxStringList;
}




wxArrayString DialogSpellCheck::GetWordListAsArray(){
  const char * suggested;
  wxArrayString wxStringList;
  const AspellWordList* PersonalWordList = aspell_speller_personal_word_list(aspellSpeller);
  AspellStringEnumeration* elements = aspell_word_list_elements(PersonalWordList);
  wxStringList.IsEmpty();
  while ( (suggested = aspell_string_enumeration_next(elements)) != NULL ){
	wxString StringADD(suggested,wxConvUTF8);   
    wxStringList.Add(StringADD);
  }
  delete_aspell_string_enumeration(elements); 
  return wxStringList;
}

int DialogSpellCheck::AddWordToDictionary(wxString strWord){
	const wxString * correctterm = &strWord;
	aspell_speller_add_to_personal(aspellSpeller, correctterm->mb_str(wxConvUTF8), strWord.Length());
	pDictionaryChanged = TRUE;
  return TRUE;
}


void DialogSpellCheck::InitSpellCheck(){
	
	nLine = false;
	curLineNumber = 0;
	curBlockNumber = -1;
	start = -1;
	end = -1;
    pDictionaryChanged = false;	
	token.len = 0;
	token.offset = 0;
	nDiff = 0;
	
	UILock(false);
	
	Aspell.Load();

	if (aspellConfig == NULL)
		aspellConfig = new_aspell_config();
	
	if (aspellConfig == NULL){
		wxMessageBox(_T("Critical Error! Config Settings Are Not Present."));
		UILock(true); // Forces the user to exit.
		return;
	
	}
	SetDefaultOptions();

  AspellCanHaveError* ret = new_aspell_speller(aspellConfig);
  if (aspell_error(ret) != 0){
		wxString A(aspell_error_message(ret),wxConvUTF8);
		wxMessageBox(wxString::Format(_T("Error: %s\n"), A));
		delete_aspell_can_have_error(ret);
		UILock(true); // Forces the user to exit.
		return;
    }
  aspellSpeller = to_aspell_speller(ret);
  Stage0();
}

 


BEGIN_EVENT_TABLE(DialogSpellCheck, wxEvtHandler)
	EVT_BUTTON(wxID_CLOSE, DialogSpellCheck::OnClose)
	EVT_BUTTON(BUTTON_ACCEPT, DialogSpellCheck::OnAccept)
	EVT_BUTTON(BUTTON_EXIT, DialogSpellCheck::OnExit)
	EVT_BUTTON(BUTTON_PREFERENCES, DialogSpellCheck::OnOptions)
	EVT_BUTTON(BUTTON_SKIP, DialogSpellCheck::OnSkip)
	EVT_BUTTON(BUTTON_ADDCUSTOM, DialogSpellCheck::OnAdd)
	EVT_LISTBOX_DCLICK(LIST_SUGGESTIONS, DialogSpellCheck::OnListDoubleClick)
	EVT_TEXT(TEXTBOX_LINE, DialogSpellCheck::OnTextChange)
END_EVENT_TABLE()


/////////
//EVENTS

void DialogSpellCheck::OnClose (wxCommandEvent &event) {
		if ((Textbox->GetValue() != current_block) && (Textbox->GetValue() != _T(""))) {
				wxMessageDialog Question(this, _T(
				"You've selected:Exit. Yet, the current line has not yet been saved. Would you like to save the changes?"),
				_T("Exit Warning"),
				wxICON_QUESTION | wxSTAY_ON_TOP | wxYES_NO | wxYES_DEFAULT,
				wxDefaultPosition);
				
				int a = Question.ShowModal();
				if (a == wxID_YES){
					BlockStore();
				}
		}
		Destroy();
}

void DialogSpellCheck::OnAccept (wxCommandEvent &event) {
	if (AcceptSuggestion->GetLabel() != _T("Replace w\\ Edit?"))
		ComputeAction(ACTION_ACCEPT);
	else
		ComputeAction(ACTION_USER_CUSTOM);
}

void DialogSpellCheck::OnExit (wxCommandEvent &event) {
		if ((Textbox->GetValue() != current_block) && (Textbox->GetValue() != _T(""))) {
				wxMessageDialog Question(this, _T(
				"You've selected:Exit. Yet, the current line has not yet been saved. Would you like to save the changes?"),
				_T("Exit Warning"),
				wxICON_QUESTION | wxSTAY_ON_TOP | wxYES_NO | wxYES_DEFAULT,
				wxDefaultPosition);
				
				int a = Question.ShowModal();
				if (a == wxID_YES){
					BlockStore();
				}
		}
		Destroy();    
}

void DialogSpellCheck::OnOptions (wxCommandEvent &event) {
	//TODO
}

void DialogSpellCheck::OnSkip (wxCommandEvent &event) {
	ComputeAction(ACTION_SKIP);
}

void DialogSpellCheck::OnAdd (wxCommandEvent &event) {
	ComputeAction(ACTION_ADD);		
}

void DialogSpellCheck::OnListDoubleClick (wxCommandEvent &event) {
	ComputeAction(ACTION_ACCEPT);
}

void DialogSpellCheck::OnTextChange (wxCommandEvent &event) {
	if (Textbox->GetValue() != current_block)   
		AcceptSuggestion->SetLabel(_T("Replace w\\ Edit?"));
	else
		AcceptSuggestion->SetLabel(_T("Accept Suggestion"));
}


void DialogSpellCheck::ComputeAction(int action) {
	const wxString correct_word = (SuggestionsList->GetSelection() == wxNOT_FOUND) ? _T("") : SuggestionsList->GetString(SuggestionsList->GetSelection());

	if ((SuggestionsList->GetSelection() == wxNOT_FOUND) && (action == ACTION_ACCEPT))
		return;
	const wxString misspelling = misspelled_word;

	switch (action)
	{
	case ACTION_ACCEPT:
		nDiff += correct_word.Length() - token.len; // Let the spell checker know what the correct replacement was
		aspell_speller_store_replacement(aspellSpeller, misspelling.mb_str(wxConvUTF8), token.len, correct_word.mb_str(wxConvUTF8), correct_word.Length());
		current_block.replace(token.offset, token.len, correct_word);// Replace the misspelled word with the replacement */
		BlockStore();
		Textbox->Clear();
		Textbox->AppendText(current_block);
		UILock(true);
		break;
	case ACTION_SKIP:
		break;
	case ACTION_ADD:
		AddWordToDictionary(misspelled_word);
		break;
	case ACTION_USER_CUSTOM:
		current_block = Textbox->GetValue();
		BlockStore();
		Textbox->Clear();
		Textbox->AppendText(current_block);
		curBlockNumber--;
		token.len = 0;
		token.offset = 0;
	default:
		break;
	}
	Stage2();
}

bool DialogSpellCheck::LineComputation() {
	AssDialogueBlockPlain *curPlain;
	current_line->ParseASSTags();
	size_t size_blocks = current_line->Blocks.size();
	for (size_t i=(curBlockNumber+1);i<size_blocks;i++) {
		curPlain = AssDialogueBlock::GetAsPlain(current_line->Blocks.at(i));
		if (curPlain) {
			current_block = curPlain->GetText();
			curBlockNumber = i;
			return true;
		}
	}
	curPlain = 0;
	curBlockNumber = -1; 
	current_line->ClearBlocks();
	return false;
}

void DialogSpellCheck::BlockStore() {
	current_line->ParseASSTags();
	AssDialogueBlockPlain *curPlain = AssDialogueBlock::GetAsPlain(current_line->Blocks.at(curBlockNumber));
		if (curPlain) {
			curPlain->text = current_block;
			current_line->UpdateText();
			current_line->UpdateData();
			subs->FlagAsModified();
			grid->CommitChanges();
		}
	curPlain = 0;
	current_line->ClearBlocks();
}

#endif

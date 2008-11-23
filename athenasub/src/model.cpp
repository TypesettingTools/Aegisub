// Copyright (c) 2008, Rodrigo Braz Monteiro
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
// AEGISUB/ATHENASUB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include "athenasub.h"
#include "model.h"
#include "controller.h"
using namespace Athenasub;


///////////////
// Constructor
CModel::CModel()
: undoLimit(20), readOnly(false)
{
}


/////////////////////////////////////////////////////////
// Adds a listener to be notified whenever things change
void CModel::AddListener(View listener)
{
	wxASSERT(listener);
	listeners.push_back(listener);
}


////////////////////////
// Notify all listeners
void CModel::DispatchNotifications(Notification notification) const
{
	for (ViewList::const_iterator cur=listeners.begin();cur!=listeners.end();cur++) {
		//(*cur)->Notify(notification);
	}
}


////////////////////////////
// Processes an action list
// type == 0 : direct action
// type == 1 : undo
// type == 2 : redo
void CModel::ProcessActionList(CActionList &_actionList,int type)
{
	// Copy the list
	//shared_ptr<CActionList> actions = shared_ptr<CActionList>(new CActionList(_actionList));
	shared_ptr<CActionList> actions = shared_ptr<CActionList>(new CActionList(_actionList));
	bool canUndo = actions->CanUndo();

	// Setup undo
	shared_ptr<CActionList> undo = shared_ptr<CActionList>(new CActionList(actions->GetModel(),actions->GetName(),actions->GetOwner(),canUndo));
	ActionStack *stack;
	if (type == 1) stack = &redoStack;
	else stack = &undoStack;

	// Execute actions
	std::list<Action>::const_iterator cur;
	std::list<Action> acts = actions->GetActions();
	for (cur=acts.begin();cur!=acts.end();cur++) {
		// Inserts the opposite into the undo action first
		if (canUndo) undo->AddActionStart((*cur)->GetAntiAction());
		
		// Execute the action itself
		(*cur)->Execute();
	}

	// Insert into undo stack
	if (canUndo) {
		stack->push_back(undo);
		if (stack->size() > undoLimit) stack->pop_front();
		if (type == 0) redoStack.clear();
	}

	// Notify listeners
	DispatchNotifications(Notification());
}


//////////////////
// Load subtitles
void CModel::Load(Reader &input,const Format _format)
{
	// Autodetect format
	if (!_format) {
		// TODO

		// No format found
		THROW_ATHENA_EXCEPTION(Exception::No_Format_Handler);
	}

	// Get handler
	FormatHandler handler = _format->GetHandler();
	if (!handler) THROW_ATHENA_EXCEPTION(Exception::No_Format_Handler);

	// Clear the model first
	Clear();

	// Load
	handler->Load(*this,input);

	// Set the format
	format = _format;
}


//////////////////
// Save subtitles
void CModel::Save(Writer output,const Format _format) const
{
	// Use another format
	if (_format && _format != format) {
		// TODO
		THROW_ATHENA_EXCEPTION(Exception::TODO);
	}

	// Get handler
	FormatHandler handler = format->GetHandler();
	if (!handler) THROW_ATHENA_EXCEPTION(Exception::No_Format_Handler);

	// Load
	handler->Save(*this,output);
}


/////////////////////////
// Inserts a new section
Section CModel::AddSection(String name)
{
	ConstSection prev = GetSection(name);
	if (prev) THROW_ATHENA_EXCEPTION(Exception::Section_Already_Exists);

	Section result = Section(new CSection(name));
	sections.push_back(result);
	return result;
}


//////////////////
// Gets a section
ConstSection CModel::GetSection(String name) const
{
	size_t len = sections.size();
	for (size_t i=0;i<len;i++) {
		if (sections[i]->GetName() == name) return sections[i];
	}
	return SectionPtr();
}

Section CModel::GetMutableSection(String name)
{
	size_t len = sections.size();
	for (size_t i=0;i<len;i++) {
		if (sections[i]->GetName() == name) return sections[i];
	}
	return SectionPtr();
}


////////////////////////
// Get section by index
ConstSection CModel::GetSectionByIndex(size_t index) const
{
	return sections.at(index);
}

Section CModel::GetMutableSectionByIndex(size_t index)
{
	return sections.at(index);
}


/////////////////////
// Get section count
size_t CModel::GetSectionCount() const
{
	return sections.size();
}


//////////
// Clear
void CModel::Clear()
{
	sections.clear();
	undoStack.clear();
	redoStack.clear();
}


//////////////////
// Can undo/redo?
bool CModel::CanUndo(const String owner) const
{
	(void) owner;
	return undoStack.size() > 0;
}
bool CModel::CanRedo(const String owner) const
{
	(void) owner;
	return redoStack.size() > 0;
}


///////////////////
// Perform an undo
void CModel::Undo(const String owner)
{
	ActivateStack(undoStack,true,owner);
}


//////////////////
// Perform a redo
void CModel::Redo(const String owner)
{
	ActivateStack(redoStack,false,owner);
}


/////////////////////
// Perform undo/redo
void CModel::ActivateStack(ActionStack stack,bool isUndo,const String &owner)
{
	// TODO: do something with this
	(void) owner;

	// Process list
	//ProcessActionList(stack.back(),isUndo?1:2);
	ProcessActionList(*static_pointer_cast<CActionList>(stack.back()),isUndo?1:2);

	// Pop original
	stack.pop_back();
}


//////////////////////////
// Get undo/redo messages
String CModel::GetUndoMessage(const String owner) const
{
	(void) owner;
	if (CanUndo()) return undoStack.back()->GetName();
	return "";
}
String CModel::GetRedoMessage(const String owner) const
{
	(void) owner;
	if (CanRedo()) return redoStack.back()->GetName();
	return "";
}


/////////////////////
// Create controller
Controller CModel::CreateController() {
	return Controller(new CController(Model(weakThis)));
}


//////////////////////////////////////////
// Sets the maximum number of undo levels
void CModel::SetUndoLimit(size_t levels)
{
	undoLimit = levels;
	while (undoStack.size() > undoLimit) {
		undoStack.pop_front();
	}
}

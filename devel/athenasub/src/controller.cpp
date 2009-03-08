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

#include "controller.h"
#include "athenasub.h"
#include "actionlist.h"
#include "format_manager.h"
#include "selection.h"
#include "reader.h"
#include "writer.h"
using namespace Athenasub;


///////////////
// Constructor
CController::CController(Model _model)
: model(dynamic_pointer_cast<CModel>(_model))
{
}


/////////////////////////
// Create an action list
ActionList CController::CreateActionList(const String title,const String owner,bool undoAble)
{
	return ActionList (new CActionList(model,title,owner,undoAble));
}


///////////////
// Load a file
void CController::LoadFile(const String filename,const String encoding)
{
	Reader reader(new CReader(filename,encoding));
	std::vector<Format> handlers = FormatManager::GetCompatibleFormatList(reader);
	size_t len = handlers.size();
	bool success = false;
	for (size_t i=0;i<len;i++) {
		try {
			model->Load(reader,handlers[i]);
			success = true;
			break;
		} catch (Athenasub::Exception &e) {
			// Ignore exception
			(void) e;
		}
	}

	if (!success) {
		THROW_ATHENA_EXCEPTION_MSG(Exception::No_Format_Handler,"Could not locate a suitable format handler.");
	}
}


///////////////
// Save a file
void CController::SaveFile(const String filename,const String encoding)
{
	Format handler = FormatManager::GetFormatFromFilename(filename,true);
	Writer writer(new CWriter(filename,encoding));
	model->Save(writer,handler);
}


//////////////
// Get format
const Format CController::GetFormat() const
{
	return model->GetFormat();
}


//////////////////
// Create entries
Dialogue CController::CreateDialogue() const
{
	return GetFormat()->CreateDialogue();
}
Style CController::CreateStyle() const
{
	return GetFormat()->CreateStyle();
}


////////
// Undo
bool CController::CanUndo(const String owner) const
{
	return model->CanUndo(owner);
}
bool CController::CanRedo(const String owner) const
{
	return model->CanRedo(owner);
}
void CController::Undo(const String owner)
{
	if (CanUndo(owner)) {
		model->Undo(owner);
	}
}
void CController::Redo(const String owner)
{
	if (CanRedo(owner)) {
		model->Redo(owner);
	}
}


////////////////////////
// Get the nth dialogue
ConstDialogue CController::GetDialogue(size_t n) const
{
	// TODO
	(void) n;
	THROW_ATHENA_EXCEPTION(Exception::TODO);
}


/////////////////////
// Get the nth style
ConstStyle CController::GetStyle(size_t n) const
{
	// TODO
	(void) n;
	THROW_ATHENA_EXCEPTION(Exception::TODO);
}


///////////////////////
// Get a style by name
ConstStyle CController::GetStyle(String name) const
{
	// Get section
	Style dummy = CreateStyle();
	String section = dummy->GetDefaultGroup();
	ConstSection sect = model->GetSection(section);
	if (!sect) THROW_ATHENA_EXCEPTION(Exception::Invalid_Section);

	// Return from index
	return dynamic_pointer_cast<const IStyle> (sect->GetFromIndex(name));
}


////////////////
// Get an entry
ConstEntry CController::GetEntry(size_t n,String section) const
{
	ConstSection sect = model->GetSection(section);
	if (!sect) THROW_ATHENA_EXCEPTION(Exception::Invalid_Section);
	return sect->GetEntry(n);
}


//////////////////////
// Create a selection
Selection CController::CreateSelection() {
	return Selection(new CSelection());
}

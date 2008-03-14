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
// AEGISUB/GORGONSUB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include "controller.h"
#include "Gorgonsub.h"
using namespace Gorgonsub;


///////////////
// Constructor
Controller::Controller(Model &_model)
: model(_model)
{
}


/////////////////////////
// Create an action list
ActionListPtr Controller::CreateActionList(const String title,const String owner,bool undoAble)
{
	return ActionListPtr (new ActionList(model,title,owner,undoAble));
}


///////////////
// Load a file
void Controller::LoadFile(const String filename,const String encoding)
{
	const FormatPtr handler = FormatManager::GetFormatFromFilename(filename,true);
	wxFileInputStream stream(filename);
	model.Load(stream,handler,encoding);
}


///////////////
// Save a file
void Controller::SaveFile(const String filename,const String encoding)
{
	const FormatPtr handler = FormatManager::GetFormatFromFilename(filename,true);
	wxFileOutputStream stream(filename);
	model.Save(stream,handler,encoding);
}


//////////////
// Get format
const FormatPtr Controller::GetFormat() const
{
	return model.GetFormat();
}


//////////////////
// Create entries
SectionEntryDialoguePtr Controller::CreateDialogue()
{
	return GetFormat()->CreateDialogue();
}
SectionEntryStylePtr Controller::CreateStyle()
{
	return GetFormat()->CreateStyle();
}


////////
// Undo
bool Controller::CanUndo(const String owner) const
{
	return model.CanUndo(owner);
}
bool Controller::CanRedo(const String owner) const
{
	return model.CanRedo(owner);
}
void Controller::Undo(const String owner)
{
	model.Undo(owner);
}
void Controller::Redo(const String owner)
{
	model.Redo(owner);
}

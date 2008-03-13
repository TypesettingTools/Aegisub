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
// AEGISUB/AEGILIB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include "aegilib.h"
using namespace Aegilib;


/////////////////////////////////////////////////////////
// Adds a listener to be notified whenever things change
void Model::AddListener(ViewPtr listener)
{
	wxASSERT(listener);
	listeners.push_back(listener);
}


////////////////////////
// Notify all listeners
void Model::DispatchNotifications(const Notification &notification) const
{
	for (ViewList::const_iterator cur=listeners.begin();cur!=listeners.end();cur++) {
		(*cur)->Notify(notification);
	}
}


////////////////////////////
// Processes an action list
void Model::ProcessActionList(const Manipulator &actionList,bool insertInStack)
{
	// Inserts the opposite into the undo stack
	if (insertInStack) {
		undoStack.push_back(CreateAntiManipulator(actionList));
		redoStack.clear();
	}

	// Do action
	// TODO

	// Notify listeners
	DispatchNotifications(Notification());
}


////////////////////////////////////////////////////////////////////////
// Create an anti-manipulator to undo the actions made by a manipulator
Manipulator Model::CreateAntiManipulator(const Manipulator &src)
{
	Manipulator dst(*this,src.actionName);
	// TODO
	return dst;
}


//////////////////
// Load subtitles
void Model::Load(wxInputStream &input,const FormatPtr format,const String encoding)
{
	// Autodetect format
	if (format == NULL) {
		// TODO

		// No format found
		throw Exception(Exception::No_Format_Handler);
	}

	// Get handler
	FormatHandlerPtr handler = format->GetHandler(*this);
	if (!handler) throw Exception(Exception::No_Format_Handler);

	// Load
	handler->Load(input,encoding);
}


//////////////////
// Save subtitles
void Model::Save(wxOutputStream &output,const FormatPtr format,const String encoding)
{
	(void) output;
	(void) format;
	(void) encoding;
	// TODO
}


///////////////
// Load a file
void Model::LoadFile(const String filename,const String encoding)
{
	const FormatPtr handler = FormatManager::GetFormatFromFilename(filename,true);
	wxFileInputStream stream(filename);
	Load(stream,handler,encoding);
}


///////////////
// Save a file
void Model::SaveFile(const String filename,const String encoding)
{
	const FormatPtr handler = FormatManager::GetFormatFromFilename(filename,true);
	wxFileOutputStream stream(filename);
	Save(stream,handler,encoding);
}


//////////////////
// Gets a section
SectionPtr Model::GetSection(String name) const
{
	std::list<SectionPtr>::const_iterator cur;
	for (cur=sections.begin();cur!=sections.end();cur++) {
		if ((*cur)->GetName() == name) return *cur;
	}
	return SectionPtr();
}


/////////////////////////
// Inserts a new section
void Model::AddSection(String name)
{
	SectionPtr prev = GetSection(name);
	if (prev) throw Exception(Exception::Section_Already_Exists);
	sections.push_back(SectionPtr(new Section(name)));
}

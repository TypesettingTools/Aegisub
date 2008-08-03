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

#include "action.h"
#include "model.h"
using namespace Athenasub;


///////////////////////////// Insert line /////////////////////////////

///////////////
// Constructor
ActionInsert::ActionInsert(Entry data,int line,const String &sName)
: entry(data), lineNumber(line), section(sName) {}


/////////////////////////////////
// Create anti-action for insert
Action ActionInsert::GetAntiAction(ConstModel model) const
{
	(void) model;
	String sect = section;
	if (section.IsEmpty()) sect = entry->GetDefaultGroup();
	return Action(new ActionRemove(lineNumber,sect));
}


/////////////////////
// Execute insertion
void ActionInsert::Execute(Model model)
{
	// Find the section to insert it on
	String sectionName = section;
	if (sectionName.IsEmpty()) sectionName = entry->GetDefaultGroup();
	Section sect = GetSection(model,sectionName);

	// Insert the line
	sect->AddEntry(entry,lineNumber);
}



///////////////////////////// Remove line /////////////////////////////


///////////////
// Constructor
ActionRemove::ActionRemove(int line,const String &sName)
: lineNumber(line), section(sName) {}


/////////////////////////////////
// Create anti-action for remove
Action ActionRemove::GetAntiAction(ConstModel model) const
{
	SectionPtr sect = GetSection(model,section);
	Entry entry = sect->GetEntry(lineNumber);
	return Action(new ActionInsert(entry,lineNumber,section));
}


///////////////////
// Execute removal
void ActionRemove::Execute(Model model)
{
	// Find the section to remote it from
	String sect = section;
	if (sect.IsEmpty()) THROW_ATHENA_EXCEPTION(Exception::TODO); // TODO
	SectionPtr section = GetSection(model,sect);

	// Remove the line
	section->RemoveEntryByIndex(lineNumber);
}


///////////////////////////// Modify line /////////////////////////////

////////////////
// Constructors
ActionModify::ActionModify(Entry data,int line,const String &sName,bool _noTextFields)
: entry(data), lineNumber(line), section(sName), noTextFields(_noTextFields) {}

ActionModify::ActionModify(shared_ptr<void> _delta,int line,const String &sName)
: delta(_delta), lineNumber(line), section(sName), noTextFields(false) {}


/////////////////////////////////
// Create anti-action for insert
Action ActionModify::GetAntiAction(ConstModel model) const
{
	// Get section and original line
	Section sect = GetSection(model,section);
	Entry oldEntry = sect->GetEntry(lineNumber);

	// Try to get a delta
	DeltaCoder deltaCoder = oldEntry->GetDeltaCoder();
	if (deltaCoder) {
		VoidPtr _delta;
		if (entry) _delta = deltaCoder->EncodeDelta(entry,oldEntry,!noTextFields);
		else _delta = deltaCoder->EncodeReverseDelta(delta,oldEntry);
		return Action(new ActionModify(_delta,lineNumber,section));
	}

	// Store the whole original line
	else {
		return Action(new ActionModify(oldEntry,lineNumber,section,noTextFields));
	}
}


/////////////////////
// Execute insertion
void ActionModify::Execute(Model model)
{
	// Find the section to modify
	String sectionName = section;
	if (sectionName.IsEmpty()) sectionName = entry->GetDefaultGroup();
	Section sect = GetSection(model,sectionName);

	// Modify the line
	if (delta) {
		Entry ref = sect->GetEntry(lineNumber);
		ref->GetDeltaCoder()->ApplyDelta(delta,ref);
	}
	else sect->GetEntryRef(lineNumber) = entry;
}


////////////////////////// Batch Modify line //////////////////////////

ActionModifyBatch::ActionModifyBatch(std::vector<Entry> _entries, std::vector<shared_ptr<void> > _deltas, Selection _selection,const String &_section,bool _noTextFields)
: entries(_entries), deltas(_deltas), selection(_selection), section(_section), noTextFields(_noTextFields) {}

Action ActionModifyBatch::GetAntiAction(ConstModel model) const
{
	// Get section
	SectionPtr sect = GetSection(model,section);
	size_t len = selection->GetCount();
	std::vector<VoidPtr> _deltas(len);
	std::vector<Entry> oldEntries(len);

	// For each line...
	for (size_t i=0;i<len;i++) {
		// Get old entry
		Entry oldEntry = sect->GetEntry(selection->GetLine(i));

		// Try to get a delta
		DeltaCoder deltaCoder = oldEntry->GetDeltaCoder();
		if (deltaCoder) {
			if (i < deltas.size() && deltas[i]) _deltas[i] = deltaCoder->EncodeReverseDelta(deltas[i],oldEntry);
			_deltas[i] = deltaCoder->EncodeDelta(entries[i],oldEntry,!noTextFields);
		}

		// Store the whole original line
		else oldEntries[i] = oldEntry;
	}

	return Action(new ActionModifyBatch(oldEntries,_deltas,selection,section,noTextFields));
}

void ActionModifyBatch::Execute(Model model)
{
	// Find the section to modify
	size_t len = selection->GetCount();
	String sectionName = section;
	if (sectionName.IsEmpty()) sectionName = entries[0]->GetDefaultGroup();
	Section sect = GetSection(model,sectionName);

	// For each line...
	for (size_t i=0;i<len;i++) {
		if (i < deltas.size() && deltas[i]) {
			Entry &ref = sect->GetEntryRef(selection->GetLine(i));
			ref->GetDeltaCoder()->ApplyDelta(deltas[i],ref);
		}
		else sect->GetEntryRef(selection->GetLine(i)) = entries[i];
	}
}

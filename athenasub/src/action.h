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

#pragma once
#include "athenastring.h"
#include "selection.h"
#include "interfaces.h"

namespace Athenasub {

	// Action base class
	class CAction : public IAction {
	private:
		Model model;

	protected:
		CAction(Model model);

		Model GetModel() const;
		Section GetSection(String name) const;
	};

	// Insert line
	class ActionInsert : public CAction {
	private:
		Entry entry;
		const String section;
		int lineNumber;

	public:
		ActionInsert(Model model,Entry entry,int line,const String &section);

		Action GetAntiAction() const;
		void Execute();
	};

	// Remove line
	class ActionRemove : public CAction {
	private:
		const String section;
		int lineNumber;

	public:
		ActionRemove(Model model,int line,const String &section);

		Action GetAntiAction() const;
		void Execute();
	};

	// Modify line
	class ActionModify : public CAction {
	private:
		Entry entry;
		VoidPtr delta;
		const String section;
		int lineNumber;
		bool noTextFields;

	public:
		ActionModify(Model model,Entry entry,int line,const String &section,bool noTextFields);
		ActionModify(Model model,shared_ptr<void> delta,int line,const String &section);

		Action GetAntiAction() const;
		void Execute();
	};

	// Modify several lines
	class ActionModifyBatch : public CAction {
	private:
		std::vector<Entry> entries;
		std::vector<VoidPtr> deltas;
		Selection selection;
		const String section;
		bool noTextFields;

		ActionModifyBatch(Model model,Selection selection,const String &section,bool noTextFields);

	public:
		ActionModifyBatch(Model model,std::vector<Entry> entries,std::vector<VoidPtr> deltas,Selection selection,const String &section,bool noTextFields);

		Action GetAntiAction() const;
		void Execute();
	};
}

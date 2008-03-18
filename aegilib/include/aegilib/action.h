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

namespace Athenasub {
	// Prototypes
	class Model;
	class Entry;
	class Action;
	class Section;
	typedef shared_ptr<Action> ActionPtr;
	typedef shared_ptr<Section> SectionPtr;

	// Action interface
	class Action {
	protected:
		SectionPtr GetSection(const Model &model,const String &name) const;

	public:
		virtual ~Action() {}
		virtual ActionPtr GetAntiAction(const Model &model) const =0;
		virtual void Execute(Model &model) =0;
	};

	// Insert line
	class ActionInsert : public Action {
	private:
		shared_ptr<Entry> entry;
		const String section;
		int lineNumber;

	public:
		ActionInsert(shared_ptr<Entry> entry,int line,const String &section);
		~ActionInsert() {}

		ActionPtr GetAntiAction(const Model &model) const;
		void Execute(Model &model);
	};

	// Remove line
	class ActionRemove : public Action {
	private:
		const String section;
		int lineNumber;

	public:
		ActionRemove(int line,const String &section);
		~ActionRemove() {}

		ActionPtr GetAntiAction(const Model &model) const;
		void Execute(Model &model);
	};

	// Modify line
	class ActionModify : public Action {
	private:
		shared_ptr<Entry> entry;
		shared_ptr<void> delta;
		const String section;
		int lineNumber;
		bool noTextFields;

	public:
		ActionModify(shared_ptr<Entry> entry,int line,const String &section,bool noTextFields);
		ActionModify(shared_ptr<void> delta,int line,const String &section);
		~ActionModify() {}

		ActionPtr GetAntiAction(const Model &model) const;
		void Execute(Model &model);
	};

	// Modify several line
	class ActionModifyBatch : public Action {
	private:
		std::vector<shared_ptr<Entry> > entries;
		std::vector<shared_ptr<void> > deltas;
		Selection selection;
		const String section;
		bool noTextFields;

	public:
		ActionModifyBatch(std::vector<shared_ptr<Entry> > entries,std::vector<shared_ptr<void> > deltas,Selection selection,const String &section,bool noTextFields);
		~ActionModifyBatch() {}

		ActionPtr GetAntiAction(const Model &model) const;
		void Execute(Model &model);
	};
}

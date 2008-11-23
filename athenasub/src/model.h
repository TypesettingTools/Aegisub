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
#include <list>
#include <vector>
#include "athenasub.h"
#include "actionlist.h"
#include "section.h"
#include "api.h"

namespace Athenasub {

	// Model class
	// Stores the subtitle data
	class CModel : public IModel {
		friend class CFormatHandler;
		friend class CActionList;
		friend class CController;
		friend class CAction;
		friend class CLibAthenaSub;

	private:
		weak_ptr<IModel> weakThis;

		std::vector<Section> sections;
		ActionStack undoStack;
		ActionStack redoStack;
		ViewList listeners;
		Format format;
		bool readOnly;
		size_t undoLimit;

		void ProcessActionList(CActionList &actionList,int type=0);

		String GetUndoMessage(const String owner="") const;
		String GetRedoMessage(const String owner="") const;
		bool CanUndo(const String owner="") const;
		bool CanRedo(const String owner="") const;
		void Undo(const String owner="");
		void Redo(const String owner="");
		void ActivateStack(ActionStack stack,bool isUndo,const String &owner);

		void SetUndoLimit(size_t levels);
		size_t GetUndoLimit() const { return undoLimit; }

		void DispatchNotifications(Notification notification) const;

		void Clear();
		void Load(Reader &input,Format format=Format());

		Section AddSection(String name);
		Section GetMutableSection(String name);
		Section GetMutableSectionByIndex(size_t index);

		void SetWeakPtr(weak_ptr<IModel> ptr) { weakThis = ptr; }

	public:
		CModel();
		Controller CreateController();
		Format GetFormat() const { return format; }

		void AddListener(View listener);
		void Save(Writer output,Format format=Format()) const;

		ConstSection GetSection(String name) const;
		ConstSection GetSectionByIndex(size_t index) const;
		size_t GetSectionCount() const;
	};

	typedef shared_ptr<CModel> ModelPtr;

}

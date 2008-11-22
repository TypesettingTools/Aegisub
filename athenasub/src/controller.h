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
#include "athenasub.h"

namespace Athenasub {
	
	// Controller class
	class CController : public IController {
		friend class CModel;

	private:
		shared_ptr<CModel> model;
		CController (Model model);

	public:
		virtual ActionList CreateActionList(const String title,const String owner="",bool undoAble=true);
		virtual Selection CreateSelection();

		virtual void LoadFile(const String filename,const String encoding="");
		virtual void SaveFile(const String filename,const String encoding="UTF-8");

		virtual bool CanUndo(const String owner="") const;
		virtual bool CanRedo(const String owner="") const;
		virtual void Undo(const String owner="");
		virtual void Redo(const String owner="");

		virtual Dialogue CreateDialogue() const;
		virtual Style CreateStyle() const;

		virtual ConstDialogue GetDialogue(size_t n) const;
		virtual ConstStyle GetStyle(size_t n) const;
		virtual ConstStyle GetStyle(String name) const;
		virtual ConstEntry GetEntry(size_t n,String section) const;

		virtual const Format GetFormat() const;
	};

}

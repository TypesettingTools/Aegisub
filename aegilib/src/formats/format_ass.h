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
#include "format_handler.h"
#include "format_ass_dialogue.h"
#include "format_ass_style.h"
#include "section.h"
#include "section_entry_dialogue.h"
#include "section_entry_style.h"
#include "tr1.h"

namespace Athenasub {

	// Prototypes
	class CModel;
	class TextFileWriter;

	// Advanced Substation Alpha format handler
	class FormatHandlerASS : public CFormatHandler {
	private:
		int formatVersion;

		Entry MakeEntry(const String &data,Section section,int version);
		void ProcessGroup(String cur,String &curGroup,int &version);
		void WriteSection(TextFileWriter &writer,Section section);
		void MakeValid();

	public:
		FormatHandlerASS(CModel &model,int version);
		~FormatHandlerASS();

		void Load(wxInputStream &file,const String encoding);
		void Save(wxOutputStream &file,const String encoding);
	};

	// Advanced Substation Alpha format base class
	class FormatASSFamily : public IFormat {
	public:
		virtual ~FormatASSFamily() {}

		bool CanStoreText() const { return true; }
		bool CanStoreImages() const { return false; }
		bool CanUseFrames() const { return false; }
		bool CanUseTime() const { return true; }

		bool HasStyles() const { return true; }
		bool HasMargins() const { return true; }
		bool HasActors() const { return true; }
		virtual bool HasUserField() const { return false; }
		virtual String GetUserFieldName() const { return _T(""); }

		virtual int GetTimingPrecision() const { return 10; }
		virtual int GetMaxTime() const { return 35999990; }

		Dialogue CreateDialogue() const { return Dialogue(new DialogueASS()); }
		Style CreateStyle() const { return Style(new StyleASS()); }
	};

	// Substation Alpha
	class FormatSSA : public FormatASSFamily {
	public:
		FormatHandler GetHandler(IModel &model) const { return FormatHandler(new FormatHandlerASS((CModel&)model,0)); }
		String GetName() const { return L"Substation Alpha"; }
		StringArray GetReadExtensions() const;
		StringArray GetWriteExtensions() const;
	};

	// Advanced Substation Alpha
	class FormatASS : public FormatASSFamily {
	public:
		FormatHandler GetHandler(IModel &model) const { return FormatHandler(new FormatHandlerASS((CModel&)model,1)); }
		String GetName() const { return L"Advanced Substation Alpha"; }
		StringArray GetReadExtensions() const;
		StringArray GetWriteExtensions() const;
	};

	// Advanced Substation Alpha 2
	class FormatASS2 : public FormatASSFamily {
	public:
		FormatHandler GetHandler(IModel &model) const { return FormatHandler(new FormatHandlerASS((CModel&)model,2)); }
		String GetName() const { return L"Advanced Substation Alpha 2"; }
		StringArray GetReadExtensions() const;
		StringArray GetWriteExtensions() const;
	};

}

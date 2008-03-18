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

#pragma once
#include "format.h"
#include "format_handler.h"
#include "format_ass_dialogue.h"
#include "format_ass_style.h"
#include "section.h"
#include "section_entry_dialogue.h"
#include "section_entry_style.h"
#include "tr1.h"

namespace Gorgonsub {

	// Prototypes
	class Model;
	class TextFileWriter;

	// Advanced Substation Alpha format handler
	class FormatHandlerASS : public FormatHandler {
	private:
		int formatVersion;

		EntryPtr MakeEntry(const String &data,SectionPtr section,int version);
		void ProcessGroup(String cur,String &curGroup,int &version);
		void WriteSection(TextFileWriter &writer,SectionPtr section);

	public:
		FormatHandlerASS(Model &model,int version);
		~FormatHandlerASS();

		void Load(wxInputStream &file,const String encoding);
		void Save(wxOutputStream &file,const String encoding);
	};

	// Advanced Substation Alpha format base class
	class FormatASSFamily : public Format {
	public:
		virtual ~FormatASSFamily() {}

		bool CanStoreText() const { return true; }
		bool CanUseTime() const { return true; }

		bool HasStyles() const { return true; }
		bool HasMargins() const { return true; }
		bool HasActors() const { return true; }

		DialoguePtr CreateDialogue() const { return DialoguePtr(new DialogueASS()); }
		StylePtr CreateStyle() const { return StylePtr(new StyleASS()); }
	};

	// Substation Alpha
	class FormatSSA : public FormatASSFamily {
	public:
		FormatHandlerPtr GetHandler(Model &model) const { return FormatHandlerPtr(new FormatHandlerASS(model,0)); }
		String GetName() const { return L"Substation Alpha"; }
		StringArray GetReadExtensions() const;
		StringArray GetWriteExtensions() const;
	};

	// Advanced Substation Alpha
	class FormatASS : public FormatASSFamily {
	public:
		FormatHandlerPtr GetHandler(Model &model) const { return FormatHandlerPtr(new FormatHandlerASS(model,1)); }
		String GetName() const { return L"Advanced Substation Alpha"; }
		StringArray GetReadExtensions() const;
		StringArray GetWriteExtensions() const;
	};

	// Advanced Substation Alpha 2
	class FormatASS2 : public FormatASSFamily {
	public:
		FormatHandlerPtr GetHandler(Model &model) const { return FormatHandlerPtr(new FormatHandlerASS(model,2)); }
		String GetName() const { return L"Advanced Substation Alpha 2"; }
		StringArray GetReadExtensions() const;
		StringArray GetWriteExtensions() const;
	};

}

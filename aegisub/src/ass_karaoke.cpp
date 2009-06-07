// Copyright (c) 2006-2007, Niels Martin Hansen
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
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:jiifurusu@gmail.com
//


#include "config.h"

#include "ass_karaoke.h"
#include "ass_override.h"

AssKaraokeSyllable::AssKaraokeSyllable()
{
	duration = 0;
	text = _T("");
	unstripped_text = _T("");
	type = _T("");
	tag = 0;
}

void ParseAssKaraokeTags(const AssDialogue *line, AssKaraokeVector &syls)
{
	// Assume line already has tags parsed
	AssKaraokeSyllable syl;

	bool brackets_open = false;

	for (int i = 0; i < (int)line->Blocks.size(); i++) {
		AssDialogueBlock *block = line->Blocks[i];

		switch (block->GetType()) {

			case BLOCK_BASE:
				break;

			case BLOCK_PLAIN:
				syl.text += block->text;
				syl.unstripped_text += block->text;
				break;

			case BLOCK_DRAWING:
				// Regard drawings as tags
				syl.unstripped_text += block->text;
				break; 

			case BLOCK_OVERRIDE: {
				AssDialogueBlockOverride *ovr = block->GetAsOverride(block);

				for (int j = 0; j < (int)ovr->Tags.size(); j++) {
					AssOverrideTag *tag = ovr->Tags[j];

					if (tag->IsValid() && tag->Name.Mid(0,2).CmpNoCase(_T("\\k")) == 0) {
						// karaoke tag
						if (brackets_open) {
							syl.unstripped_text += _T("}");
							brackets_open = false;
						}
						
						// Store syllable
						syls.push_back(syl);

						syl.text = _T("");
						syl.unstripped_text = _T("");
						syl.tag = tag;
						syl.type = tag->Name;
						syl.duration = tag->Params[0]->AsInt();

					} else {
						// not karaoke tag
						if (!brackets_open) {
							syl.unstripped_text += _T("{");
							brackets_open = true;
						}
						syl.unstripped_text += tag->ToString();
					}
				}

				if (brackets_open) {
					brackets_open = false;
					syl.unstripped_text += _T("}");
				}

				break;
			}
		}
	}

	syls.push_back(syl);
}


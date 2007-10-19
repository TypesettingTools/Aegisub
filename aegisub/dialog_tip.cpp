// Copyright (c) 2005, Rodrigo Braz Monteiro
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
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/window.h>
#include <wx/msgdlg.h>
#include "dialog_tip.h"
#include "options.h"


///////////////
// Constructor
TipOfTheDay::TipOfTheDay (size_t currentTip): wxTipProvider(currentTip) {
	curTip = currentTip;
	tips.push_back(_("Aegisub can export subtitles to many different formats, character encodings, and even compensate Variable Frame Rate so you can hardsub them - it's all in the Export option in File menu."));
	tips.push_back(_("You can easily translate subtitle files using the translation assistant."));
	tips.push_back(_("Styles can be stored in different storages, so that you can keep your projects organized."));
	tips.push_back(_("Use keyboard shortcuts! They make your life easier, for example, Ctrl+Enter updates changes on current line without going to next. Check the manual for a complete list."));
	tips.push_back(_("There is no reason to use the SSA format (as opposed to ASS). ASS is very similar, but adds some important functionality. Most importantly, however, is that only ASS supports certain override tags (such as \\pos and \\t). Those only work on SSA files because VSFilter/Textsub is merciful."));
	tips.push_back(_("DON'T PANIC!"));
	tips.push_back(_("Aegisub has several features to make sure you will never lose your work. It will periodically save your subtitles to autosave folder, and will create a copy of subs whenever you open them, to autoback folder. Also, if it crashes, it will attempt to save a restore file."));
	tips.push_back(_("The styling assistant is a practical way to set styles to each line, when each actor has a different style assigned to it."));
	tips.push_back(_("The fonts collector is one of the most useful features, which resumes the boring task of hunting down fonts into a matter of a few clicks."));
	tips.push_back(_("When you are done with your subtitles and ready to distribute them, remember: say no to MP4, OGM or AVI. Matroska is your friend."));
	tips.push_back(_("Much like anything loaded via DirectShow, certain files may have a strange structure (such as h.264 into AVI or XviD will null frames) which may cause unreliable seeking (that is, video frames might be off by one frame). This is not an Aegisub bug - you may consider reencoding those videos before working with them."));
	tips.push_back(_("Try the spectrum mode for the audio display, it can make it much easier to spot where the important points in the audio are."));
	tips.push_back(_("If you decode your audio to a PCM WAV file before loading it in Aegisub, you don't have to wait for it to be decoded before you can use it."));
	tips.push_back(_("Having video open is often more a nuisance than a help when timing subtitles. Timing with only audio open is often much easier. You can always adjust the subtitles to match the video later on."));
	tips.push_back(_("If the audio doesn't seem to work properly during video playback, try loading audio separately. Just select Audio->Load from video, that usually makes the audio much more reliable."));
	tips.push_back(_("If anything goes wrong, blame movax."));
	//tips.push_back(_(""));
}


//////////////
// Destructor
TipOfTheDay::~TipOfTheDay() {
	tips.clear();
}


////////////////
// Get next tip
wxString TipOfTheDay::GetTip() {
	curTip = curTip % tips.size();
	wxString result = tips.at(curTip);
	curTip++;
	return result;
}


//////////////////////////////
// Show tip of the day dialog
void TipOfTheDay::Show(wxWindow *parent) {
	try {
		if (Options.AsBool(_T("Tips enabled"))) {
			TipOfTheDay *tip = new TipOfTheDay(Options.AsInt(_T("Tips current")));
			bool show = wxShowTip(parent, tip, true);
			if (!show) Options.SetBool(_T("Tips enabled"),false);
			Options.SetInt(_T("Tips current"),tip->curTip);
			Options.Save();
			delete tip;
		}
	}
	catch (wxString error) {
		wxMessageBox(_T("Error showing tips: ") + error,_T("Error"), wxOK | wxICON_ERROR, NULL);
	}
	catch (...) {
		wxMessageBox(_T("Unknown error showing tips."),_T("Error"), wxOK | wxICON_ERROR, NULL);
	}
}


//////////
// Static
size_t TipOfTheDay::curTip;

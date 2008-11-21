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

//#define ATHENA_DLL
#include <wx/wfstream.h>
#include <athenasub/athenasub.h>
#include <iostream>
#include <wx/stopwatch.h>
#include "text_file_reader.h"
#include "text_file_writer.h"

int main()
{
	using namespace std;
	using namespace Athenasub;

	cout << "Athenasub test program by amz.\n\n";

	try {
		// Set up the lib
		cout << "Loading library... ";
		/*
#ifdef WXDEBUG
		HMODULE module = LoadLibrary(_T("athenasub_d.dll"));
#else
		HMODULE module = LoadLibrary(_T("athenasub.dll"));
#endif
		if (!module) {
			cout << "Failed to load library, aborting.\n";
			system("pause");
			return 1;
		}
		*/
		cout << "Done.\nCreating library...";

		LibAthenaSub lib = Athenasub::Create("Aegilib test program");
		//LibAthenaSub lib = Athenasub::Create(module,"Aegilib test program");
		cout << "Done.\n";

		// Subtitles model
		cout << "Creating model... ";
		Model subs = lib->CreateModel();
		cout << "Creating controller...\n";
		Controller control = subs->CreateController();
		wxStopWatch timer;

		// Load subtitles
		cout << "Loading file... ";
		timer.Start();
		control->LoadFile("subs_in.ass","UTF-8");
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";

		// Save subtitles
		cout << "Saving file... ";
		timer.Start();
		control->SaveFile("subs_out.ass","UTF-8");
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";

		// Issue an action
#ifdef WXDEBUG
		int n = 1;
#else
		int n = 100;
#endif
		cout << "Executing action " << n << " times... ";
		timer.Start();
		for (int i=0;i<n;i++) {
			ActionList actions = control->CreateActionList("Test");
			Selection selection = control->CreateSelection();
			selection->AddRange(Range(0,5000));
			selection->AddRange(Range(4500,5500));
			selection->AddRange(Range(9000,9100));
			std::vector<Entry> entries = actions->ModifyLines(selection,"Events");
			size_t len = entries.size();
			for (size_t i=0;i<len;i++) {
				Dialogue diag = dynamic_pointer_cast<IDialogue> (entries[i]);
				diag->SetStartTime(diag->GetStartTime() - 55555);
				diag->SetEndTime(diag->GetEndTime() + 5555);
			}
			actions->Finish();
		}
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";

		// Rollback
		cout << "Undoing " << n-1 << " times... ";
		timer.Start();
		for (int i=0;i<n-1;i++) {
			control->Undo();
		}
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";

		// Undo
		n = 10;
		cout << "Undoing and redoing " << n << " times... ";
		timer.Start();
		for (int i=0;i<n;i++) {
			control->Undo();
			control->Redo();
		}
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";

		// Get style test
		ConstStyle style = control->GetStyle("japro1_star");
		cout << "Style " << style->GetName() << " font is " << style->GetFontName() << " " << style->GetFontSize() << ".\n";

		// Save a few more
		control->SaveFile("subs_out2.ass","UTF-8");
		control->Undo();
		control->SaveFile("subs_out3.ass","UTF-8");
	}

	catch (std::exception &e) {
		cout << "\n\nException: " << e.what() << endl << endl;
	}

	return true;
}

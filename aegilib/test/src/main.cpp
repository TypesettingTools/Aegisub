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

#include <aegilib/gorgonsub.h>
#include <wx/wfstream.h>
#include <iostream>
#include <wx/stopwatch.h>
#include "text_file_reader.h"
#include "text_file_writer.h"


int main() {
	using namespace std;
	using namespace Gorgonsub;

	cout << "Gorgonsub test program by amz.\n\n";

	try {
		// Set up the lib
		FormatManager::InitializeFormats();
		Gorgonsub::SetHostApplicationName(L"Aegilib test program");

		// Subtitles model
		Model subs;
		Controller control(subs);
		wxStopWatch timer;

		// Load subtitles
		cout << "Loading file... ";
		timer.Start();
		control.LoadFile(L"subs_in.ass",L"UTF-16LE");
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";
		//system("pause");

		// Save subtitles
		cout << "Saving file... ";
		timer.Start();
		control.SaveFile(L"subs_out.ass",L"UTF-8");
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";

		// Create line to be inserted
		cout << "Creating data... ";
		SectionEntryDialoguePtr line = control.CreateDialogue();
		line->SetText(L"Hi, testing insertion of lines!");
		cout << "Done.\n";

		// Create action list
		cout << "Processing actions... ";
		timer.Start();
		ActionListPtr actions = control.CreateActionList(L"Insert line");
		actions->InsertLine(line,2);
		actions->RemoveLine(3,L"Events");
		actions->Finish();
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";

		// Undo
		cout << "Undoing and redoing 1000 times... ";
		timer.Start();
		for (size_t i=0;i<1000;i++) {
			control.Undo();
			control.Redo();
		}
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";
	}

	catch (std::exception &e) {
		cout << "\n\nException: " << e.what() << endl << endl;
	}

	return true;
}

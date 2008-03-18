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

int main()
{
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
		control.LoadFile(L"subs_in.ass",L"UTF-8");
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";
		//system("pause");

		// Save subtitles
		cout << "Saving file... ";
		timer.Start();
		control.SaveFile(L"subs_out.ass",L"UTF-8");
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";

		// Issue an action
#ifdef WXDEBUG
		const int n = 1;
#else
		const int n = 100;
#endif
		cout << "Executing action " << n << " times... ";
		timer.Start();
		for (size_t i=0;i<n;i++) {
			ActionListPtr actions = control.CreateActionList(L"Test");
			Selection selection;
			selection.AddRange(Range(0,5000));
			selection.AddRange(Range(4500,5500));
			selection.AddRange(Range(9000,9100));
			std::vector<EntryPtr> entries = actions->ModifyLines(selection,L"Events");
			size_t len = entries.size();
			for (size_t i=0;i<len;i++) {
				DialoguePtr diag = dynamic_pointer_cast<Dialogue> (entries[i]);
				diag->SetStartTime(diag->GetStartTime() - 55555);
				diag->SetEndTime(diag->GetEndTime() + 5555);
			}
			actions->Finish();
		}
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";

		// Rollback
		cout << "Undoing " << n << " times... ";
		for (size_t i=0;i<n-1;i++) {
			control.Undo();
		}
		cout << "Done.\n";

		// Undo
		cout << "Undoing and redoing " << n*10 << " times... ";
		timer.Start();
		for (size_t i=0;i<n*10;i++) {
			control.Undo();
			control.Redo();
		}
		timer.Pause();
		cout << "Done in " << timer.Time() << " ms.\n";

		// Get style test
		StyleConstPtr style = control.GetStyle(L"japro1_star");
		cout << "Style " << style->GetName().mb_str() << " font is " << style->GetFontName().mb_str() << " " << style->GetFontSize() << ".\n";

		// Save a few more
		control.SaveFile(L"subs_out2.ass",L"UTF-8");
		control.Undo();
		control.SaveFile(L"subs_out3.ass",L"UTF-8");
	}

	catch (std::exception &e) {
		cout << "\n\nException: " << e.what() << endl << endl;
	}

	if (false) {
		wchar_t myArray[] = { 0xD834, 0xDD1E, 0 };
		String str = wxString(myArray);
		cout << "Length: " << str.Length() << ". Contents: " << str[0] << "," << str[1] << endl;
		wxCharBuffer buf = str.mb_str(wxConvUTF8);
		unsigned char *chr = (unsigned char *) buf.data();
		cout << "UTF-8 Length: " << strlen(buf) << ". Contents: " << (size_t)chr[0] << "," << (size_t)chr[1] << "," << (size_t)chr[2] << "," << (size_t)chr[3] << endl;
		str = wxString(buf,wxConvUTF8);
		cout << "Length: " << str.Length() << ". Contents: " << str[0] << "," << str[1] << endl;
	}
	return true;
}

// Copyright (c) 2005-2006, Rodrigo Braz Monteiro, Fredrik Mellbin
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
#include "vfr.h"
#include "utils.h"
#include <wx/filename.h>
#include <fstream>
#include <algorithm>

/////////////////////
// V2 Clear function
void FrameRate::Clear () {
	Frame.clear();
}


////////////////
// V2 Add frame
void FrameRate::AddFrame(int ms) {
	Frame.push_back(ms);
}


//////////////////
// V2 Get Average
void FrameRate::CalcAverage() {

	if (Frame.size() <= 1)
		throw _("No timecodes to average");

	AverageFrameRate = double(Frame.back()) / (Frame.size()-1);
}


//////////////////////// FrameRate //////////////////////
///////////////
// Constructor
FrameRate::FrameRate() {
	Unload();
}


//////////////
// Destructor
FrameRate::~FrameRate() {
	Unload();
}


//////////////////
// Loads VFR file
void FrameRate::Load(wxString filename) {
	using namespace std;
	
	Unload();

	// Check if file exists
	wxFileName filetest(filename);
	if (!filetest.FileExists()) throw _T("File not found.");

	// Open file
	ifstream file;
	file.open(filename.mb_str(wxConvLocal));
	if (!file.is_open()) throw _T("Could not open file.");

	try {

		// Read header
		char buffer[65536];
		file.getline(buffer,65536);
		wxString header(buffer,wxConvUTF8);

		// V1, code converted from avcvfr9
		if (header == _T("# timecode format v1")) {
			//locate the default fps line
			
			while (!file.eof()) {
				file.getline(buffer,65536);
				wxString curLine(buffer,wxConvUTF8);

				//skip empty lines and comments
				if (curLine == _T("") || curLine.Left(1) == _T("#"))
					continue;
				//fix me? should be case insensitive comparison
				else if (curLine.Left(7) != _T("Assume "))
					throw _T("Encountered data before 'Assume <fps>' line");
				else {
					if (!curLine.Mid(6).ToDouble(&AverageFrameRate) || AverageFrameRate <= 0)
						throw _T("Invalid 'Assume <fps>' line");
					break;
				}
			}

			//read and expand all timecodes to v2
			wxString curline;

			double currenttime = 0;
			int lposition = -1;

			long lstart;
			long lend;
			double lfps;

			while (!file.eof()) {
				file.getline(buffer,65536);
				wxString curLine(buffer,wxConvUTF8);

				//skip empty lines and comments
				if (curLine == _T("") || curLine.Left(1) == _T("#"))
					continue;
				
				wxString tmp = curLine.AfterFirst(_T(','));
				wxString temp = curLine.BeforeFirst(_T(','));
				if (!temp.ToLong(&lstart) || lstart < 0)
					throw _T("Timecode parsing error, invalid start format found");
				temp = tmp.BeforeLast(_T(','));
				if (!temp.ToLong(&lend) || lend < 0)
					throw _T("Timecode parsing error, invalid end format found");
				temp = tmp.AfterLast(_T(','));
				if (!temp.ToDouble(&lfps) || lfps <= 0)
					throw _T("Timecode parsing error, invalid fps format found");


				for (int i = 0; i <= lstart - lposition - 2; i++)
					AddFrame(floor(currenttime+(i*1000) / AverageFrameRate));

				currenttime += ((lstart - lposition - 1)*1000) / AverageFrameRate;

				for (int i = 0; i <= lend - lstart; i++)
					AddFrame(floor(currenttime+(i*1000) / lfps));

				currenttime += ((lend - lstart + 1)*1000) / lfps;

				lposition = lend;
			}

			last_time = currenttime;
			last_frame = Frame.size();
		}

		// V2
		else if (header == _T("# timecode format v2")) {
			// Assigns new VFR file
			FrameRateType = VFR;

			long lftime = -1;
			long cftime = 0;
			last_frame = 0;

			// Reads body
			while (!file.eof()) {
				file.getline (buffer,65536);
				wxString curLine(buffer,wxConvUTF8);

				//skip empty lines and comments
				if (curLine == _T("") || curLine.Left(1) == _T("#"))
					continue;

				wxString tmp = curLine.BeforeFirst(_T('.'));
				tmp.ToLong(&cftime);

				if (lftime >= cftime)
					throw _T("Out of order/too close timecodes found");

				AddFrame(cftime);
				lftime = cftime;
			}

			last_time = cftime;
			last_frame = Frame.size();

			CalcAverage();

		}

		// Unknown
		else 
			throw _T("Unknown file format.");

		// Run test
		/*bool doTest = false;
		if (doTest) {
			int fail = 0;
			int res;
			for (int i=0;i<1000;i++) {
				res = GetFrameAtTime(GetTimeAtFrame(i));
				if (res != i) {
					wxLogMessage(wxString::Format(_T("Expected %i but got %i (%i)"),i,res,GetTimeAtFrame(i)));
					fail++;
				}
			}
			if (fail) wxLogMessage(wxString::Format(_T("Failed %i times"),fail));
			else wxLogMessage(_T("VFR passes test"));
		}*/
	} catch (wchar_t *) {
		file.close();
		Unload();
		throw;
	}

	// Close file
	file.close();
	loaded = true;
	vfrFile = filename;
	FrameRateType = VFR;
}


//////////
// Unload
void FrameRate::Unload () {
	FrameRateType = NONE;
	AverageFrameRate = 0;
	last_time = 0;
	last_frame = 0;
	Clear();
	loaded = false;
	vfrFile = _T("");
}


///////////////
// Sets to CFR
void FrameRate::SetCFR(double fps) {
	Unload();
	loaded = true;
	FrameRateType = CFR;
	AverageFrameRate = fps;
}


///////////////
// Sets to VFR
void FrameRate::SetVFR(std::vector<int> newTimes) {
	// Prepare
	Unload();

	loaded = true;
	FrameRateType = VFR;

	// Set new VFR;
	Frame = newTimes;
	CalcAverage();
	last_time = newTimes.back();
	last_frame = newTimes.size();
}


/////////////////////////////
// Gets frame number at time
int FrameRate::PFrameAtTime(int ms,bool useceil) {
	//wxASSERT(loaded);

	if (!loaded) return -1;

	ms = MAX(ms,0); //fix me, unsafe for GetFrame... for frame 0?

	if (FrameRateType == CFR) {
		if (useceil) return ceil(double(ms)/1000.0 * AverageFrameRate);
		else return floor(double(ms)/1000.0 * AverageFrameRate);
	}
	else if (FrameRateType == VFR) {
		if (ms < floor(last_time)) {
			// Binary search
			size_t start = 0;
			size_t end = last_frame;
			size_t cur;
			bool largerEqual;
			while (start <= end) {
				cur = (start+end)>>1;
				largerEqual = Frame[cur] >= ms;

				// Found
				if (largerEqual && (cur == 0 || Frame[cur-1] < ms))
					return cur;
				
				// Not found
				if (largerEqual) end = cur-1;
				else start = cur+1;
			}
		} else {
			return last_frame + floor((ms-last_time) * AverageFrameRate / 1000);
		}
	}
	return -1;
}


//////////////////////
// Gets time at frame
int FrameRate::PTimeAtFrame(int frame) {
	//wxASSERT(loaded);

	if (!loaded) return -1;
	if (frame <= 0) return 0;

	if (FrameRateType == CFR) {
		return floor(double(frame) / AverageFrameRate * 1000.0);
	} else if (FrameRateType == VFR) {
		if (frame < last_frame)
			return Frame.at(frame);
		else 
			return floor(last_time + double(frame-last_frame) / AverageFrameRate * 1000.0);
	}
	return -1;
}


/////////////////////////////
// Get correct frame at time
// returns the adjusted time for end frames when start=false
// otherwise for start frames
int FrameRate::GetFrameAtTime(int ms,bool start) {
	if (start) {
		if (FrameRateType == VFR) return PFrameAtTime(ms);
		else return PFrameAtTime(ms,start);
	}
	else {
		return PFrameAtTime(ms);
	}
}


/////////////////////////////
// Get correct time at frame
// compensates and returns an end time when start=false
int FrameRate::GetTimeAtFrame(int frame,bool start) {
	int finalTime;
	if (start) {
		finalTime = (PTimeAtFrame(frame-1) + PTimeAtFrame(frame))/2;
	}
	else {
		if (FrameRateType == VFR) finalTime = PTimeAtFrame(frame);
		else finalTime = (PTimeAtFrame(frame) + PTimeAtFrame(frame+1))/2;
	}

	return finalTime;
}


///////////
// Globals
FrameRate VFR_Output;
FrameRate VFR_Input;

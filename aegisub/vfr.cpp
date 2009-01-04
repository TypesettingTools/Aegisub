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
#include "config.h"

#include <wx/filename.h>
#include "options.h"
#include "vfr.h"
#include "utils.h"
#include "text_file_reader.h"
#include "text_file_writer.h"


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
	TextFileReader file(filename);

	try {
		// Read header
		wxString curLine;
		curLine = file.ReadLineFromFile();
		wxString header = curLine;
		bool first = (header.Left(7).Lower() == _T("assume "));

		// V1, code converted from avcvfr9
		if (header == _T("# timecode format v1") || first) {
			// Locate the default fps line
			do {
				// Get next line
				if (!first) curLine = file.ReadLineFromFile();
				first = false;

				// Skip empty lines and comments
				if (curLine == _T("") || curLine.Left(1) == _T("#")) continue;

				else if (curLine.Left(7).Lower() != _T("assume ")) throw _T("Encountered data before 'Assume <fps>' line");
				else {
					if (!curLine.Mid(6).ToDouble(&AverageFrameRate) || AverageFrameRate <= 0) throw _T("Invalid 'Assume <fps>' line");
					break;
				}
			} while (file.HasMoreLines());

			// Read and expand all timecodes to v2
			wxString curline;

			double currenttime = 0;
			int lposition = -1;

			long lstart;
			long lend;
			double lfps;

			while (file.HasMoreLines()) {
				curLine = file.ReadLineFromFile();

				// Skip empty lines and comments
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

				if (lstart <= lposition)
					throw _T("Timecode parsing error, out of order or overlapping timecode range found"); 


				for (int i = 0; i <= lstart - lposition - 2; i++)
					AddFrame((int)(floor(currenttime+(i*1000) / AverageFrameRate)));

				currenttime += ((lstart - lposition - 1)*1000) / AverageFrameRate;

				for (int i = 0; i <= lend - lstart; i++)
					AddFrame((int)(floor(currenttime+(i*1000) / lfps)));

				currenttime += ((lend - lstart + 1)*1000) / lfps;

				lposition = lend;
			}

			last_time = currenttime;
			last_frame = (int)Frame.size();
		}

		// V2
		else if (header == _T("# timecode format v2")) {
			// Assigns new VFR file
			FrameRateType = VFR;

			long lftime = -1;
			long cftime = 0;
			last_frame = 0;

			// Reads body
			while (file.HasMoreLines()) {
				curLine = file.ReadLineFromFile();

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
			last_frame = (int)Frame.size();

			CalcAverage();

		}

		// Unknown
		else {
			throw _T("Unknown time code file format.");
		}

	}
	catch (...) {
		Unload();
		throw;
	}

	// Close file
	loaded = true;
	vfrFile = filename;
	FrameRateType = VFR;

	// Add to recent
	Options.AddToRecentList(filename,_T("Recent timecodes"));
}


////////
// Save
void FrameRate::Save(wxString filename) {
	TextFileWriter file(filename,_T("ASCII"));
	file.WriteLineToFile(_T("# timecode format v2"));
	for (size_t i=0;i<Frame.size();i++) {
		file.WriteLineToFile(wxString::Format(_T("%f"),(float)Frame[i]));
	}
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
	last_frame = (int)newTimes.size();
}


/////////////////////////////
// Gets frame number at time
int FrameRate::PFrameAtTime(int ms,bool useceil) {
	// Check if it's loaded
	if (!loaded) return -1;

	// Normalize miliseconds
	ms = MAX(ms,0);

	// Get for constant frame rate
	if (FrameRateType == CFR || Frame.size() == 0) {
		double value = double(ms) * AverageFrameRate / 1000.0;
		if (useceil) return (int)ceil(value);
		else return (int)floor(value);
	}

	// Get for variable frame rate
	else if (FrameRateType == VFR) {
		// Get last
		double trueLast;
		//if (useceil) trueLast = ceil(last_time);
		//else trueLast = floor(last_time);
		trueLast = Frame[Frame.size()-1];

		// Inside VFR range
		if (ms < trueLast) {
			// Prepare binary search
			size_t start = 0;
			size_t end = last_frame;
			size_t cur;
			bool largerEqual;

			// Do binary search
			while (start <= end) {
				// Current frame being checked
				cur = (start+end)>>1;

				// Is it larger or equal to searched time?
				largerEqual = Frame[cur] >= ms;

				// If it is, is the previous smaller?
				// If so, this is the frame we're looking for
				if (largerEqual && (cur == 0 || Frame[cur-1] < ms))	{
					if (useceil) return (int)cur;
					return (int)(cur)-1;
				}
				
				// Not found, continue search
				if (largerEqual) end = cur-1;
				else start = cur+1;
			}
		}
		
		// After VFR range
		else {
			if (useceil) return (int)(last_frame + ceil((ms-last_time) * AverageFrameRate / 1000));
			else return (int)(last_frame + floor((ms-last_time) * AverageFrameRate / 1000));
		}
	}
	return -1;
}


//////////////////////
// Gets time at frame
int FrameRate::PTimeAtFrame(int frame) {
	// Not loaded
	if (!loaded) return -1;

	// For negative/zero times, fallback to zero
	if (frame <= 0) return 0;

	// Constant frame rate
	if (FrameRateType == CFR || Frame.size() == 0) {
		return (int)floor(double(frame) / AverageFrameRate * 1000.0);
	}
	
	// Variable frame rate
	else if (FrameRateType == VFR) {
		// Is it inside frame rate range? If so, just get the value from timecodes table
		if (frame < (signed) Frame.size()) return Frame.at(frame);

		// Otherwise, calculate it
		else return (int)floor(Frame.back() + double(frame-Frame.size()+1) / AverageFrameRate * 1000.0);
	}

	// Unknown frame rate type
	return -1;
}


/////////////////////////////
// Get correct frame at time
// returns the adjusted time for end frames when start=false
// otherwise for start frames
int FrameRate::GetFrameAtTime(int ms,bool start) {
	return PFrameAtTime(ms,start);
}


/////////////////////////////
// Get correct time at frame
// compensates and returns an end time when start=false
int FrameRate::GetTimeAtFrame(int frame,bool start,bool exact) {
	int finalTime;

	// Exact, for display
	if (exact) {
		finalTime = PTimeAtFrame(frame);
	}

	// Adjusted, for subs sync
	else {
		if (start) {
			finalTime = (PTimeAtFrame(frame-1) + PTimeAtFrame(frame))/2;
		}
		else {
			//if (FrameRateType == VFR) finalTime = PTimeAtFrame(frame);
			//else finalTime = (PTimeAtFrame(frame) + PTimeAtFrame(frame+1))/2;
			finalTime = (PTimeAtFrame(frame) + PTimeAtFrame(frame+1))/2;
		}
	}

	return finalTime;
}


////////////////////////////////////////
// Get the current list of frames/times
Aegisub::IntArray FrameRate::GetFrameTimeList() {
	return Frame;
}


///////////////////////////////////////////
// Calculate the common FPS for evil stuff
// e.g., in a mix of 24fps and 30fps, returns 120fps
double FrameRate::GetCommonFPS() {
	// Variables
	int curDist;
	int lastDist = 0;
	int sectionStart = 0;
	double curFps;

	// List of likely frame rates
	std::vector<double> frameRates;
	frameRates.push_back(15.0 / 1.001);
	frameRates.push_back(15);
	frameRates.push_back(24.0 / 1.001);
	frameRates.push_back(24);
	frameRates.push_back(30.0 / 1.001);
	frameRates.push_back(30);
	frameRates.push_back(120.0 / 1.001);
	frameRates.push_back(120);

	// List of rates found
	std::vector<double> found;

	// Find the relative fps of each area
	for (unsigned int i=1;i<Frame.size();i++) {
		// Find the current frame distance
		curDist = Frame[i]-Frame[i-1];

		// See if it's close enough to the last
		if ((abs(curDist - lastDist) < 2 || i-1 == (unsigned) sectionStart) && i != Frame.size()-1) {
			lastDist = curDist;
			continue;
		}

		// Calculate section fps
		curFps = (i - sectionStart - 1) * 1000.0 / double(Frame[i-1]-Frame[sectionStart]);
		sectionStart = i;
		lastDist = curDist;

		// See if it's close enough to one of the likely rates
		for (unsigned int j=0;j<frameRates.size();j++) {
			if (curFps-0.01 <= frameRates[j] && curFps+0.01 >= frameRates[j]) {
				curFps = frameRates[j];
				break;
			}
		}

		// See if it's on list
		bool onList = false;
		for (unsigned int j=0;j<found.size();j++) {
			if (found[j] == curFps) {
				onList = true;
				break;
			}
		}

		// If not, add it
		if (!onList) found.push_back(curFps);
	}

	// Find common between them
	double v1,v2,minInt,tempd;
	int tempi1,tempi2;
	while (found.size() > 1) {
		// Extract last two values
		v1 = found.back();
		found.pop_back();
		v2 = found.back();
		found.pop_back();

		// Divide them
		v2 = v1/v2;

		// Find what it takes to make it an integer
		for (minInt = 1;minInt<20;minInt++) {
			tempd = v2 * minInt;
			tempi1 = (int)(tempd-0.001);
			tempi2 = (int)(tempd+0.001);
			if (tempi1 != tempi2) break;
		}
		if (minInt != 20) v1 = v1*minInt;

		// See if it's close enough to one of the likely rates
		for (unsigned int j=0;j<frameRates.size();j++) {
			if (v1-0.01 <= frameRates[j] && v1+0.01 >= frameRates[j]) {
				v1 = frameRates[j];
				break;
			}
		}

		// Re-insert obtained result
		found.push_back(v1);
	}

	return found.back();
}


///////////
// Globals
FrameRate VFR_Output;
FrameRate VFR_Input;

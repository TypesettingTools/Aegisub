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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file vfr.cpp
/// @brief Handle variable frame rate files
/// @ingroup video_input
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/filename.h>
#endif

#include "compat.h"
#include "main.h"
#include "options.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "utils.h"
#include "vfr.h"


/// @brief V2 Clear function 
void FrameRate::Clear () {
	Frame.clear();
}

/// @brief V2 Add frame 
/// @param ms 
void FrameRate::AddFrame(int ms) {
	Frame.push_back(ms);
}

/// @brief V2 Get Average 
void FrameRate::CalcAverage() {
	if (Frame.size() <= 1)
		throw _("No timecodes to average");

	AverageFrameRate = double(Frame.back()) / (Frame.size()-1);
}

/// @brief Constructor
FrameRate::FrameRate() {
	Unload();
}

/// @brief Destructor 
FrameRate::~FrameRate() {
	Unload();
}

/// @brief Loads VFR file 
/// @param filename 
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

			AddFrame(currenttime);
			last_time = currenttime;
			last_frame = (int)Frame.size() - 1;
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
			last_frame = (int)Frame.size() - 1;

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
	config::mru->Add("Timecodes", STD_STR(filename));
}

/// @brief Save 
/// @param filename 
void FrameRate::Save(wxString filename) {
	TextFileWriter file(filename,_T("ASCII"));
	file.WriteLineToFile(_T("# timecode format v2"));
	for (size_t i=0;i<Frame.size();i++) {
		file.WriteLineToFile(wxString::Format(_T("%f"),(float)Frame[i]));
	}
}

/// @brief Unload 
void FrameRate::Unload () {
	FrameRateType = NONE;
	AverageFrameRate = 0;
	last_time = 0;
	last_frame = 0;
	Clear();
	loaded = false;
	vfrFile = _T("");
}

/// @brief Sets to CFR 
/// @param fps 
void FrameRate::SetCFR(double fps) {
	Unload();
	loaded = true;
	FrameRateType = CFR;
	AverageFrameRate = fps;
}

/// @brief Sets to VFR 
/// @param newTimes 
void FrameRate::SetVFR(std::vector<int> newTimes) {
	Unload();

	loaded = true;
	FrameRateType = VFR;

	// Set new VFR;
	Frame = newTimes;
	CalcAverage();
	last_time = newTimes.back();
	last_frame = (int)newTimes.size();
}

/// @brief Gets frame number at time 
/// @param ms      
/// @param start
/// @return 
int FrameRate::PFrameAtTime(int ms,bool start) const {
	if (!loaded) return -1;

	// Lines begin on the first frame whose start time is greater than or equal
	// to the line's start time, and are last visible on the last frame whose
	// start time is less than (note: not equal) the line's end time

	if (FrameRateType == CFR || Frame.size() == 0 || ms < 0) {
		double value = double(ms) * AverageFrameRate / 1000.;
		if (start) return (int)ceil(value);
		else return (int)floor(value - .0001);
	}
	else if (FrameRateType == VFR) {
		// Inside VFR range
		if (ms <= Frame.back()) {
			int frame = std::distance(Frame.begin(), std::lower_bound(Frame.begin(), Frame.end(), ms));
			if (!start && frame > 0) {
				// In the end case, frame is the first frame in which the line
				// is no longer visible, so subtract 1

				// Don't need to worry about the equal case here as lower_bound
				// finds the entry >= ms

				// The frame > 0 check isn't actually correct -- the frame
				// ending at time 0 should be -1, but parts of the program
				// (like PTimeAtFrame below) assume that frames are positive
				--frame;
			}
			return frame;
		}
		// After VFR range
		else {
			if (start) return (int)(last_frame + ceil((ms-last_time) * AverageFrameRate / 1000.));
			else return (int)(last_frame + floor((ms-last_time - .0001) * AverageFrameRate / 1000.));
		}
	}
	return -1;
}

/// @brief Gets time at frame 
/// @param frame 
/// @return 
int FrameRate::PTimeAtFrame(int frame) const {
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

/// @brief otherwise for start frames returns the adjusted time for end frames when start=false Get correct frame at time 
/// @param ms    
/// @param start 
/// @return 
int FrameRate::GetFrameAtTime(int ms,bool start) const {
	return PFrameAtTime(ms,start);
}

/// @brief compensates and returns an end time when start=false Get correct time at frame 
/// @param frame Frame number
/// @param start Adjust for start time
/// @param exact Don't do awful things to avoid rounding errors
/// @return 
int FrameRate::GetTimeAtFrame(int frame,bool start,bool exact) const {
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

/// @brief Get the current list of frames/times 
/// @return 
std::vector<int> FrameRate::GetFrameTimeList() const {
	return Frame;
}

bool FrameRate::operator==(FrameRate const& rgt) {
	if (FrameRateType != rgt.FrameRateType) return false;
	if (FrameRateType == NONE) return true;
	if (FrameRateType == CFR) return AverageFrameRate == rgt.AverageFrameRate;
	return Frame == rgt.Frame;
}

/// DOCME
FrameRate VFR_Output;

/// DOCME
FrameRate VFR_Input;

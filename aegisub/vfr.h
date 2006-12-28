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

// The FrameRate class stores all times internally as ints in ms precision
// V1 timecodes are partially expanded to v2 up until their last override line
// V2 timecodes are kept as is and if n frames beyond the end is requested a
// time is calculated by last_time+n/average_fps

#pragma once


///////////
// Headers
#include <list>
#include <vector>
#include <wx/wxprec.h>


///////////////////////
// Framerate type enum
enum ASS_FrameRateType {
	NONE,
	CFR,
	VFR
};

///////////////////
// Framerate class
class FrameRate {
private:
	double last_time;
	int last_frame;
	std::vector<int> Frame;

	// contains the assumed fps for v1 timecodes, average for v2 and actual fps for cfr
	double AverageFrameRate; 

	void AddFrame(int ms);
	void Clear();

	void CalcAverage();
	int PFrameAtTime(int ms,bool useCeil=false);
	int PTimeAtFrame(int frame);

	ASS_FrameRateType FrameRateType;
	bool loaded;
	wxString vfrFile;
public:
	FrameRate();
	~FrameRate();

	void SetCFR(double fps);
	void SetVFR(std::vector<int> times);

	// Loading always unloads even on failure
	void Load(wxString file);
	void Unload();
	int GetFrameAtTime(int ms,bool start=true);
	int GetTimeAtFrame(int frame,bool start=true,bool exact=false);

	double GetAverage() { return AverageFrameRate; };
	bool IsLoaded() { return loaded; };
	ASS_FrameRateType GetFrameRateType() { return FrameRateType; };
	wxString GetFilename() { return vfrFile; };
};


///////////
// Globals
extern FrameRate VFR_Output;
extern FrameRate VFR_Input;

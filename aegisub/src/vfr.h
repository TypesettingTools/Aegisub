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

/// @file vfr.h
/// @see vfr.cpp
/// @ingroup video_input
///

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
#include <wx/dynarray.h>
#include "include/aegisub/aegisub.h"



/// DOCME
enum ASS_FrameRateType {

	/// DOCME
	NONE,

	/// DOCME
	CFR,

	/// DOCME
	VFR
};


/// DOCME
/// @class FrameRate
/// @brief DOCME
///
/// DOCME
class FrameRate {
	friend class VideoContext;

private:

	/// DOCME
	double last_time;

	/// DOCME
	int last_frame;

	/// DOCME
	std::vector<int> Frame;


	/// DOCME
	double AverageFrameRate; 

	void AddFrame(int ms);
	void Clear();

	void CalcAverage();
	int PFrameAtTime(int ms,bool useCeil=false);
	int PTimeAtFrame(int frame);


	/// DOCME
	ASS_FrameRateType FrameRateType;

	/// DOCME
	bool loaded;

	/// DOCME
	wxString vfrFile;

public:
	FrameRate();
	~FrameRate();

	void SetCFR(double fps);
	void SetVFR(std::vector<int> times);

	// Loading always unloads even on failure
	void Load(wxString file);
	void Save(wxString file);
	void Unload();

	int GetFrameAtTime(int ms,bool start=true);
	int GetTimeAtFrame(int frame,bool start=true,bool exact=false);


	/// @brief DOCME
	/// @return 
	///
	double GetAverage() { return AverageFrameRate; };

	/// @brief DOCME
	/// @return 
	///
	bool IsLoaded() { return loaded; };

	/// @brief DOCME
	/// @return 
	///
	ASS_FrameRateType GetFrameRateType() { return FrameRateType; };

	/// @brief DOCME
	///
	wxString GetFilename() { return vfrFile; };

	std::vector<int> GetFrameTimeList();
	double GetCommonFPS();
};


///////////
// Globals
extern FrameRate VFR_Output;
extern FrameRate VFR_Input;



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
#include "vfr.h"
#include <wx/filename.h>
#include <fstream>
#include <algorithm>



////////////////////// V1 //////////////////////
//////////////////
// V1 Constructor
VFR_v1::VFR_v1 () {
	Clear();
}


/////////////////
// V1 Destructor
VFR_v1::~VFR_v1 () {
	Clear();
}


/////////////////////
// V1 Clear function
void VFR_v1::Clear () {
	Range.clear();
}


////////////////
// V1 Add range
void VFR_v1::AddRange(int start,int end,double fps,bool isdefault) {
	VFR_v1_Range newRange;
	newRange.start = start;
	newRange.end = end;
	newRange.fps = fps;
	newRange.isDefault = isdefault;
	if (isdefault) DefaultFPS = fps;
	Range.push_back(newRange);
}


////////////////////////
// V1 Get frame at time
int VFR_v1::GetFrameAtTime(int _ms) {
	double ms = _ms;
	double curms = 0;
	double prevms = 0;
	int curframe = 0;
	double fpms;
	for (std::list<VFR_v1_Range>::iterator cur=Range.begin();cur!=Range.end();cur++) {
		fpms = (*cur).fps/1000.0;
		curms += double((*cur).end - (*cur).start + 1) / fpms;
		if (curms > ms) {
			//return curframe + floor((ms - prevms) * fpms + 0.5);
			//return curframe + floor((ms - prevms) * fpms);
			double msValue = (ms - prevms) * fpms;
			double floorValue = ceil(msValue);
			int final = curframe + floorValue;
			return final;
		}
		curframe = (*cur).end+1;
		prevms = curms;
	}
	//return curframe + floor((ms - prevms) * DefaultFPS/1000.0 + 0.5);
	return curframe + floor((ms - prevms) * DefaultFPS/1000.0);
}


////////////////////////
// V1 Get time at frame
int VFR_v1::GetTimeAtFrame(int frame) {
	double acum = 0;
	int last = 0;
	for (std::list<VFR_v1_Range>::iterator cur=Range.begin();cur!=Range.end();cur++) {
		if (frame <= (*cur).end) {
			acum += double(frame - (*cur).start) / (*cur).fps;
			//return floor(acum*1000.0+0.5);
			return floor(acum*1000.0);
		}
		else {
			acum += double((*cur).end - (*cur).start + 1) / (*cur).fps;
			last = (*cur).end+1;
		}
	}
	acum += double(frame - last) / DefaultFPS;
	//return floor(acum*1000.0+0.5);
	return floor(acum*1000.0);
}


//////////////////
// V1 Get Average
double VFR_v1::GetAverage () {
	return 0;
}


////////////////////// V2 //////////////////////
//////////////////
// V2 Constructor
VFR_v2::VFR_v2 () {
	Clear();
}


/////////////////
// V2 Destructor
VFR_v2::~VFR_v2 () {
	Clear();
}


/////////////////////
// V2 Clear function
void VFR_v2::Clear () {
	Frame.clear();
}


////////////////
// V2 Add frame
void VFR_v2::AddFrame(double fps) {
	Frame.push_back(fps);
}


////////////////////////
// V2 Get frame at time
int VFR_v2::GetFrameAtTime(int ms) {
	// Binary search
	size_t start = 0;
	size_t end = Frame.size()-1;
	size_t cur;
	bool largerEqual;
	while (start <= end) {
		cur = (start+end)>>1;
		largerEqual = floor(Frame[cur]) + 0.5 >= ms;

		// Found
		if (largerEqual && (cur == 0 || floor(Frame[cur-1]) + 0.5 < ms)) return cur;
		
		// Not found
		if (largerEqual) end = cur-1;
		else start = cur+1;
	}

	// Couldn't find
	return -1;
}


////////////////////////
// V2 Get time at frame
int VFR_v2::GetTimeAtFrame(int frame) {
	if (Frame.size() > (size_t) frame) return floor(Frame.at(frame));
	return -1;
}


//////////////////
// V2 Get Average
double VFR_v2::GetAverage () {
	double last = 0.0;
	int frames = 0;
	for (std::vector<double>::iterator cur=Frame.begin();cur!=Frame.end();cur++) {
		last = *cur;
		frames++;
	}
	return double(frames)*1000.0/last;
}


//////////////////////// FrameRate //////////////////////
///////////////
// Constructor
FrameRate::FrameRate() {
	loaded = false;
	FrameRateType = NONE;
	AverageFrameRate = 0;
	vfr = NULL;
}


//////////////
// Destructor
FrameRate::~FrameRate() {
	if (vfr) delete vfr;
	vfr = NULL;
	loaded = false;
}


/////////////////////////////
// Gets frame number at time
int FrameRate::GetFrameAtTime(int ms) {
	if (!loaded) return -1;

	if (FrameRateType == CFR) {
		//return int((double(ms)/1000.0) * AverageFrameRate + 0.5);
		return floor((double(ms)/1000.0) * AverageFrameRate);
	}
	else if (FrameRateType == VFR) {
		if (vfr) return vfr->GetFrameAtTime(ms);
		else throw _T("VFR error");
	}
	return -1;
}


//////////////////////
// Gets time at frame
int FrameRate::GetTimeAtFrame(int frame) {
	if (!loaded) return -1;

	if (FrameRateType == CFR) {
		//return int(double(frame) / AverageFrameRate * 1000 + 0.5);
		return floor(double(frame) / AverageFrameRate * 1000);
	}
	else if (FrameRateType == VFR) {
		if (vfr) return vfr->GetTimeAtFrame(frame);
		else throw _T("VFR error");
	}
	return -1;
}


//////////////////
// Loads VFR file
void FrameRate::Load(wxString filename) {
	using namespace std;
	
	// Check if file exists
	wxFileName filetest(filename);
	if (!filetest.FileExists()) throw _T("File not found.");

	// Open file
	ifstream file;
	file.open(filename.mb_str(wxConvLocal));
	if (!file.is_open()) throw _T("Could not open file.");

	// Read header
	char buffer[65536];
	file.getline(buffer,65536);
	wxString header(buffer,wxConvUTF8);
	header.LowerCase();
	header.Trim(true);
	header.Trim(false);
	bool forceV1 = false;
	if (header.Left(17) != _T("# timecode format")) {
		if (header.Left(6) == _T("assume")) forceV1 = true;
		else {
			file.close();
			throw _T("Unknown file format.");
		}
	}

	// V1
	if (forceV1 || header.Mid(18,2) == _T("v1")) {
		// Read "assume" line
		wxString curLine;
		if (!forceV1) {
			file.getline(buffer,65536);
			wxString tmp(buffer,wxConvUTF8);
			curLine = tmp;
		}
		else curLine = header;

		// Process "assume" line
		curLine.LowerCase();
		curLine.Trim(true);
		curLine.Trim(false);
		if (curLine.Left(6) != _T("assume")) {
			file.close();
			throw _T("Error parsing file.");
		}
		double temp;
		curLine.Mid(6).ToDouble(&temp);
		double assume = GetTrueRate(temp);

		// Assigns new VFR file
		if (vfr) delete vfr;
		VFR_v1 *workvfr = new VFR_v1;
		vfr = workvfr;
		FrameRateType = VFR;

		// Reads body
		wxString curline;
		size_t pos;
		size_t end;
		int startf = -1;
		int endf = -1;
		double fps;
		//int n = 0;
		while (!file.eof()) {
			file.getline (buffer,65536);
			wxString wxbuffer (buffer,wxConvUTF8);
			curline = wxbuffer;
			if (curline == _T("")) continue;
			wxString temp;

			// Get start frame
			pos = 0;
			end = curline.find(_T(","),pos);
			//startf = atoi(curline.substr(pos,end-pos).c_str());
			temp = curline.substr(pos,end-pos);
			long templ;
			temp.ToLong(&templ);
			startf = templ;

			// Fill default's blank
			if (endf != startf-1) workvfr->AddRange(endf+1,startf-1,assume,true);

			// Get end frame
			pos = end+1;
			end = curline.find(_T(","),pos);
			//endf = atoi(curline.substr(pos,end-pos).c_str());
			temp = curline.substr(pos,end-pos);
			temp.ToLong(&templ);
			endf = templ;

			// Get fps
			pos = end+1;
			end = curline.find(_T(","),pos);
			//fps = atof(curline.substr(pos,end-pos).c_str());
			temp = curline.substr(pos,end-pos);
			temp.ToDouble(&fps);
			fps = GetTrueRate(fps);
		
			//n++;
			//wxLogMessage(wxString::Format(_T("Range %i added: %i -> %i at %f fps"),n,startf,endf,fps));
			workvfr->AddRange(startf,endf,fps,false);
		}
	}

	// V2
	else if (header.Mid(18,2) == _T("v2")) {
		// Assigns new VFR file
		if (vfr) delete vfr;
		VFR_v2 *workvfr = new VFR_v2;
		vfr = workvfr;
		FrameRateType = VFR;

		// Reads body
		while (!file.eof()) {
			file.getline (buffer,65536);
			if (strcmp(buffer,"") == 0) continue;
			workvfr->AddFrame(atof(buffer));
		}

		// Sorts vector
		sort(workvfr->Frame.begin(),workvfr->Frame.end());
	}

	// Unknown
	else {
		file.close();
		throw _T("Unsupported file version.");
	}

	// Run test
	bool doTest = false;
	if (doTest) {
		int fail = 0;
		int res;
		for (int i=0;i<1000;i++) {
			res = vfr->GetFrameAtTime(vfr->GetTimeAtFrame(i));
			if (res != i) {
				wxLogMessage(wxString::Format(_T("Expected %i but got %i (%i)"),i,res,vfr->GetTimeAtFrame(i)));
				fail++;
			}
		}
		if (fail) wxLogMessage(wxString::Format(_T("Failed %i times"),fail));
		else wxLogMessage(_T("VFR passes test"));
	}

	// Close file
	file.close();
	loaded = true;
	vfrFile = filename;
	AverageFrameRate = vfr->GetAverage();
}


//////////
// Unload
void FrameRate::Unload () {
	FrameRateType = NONE;
	AverageFrameRate = 0;
	if (vfr) delete vfr;
	vfr = NULL;
	loaded = false;
	vfrFile = _T("");
}


///////////////
// Sets to CFR
void FrameRate::SetCFR(double fps,bool ifunset) {
	if (loaded && ifunset) return;

	if (vfr) delete vfr;
	vfr = NULL;
	loaded = true;
	vfrFile = _T("");
	FrameRateType = CFR;
	AverageFrameRate = fps;
}


////////////////////////////
// Improve precision of fps
double FrameRate::GetTrueRate(double rate) {
	//if (rate == 23.976) {
	//	rate = 24.0 / 1.001;
	//}
	//if (rate == 29.97) {
	//	rate = 30.0 / 1.001;
	//}
	return rate;
}


/////////////////////////////
// Get correct frame at time
int FrameRate::CorrectFrameAtTime(int ms,bool start) {
	int frame;

	// CFR
	if (FrameRateType == CFR) {
		int delta = 0;
		if (start) delta = 1;
		frame = GetFrameAtTime(ms-delta)+delta;
		int time = GetTimeAtFrame(frame);
		if (start && time < ms) frame++;
		if (!start && time > ms) frame--;
	}

	// VFR
	else {
		int delta = 0;
		if (!start) delta = -1;
		//frame = GetFrameAtTime(ms-delta)+delta;
		frame = GetFrameAtTime(ms)+delta;
	}

	return frame;
}


/////////////////////////////
// Get correct time at frame
int FrameRate::CorrectTimeAtFrame(int frame,bool start) {
	//int startDelta = 0;
	//if (start) startDelta = -1;
	//int delta = 1;
	//if (FrameRateType == VFR) delta = 1;
	//return GetTimeAtFrame(frame+delta+startDelta)+startDelta;

	int delta = 0;
	if (!start) delta = 1;
	return GetTimeAtFrame(frame+delta);
}


///////////
// Globals
FrameRate VFR_Output;
FrameRate VFR_Input;

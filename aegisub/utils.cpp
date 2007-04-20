// Copyright (c) 2005-2006, Rodrigo Braz Monteiro
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
#include <wx/filename.h>
#include "utils.h"
#ifdef __UNIX__
#include <unistd.h>
#endif


#ifndef __LINUX__
//////////////////////////
// Absolute of 64 bit int
__int64 abs64(__int64 input) {
	if (input < 0) return -input;
	return input;
}
#endif


///////////////////////////////////////
// Count number of matches of a substr
int CountMatches(wxString parent,wxString child) {
	size_t pos = wxString::npos;
	int n = 0;
	while ((pos = parent.find(child,pos+1)) != wxString::npos) n++;
	return n;
}


///////////////
// Copy a file
#ifdef __WINDOWS__
bool Copy(wxString src,wxString dst) {
	BOOL result = CopyFile(src.wc_str(),dst.wc_str(),false);
	return (result != 0);
}
#endif

////////////////
// Backup a file
bool Backup(wxString src,wxString dst) {
	// Windows
	#if defined(__WINDOWS__)
	return Copy(src,dst);

	// Linux
	#elif defined(__UNIX__)
	return link(src.mb_str(),dst.mb_str()) != 0;

	// Error
	#else
	#error "don't know how to backup files"
	#endif
}

/////////////////////////////////////
// Make a path relative to reference
wxString MakeRelativePath(wxString _path,wxString reference) {
	if (_path.IsEmpty()) return _T("");
	if (_path.Left(1) == _T("?")) return _path;
	wxFileName path(_path);
	wxFileName refPath(reference);
	path.MakeRelativeTo(refPath.GetPath());
	return path.GetFullPath();
}


///////////////////////////////////////
// Extract original path from relative
wxString DecodeRelativePath(wxString _path,wxString reference) {
	if (_path.IsEmpty()) return _T("");
	if (_path.Left(1) == _T("?")) return _path;
	wxFileName path(_path);
	wxFileName refPath(reference);
	if (!path.IsAbsolute()) path.MakeAbsolute(refPath.GetPath());
#ifdef __UNIX__
	return path.GetFullPath(wxPATH_UNIX); // also works on windows
										  // No, it doesn't, this ommits drive letter in Windows. ~ amz
#else
	return path.GetFullPath();
#endif
}


////////////////
// Pretty float
wxString PrettyFloat(wxString src) {
	if (src.Contains(_T("."))) {
		size_t len = src.Length();
		while (src.Right(1) == _T("0")) {
			len--;
			src.Truncate(len);
		}
		if (src.Right(1) == _T(".")) {
			len--;
			src.Truncate(len);
		}
	}
	return src;
}

wxString PrettyFloatF(float src) { return PrettyFloat(wxString::Format(_T("%f"),src)); }
wxString PrettyFloatD(double src) { return PrettyFloat(wxString::Format(_T("%f"),src)); }


///////////////////
// Float to string
wxString FloatToString(double value) {
	return PrettyFloat(wxString::Format(_T("%f"),value));
}


/////////////////
// Int to string
wxString IntegerToString(int value) {
	return wxString::Format(_T("%i"),value);
}


//////////////////////////
// Pretty reading of size
// There shall be no kiB, MiB stuff here
wxString PrettySize(int bytes) {
	// Suffixes
	wxArrayString suffix;
	suffix.Add(_T(""));
	suffix.Add(_T(" kB"));
	suffix.Add(_T(" MB"));
	suffix.Add(_T(" GB"));
	suffix.Add(_T(" TB"));
	suffix.Add(_T(" PB"));

	// Set size
	int i = 0;
	double size = bytes;
	while (size > 1024) {
		size = size / 1024.0;
		i++;
		if (i == 6) {
			i--;
			break;
		}
	}
	
	// Set number of decimal places
	wxString final;
	if (size < 10) final = wxString::Format(_T("%.2f"),size);
	else if (size < 100) final = wxString::Format(_T("%.1f"),size);
	else final = wxString::Format(_T("%.0f"),size);
	return final + suffix[i];
}


//////////////////////////////////
// Append a menu item with bitmap
wxMenuItem* AppendBitmapMenuItem (wxMenu* parentMenu,int id,wxString text,wxString help,wxBitmap bmp,int pos) {
	wxMenuItem *cur = new wxMenuItem(parentMenu,id,text,help);
	cur->SetBitmap(bmp);
	if (pos == -1) parentMenu->Append(cur);
	else parentMenu->Insert(pos,cur);
	return cur;
}


///////////////////////////////////////////////////////////////
// Get the smallest power of two that is greater or equal to x
// Code from http://bob.allegronetwork.com/prog/tricks.html
int SmallestPowerOf2(int x) {
	x--;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	x++;
	return x;
}


///////////////////////
// Get word boundaries
void GetWordBoundaries(const wxString text,IntPairVector &results,int start,int end) {
	// Variables
	wxChar cur;
	int curPos;
	int lastpos = -1;
	int depth = 0;
	if (end < 0) end = text.Length();
	bool isDelim;

	// Delimiters
	wxString delim = _T(" .,;:!?-(){}[]\"/\\");
	wxChar temp = 0xBF;
	delim += temp;
	temp = 0xA1;
	delim += temp;

	// Scan
	for (int i=start;i<end+1;i++) {
		// Current character
		curPos = i;
		if (i < end) cur = text[i];
		else cur = '.';
		isDelim = false;

		// Increase depth
		if (cur == '{') {
			depth++;
			if (depth == 1) {
				if (lastpos+1 != curPos) {
					results.push_back(std::pair<int,int>(lastpos+1,curPos));
				}
				continue;
			}
		}

		// Decrease depth
		if (cur == '}') {
			depth--;
			if (depth == 0) {
				lastpos = i;
				continue;
			}
		}

		// Wrong depth
		if (depth != 0) continue;

		// Check if it is \n or \N
		if (cur == '\\' && i < end-1 && (text[i+1] == 'N' || text[i+1] == 'n' || text[i+1] == 'h')) {
			isDelim = true;
			i++;
		}

		// Check for standard delimiters
		if (delim.Find(cur) != wxNOT_FOUND) {
			isDelim = true;
		}

		// Is delimiter?
		if (isDelim) {
			if (lastpos+1 != curPos) {
				results.push_back(std::pair<int,int>(lastpos+1,curPos));
			}
			lastpos = i;
		}
	}
}


/////////////////////
// String to integer
// wxString::ToLong() is slow and not as flexible
int StringToInt(const wxString &str,size_t start,size_t end) {
	// Initialize to zero and get length if end set to -1
	int sign = 1;
	int value = 0;
	if (end == -1) end = str.Length();

	for (size_t pos=start;pos<end;pos++) {
		// Get value and check if it's a number
		int val = (int)(str[pos]);
		if (val == _T(' ') || val == _T('\t')) continue;
		if (val == _T('-')) sign = -1;
		if (val < _T('0') || val > _T('9')) break;

		// Shift value to next decimal place and increment the value just read
		value = value * 10 + (val - _T('0'));
	}

	return value*sign;
}



/////////////////////////
// String to fixed point
int StringToFix(const wxString &str,size_t decimalPlaces,size_t start,size_t end) {
	// Parts of the number
	int sign = 1;
	int major = 0;
	int minor = 0;
	if (end == -1) end = str.Length();
	bool inMinor = false;
	int *dst = &major;
	size_t mCount = 0;

	for (size_t pos=start;pos<end;pos++) {
		// Get value and check if it's a number
		int val = (int)(str[pos]);
		if (val == _T(' ') || val == _T('\t')) continue;
		if (val == _T('-')) sign = -1;

		// Switch to minor
		if (val == _T('.') || val == _T(',')) {
			if (inMinor) break;
			inMinor = true;
			dst = &minor;
			mCount = 0;
			continue;
		}
		if (val < _T('0') || val > _T('9')) break;
		*dst = (*dst * 10) + (val - _T('0'));
		mCount++;
	}

	// Change minor to have the right number of decimal places
	while (mCount > decimalPlaces) {
		minor /= 10;
		mCount--;
	}
	while (mCount < decimalPlaces) {
		minor *= 10;
		mCount++;
	}

	// Shift major and return
	for (size_t i=0;i<decimalPlaces;i++) major *= 10;
	return (major + minor)*sign;
}

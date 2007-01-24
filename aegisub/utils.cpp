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
void AppendBitmapMenuItem (wxMenu* parentMenu,int id,wxString text,wxString help,wxBitmap bmp) {
	wxMenuItem *cur = new wxMenuItem(parentMenu,id,text,help);
	cur->SetBitmap(bmp);
	parentMenu->Append(cur);
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

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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file utils.cpp
/// @brief Misc. utility functions
/// @ingroup utility
///

#include "config.h"

#include "utils.h"

#ifndef AGI_PRE
#ifdef __UNIX__
#include <unistd.h>
#endif
#include <map>

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/window.h>
#endif

#include <libaegisub/log.h>

#ifdef __APPLE__
#include <libaegisub/util_osx.h>
#endif

#include "compat.h"

wxString MakeRelativePath(wxString _path, wxString reference) {
	if (_path.empty() || _path[0] == '?') return _path;
	wxFileName path(_path);
	wxFileName refPath(reference);
	path.MakeRelativeTo(refPath.GetPath());
	return path.GetFullPath();
}

wxString DecodeRelativePath(wxString _path,wxString reference) {
	if (_path.empty() || _path[0] == '?') return _path;
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

wxString AegiFloatToString(double value) {
	return wxString::Format("%g",value);
}

wxString AegiIntegerToString(int value) {
	return wxString::Format("%i",value);
}

/// @brief There shall be no kiB, MiB stuff here Pretty reading of size
wxString PrettySize(int bytes) {
	const char *suffix[] = { "", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };

	// Set size
	int i = 0;
	double size = bytes;
	while (size > 1024 && i < 9) {
		size /= 1024.0;
		i++;
	}

	// Set number of decimal places
	const char *fmt = "%.0f";
	if (size < 10)
		fmt = "%.2f";
	else if (size < 100)
		fmt = "%1.f";
	return wxString::Format(fmt, size) + " " + suffix[i];
}

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

void GetWordBoundaries(wxString const& text, IntPairVector &results, int start, int end) {
	int depth = 0;
	bool in_draw_mode = false;
	if (end < 0) end = text.size();

	// Delimiters
	const wxUniChar delims[] = {
		0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0028,
		0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 0x003a,
		0x003b, 0x003d, 0x003f, 0x0040, 0x005b, 0x005c, 0x005d, 0x005e,
		0x005f, 0x0060, 0x007b, 0x007c, 0x007d, 0x007e, 0x00a1, 0x00a2,
		0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00aa, 0x00ab,
		0x00b0, 0x00b6, 0x00b7, 0x00ba, 0x00bb, 0x00bf, 0x02dc, 0x0e3f,
		0x2010, 0x2013, 0x2014, 0x2015, 0x2018, 0x2019, 0x201c, 0x201d,
		0x2020, 0x2021, 0x2022, 0x2025, 0x2026, 0x2026, 0x2030, 0x2031,
		0x2032, 0x203b, 0x203b, 0x203d, 0x2042, 0x2044, 0x20a6, 0x20a9,
		0x20aa, 0x20ac, 0x20ad, 0x2116, 0x2234, 0x2235, 0x2420, 0x2422,
		0x2423, 0x2506, 0x25ca, 0x2605, 0x261e, 0x2e2e, 0x3000, 0x3001,
		0x3002, 0x3008, 0x3009, 0x300a, 0x300b, 0x300c, 0x300d, 0x300e,
		0x300f, 0x3010, 0x3011, 0x3014, 0x3015, 0x3016, 0x3017, 0x3018,
		0x3019, 0x301a, 0x301b, 0x301c, 0x3030, 0x303d, 0x30fb, 0xff0a,
		0xff5b, 0xff5d, 0xff5e
	};

	for (int i = start; i < end + 1; ++i) {
		// Current character
		wxUniChar cur = i < end ? text[i] : wxUniChar('.');

		// Increase depth
		if (cur == '{') {
			depth++;
			if (depth == 1 && start != i && !in_draw_mode)
				results.push_back(std::make_pair(start, i));
		}
		// Decrease depth
		else if (cur == '}') {
			depth--;
			start = i + 1;
		}
		else if (depth > 0) {
			// Check for draw mode
			if (cur == '\\' && i + 1 < end && text[i + 1] == 'p') {
				i += 2;

				// Eat leading zeros
				while (i < end && text[i] == '0') ++i;

				in_draw_mode = i < end && text[i] >= '0' && text[i] <= '9';
				if (!in_draw_mode) --i;
			}
		}
		else if (!in_draw_mode) {
			// Check if it is \n or \N
			if (cur == '\\' && i < end-1 && (text[i+1] == 'N' || text[i+1] == 'n' || text[i+1] == 'h')) {
				if (start != i)
					results.push_back(std::make_pair(start, i));
				start = i + 2;
				i++;
			}
			// Check for standard delimiters
			else if (std::binary_search(delims, delims + sizeof(delims) / sizeof(delims[0]), cur)) {
				if (start != i)
					results.push_back(std::make_pair(start, i));
				start = i + 1;
			}
		}
	}
}

bool IsWhitespace(wchar_t c)
{
	const wchar_t whitespaces[] = {
		// http://en.wikipedia.org/wiki/Space_(punctuation)#Table_of_spaces
		0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x0020, 0x0085, 0x00A0,
		0x1680, 0x180E, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005,
		0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x2028, 0x2029, 0x202F,
		0x025F, 0x3000, 0xFEFF
	};
	const size_t num_chars = sizeof(whitespaces) / sizeof(whitespaces[0]);
	return std::binary_search(whitespaces, whitespaces + num_chars, c);
}

bool StringEmptyOrWhitespace(const wxString &str)
{
	for (size_t i = 0; i < str.size(); ++i)
		if (!IsWhitespace(str[i]))
			return false;

	return true;
}

int AegiStringToInt(const wxString &str,int start,int end) {
	// Initialize to zero and get length if end set to -1
	int sign = 1;
	int value = 0;
	if (end == -1) end = str.Length();

	for (int pos=start;pos<end;pos++) {
		// Get value and check if it's a number
		int val = (int)(str[pos]);
		if (val == ' ' || val == '\t') continue;
		if (val == '-') sign = -1;
		if (val < '0' || val > '9') break;

		// Shift value to next decimal place and increment the value just read
		value = value * 10 + (val - '0');
	}

	return value*sign;
}

int AegiStringToFix(const wxString &str,size_t decimalPlaces,int start,int end) {
	// Parts of the number
	int sign = 1;
	int major = 0;
	int minor = 0;
	if (end == -1) end = str.Length();
	bool inMinor = false;
	int *dst = &major;
	size_t mCount = 0;

	for (int pos=start;pos<end;pos++) {
		// Get value and check if it's a number
		int val = (int)(str[pos]);
		if (val == ' ' || val == '\t') continue;
		if (val == '-') sign = -1;

		// Switch to minor
		if (val == '.' || val == ',') {
			if (inMinor) break;
			inMinor = true;
			dst = &minor;
			mCount = 0;
			continue;
		}
		if (val < '0' || val > '9') break;
		*dst = (*dst * 10) + (val - '0');
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

void RestartAegisub() {
#if defined(__WXMSW__)
	wxStandardPaths stand;
	wxExecute("\"" + stand.GetExecutablePath() + "\"");
#elif defined(__WXMAC__)
	char *bundle_path = agi::util::OSX_GetBundlePath();
	char *support_path = agi::util::OSX_GetBundleSupportFilesDirectory();
	if (!bundle_path || !support_path) return; // oops
	wxString exec = wxString::Format("\"%s/MacOS/restart-helper\" /usr/bin/open -n \"%s\"'", wxString(support_path, wxConvUTF8), wxString(bundle_path, wxConvUTF8));
	LOG_I("util/restart/exec") << exec;
	wxExecute(exec);
	free(bundle_path);
	free(support_path);
#else
	wxStandardPaths stand;
	wxExecute(stand.GetExecutablePath());
#endif
}

bool ForwardMouseWheelEvent(wxWindow *source, wxMouseEvent &evt) {
	wxWindow *target = wxFindWindowAtPoint(wxGetMousePosition());
	if (!target || target == source) return true;

	// If the mouse is over a parent of the source window just pretend it's
	// over the source window, so that the mouse wheel works on borders and such
	wxWindow *parent = source->GetParent();
	while (parent && parent != target) parent = parent->GetParent();
	if (parent == target) return true;

	// Otherwise send it to the new target
	target->GetEventHandler()->ProcessEvent(evt);
	evt.Skip(false);
	return false;
}

namespace {
class cache_cleaner : public wxThread {
	wxString directory;
	wxString file_type;
	int64_t max_size;
	size_t max_files;

	ExitCode Entry() {
		static wxMutex cleaning_mutex;
		wxMutexLocker lock(cleaning_mutex);

		if (!lock.IsOk()) {
			LOG_D("utils/clean_cache") << "cleaning already in progress, thread exiting";
			return (ExitCode)1;
		}

		wxDir cachedir;
		if (!cachedir.Open(directory)) {
			LOG_D("utils/clean_cache") << "couldn't open cache directory " << STD_STR(directory);
			return (wxThread::ExitCode)1;
		}

		// sleep for a bit so we (hopefully) don't thrash the disk too much while indexing is in progress
		wxThread::This()->Sleep(2000);

		if (!cachedir.HasFiles(file_type)) {
			LOG_D("utils/clean_cache") << "no files of the checked type in directory, exiting";
			return (wxThread::ExitCode)0;
		}

		// unusually paranoid sanity check
		// (someone might have deleted the file(s) after we did HasFiles() above; does wxDir.Open() lock the dir?)
		wxString curfn_str;
		if (!cachedir.GetFirst(&curfn_str, file_type, wxDIR_FILES)) {
			LOG_D("utils/clean_cache") << "undefined error";
			return (wxThread::ExitCode)1;
		}

		int64_t total_size = 0;
		std::multimap<int64_t,wxFileName> cachefiles;
		do {
			wxFileName curfn(directory, curfn_str);
			wxDateTime curatime;
			curfn.GetTimes(&curatime, NULL, NULL);
			cachefiles.insert(std::make_pair(curatime.GetTicks(), curfn));
			total_size += curfn.GetSize().GetValue();

			wxThread::This()->Sleep(250);
		} while (cachedir.GetNext(&curfn_str));

		if (cachefiles.size() <= max_files && total_size <= max_size) {
			LOG_D("utils/clean_cache")
				<< "cache does not need cleaning (maxsize=" << max_size
				<< ", cursize=" << total_size
				<< "; maxfiles=" << max_files
				<< ", numfiles=" << cachefiles.size()
				<< "), exiting";
			return (wxThread::ExitCode)0;
		}

		int deleted = 0;
		for (std::multimap<int64_t,wxFileName>::iterator i = cachefiles.begin(); i != cachefiles.end(); i++) {
			// stop cleaning?
			if ((total_size <= max_size && cachefiles.size() - deleted <= max_files) || cachefiles.size() - deleted < 2)
				break;

			int64_t fsize = i->second.GetSize().GetValue();
			if (!wxRemoveFile(i->second.GetFullPath())) {
				LOG_D("utils/clean_cache") << "failed to remove file " << STD_STR(i->second.GetFullPath());
				continue;
			}

			total_size -= fsize;
			++deleted;

			wxThread::This()->Sleep(250);
		}

		LOG_D("utils/clean_cache") << "deleted " << deleted << " files, exiting";
		return (wxThread::ExitCode)0;
	}

public:
	cache_cleaner(wxString const& directory, wxString const& file_type, int64_t max_size, int64_t max_files)
	: wxThread(wxTHREAD_DETACHED)
	, directory(directory)
	, file_type(file_type)
	, max_size(max_size << 20)
	{
		if (max_files < 1)
			this->max_files = (size_t)-1;
		else
			this->max_files = (size_t)max_files;
	}
};
}

void CleanCache(wxString const& directory, wxString const& file_type, int64_t max_size, int64_t max_files) {
	LOG_D("utils/clean_cache") << "attempting to start cleaner thread";
	wxThread *CleaningThread = new cache_cleaner(directory, file_type, max_size, max_files);

	if (CleaningThread->Create() != wxTHREAD_NO_ERROR) {
		LOG_D("utils/clean_cache") << "thread creation failed";
		delete CleaningThread;
	}
	else if (CleaningThread->Run() != wxTHREAD_NO_ERROR) {
		LOG_D("utils/clean_cache") << "failed to start thread";
		delete CleaningThread;
	}

	LOG_D("utils/clean_cache") << "thread started successfully";
}

// OS X implementation in osx_utils.mm
#ifndef __WXOSX_COCOA__
void AddFullScreenButton(wxWindow *) { }
void SetFloatOnParent(wxWindow *) { }
#endif

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

/// @file utils.cpp
/// @brief Misc. utility functions
/// @ingroup utility
///

#include "config.h"

#include "utils.h"

#include "compat.h"
#include "options.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/fs.h>
#include <libaegisub/log.h>

#ifdef __UNIX__
#include <unistd.h>
#endif
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <map>

#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/window.h>

#ifdef __APPLE__
#include <libaegisub/util_osx.h>
#endif

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
	size_t i = 0;
	double size = bytes;
	while (size > 1024 && i + 1 < sizeof(suffix) / sizeof(suffix[0])) {
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

bool IsWhitespace(wchar_t c) {
	const wchar_t whitespaces[] = {
		// http://en.wikipedia.org/wiki/Space_(punctuation)#Table_of_spaces
		0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x0020, 0x0085, 0x00A0,
		0x1680, 0x180E, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005,
		0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x2028, 0x2029, 0x202F,
		0x025F, 0x3000, 0xFEFF
	};
	return std::binary_search(whitespaces, std::end(whitespaces), c);
}

bool StringEmptyOrWhitespace(const wxString &str) {
	return std::all_of(str.begin(), str.end(), IsWhitespace);
}

void RestartAegisub() {
	config::opt->Flush();

#if defined(__WXMSW__)
	wxStandardPaths stand;
	wxExecute("\"" + stand.GetExecutablePath() + "\"");
#elif defined(__WXMAC__)
	std::string bundle_path = agi::util::OSX_GetBundlePath();
	std::string helper_path = agi::util::OSX_GetBundleAuxillaryExecutablePath("restart-helper");
	if (bundle_path.empty() || helper_path.empty()) return;

	wxString exec = wxString::Format("\"%s\" /usr/bin/open -n \"%s\"'", to_wx(helper_path), to_wx(bundle_path));
	LOG_I("util/restart/exec") << exec;
	wxExecute(exec);
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

std::string GetClipboard() {
	wxString data;
	wxClipboard *cb = wxClipboard::Get();
	if (cb->Open()) {
		if (cb->IsSupported(wxDF_TEXT)) {
			wxTextDataObject raw_data;
			cb->GetData(raw_data);
			data = raw_data.GetText();
		}
		cb->Close();
	}
	return from_wx(data);
}

void SetClipboard(std::string const& new_data) {
	wxClipboard *cb = wxClipboard::Get();
	if (cb->Open()) {
		cb->SetData(new wxTextDataObject(to_wx(new_data)));
		cb->Close();
		cb->Flush();
	}
}

void SetClipboard(wxBitmap const& new_data) {
	wxClipboard *cb = wxClipboard::Get();
	if (cb->Open()) {
		cb->SetData(new wxBitmapDataObject(new_data));
		cb->Close();
		cb->Flush();
	}
}

void CleanCache(agi::fs::path const& directory, std::string const& file_type, uint64_t max_size, uint64_t max_files) {
	static std::unique_ptr<agi::dispatch::Queue> queue;
	if (!queue)
		queue = agi::dispatch::Create();

	max_size <<= 20;
	if (max_files == 0)
		max_files = std::numeric_limits<uint64_t>::max();
	queue->Async([=]{
		LOG_D("utils/clean_cache") << "cleaning " << directory/file_type;
		uint64_t total_size = 0;
		std::multimap<int64_t, agi::fs::path> cachefiles;
		for (auto const& file : agi::fs::DirectoryIterator(directory, file_type)) {
			agi::fs::path path = directory/file;
			cachefiles.insert(std::make_pair(agi::fs::ModifiedTime(path), path));
			total_size += agi::fs::Size(path);
		}

		if (cachefiles.size() <= max_files && total_size <= max_size) {
			LOG_D("utils/clean_cache") << boost::format("cache does not need cleaning (maxsize=%d, cursize=%d, maxfiles=%d, numfiles=%d), exiting")
				% max_size % total_size % max_files % cachefiles.size();
			return;
		}

		int deleted = 0;
		for (auto const& i : cachefiles) {
			// stop cleaning?
			if ((total_size <= max_size && cachefiles.size() - deleted <= max_files) || cachefiles.size() - deleted < 2)
				break;

			uint64_t size = agi::fs::Size(i.second);
			try {
				agi::fs::Remove(i.second);
				LOG_D("utils/clean_cache") << "deleted " << i.second;
			}
			catch  (agi::Exception const& e) {
				LOG_D("utils/clean_cache") << "failed to delete file " << i.second << ": " << e.GetChainedMessage();
				continue;
			}

			total_size -= size;
			++deleted;
		}

		LOG_D("utils/clean_cache") << "deleted " << deleted << " files, exiting";
	});
}

size_t MaxLineLength(std::string const& text) {
	size_t max_line_length = 0;
	size_t current_line_length = 0;
	bool last_was_slash = false;
	bool in_ovr = false;

	for (auto const& c : text) {
		if (in_ovr) {
			in_ovr = c != '}';
			continue;
		}

		if (c == '\\') {
			current_line_length += last_was_slash; // for the slash before this one
			last_was_slash = true;
			continue;
		}

		if (last_was_slash) {
			last_was_slash = false;
			if (c == 'h') {
				++current_line_length;
				continue;
			}

			if (c == 'n' || c == 'N') {
				max_line_length = std::max(max_line_length, current_line_length);
				current_line_length = 0;
				continue;
			}

			// Not actually an escape so add the character for the slash and fall through
			++current_line_length;
		}

		if (c == '{')
			in_ovr = true;
		else
			++current_line_length;
	}

	current_line_length += last_was_slash;
	return std::max(max_line_length, current_line_length);
}

// OS X implementation in osx_utils.mm
#ifndef __WXOSX_COCOA__
void AddFullScreenButton(wxWindow *) { }
void SetFloatOnParent(wxWindow *) { }
#endif


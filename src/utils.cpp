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

#include "utils.h"

#include "compat.h"
#include "frame_main.h"
#include "options.h"
#include "retina_helper.h"

#include <libaegisub/ass/dialogue_parser.h>
#include <libaegisub/dispatch.h>
#include <libaegisub/fs.h>
#include <libaegisub/log.h>

#ifdef __UNIX__
#include <unistd.h>
#endif
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/locale/boundary.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <map>
#include <unicode/uchar.h>
#include <unicode/utf8.h>

#include <wx/clipbrd.h>
#include <wx/filedlg.h>
#include <wx/stdpaths.h>
#include <wx/window.h>

#ifdef __APPLE__
#include <libaegisub/util_osx.h>
#include <CoreText/CTFont.h>
#endif

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

std::string float_to_string(double val) {
	std::string s = str(boost::format("%.3f") % val);
	size_t pos = s.find_last_not_of("0");
	if (pos != s.find(".")) ++pos;
	s.erase(begin(s) + pos, end(s));
	return s;
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

void RestartAegisub() {
	config::opt->Flush();

#if defined(__WXMSW__)
	wxExecute("\"" + wxStandardPaths::Get().GetExecutablePath() + "\"");
#elif defined(__WXMAC__)
	std::string bundle_path = agi::util::GetBundlePath();
	std::string helper_path = agi::util::GetBundleAuxillaryExecutablePath("restart-helper");
	if (bundle_path.empty() || helper_path.empty()) return;

	wxString exec = wxString::Format("\"%s\" /usr/bin/open -n \"%s\"'", to_wx(helper_path), to_wx(bundle_path));
	LOG_I("util/restart/exec") << exec;
	wxExecute(exec);
#else
	wxExecute(wxStandardPaths::Get().GetExecutablePath());
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

size_t MaxLineLength(std::string const& text, bool ignore_whitespace) {
	auto tokens = agi::ass::TokenizeDialogueBody(text);
	agi::ass::MarkDrawings(text, tokens);

	size_t pos = 0;
	size_t max_line_length = 0;
	size_t current_line_length = 0;
	for (auto token : tokens) {
		if (token.type == agi::ass::DialogueTokenType::LINE_BREAK) {
			if (text[pos + 1] == 'h') {
				if (!ignore_whitespace)
					current_line_length += 1;
			}
			else { // N or n
				max_line_length = std::max(max_line_length, current_line_length);
				current_line_length = 0;
			}
		}
		else if (token.type == agi::ass::DialogueTokenType::TEXT) {
			using namespace boost::locale::boundary;
			const ssegment_index characters(character, begin(text) + pos, begin(text) + pos + token.length);
			if (!ignore_whitespace)
				current_line_length += boost::distance(characters);
			else {
				// characters.rule(word_any) doesn't seem to work for character indexes (everything is word_none)
				for (auto const& chr : characters) {
					UChar32 c;
					int i = 0;
					U8_NEXT_UNSAFE(chr.begin(), i, c);
					if (!u_isUWhiteSpace(c))
						++current_line_length;
				}
			}
		}

		pos += token.length;
	}

	return std::max(max_line_length, current_line_length);
}

#ifndef __WXOSX_COCOA__
// OS X implementation in osx_utils.mm
void AddFullScreenButton(wxWindow *) { }
void SetFloatOnParent(wxWindow *) { }

// OS X implementation in retina_helper.mm
RetinaHelper::RetinaHelper(wxWindow *) { }
RetinaHelper::~RetinaHelper() { }
int RetinaHelper::GetScaleFactor() const { return 1; }
#endif

wxString FontFace(std::string opt_prefix) {
	opt_prefix += "/Font Face";
	auto value = OPT_GET(opt_prefix)->GetString();
#ifdef __WXOSX_COCOA__
	if (value.empty()) {
		auto default_font = CTFontCreateUIFontForLanguage(kCTFontUserFontType, 0, nullptr);
		auto default_font_name = CTFontCopyPostScriptName(default_font);
		CFRelease(default_font);

		auto utf8_str = CFStringGetCStringPtr(default_font_name, kCFStringEncodingUTF8);
		if (utf8_str)
			value = utf8_str;
		else {
			char buffer[1024];
			CFStringGetCString(default_font_name, buffer, sizeof(buffer), kCFStringEncodingUTF8);
			buffer[1023] = '\0';
			value = buffer;
		}

		CFRelease(default_font_name);
	}
#endif
	return to_wx(value);
}

agi::fs::path FileSelector(wxString const& message, std::string const& option_name, std::string const& default_filename, std::string const& default_extension, wxString const& wildcard, int flags, wxWindow *parent) {
	wxString path;
	if (!option_name.empty())
		path = to_wx(OPT_GET(option_name)->GetString());
	agi::fs::path filename = wxFileSelector(message, path, to_wx(default_filename), to_wx(default_extension), wildcard, flags, parent).wx_str();
	if (!filename.empty() && !option_name.empty())
		OPT_SET(option_name)->SetString(filename.parent_path().string());
	return filename;
}

agi::fs::path OpenFileSelector(wxString const& message, std::string const& option_name, std::string const& default_filename, std::string const& default_extension, wxString const& wildcard, wxWindow *parent) {
	return FileSelector(message, option_name, default_filename, default_extension, wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST, parent);
}

agi::fs::path SaveFileSelector(wxString const& message, std::string const& option_name, std::string const& default_filename, std::string const& default_extension, wxString const& wildcard, wxWindow *parent) {
	return FileSelector(message, option_name, default_filename, default_extension, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT, parent);
}

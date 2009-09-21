// Copyright (c) 2009, Amar Takhar <verm@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @@file upload.h
/// @see upload.cpp

#ifndef R_PRECMP
#include <curl/curl.h>

#include <wx/file.h>
#endif

#include "progress.h"

/// @class Upload
/// @brief Upload files to the server.
class Upload {
private:
	/// Curl handle.
	CURL *handle;

	/// Progress instance.
	Progress *progress;
	static int CBProgress(void *p, double dt, double dc, double ut, double uc);
	static size_t CBRead(char *p, size_t size, size_t nmemb, void *filep);
	void Error(CURLcode error);
public:
	Upload(Progress *prog);
	~Upload();
	void Report(wxString report);
	bool SendFile(const char *url, wxFile &file);
};

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

/// @file upload.cpp
/// @brief Handle uploading of data.
/// @ingroup base io

#ifndef R_PRECOMP
#include <stdint.h>
#include <stdlib.h>

#include <wx/file.h>
#endif

#include "upload.h"

/// @brief Constructor.
Upload::Upload(Progress *prog) {
	/// XXX: error checking.
	progress = prog;
	handle = curl_easy_init();
//	curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(handle, CURLOPT_USERAGENT, "Reporter v1.0");
	curl_easy_setopt(handle, CURLOPT_READFUNCTION, CBRead);
	curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, CBProgress);
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, progress);
}


/// @brief Destructor.
Upload::~Upload() {
	curl_free(handle);
}


/// @brief Submit a JSON report.
/// @param report filename of the report.
void Upload::Report(wxString report) {
	wxFile file(report, wxFile::read);
	SendFile("http://reporter.darkbeer.org/PUT/", file);
}


/// @brief Progress callback.
/// @param p   pointer to progress info.
/// @param dlt download size.
/// @param dln downloaded.
/// @param ult upload size.
/// @param uln uploaded.
int Upload::CBProgress(void *p, double dlt, double dln, double ult, double uln) {

	if (uln > 0) {
		Progress *progress = (Progress*) p;
		// Update returns false if the user has hit abort.
		if (!progress->Update(floor(ult / uln + 0.5) * 100))
			// Returning non-zero will cause curl to abort the transfer.
			return 1;
	}
	return 0;
}


/// @brief Callback to read data from a FD.
/// @param p buffer to fill.
/// @param size byte length.
/// @param nmemb object size.
/// @param filep FP to read from.
size_t Upload::CBRead(char *p, size_t size, size_t nmemb, void *filep) {
	// This is on purpose to wx doesn't close the fp, curl does that for us.
	wxFile *file = new wxFile((uintptr_t)filep);

	if (file->Eof())
		return 0;

	int ret = file->Read(p, file->Length());
	if (ret == wxInvalidOffset)
		return CURL_READFUNC_ABORT;

	return ret;
}


/// @brief Send a file to the server
/// @param file file to send.
/// @return 1/0 for success/failure.
bool Upload::SendFile(const char *url, wxFile &file) {
	progress->Update(0, _("Connecting..."));
	curl_easy_setopt(handle, CURLOPT_URL, url);
	Error(curl_easy_perform(handle));

	curl_easy_setopt(handle, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(handle, CURLOPT_PUT, 1L);
	curl_easy_setopt(handle, CURLOPT_READDATA, file.fd());
	curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file.Length());
	CURLcode res = curl_easy_perform(handle);
	Error(res);

	curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L); // Reset to HTTP GET
	curl_easy_cleanup(handle);
	if (res == CURLE_OK)
		return 1;

	return 0;
}


/// @brief Handle error codes
/// @param error CURLcode.
void Upload::Error(CURLcode error) {
	switch (error) {
		case CURLE_OK:
			progress->Update(0, _("Sending data..."));
			break;
		case CURLE_COULDNT_RESOLVE_HOST:
			progress->Update(0, _("Couldn't resolve host."));
			break;
		case CURLE_COULDNT_CONNECT:
			progress->Update(0, _("Couldn't connect to server."));
			break;

		case CURLE_SEND_ERROR:			// socket error
		case CURLE_RECV_ERROR:			// socket error
			progress->Update(0, _("Connection error."));
			break;
		case CURLE_GOT_NOTHING:			// no response from server
		case CURLE_HTTP_RETURNED_ERROR:	// HTTP error >= 400
			progress->Update(0, _("Server error."));
			break;
		case CURLE_OPERATION_TIMEDOUT:
			progress->Update(0, _("Operation timeout."));
			break;
		default:
			progress->Update(0, wxString::Format("Transfer error. (%d)\n", error));
	}
}

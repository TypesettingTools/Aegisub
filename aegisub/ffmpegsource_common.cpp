// Copyright (c) 2008, Karl Blomster
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

#include "config.h"

#ifdef WITH_FFMPEGSOURCE

///////////
// Headers
#include "ffmpegsource_common.h"
#include "md5.h"
#include "standard_paths.h"
#include "main.h"
#include "frame_main.h"

///////////////
// Update indexing progress
int FFMS_CC FFmpegSourceProvider::UpdateIndexingProgress(int State, int64_t Current, int64_t Total, void *Private) {
	IndexingProgressDialog *Progress = (IndexingProgressDialog *)Private;

	if (Progress->IndexingCanceled)
		return 1;

	// noone cares about a little bit of a rounding error here anyway
	Progress->ProgressDialog->SetProgress((1000*Current)/Total, 1000);
	
	return 0;
}


///////////
// Do indexing
FrameIndex *FFmpegSourceProvider::DoIndexing(FrameIndex *Index, wxString FileNameWX, wxString CacheName, int Trackmask, bool IgnoreDecodeErrors) {
	char FFMSErrMsg[1024];
	unsigned MsgSize = sizeof(FFMSErrMsg);
	wxString MsgString;

	// set up progress dialog callback
	IndexingProgressDialog Progress;
	Progress.IndexingCanceled = false;
	Progress.ProgressDialog = new DialogProgress(AegisubApp::Get()->frame, _("Indexing"), &Progress.IndexingCanceled, _("Reading timecodes and frame/sample data"), 0, 1);
	Progress.ProgressDialog->Show();
	Progress.ProgressDialog->SetProgress(0,1);

	// index all audio tracks
	Index = FFMS_MakeIndex(FileNameWX.mb_str(wxConvLocal), Trackmask, FFMSTrackMaskNone, NULL, IgnoreDecodeErrors, FFmpegSourceProvider::UpdateIndexingProgress, &Progress, FFMSErrMsg, MsgSize);
	if (!Index) {
		Progress.ProgressDialog->Destroy();
		wxString temp(FFMSErrMsg, wxConvUTF8);
		MsgString << _T("Failed to index: ") << temp;
		throw MsgString;
	}
	Progress.ProgressDialog->Destroy();

	// write index to disk for later use
	// ignore write errors for now
	FFMS_WriteIndex(CacheName.char_str(), Index, FFMSErrMsg, MsgSize);
	/*if (FFMS_WriteIndex(CacheName.char_str(), Index, FFMSErrMsg, MsgSize)) {
		wxString temp(FFMSErrMsg, wxConvUTF8);
		MsgString << _T("Failed to write index: ") << temp;
		throw MsgString;
	} */

	return Index;
}

/////////////////////
// Creates a name for the ffmpegsource2 index and prepares the folder if it doesn't exist
// method by amz
wxString FFmpegSourceProvider::GetCacheFilename(const wxString& filename)
{
	// Get the size of the file to be hashed
	wxFileOffset len = 0;
	{
		wxFile file(filename,wxFile::read);
		if (file.IsOpened()) len = file.Length();
	}

	// Generate string to be hashed
	wxString toHash = filename + wxString::Format(_T(":%i"),len);

	// Get the MD5 digest of the string
	md5_state_t state;
	md5_byte_t digest[16];
	md5_init(&state);
	md5_append(&state,(md5_byte_t*)toHash.c_str(),toHash.Length()*sizeof(wxChar));
	md5_finish(&state,digest);

	// Generate the filename
	unsigned int *md5 = (unsigned int*) digest;
	wxString result = wxString::Format(_T("?user/ffms2cache/%08X%08X%08X%08X.ffindex"),md5[0],md5[1],md5[2],md5[3]);
	result = StandardPaths::DecodePath(result);

	// Ensure that folder exists
	wxFileName fn(result);
	wxString dir = fn.GetPath();
	if (!wxFileName::DirExists(dir)) {
		wxFileName::Mkdir(dir);
	}

	wxFileName dirfn(dir);
	return dirfn.GetShortPath() + _T("/") + fn.GetFullName();
}

#endif WITH_FFMPEGSOURCE

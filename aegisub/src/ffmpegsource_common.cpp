// Copyright (c) 2008-2009, Karl Blomster
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
#include <wx/wxprec.h>
#include <wx/dir.h>
#include "ffmpegsource_common.h"
#include "md5.h"
#include "standard_paths.h"
#include "main.h"
#include "frame_main.h"
#include "options.h"
#include <map>


// lookit dis here static storage
wxMutex FFmpegSourceProvider::CleaningInProgress;


///////////////
// Update indexing progress
int FFMS_CC FFmpegSourceProvider::UpdateIndexingProgress(int64_t Current, int64_t Total, void *Private) {
	IndexingProgressDialog *Progress = (IndexingProgressDialog *)Private;

	if (Progress->IndexingCanceled)
		return 1;

	// no one cares about a little bit of a rounding error here anyway
	Progress->ProgressDialog->SetProgress(((int64_t)1000*Current)/Total, 1000);
	
	return 0;
}


///////////
// Do indexing
FFIndex *FFmpegSourceProvider::DoIndexing(FFIndexer *Indexer, const wxString &CacheName, int Trackmask, bool IgnoreDecodeErrors) {
	char FFMSErrMsg[1024];
	unsigned MsgSize = sizeof(FFMSErrMsg);
	wxString MsgString;

	// set up progress dialog callback
	IndexingProgressDialog Progress;
	Progress.IndexingCanceled = false;
	Progress.ProgressDialog = new DialogProgress(AegisubApp::Get()->frame, _("Indexing"), &Progress.IndexingCanceled,
		_("Reading timecodes and frame/sample data"), 0, 1);
	Progress.ProgressDialog->Show();
	Progress.ProgressDialog->SetProgress(0,1);

	// index all audio tracks
	FFIndex *Index = FFMS_DoIndexing(Indexer, Trackmask, FFMS_TRACKMASK_NONE, NULL, NULL, IgnoreDecodeErrors,
		FFmpegSourceProvider::UpdateIndexingProgress, &Progress, FFMSErrMsg, MsgSize);
	if (Index == NULL) {
		Progress.ProgressDialog->Destroy();
		MsgString.Append(_T("Failed to index: ")).Append(wxString(FFMSErrMsg, wxConvUTF8));
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

///////////
// Find all tracks of the given typo and return their track numbers and respective codec names
std::map<int,wxString> FFmpegSourceProvider::GetTracksOfType(FFIndexer *Indexer, FFMS_TrackType Type) {
	std::map<int,wxString> TrackList;
	int NumTracks = FFMS_GetNumTracksI(Indexer);

	for (int i=0; i<NumTracks; i++) {
		if (FFMS_GetTrackTypeI(Indexer, i) == Type) {
			wxString CodecName(FFMS_GetCodecNameI(Indexer, i), wxConvUTF8);
			TrackList.insert(std::pair<int,wxString>(i, CodecName));
		}
	}
	
	return TrackList;
}

///////////
// Ask user for which track he wants to load
int FFmpegSourceProvider::AskForTrackSelection(const std::map<int,wxString> &TrackList, FFMS_TrackType Type) {
	std::vector<int> TrackNumbers;
	wxArrayString Choices;
	wxString TypeName = _T("");
	if (Type == FFMS_TYPE_VIDEO)
		TypeName = _("video");
	else if (Type == FFMS_TYPE_AUDIO)
		TypeName = _("audio");
	
	for (std::map<int,wxString>::const_iterator i = TrackList.begin(); i != TrackList.end(); i++) {
		Choices.Add(wxString::Format(_("Track %02d: %s"), i->first, i->second.c_str()));
		TrackNumbers.push_back(i->first);
	}
	
	int Choice = wxGetSingleChoiceIndex(wxString::Format(_("Multiple %s tracks detected, please choose the one you wish to load:"), TypeName.c_str()),
		wxString::Format(_("Choose %s track"), TypeName.c_str()), Choices);

	if (Choice < 0)
		return Choice;
	else
		return TrackNumbers[Choice];
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
	const wchar_t *tmp = toHash.wc_str();
	md5_state_t state;
	md5_byte_t digest[16];
	md5_init(&state);
	md5_append(&state,(md5_byte_t*)tmp,toHash.Length()*sizeof(wchar_t));
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

/////////////////////
// fire and forget cleaning thread (well, almost)
bool FFmpegSourceProvider::CleanCache() {
	wxLogDebug(_T("FFmpegSourceCacheCleaner: attempting to start thread"));

	FFmpegSourceCacheCleaner *CleaningThread = new FFmpegSourceCacheCleaner(this);

	if (CleaningThread->Create() != wxTHREAD_NO_ERROR) {
		wxLogDebug(_T("FFmpegSourceCacheCleaner: thread creation failed"));
		delete CleaningThread;
		CleaningThread = NULL;
		return false;
	}
	if (CleaningThread->Run() != wxTHREAD_NO_ERROR) {
		wxLogDebug(_T("FFmpegSourceCacheCleaner: failed to start thread"));
		delete CleaningThread;
		CleaningThread = NULL;
		return false;
	}

	wxLogDebug(_T("FFmpegSourceCacheCleaner: thread started successfully"));
	return true;
}


/////////////////////
// constructor
FFmpegSourceCacheCleaner::FFmpegSourceCacheCleaner(FFmpegSourceProvider *par) : wxThread(wxTHREAD_DETACHED) {
	parent = par;
}

/////////////////////
// all actual work happens here because I was too lazy to write more functions
wxThread::ExitCode FFmpegSourceCacheCleaner::Entry() {
	wxMutexLocker lock(FFmpegSourceProvider::CleaningInProgress);
	if (!lock.IsOk()) {
		wxLogDebug(_T("FFmpegSourceCacheCleaner: cleaning already in progress, thread exiting"));
		return (wxThread::ExitCode)1;
	}

	wxString cachedirname = StandardPaths::DecodePath(_T("?user/ffms2cache/"));
	wxDir cachedir;
	if (!cachedir.Open(cachedirname)) {
		wxLogDebug(_T("FFmpegSourceCacheCleaner: couldn't open cache directory %s"), cachedirname.c_str());
		return (wxThread::ExitCode)1;
	}

	// sleep for a bit so we (hopefully) don't thrash the disk too much while indexing is in progress
	wxThread::This()->Sleep(2000);

	// the option is in megabytes, we need bytes
	// shift left by 20 is CLEARLY more efficient than multiplying by 1048576
	int64_t maxsize = Options.AsInt(_T("FFmpegSource max cache size")) << 20;
	int64_t cursize = wxDir::GetTotalSize(cachedirname).GetValue();
	int maxfiles	= Options.AsInt(_T("FFmpegSource max cache files"));

	if (!cachedir.HasFiles(_T("*.ffindex"))) {
		wxLogDebug(_T("FFmpegSourceCacheCleaner: no index files in cache folder, exiting"));
		return (wxThread::ExitCode)0;
	}
	
	int deleted = 0;
	int numfiles = 0;
	std::multimap<int64_t,wxFileName> cachefiles;
	wxString curfn_str;
	wxFileName curfn;
	wxDateTime curatime;

	// unusually paranoid sanity check
	// (someone might have deleted the file(s) after we did HasFiles() above; does wxDir.Open() lock the dir?)
	if (!cachedir.GetFirst(&curfn_str, _T("*.ffindex"), wxDIR_FILES)) {
		wxLogDebug(_T("FFmpegSourceCacheCleaner: insanity/race condition/index dir fuckery detected, exiting"));
		return (wxThread::ExitCode)1;
	}

	numfiles++;
	curfn = wxFileName(cachedirname, curfn_str);
	curfn.GetTimes(&curatime, NULL, NULL);
	// FIXME: will break when the time_t's wrap around!!1!
	cachefiles.insert(std::pair<int64_t,wxFileName>(curatime.GetTicks(),curfn));

	while (cachedir.GetNext(&curfn_str)) {
		curfn = wxFileName(cachedirname, curfn_str);
		curfn.GetTimes(&curatime, NULL, NULL);
		cachefiles.insert(std::pair<int64_t,wxFileName>(curatime.GetTicks(),curfn));
		numfiles++;

		wxThread::This()->Sleep(250);
	}

	if (numfiles <= maxfiles && cursize <= maxsize) {
		wxLogDebug(_T("FFmpegSourceCacheCleaner: cache does not need cleaning (maxsize=%d, cursize=%d; maxfiles=%d, numfiles=%d), exiting"),
			(int)maxsize, (int)cursize, maxfiles, numfiles);
		return (wxThread::ExitCode)0;
	}

	for (std::multimap<int64_t,wxFileName>::iterator i = cachefiles.begin(); i != cachefiles.end(); i++) {
		// stop cleaning?
		if ((cursize <= maxsize && numfiles <= maxfiles) || numfiles <= 1)
			break;
		
		int64_t fsize = i->second.GetSize().GetValue();
		if (!wxRemoveFile(i->second.GetFullPath())) {
			wxLogDebug(_T("FFmpegSourceCacheCleaner: failed to remove file %s"),i->second.GetFullPath().c_str());
			continue;
		}
		cursize -= fsize;
		numfiles--;
		deleted++;

		wxThread::This()->Sleep(250);
	}

	wxLogDebug(_T("FFmpegSourceCacheCleaner: deleted %d files"), deleted);
	wxLogDebug(_T("FFmpegSourceCacheCleaner: done, exiting"));

	return (wxThread::ExitCode)0;
}


#endif // WITH_FFMPEGSOURCE

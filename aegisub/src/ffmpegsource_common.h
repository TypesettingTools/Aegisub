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

#pragma once
#ifdef WITH_FFMPEGSOURCE

///////////
// Headers
#include <wx/wxprec.h>
#include <wx/thread.h>
#include "include/aegisub/aegisub.h"
#define FFMS_BETA_10_COMPAT
#include <ffms.h>
#include "dialog_progress.h"


#define FFMS_TRACKMASK_ALL		0
#define FFMS_TRACKMASK_NONE		-1


class FFmpegSourceProvider {
	friend class FFmpegSourceCacheCleaner;
public:
	// constants stolen from avutil/log.h
	// hope the ffmpeg devs don't change them around too much
	enum FFMS_LogLevel {
		FFMS_LOG_QUIET		= -8,
		FFMS_LOG_PANIC		= 0,
		FFMS_LOG_FATAL		= 8,
		FFMS_LOG_ERROR		= 16,
		FFMS_LOG_WARNING	= 24,
		FFMS_LOG_INFO		= 32,
		FFMS_LOG_VERBOSE	= 40,
		FFMS_LOG_DEBUG		= 48,
	};

	struct IndexingProgressDialog {
		volatile bool IndexingCanceled;
		DialogProgress *ProgressDialog;
	};

	static wxMutex CleaningInProgress;
	bool CleanCache();

	static int FFMS_CC UpdateIndexingProgress(int64_t Current, int64_t Total, void *Private);
	
	FFMS_Index *DoIndexing(FFMS_Indexer *Indexer, const wxString& Cachename, int Trackmask, bool IgnoreDecodeErrors);
	std::map<int,wxString> GetTracksOfType(FFMS_Indexer *Indexer, FFMS_TrackType Type);
	int AskForTrackSelection(const std::map<int,wxString>& TrackList, FFMS_TrackType Type);
	wxString GetCacheFilename(const wxString& filename);
	void SetLogLevel();

	virtual FFmpegSourceProvider::~FFmpegSourceProvider() {}
};


class FFmpegSourceCacheCleaner : public wxThread {
private:
	FFmpegSourceProvider *parent;

public:
	FFmpegSourceCacheCleaner(FFmpegSourceProvider *par);
	~FFmpegSourceCacheCleaner() {};
	wxThread::ExitCode Entry();
};


#endif /* WITH_FFMPEGSOURCE */


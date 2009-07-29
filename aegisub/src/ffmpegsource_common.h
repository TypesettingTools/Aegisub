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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file ffmpegsource_common.h
/// @see ffmpegsource_common.cpp
/// @ingroup video_input audio_input ffms2
///

#pragma once
#ifdef WITH_FFMPEGSOURCE

///////////
// Headers
#include <wx/wxprec.h>
#include <wx/thread.h>
#include "include/aegisub/aegisub.h"

/// DOCME
#define FFMS_BETA_10_COMPAT
#include <ffms.h>
#include "dialog_progress.h"



/// DOCME
#define FFMS_TRACKMASK_ALL		-1

/// DOCME
#define FFMS_TRACKMASK_NONE		0



/// DOCME
/// @class FFmpegSourceProvider
/// @brief DOCME
///
/// DOCME
class FFmpegSourceProvider {
	friend class FFmpegSourceCacheCleaner;
public:

	/// DOCME
	enum FFMS_LogLevel {

		/// DOCME
		FFMS_LOG_QUIET		= -8,

		/// DOCME
		FFMS_LOG_PANIC		= 0,

		/// DOCME
		FFMS_LOG_FATAL		= 8,

		/// DOCME
		FFMS_LOG_ERROR		= 16,

		/// DOCME
		FFMS_LOG_WARNING	= 24,

		/// DOCME
		FFMS_LOG_INFO		= 32,

		/// DOCME
		FFMS_LOG_VERBOSE	= 40,

		/// DOCME
		FFMS_LOG_DEBUG		= 48,
	};


	/// DOCME
	struct IndexingProgressDialog {

		/// DOCME
		volatile bool IndexingCanceled;

		/// DOCME
		DialogProgress *ProgressDialog;
	};


	/// DOCME
	static wxMutex CleaningInProgress;
	bool CleanCache();

	static int FFMS_CC UpdateIndexingProgress(int64_t Current, int64_t Total, void *Private);
	
	FFIndex *DoIndexing(FFIndexer *Indexer, const wxString& Cachename, int Trackmask, bool IgnoreDecodeErrors);
	std::map<int,wxString> GetTracksOfType(FFIndexer *Indexer, FFMS_TrackType Type);
	int AskForTrackSelection(const std::map<int,wxString>& TrackList, FFMS_TrackType Type);
	wxString GetCacheFilename(const wxString& filename);
	void SetLogLevel();


	/// @brief DOCME
	///
	virtual ~FFmpegSourceProvider() {}
};



/// DOCME
/// @class FFmpegSourceCacheCleaner
/// @brief DOCME
///
/// DOCME
class FFmpegSourceCacheCleaner : public wxThread {
private:

	/// DOCME
	FFmpegSourceProvider *parent;

public:
	FFmpegSourceCacheCleaner(FFmpegSourceProvider *par);

	/// @brief DOCME
	///
	~FFmpegSourceCacheCleaner() {};
	wxThread::ExitCode Entry();
};


#endif /* WITH_FFMPEGSOURCE */




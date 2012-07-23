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

/// @file audio_provider_ffmpegsource.cpp
/// @brief ffms2-based audio provider
/// @ingroup audio_input ffms
///

#include "config.h"

#ifdef WITH_FFMS2

#ifndef AGI_PRE
#ifdef WIN32
#include <objbase.h>
#endif

#include <map>
#endif

#include "audio_provider_ffmpegsource.h"

#include "audio_controller.h"
#include "compat.h"
#include "main.h"

/// @brief Constructor
/// @param filename The filename to open
FFmpegSourceAudioProvider::FFmpegSourceAudioProvider(wxString filename) try
: AudioSource(NULL, FFMS_DestroyAudioSource)
{
	ErrInfo.Buffer		= FFMSErrMsg;
	ErrInfo.BufferSize	= sizeof(FFMSErrMsg);
	ErrInfo.ErrorType	= FFMS_ERROR_SUCCESS;
	ErrInfo.SubType		= FFMS_ERROR_SUCCESS;
	SetLogLevel();

	LoadAudio(filename);
}
catch (wxString const& err) {
	throw agi::AudioProviderOpenError(STD_STR(err), 0);
}
catch (const char *err) {
	throw agi::AudioProviderOpenError(err, 0);
}

void FFmpegSourceAudioProvider::LoadAudio(wxString filename) {
	wxString FileNameShort = wxFileName(filename).GetShortPath();

	FFMS_Indexer *Indexer = FFMS_CreateIndexer(FileNameShort.utf8_str(), &ErrInfo);
	if (!Indexer)
		throw agi::FileNotFoundError(ErrInfo.Buffer);

	std::map<int,wxString> TrackList = GetTracksOfType(Indexer, FFMS_TYPE_AUDIO);
	if (TrackList.size() <= 0)
		throw agi::AudioDataNotFoundError("no audio tracks found", 0);

	// initialize the track number to an invalid value so we can detect later on
	// whether the user actually had to choose a track or not
	int TrackNumber = -1;
	if (TrackList.size() > 1) {
		TrackNumber = AskForTrackSelection(TrackList, FFMS_TYPE_AUDIO);
		// if it's still -1 here, user pressed cancel
		if (TrackNumber == -1)
			throw agi::UserCancelException("audio loading canceled by user");
	}

	// generate a name for the cache file
	wxString CacheName = GetCacheFilename(filename);

	// try to read index
	agi::scoped_holder<FFMS_Index*, void (FFMS_CC*)(FFMS_Index*)>
		Index(FFMS_ReadIndex(CacheName.utf8_str(), &ErrInfo), FFMS_DestroyIndex);

	if (Index && FFMS_IndexBelongsToFile(Index, FileNameShort.utf8_str(), &ErrInfo))
		Index = NULL;

	// index valid but track number still not set?
	if (Index) {
		// track number not set? just grab the first track
		if (TrackNumber < 0)
			TrackNumber = FFMS_GetFirstTrackOfType(Index, FFMS_TYPE_AUDIO, &ErrInfo);
		if (TrackNumber < 0)
			throw agi::AudioDataNotFoundError(std::string("Couldn't find any audio tracks: ") + ErrInfo.Buffer, 0);

		// index is valid and track number is now set,
		// but do we have indexing info for the desired audio track?
		FFMS_Track *TempTrackData = FFMS_GetTrackFromIndex(Index, TrackNumber);
		if (FFMS_GetNumFrames(TempTrackData) <= 0)
			Index = NULL;
	}
	// no valid index exists and the file only has one audio track, index it
	else if (TrackNumber < 0)
		TrackNumber = FFMS_TRACKMASK_ALL;
	// else: do nothing (keep track mask as it is)

	// reindex if the error handling mode has changed
	FFMS_IndexErrorHandling ErrorHandling = GetErrorHandlingMode();
#if FFMS_VERSION >= ((2 << 24) | (17 << 16) | (2 << 8) | 0)
	if (Index && FFMS_GetErrorHandling(Index) != ErrorHandling)
		Index = NULL;
#endif

	// moment of truth
	if (!Index) {
		int TrackMask;
		if (OPT_GET("Provider/FFmpegSource/Index All Tracks")->GetBool() || TrackNumber == FFMS_TRACKMASK_ALL)
			TrackMask = FFMS_TRACKMASK_ALL;
		else
			TrackMask = (1 << TrackNumber);

		Index = DoIndexing(Indexer, CacheName, TrackMask, ErrorHandling);

		// if tracknumber still isn't set we need to set it now
		if (TrackNumber == FFMS_TRACKMASK_ALL)
			TrackNumber = FFMS_GetFirstTrackOfType(Index, FFMS_TYPE_AUDIO, &ErrInfo);
	}
	else {
		FFMS_CancelIndexing(Indexer);
	}

	// update access time of index file so it won't get cleaned away
	if (!wxFileName(CacheName).Touch()) {
		// warn user?
	}

	AudioSource = FFMS_CreateAudioSource(FileNameShort.utf8_str(), TrackNumber, Index, -1, &ErrInfo);
	if (!AudioSource)
		throw agi::AudioProviderOpenError(std::string("Failed to open audio track: ") + ErrInfo.Buffer, 0);

	const FFMS_AudioProperties AudioInfo = *FFMS_GetAudioProperties(AudioSource);

	channels	= AudioInfo.Channels;
	sample_rate	= AudioInfo.SampleRate;
	num_samples = AudioInfo.NumSamples;
	if (channels <= 0 || sample_rate <= 0 || num_samples <= 0)
		throw agi::AudioProviderOpenError("sanity check failed, consult your local psychiatrist", 0);

	switch (AudioInfo.SampleFormat) {
		case FFMS_FMT_U8:  bytes_per_sample = 1; float_samples = false; break;
		case FFMS_FMT_S16: bytes_per_sample = 2; float_samples = false; break;
		case FFMS_FMT_S32: bytes_per_sample = 4; float_samples = false; break;
		case FFMS_FMT_FLT: bytes_per_sample = 4; float_samples = true; break;
		case FFMS_FMT_DBL: bytes_per_sample = 8; float_samples = true; break;
		default:
			throw agi::AudioProviderOpenError("unknown or unsupported sample format", 0);
	}
}

void FFmpegSourceAudioProvider::GetAudio(void *Buf, int64_t Start, int64_t Count) const {
	if (FFMS_GetAudio(AudioSource, Buf, Start, Count, &ErrInfo)) {
		throw AudioDecodeError(std::string("Failed to get audio samples: ") + ErrInfo.Buffer);
	}
}
#endif /* WITH_FFMS2 */

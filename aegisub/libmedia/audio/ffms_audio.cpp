// Copyright (c) 2008-2009, Karl Blomster <thefluff@aegisub.org>
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

/// @file ffms_audio.cpp
/// @brief FFmpegSource Audio support.
/// @ingroup fmms audio


#include "config.h"

#ifndef AGI_PRE
#endif

#include "ffms_audio.h"
#include "libmedia/audio.h"


namespace media {
	namespace ffms {


/// @brief Constructor
/// @param filename
///
Audio::Audio(std::string filename)
: AudioSource(NULL)
, COMInited(false)
{
#ifdef WIN32
	HRESULT res;
	res = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(res))
		COMInited = true;
	else if (res != RPC_E_CHANGED_MODE)
		throw AudioOpenError("COM initialization failure");
#endif
	// initialize ffmpegsource
	// FIXME: CPU detection?
#if FFMS_VERSION >= ((2 << 24) | (14 << 16) | (0 << 8) | 0)
	FFMS_Init(0, 1);
#else
	FFMS_Init(0);
#endif

	ErrInfo.Buffer		= FFMSErrMsg;
	ErrInfo.BufferSize	= sizeof(FFMSErrMsg);
	ErrInfo.ErrorType	= FFMS_ERROR_SUCCESS;
	ErrInfo.SubType		= FFMS_ERROR_SUCCESS;
//	SetLogLevel();

	try {
		LoadAudio(filename);
	} catch (...) {
		Close();
		throw;
	}
}

/// @brief Load audio file 
/// @param filename 
///
void Audio::LoadAudio(std::string filename) {
//	wxString FileNameShort = wxFileName(filename).GetShortPath();

	FFMS_Indexer *Indexer = FFMS_CreateIndexer(filename.c_str(), &ErrInfo);
	if (Indexer == NULL) {
		throw agi::FileNotFoundError(ErrInfo.Buffer);
	}

	std::map<int,std::string> TrackList = GetTracksOfType(Indexer, FFMS_TYPE_AUDIO);
	if (TrackList.size() <= 0)
		throw AudioOpenError("no audio tracks found");

	// initialize the track number to an invalid value so we can detect later on
	// whether the user actually had to choose a track or not
	int TrackNumber = -1;
	if (TrackList.size() > 1) {
		TrackNumber = AskForTrackSelection(TrackList, FFMS_TYPE_AUDIO);
		// if it's still -1 here, user pressed cancel
		if (TrackNumber == -1)
			throw agi::UserCancelException("audio loading cancelled by user");
	}

	// generate a name for the cache file
	std::string CacheName = GetCacheFilename(filename);

	// try to read index
	FFMS_Index *Index = NULL;
	Index = FFMS_ReadIndex(CacheName.c_str(), &ErrInfo);
	bool IndexIsValid = false;
	if (Index != NULL) {
		if (FFMS_IndexBelongsToFile(Index, filename.c_str(), &ErrInfo)) {
			FFMS_DestroyIndex(Index);
			Index = NULL;
		}
		else
			IndexIsValid = true;
	}
	
	// index valid but track number still not set?
	if (IndexIsValid) {
		// track number not set? just grab the first track
		if (TrackNumber < 0)
			TrackNumber = FFMS_GetFirstTrackOfType(Index, FFMS_TYPE_AUDIO, &ErrInfo);
		if (TrackNumber < 0) {
			FFMS_DestroyIndex(Index);
			Index = NULL;
			throw AudioOpenError(std::string("Couldn't find any audio tracks: ") + ErrInfo.Buffer);
		}

		// index is valid and track number is now set,
		// but do we have indexing info for the desired audio track?
		FFMS_Track *TempTrackData = FFMS_GetTrackFromIndex(Index, TrackNumber);
		if (FFMS_GetNumFrames(TempTrackData) <= 0) {
			IndexIsValid = false;
			FFMS_DestroyIndex(Index);
			Index = NULL;
		}
	}
	// no valid index exists and the file only has one audio track, index it
	else if (TrackNumber < 0)
		TrackNumber = FFMS_TRACKMASK_ALL;
	// else: do nothing (keep track mask as it is)

	// moment of truth
	if (!IndexIsValid) {
		int TrackMask;
//		if (OPT_GET("Provider/FFmpegSource/Index All Tracks")->GetBool() || TrackNumber == FFMS_TRACKMASK_ALL)
		if (TrackNumber == FFMS_TRACKMASK_ALL)
			TrackMask = FFMS_TRACKMASK_ALL;
		else
			TrackMask = (1 << TrackNumber);

		try {
			Index = DoIndexing(Indexer, CacheName, TrackMask, GetErrorHandlingMode());
		}
		catch (std::string const& err) {
			throw AudioOpenError(err);
		}

		// if tracknumber still isn't set we need to set it now
		if (TrackNumber == FFMS_TRACKMASK_ALL)
			TrackNumber = FFMS_GetFirstTrackOfType(Index, FFMS_TYPE_AUDIO, &ErrInfo);
	}

	// update access time of index file so it won't get cleaned away
///XXX: Add something to libaegisub to support this.
//	if (!wxFileName(CacheName).Touch()) {
		// warn user?
//	}

#if FFMS_VERSION >= ((2 << 24) | (14 << 16) | (1 << 8) | 0)
	AudioSource = FFMS_CreateAudioSource(filename.c_str(), TrackNumber, Index, -1, &ErrInfo);
#else
	AudioSource = FFMS_CreateAudioSource(filename.c_str(), TrackNumber, Index, &ErrInfo);
#endif
	FFMS_DestroyIndex(Index);
	Index = NULL;
	if (!AudioSource) {
		throw AudioOpenError(std::string("Failed to open audio track: %s") + ErrInfo.Buffer);
	}

	const FFMS_AudioProperties AudioInfo = *FFMS_GetAudioProperties(AudioSource);

	channels	= AudioInfo.Channels;
	sample_rate	= AudioInfo.SampleRate;
	num_samples = AudioInfo.NumSamples;
	if (channels <= 0 || sample_rate <= 0 || num_samples <= 0)
		throw AudioOpenError("sanity check failed, consult your local psychiatrist");

	// FIXME: use the actual sample format too?
	// why not just bits_per_sample/8? maybe there's some oddball format with half bytes out there somewhere...
	switch (AudioInfo.BitsPerSample) {
		case 8:		bytes_per_sample = 1; break;
		case 16:	bytes_per_sample = 2; break;
		case 24:	bytes_per_sample = 3; break;
		case 32:	bytes_per_sample = 4; break;
		default:
			throw AudioOpenError("unknown or unsupported sample format");
	}
}

/// @brief Destructor 
///
Audio::~Audio() {
	Close();
}

/// @brief Clean up
///
void Audio::Close() {
	if (AudioSource) FFMS_DestroyAudioSource(AudioSource);
#ifdef WIN32
	if (COMInited)
		CoUninitialize();
#endif
}

/// @brief Get audio 
/// @param Buf   
/// @param Start 
/// @param Count 
///
void Audio::GetAudio(void *Buf, int64_t Start, int64_t Count) const {
	if (FFMS_GetAudio(AudioSource, Buf, Start, Count, &ErrInfo)) {
		throw AudioDecodeError(std::string("Failed to get audio samples: ") + ErrInfo.Buffer);
	}
}
	} // namespace ffms
} // namespace media

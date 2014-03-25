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

/// @file audio_provider_ffmpegsource.cpp
/// @brief ffms2-based audio provider
/// @ingroup audio_input ffms
///

#include "config.h"

#ifdef WITH_FFMS2
#include "include/aegisub/audio_provider.h"

#include "audio_controller.h"
#include "ffmpegsource_common.h"
#include "options.h"

#include <libaegisub/fs.h>
#include <libaegisub/util.h>

#include <map>

namespace {
class FFmpegSourceAudioProvider final : public AudioProvider, FFmpegSourceProvider {
	/// audio source object
	agi::scoped_holder<FFMS_AudioSource*, void (FFMS_CC *)(FFMS_AudioSource*)> AudioSource;

	mutable char FFMSErrMsg[1024];			///< FFMS error message
	mutable FFMS_ErrorInfo ErrInfo;			///< FFMS error codes/messages

	void LoadAudio(agi::fs::path const& filename);
	void FillBuffer(void *Buf, int64_t Start, int64_t Count) const override {
		if (FFMS_GetAudio(AudioSource, Buf, Start, Count, &ErrInfo))
			throw AudioDecodeError(std::string("Failed to get audio samples: ") + ErrInfo.Buffer);
	}

public:
	FFmpegSourceAudioProvider(agi::fs::path const& filename, agi::BackgroundRunner *br);

	bool NeedsCache() const override { return true; }
};

/// @brief Constructor
/// @param filename The filename to open
FFmpegSourceAudioProvider::FFmpegSourceAudioProvider(agi::fs::path const& filename, agi::BackgroundRunner *br) try
: FFmpegSourceProvider(br)
, AudioSource(nullptr, FFMS_DestroyAudioSource)
{
	ErrInfo.Buffer		= FFMSErrMsg;
	ErrInfo.BufferSize	= sizeof(FFMSErrMsg);
	ErrInfo.ErrorType	= FFMS_ERROR_SUCCESS;
	ErrInfo.SubType		= FFMS_ERROR_SUCCESS;
	SetLogLevel();

	LoadAudio(filename);
}
catch (std::string const& err) {
	throw agi::AudioProviderOpenError(err, nullptr);
}
catch (const char *err) {
	throw agi::AudioProviderOpenError(err, nullptr);
}

void FFmpegSourceAudioProvider::LoadAudio(agi::fs::path const& filename) {
	FFMS_Indexer *Indexer = FFMS_CreateIndexer(filename.string().c_str(), &ErrInfo);
	if (!Indexer) {
		if (ErrInfo.SubType == FFMS_ERROR_FILE_READ)
			throw agi::fs::FileNotFound(std::string(ErrInfo.Buffer));
		else
			throw agi::AudioDataNotFoundError(ErrInfo.Buffer, nullptr);
	}

	std::map<int, std::string> TrackList = GetTracksOfType(Indexer, FFMS_TYPE_AUDIO);
	if (TrackList.empty())
		throw agi::AudioDataNotFoundError("no audio tracks found", nullptr);

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
	agi::fs::path CacheName = GetCacheFilename(filename);

	// try to read index
	agi::scoped_holder<FFMS_Index*, void (FFMS_CC*)(FFMS_Index*)>
		Index(FFMS_ReadIndex(CacheName.string().c_str(), &ErrInfo), FFMS_DestroyIndex);

	if (Index && FFMS_IndexBelongsToFile(Index, filename.string().c_str(), &ErrInfo))
		Index = nullptr;

	// index valid but track number still not set?
	if (Index) {
		// track number not set? just grab the first track
		if (TrackNumber < 0)
			TrackNumber = FFMS_GetFirstTrackOfType(Index, FFMS_TYPE_AUDIO, &ErrInfo);
		if (TrackNumber < 0)
			throw agi::AudioDataNotFoundError(std::string("Couldn't find any audio tracks: ") + ErrInfo.Buffer, nullptr);

		// index is valid and track number is now set,
		// but do we have indexing info for the desired audio track?
		FFMS_Track *TempTrackData = FFMS_GetTrackFromIndex(Index, TrackNumber);
		if (FFMS_GetNumFrames(TempTrackData) <= 0)
			Index = nullptr;
	}
	// no valid index exists and the file only has one audio track, index it
	else if (TrackNumber < 0)
		TrackNumber = FFMS_TRACKMASK_ALL;
	// else: do nothing (keep track mask as it is)

	// reindex if the error handling mode has changed
	FFMS_IndexErrorHandling ErrorHandling = GetErrorHandlingMode();
#if FFMS_VERSION >= ((2 << 24) | (17 << 16) | (2 << 8) | 0)
	if (Index && FFMS_GetErrorHandling(Index) != ErrorHandling)
		Index = nullptr;
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
	else
		FFMS_CancelIndexing(Indexer);

	// update access time of index file so it won't get cleaned away
	agi::fs::Touch(CacheName);

	AudioSource = FFMS_CreateAudioSource(filename.string().c_str(), TrackNumber, Index, -1, &ErrInfo);
	if (!AudioSource)
		throw agi::AudioProviderOpenError(std::string("Failed to open audio track: ") + ErrInfo.Buffer, nullptr);

	const FFMS_AudioProperties AudioInfo = *FFMS_GetAudioProperties(AudioSource);

	channels	= AudioInfo.Channels;
	sample_rate	= AudioInfo.SampleRate;
	num_samples = AudioInfo.NumSamples;
	if (channels <= 0 || sample_rate <= 0 || num_samples <= 0)
		throw agi::AudioProviderOpenError("sanity check failed, consult your local psychiatrist", nullptr);

	switch (AudioInfo.SampleFormat) {
		case FFMS_FMT_U8:  bytes_per_sample = 1; float_samples = false; break;
		case FFMS_FMT_S16: bytes_per_sample = 2; float_samples = false; break;
		case FFMS_FMT_S32: bytes_per_sample = 4; float_samples = false; break;
		case FFMS_FMT_FLT: bytes_per_sample = 4; float_samples = true; break;
		case FFMS_FMT_DBL: bytes_per_sample = 8; float_samples = true; break;
		default:
			throw agi::AudioProviderOpenError("unknown or unsupported sample format", nullptr);
	}

#if FFMS_VERSION >= ((2 << 24) | (17 << 16) | (4 << 8) | 0)
	if (channels > 1 || bytes_per_sample != 2) {
		std::unique_ptr<FFMS_ResampleOptions, decltype(&FFMS_DestroyResampleOptions)>
			opt(FFMS_CreateResampleOptions(AudioSource), FFMS_DestroyResampleOptions);
		opt->ChannelLayout = FFMS_CH_FRONT_CENTER;
		opt->SampleFormat = FFMS_FMT_S16;

		// Might fail if FFMS2 wasn't built with libavresample
		if (!FFMS_SetOutputFormatA(AudioSource, opt.get(), nullptr)) {
			channels = 1;
			bytes_per_sample = 2;
			float_samples = false;
		}
	}
#endif
}

}

std::unique_ptr<AudioProvider> CreateFFmpegSourceAudioProvider(agi::fs::path const& file, agi::BackgroundRunner *br) {
	return agi::util::make_unique<FFmpegSourceAudioProvider>(file, br);
}

#endif /* WITH_FFMS2 */

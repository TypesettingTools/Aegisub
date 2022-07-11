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

#ifdef WITH_FFMS2
#include <libaegisub/audio/provider.h>

#include "ffmpegsource_common.h"
#include "options.h"

#include <libaegisub/fs.h>
#include <libaegisub/make_unique.h>

#include <map>

namespace {
class FFmpegSourceAudioProvider final : public agi::AudioProvider, FFmpegSourceProvider {
	/// audio source object
	agi::scoped_holder<FFMS_AudioSource*, void (FFMS_CC *)(FFMS_AudioSource*)> AudioSource;

	mutable char FFMSErrMsg[1024];			///< FFMS error message
	mutable FFMS_ErrorInfo ErrInfo;			///< FFMS error codes/messages

	void LoadAudio(agi::fs::path const& filename);
	void FillBuffer(void *Buf, int64_t Start, int64_t Count) const override {
		if (FFMS_GetAudio(AudioSource, Buf, Start, Count, &ErrInfo))
			throw agi::AudioDecodeError(std::string("Failed to get audio samples: ") + ErrInfo.Buffer);
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
catch (agi::EnvironmentError const& err) {
	throw agi::AudioProviderError(err.GetMessage());
}

void FFmpegSourceAudioProvider::LoadAudio(agi::fs::path const& filename) {
	FFMS_Indexer *Indexer = FFMS_CreateIndexer(filename.string().c_str(), &ErrInfo);
	if (!Indexer) {
		if (ErrInfo.SubType == FFMS_ERROR_FILE_READ)
			throw agi::fs::FileNotFound(std::string(ErrInfo.Buffer));
		else
			throw agi::AudioDataNotFound(ErrInfo.Buffer);
	}

	std::map<int, std::string> TrackList = GetTracksOfType(Indexer, FFMS_TYPE_AUDIO);

	// initialize the track number to an invalid value so we can detect later on
	// whether the user actually had to choose a track or not
	int TrackNumber = -1;
	if (TrackList.size() > 1) {
		auto Selection = AskForTrackSelection(TrackList, FFMS_TYPE_AUDIO);
		if (Selection == TrackSelection::None)
			throw agi::UserCancelException("audio loading canceled by user");
		TrackNumber = static_cast<int>(Selection);
	}
	else if (TrackList.size() == 1)
		TrackNumber = TrackList.begin()->first;
	else
		throw agi::AudioDataNotFound("no audio tracks found");

	// generate a name for the cache file
	agi::fs::path CacheName = GetCacheFilename(filename);

	// try to read index
	agi::scoped_holder<FFMS_Index*, void (FFMS_CC*)(FFMS_Index*)>
		Index(FFMS_ReadIndex(CacheName.string().c_str(), &ErrInfo), FFMS_DestroyIndex);

	if (Index && FFMS_IndexBelongsToFile(Index, filename.string().c_str(), &ErrInfo))
		Index = nullptr;

	if (Index) {
		// we already have an index, but the desired track may not have been
		// indexed, and if it wasn't we need to reindex
		FFMS_Track *TempTrackData = FFMS_GetTrackFromIndex(Index, TrackNumber);
		if (FFMS_GetNumFrames(TempTrackData) <= 0)
			Index = nullptr;
	}

	// reindex if the error handling mode has changed
	FFMS_IndexErrorHandling ErrorHandling = GetErrorHandlingMode();
	if (Index && FFMS_GetErrorHandling(Index) != ErrorHandling)
		Index = nullptr;

	// moment of truth
	if (!Index) {
		TrackSelection TrackMask = static_cast<TrackSelection>(TrackNumber);
		if (OPT_GET("Provider/FFmpegSource/Index All Tracks")->GetBool())
			TrackMask = TrackSelection::All;
		Index = DoIndexing(Indexer, CacheName, TrackMask, ErrorHandling);
	}
	else
		FFMS_CancelIndexing(Indexer);

	// update access time of index file so it won't get cleaned away
	agi::fs::Touch(CacheName);

	AudioSource = FFMS_CreateAudioSource(filename.string().c_str(), TrackNumber, Index, FFMS_DELAY_FIRST_VIDEO_TRACK, &ErrInfo);
	if (!AudioSource)
		throw agi::AudioProviderError(std::string("Failed to open audio track: ") + ErrInfo.Buffer);

	const FFMS_AudioProperties AudioInfo = *FFMS_GetAudioProperties(AudioSource);

	channels	= AudioInfo.Channels;
	sample_rate	= AudioInfo.SampleRate;
	num_samples = AudioInfo.NumSamples;
	decoded_samples = AudioInfo.NumSamples;
	if (channels <= 0 || sample_rate <= 0 || num_samples <= 0)
		throw agi::AudioProviderError("sanity check failed, consult your local psychiatrist");

	switch (AudioInfo.SampleFormat) {
		case FFMS_FMT_U8:  bytes_per_sample = 1; float_samples = false; break;
		case FFMS_FMT_S16: bytes_per_sample = 2; float_samples = false; break;
		case FFMS_FMT_S32: bytes_per_sample = 4; float_samples = false; break;
		case FFMS_FMT_FLT: bytes_per_sample = 4; float_samples = true; break;
		case FFMS_FMT_DBL: bytes_per_sample = 8; float_samples = true; break;
		default:
			throw agi::AudioProviderError("unknown or unsupported sample format");
	}

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
}

}

std::unique_ptr<agi::AudioProvider> CreateFFmpegSourceAudioProvider(agi::fs::path const& file, agi::BackgroundRunner *br) {
	return agi::make_unique<FFmpegSourceAudioProvider>(file, br);
}

#endif /* WITH_FFMS2 */

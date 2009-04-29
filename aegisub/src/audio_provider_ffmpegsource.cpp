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
#include "include/aegisub/aegisub.h"
#include "audio_provider_ffmpegsource.h"


///////////
// Constructor
FFmpegSourceAudioProvider::FFmpegSourceAudioProvider(Aegisub::String filename) {
	FFMS_Init();

	MsgSize = sizeof(FFMSErrMsg);
	MsgString = _T("FFmpegSource audio provider: ");

	AudioSource = NULL;

	try {
		LoadAudio(filename);
	} catch (...) {
		Close();
		throw;
	}
}


///////////
// Load audio file
void FFmpegSourceAudioProvider::LoadAudio(Aegisub::String filename) {
	// clean up
	Close();

	wxString FileNameWX = wxFileName(wxString(filename.c_str(), wxConvFile)).GetShortPath();

	// generate a default name for the cache file
	wxString CacheName = GetCacheFilename(filename.c_str());

	FrameIndex *Index;
	Index = FFMS_ReadIndex(CacheName.char_str(), FFMSErrMsg, MsgSize);
	if (Index == NULL) {
		// index didn't exist or was invalid, we'll have to (re)create it
		try {
			Index = DoIndexing(Index, FileNameWX, CacheName, FFMSTrackMaskAll, false);
		} catch (wxString temp) {
			MsgString << temp;
			throw MsgString;
		} catch (...) {
			throw;
		}
	} else {
		// index exists, but does it have indexing info for the audio track(s)?
		int NumTracks = FFMS_GetNumTracks(Index);
		if (NumTracks <= 0) {
			FFMS_DestroyFrameIndex(Index);
			Index = NULL;
			throw _T("FFmpegSource audio provider: no tracks found in index file");
		}

		for (int i = 0; i < NumTracks; i++) {
			FrameInfoVector *FrameData = FFMS_GetTITrackIndex(Index, i, FFMSErrMsg, MsgSize);
			if (FrameData == NULL) {
				FFMS_DestroyFrameIndex(Index);
				Index = NULL;
				wxString temp(FFMSErrMsg, wxConvUTF8);
				MsgString << _T("Couldn't get track data: ") << temp;
				throw MsgString;
			}

			// does the track have any indexed frames?
			if (FFMS_GetNumFrames(FrameData) <= 0 && (FFMS_GetTrackType(FrameData) == FFMS_TYPE_AUDIO)) {
				// found an unindexed audio track, we'll need to reindex
				try {
					FFMS_DestroyFrameIndex(Index);
					Index = NULL;
					Index = DoIndexing(Index, FileNameWX, CacheName, FFMSTrackMaskAll, false);
				} catch (wxString temp) {
					MsgString << temp;
					throw MsgString;
				} catch (...) {
					throw;
				}

				// don't reindex more than once
				break;
			}
		}
	}

	// FIXME: provide a way to choose which audio track to load?
	int TrackNumber = FFMS_GetFirstTrackOfType(Index, FFMS_TYPE_AUDIO, FFMSErrMsg, MsgSize);
	if (TrackNumber < 0) {
		FFMS_DestroyFrameIndex(Index);
		Index = NULL;
		wxString temp(FFMSErrMsg, wxConvUTF8);
		MsgString << _T("Couldn't find any audio tracks: ") << temp;
		throw MsgString;
	}

	AudioSource = FFMS_CreateAudioSource(FileNameWX.mb_str(wxConvLocal), TrackNumber, Index, FFMSErrMsg, MsgSize);
	FFMS_DestroyFrameIndex(Index);
	Index = NULL;
	if (!AudioSource) {
			wxString temp(FFMSErrMsg, wxConvUTF8);
			MsgString << _T("Failed to open audio track: ") << temp;
			throw MsgString;
	}
		
	const AudioProperties AudioInfo = *FFMS_GetAudioProperties(AudioSource);

	if (AudioInfo.Float)
		throw _T("FFmpegSource audio provider: I don't know what to do with floating point audio");

	channels	= AudioInfo.Channels;
	sample_rate	= AudioInfo.SampleRate;
	num_samples = AudioInfo.NumSamples;
	if (channels <= 0 || sample_rate <= 0 || num_samples <= 0)
		throw _T("FFmpegSource audio provider: sanity check failed, consult your local psychiatrist");

	// why not just bits_per_sample/8? maybe there's some oddball format with half bytes out there somewhere...
	switch (AudioInfo.BitsPerSample) {
		case 8:		bytes_per_sample = 1; break;
		case 16:	bytes_per_sample = 2; break;
		case 24:	bytes_per_sample = 3; break;
		case 32:	bytes_per_sample = 4; break;
		default:
			throw _T("FFmpegSource audio provider: unknown or unsupported sample format");
	}
}


///////////
// Destructor
FFmpegSourceAudioProvider::~FFmpegSourceAudioProvider() {
	Close();
}


///////////
// Clean up
void FFmpegSourceAudioProvider::Close() {
	FFMS_DestroyAudioSource(AudioSource);
	AudioSource = NULL;
}


///////////
// Get audio
void FFmpegSourceAudioProvider::GetAudio(void *Buf, int64_t Start, int64_t Count) {
	if (FFMS_GetAudio(AudioSource, Buf, Start, Count, FFMSErrMsg, MsgSize)) {
		wxString temp(FFMSErrMsg, wxConvUTF8);
		MsgString << _T("Failed to get audio samples: ") << temp;
		throw MsgString;
	}
}


#endif /* WITH_FFMPEGSOURCE */

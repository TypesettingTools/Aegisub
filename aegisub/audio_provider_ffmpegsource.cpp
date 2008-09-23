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

#ifdef WITH_FFMPEGSOURCE

///////////
// Headers
#include "audio_provider_ffmpegsource.h"


///////////
// Constructor
FFmpegSourceAudioProvider::FFmpegSourceAudioProvider(Aegisub::String filename) {
	FFMS_Init();

	MsgSize = sizeof(FFMSErrMsg);

	AudioSource = NULL;
	Index = NULL;

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

	wxString FileNameWX(filename.c_str(), wxConvUTF8);

	// generate a default name for the cache file
	wxString CacheName(filename.c_str());
	CacheName.append(_T(".ffindex"));

	FrameIndex *Index;
	Index = FFMS_ReadIndex(CacheName.char_str(), FFMSErrMsg, MsgSize);
	if (Index == NULL) {
		// index didn't exist or was invalid, we'll have to (re)create it
		IndexingProgressDialog Progress;
		Progress.IndexingCanceled = false;
		Progress.ProgressDialog = new DialogProgress(NULL, _("Indexing"), &Progress.IndexingCanceled, _("Indexing audio file"), 0, 1);
		Progress.ProgressDialog->Show();
		Progress.ProgressDialog->SetProgress(0,1);

		// index all audio tracks
		Index = FFMS_MakeIndex(FileNameWX.char_str(), FFMSTrackMaskAll, CacheName.char_str(), FFmpegSourceProvider::UpdateIndexingProgress, &Progress, FFMSErrMsg, MsgSize);
		if (Index == NULL) {
			Progress.ProgressDialog->Destroy();
			MsgString.Printf(_T("FFmpegSource audio provider: %s"), FFMSErrMsg);
			throw MsgString;
		}
		Progress.ProgressDialog->Destroy();

		// write index to disk for later use
		if (FFMS_WriteIndex(CacheName.char_str(), Index, FFMSErrMsg, MsgSize)) {
			MsgString.Printf(_T("FFmpegSource audio provider: %s"), FFMSErrMsg);
			throw MsgString;
		}
	} 

	// FIXME: provide a way to choose which audio track to load?
	AudioSource = FFMS_CreateAudioSource(FileNameWX.char_str(), FFMSFirstSuitableTrack, Index, FFMSErrMsg, MsgSize);
	if (!AudioSource) {
			MsgString.Printf(_T("FFmpegSource audio provider: %s"), FFMSErrMsg);
			throw MsgString;
	}
		
	const AudioProperties AudioInfo = *FFMS_GetAudioProperties(AudioSource);

	if (AudioInfo.Float)
		throw _T("FFmpegSource audio provider: don't know what to do with floating point audio");

	channels	= AudioInfo.Channels;
	sample_rate	= AudioInfo.SampleRate;
	num_samples = AudioInfo.NumSamples;

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
	if (AudioSource)
		FFMS_DestroyAudioSource(AudioSource);
	AudioSource = NULL;
	if (Index)
		FFMS_DestroyFrameIndex(Index);
	Index = NULL;

}


///////////
// Get audio
void FFmpegSourceAudioProvider::GetAudio(void *Buf, int64_t Start, int64_t Count) {
	if (FFMS_GetAudio(AudioSource, Buf, Start, Count, FFMSErrMsg, MsgSize)) {
		MsgString.Printf(_T("FFmpegSource audio provider: %s"), FFMSErrMsg);
		throw MsgString;
	}
}


#endif /* WITH_FFMPEGSOURCE */
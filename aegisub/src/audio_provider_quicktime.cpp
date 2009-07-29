// Copyright (c) 2009, Karl Blomster
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

/// @file audio_provider_quicktime.cpp
/// @brief QuickTime-based audio provider
/// @ingroup audio_input quicktime
///


#include "audio_provider_quicktime.h"

#ifdef WITH_QUICKTIME


/// @brief DOCME
/// @param filename 
///
QuickTimeAudioProvider::QuickTimeAudioProvider(wxString filename) {
	movie		= NULL;
	in_dataref	= NULL;
	extract_ref	= NULL;
	inited		= false;
	qt_err		= noErr;
	qt_status	= noErr;
	errmsg		= _T("QuickTime audio provider: ");

	// try to init quicktime
	try {
		InitQuickTime();
	}
	catch (wxString temp) {
		errmsg.Append(temp);
		throw errmsg;
	}
	catch (...) {
		throw;
	}

	// try to load audio
	try {
		LoadAudio(filename);
	}
	catch (wxString temp) {
		errmsg.Append(temp);
		throw errmsg;
	}
	catch (...) {
		throw;
	}
}



/// @brief DOCME
///
QuickTimeAudioProvider::~QuickTimeAudioProvider() {
	Close();
	DeInitQuickTime();
}



/// @brief DOCME
///
void QuickTimeAudioProvider::Close() {
	if (movie)
		DisposeMovie(movie);
	movie = NULL;
	if (in_dataref)
		DisposeHandle(in_dataref);
	in_dataref = NULL;
	if (inited)
		MovieAudioExtractionEnd(extract_ref);
	inited = false;
}



/// @brief DOCME
/// @param filename 
///
void QuickTimeAudioProvider::LoadAudio(wxString filename) {
	OSType in_dataref_type;
	wxStringToDataRef(filename, &in_dataref, &in_dataref_type);

	// verify that file is openable
	if (!CanOpen(in_dataref, in_dataref_type))
		throw wxString(_T("QuickTime cannot open file as audio"));

	// actually open file
	short res_id = 0;
	qt_err = NewMovieFromDataRef(&movie, 0, &res_id, in_dataref, in_dataref_type);
	QTCheckError(qt_err, wxString(_T("Failed to open file")));

	// disable automagic screen rendering just to be safe
	qt_err = SetMovieVisualContext(movie, NULL);
	QTCheckError(qt_err, wxString(_T("Failed to disable visual context")));

	qt_status = MovieAudioExtractionBegin(movie, 0, &extract_ref);
	QTCheckError(qt_status, wxString(_T("Failed to initialize audio extraction")));
	inited = true;

	// and here I thought I knew what "verbose" meant...
	AudioStreamBasicDescription asbd;
	qt_status = MovieAudioExtractionGetProperty(extract_ref, kQTPropertyClass_MovieAudioExtraction_Audio,
		kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription, sizeof(asbd), &asbd, NULL);
	QTCheckError(qt_status, wxString(_T("Failed to retreive audio properties")));

	sample_rate			= (int)asbd.mSampleRate;
	channels			= 1; // FIXME: allow more than one channel
	bytes_per_sample	= 2;

	// lazy hack: set the movie time scale to same as the sample rate, to allow for easy seeking
	SetMovieTimeScale(movie, (TimeScale)asbd.mSampleRate);
	num_samples = GetMovieDuration(movie);

	asbd.mFormatFlags		= kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked | kAudioFormatFlagsNativeEndian;
	asbd.mBitsPerChannel	= sizeof(int16_t) * 8;
	asbd.mBytesPerFrame		= sizeof(int16_t);
	asbd.mBytesPerPacket	= asbd.mBytesPerFrame;
	asbd.mChannelsPerFrame	= 1;

	qt_status = MovieAudioExtractionSetProperty(extract_ref, kQTPropertyClass_MovieAudioExtraction_Audio, 
		kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription, sizeof(asbd), &asbd);
	QTCheckError(qt_status, wxString(_T("Failed to set audio properties")));

	AudioChannelLayout ch_layout;
	ch_layout.mChannelLayoutTag				= kAudioChannelLayoutTag_Mono;
	ch_layout.mChannelBitmap				= 0;
	ch_layout.mNumberChannelDescriptions	= 0;
	qt_status = MovieAudioExtractionSetProperty(extract_ref, kQTPropertyClass_MovieAudioExtraction_Audio,
		kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout, sizeof(ch_layout), &ch_layout);
	QTCheckError(qt_status, wxString(_T("Failed to set channel layout")));
}



/// @brief DOCME
/// @param buf   
/// @param start 
/// @param count 
///
void QuickTimeAudioProvider::GetAudio(void *buf, int64_t start, int64_t count) {
	TimeRecord trec;
	trec.scale		= GetMovieTimeScale(movie);
	trec.base		= NULL;
	trec.value.hi	= (int32_t)(start >> 32);
	trec.value.lo	= (int32_t)((start & 0xFFFFFFFF00000000ULL) >> 32);

	qt_status = MovieAudioExtractionSetProperty(extract_ref, kQTPropertyClass_MovieAudioExtraction_Movie,
		kQTMovieAudioExtractionMoviePropertyID_CurrentTime, sizeof(TimeRecord), &trec);
	QTCheckError(qt_status, wxString(_T("QuickTime audio provider: Failed to seek in file")));

	// FIXME: hack something up to actually handle very big counts correctly,
	// maybe with multiple buffers?
	AudioBufferList dst_buflist;
	dst_buflist.mNumberBuffers = 1;
	dst_buflist.mBuffers[0].mNumberChannels = 1;
	dst_buflist.mBuffers[0].mDataByteSize	= count * bytes_per_sample;
	dst_buflist.mBuffers[0].mData			= buf;

	UInt32 flags;
	UInt32 decode_count = (UInt32)count;
	qt_status = MovieAudioExtractionFillBuffer(extract_ref, &decode_count, &dst_buflist, &flags);
	QTCheckError(qt_status, wxString(_T("QuickTime audio provider: Failed to decode audio")));

	if (count != decode_count)
		wxLogDebug(_T("QuickTime audio provider: GetAudio: Warning: decoded samplecount %d not same as requested count %d"),
			decode_count, (uint32_t)count);
}


#endif /* WITH_QUICKTIME */



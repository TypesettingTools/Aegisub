//  Copyright (c) 2007-2009 Fredrik Mellbin
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

extern "C" {
#include <libavcodec/avcodec.h>
}

#include "ffms.h"
#include "ffvideosource.h"
#include "ffaudiosource.h"
#include "indexing.h"

#ifdef __UNIX__
#define _snprintf snprintf
#endif

TFrameInfo::TFrameInfo(int64_t DTS, bool KeyFrame) {
	this->DTS = DTS;
	this->SampleStart = 0;
	this->FilePos = 0;
	this->FrameSize = 0;
	this->KeyFrame = KeyFrame;
}

TFrameInfo::TFrameInfo(int64_t DTS, int64_t SampleStart, bool KeyFrame) {
	this->DTS = DTS;
	this->SampleStart = SampleStart;
	this->FilePos = 0;
	this->FrameSize = 0;
	this->KeyFrame = KeyFrame;
}

TFrameInfo::TFrameInfo(int64_t SampleStart, int64_t FilePos, unsigned int FrameSize, bool KeyFrame) {
	this->DTS = 0;
	this->SampleStart = SampleStart;
	this->FilePos = FilePos;
	this->FrameSize = FrameSize;
	this->KeyFrame = KeyFrame;
}

FFMS_API(void) FFMS_Init() {
	static bool InitDone = false;
	if (!InitDone) {
		av_register_all();
		av_log_set_level(AV_LOG_QUIET);
	}
	InitDone = true;
}

FFMS_API(int) FFMS_GetLogLevel() {
	return av_log_get_level();
}

FFMS_API(void) FFMS_SetLogLevel(int Level) {
	av_log_set_level(AV_LOG_QUIET);
}

FFMS_API(FFVideo *) FFMS_CreateVideoSource(const char *SourceFile, int Track, FFIndex *Index, const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize) {
	try {
		switch (Index->Decoder) {
			case 0: return new FFVideoSource(SourceFile, Track, Index, PP, Threads, SeekMode, ErrorMsg, MsgSize);
			case 1: return new MatroskaVideoSource(SourceFile, Track, Index, PP, Threads, ErrorMsg, MsgSize);
#ifdef HAALISOURCE
			case 2: return new HaaliVideoSource(SourceFile, Track, Index, PP, Threads, 0, ErrorMsg, MsgSize);
			case 3: return new HaaliVideoSource(SourceFile, Track, Index, PP, Threads, 1, ErrorMsg, MsgSize);
#endif
			default: 
				_snprintf(ErrorMsg, MsgSize, "Unsupported format");
				return NULL;
		}
	} catch (...) {
		return NULL;
	}
}

FFMS_API(FFAudio *) FFMS_CreateAudioSource(const char *SourceFile, int Track, FFIndex *Index, char *ErrorMsg, unsigned MsgSize) {
	try {
		switch (Index->Decoder) {
			case 0: return new FFAudioSource(SourceFile, Track, Index, ErrorMsg, MsgSize);
			case 1: return new MatroskaAudioSource(SourceFile, Track, Index, ErrorMsg, MsgSize);
			default: 
				_snprintf(ErrorMsg, MsgSize, "Unsupported format");
				return NULL;
		}
	} catch (...) {
		return NULL;
	}
}

FFMS_API(void) FFMS_DestroyVideoSource(FFVideo *V) {
	delete V;
}

FFMS_API(void) FFMS_DestroyAudioSource(FFAudio *A) {
	delete A;
}

FFMS_API(const TVideoProperties *) FFMS_GetTVideoProperties(FFVideo *V) {
	return &V->GetTVideoProperties();
}

FFMS_API(const TAudioProperties *) FFMS_GetTAudioProperties(FFAudio *A) {
	return &A->GetTAudioProperties();
}

FFMS_API(const TAVFrameLite *) FFMS_GetFrame(FFVideo *V, int n, char *ErrorMsg, unsigned MsgSize) {
	return (TAVFrameLite *)V->GetFrame(n, ErrorMsg, MsgSize);
}

FFMS_API(const TAVFrameLite *) FFMS_GetFrameByTime(FFVideo *V, double Time, char *ErrorMsg, unsigned MsgSize) {
	return (TAVFrameLite *)V->GetFrameByTime(Time, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_GetAudio(FFAudio *A, void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize) {
	return A->GetAudio(Buf, Start, Count, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_SetOutputFormat(FFVideo *V, int TargetFormat, int Width, int Height, char *ErrorMsg, unsigned MsgSize) {
	return V->SetOutputFormat(TargetFormat, Width, Height, ErrorMsg, MsgSize);
}

FFMS_API(void) FFMS_ResetOutputFormat(FFVideo *V) {
	V->ResetOutputFormat();
}

FFMS_API(void) FFMS_DestroyFFIndex(FFIndex *Index) {
	delete Index;
}

FFMS_API(int) FFMS_GetFirstTrackOfType(FFIndex *Index, int TrackType, char *ErrorMsg, unsigned MsgSize) {
	for (int i = 0; i < static_cast<int>(Index->size()); i++)
		if ((*Index)[i].TT == TrackType)
			return i;
	_snprintf(ErrorMsg, MsgSize, "No suitable track found");
	return -1;
}

FFMS_API(int) FFMS_GetNumTracks(FFIndex *Index) {
	return Index->size();
}

FFMS_API(int) FFMS_GetTrackType(FFTrack *T) {
	return T->TT;
}

FFMS_API(int) FFMS_GetNumFrames(FFTrack *T) {
	return T->size();
}

FFMS_API(const TFrameInfo *) FFMS_GetTFrameInfo(FFTrack *T, int Frame, char *ErrorMsg, unsigned MsgSize) {
	if (Frame < 0 || Frame >= static_cast<int>(T->size())) {
		_snprintf(ErrorMsg, MsgSize, "Invalid frame specified");
		return NULL;
	} else {
		return &(*T)[Frame];
	}	
}

FFMS_API(FFTrack *) FFMS_GetTITrackIndex(FFIndex *Index, int Track, char *ErrorMsg, unsigned MsgSize) {
	if (Track < 0 || Track >= static_cast<int>(Index->size())) {
		_snprintf(ErrorMsg, MsgSize, "Invalid track specified");
		return NULL;
	} else {
		return &(*Index)[Track];
	}	
}

FFMS_API(FFTrack *) FFMS_GetVSTrackIndex(FFVideo *V) {
	return V->GetFFTrack();
}

FFMS_API(int) FFMS_FindClosestKeyFrame(FFTrack *T, int Frame, char *ErrorMsg, unsigned MsgSize) {
	if (Frame < 0 || Frame >= static_cast<int>(T->size())) {
		_snprintf(ErrorMsg, MsgSize, "Out of range frame specified");
		return -1;
	} else {
		return T->FindClosestKeyFrame(Frame);
	}
}

FFMS_API(const TTrackTimeBase *) FFMS_GetTimeBase(FFTrack *T) {
	return &T->TB;
}

FFMS_API(int) FFMS_WriteTimecodes(FFTrack *T, const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize) {
	return T->WriteTimecodes(TimecodeFile, ErrorMsg, MsgSize);
}

FFMS_API(FFIndex *) FFMS_MakeIndex(const char *SourceFile, int IndexMask, int DumpMask, const char *AudioFile, bool IgnoreDecodeErrors, TIndexCallback IP, void *Private, char *ErrorMsg, unsigned MsgSize) {
	return MakeIndex(SourceFile, IndexMask, DumpMask, AudioFile, IgnoreDecodeErrors, IP, Private, ErrorMsg, MsgSize);
}

FFMS_API(FFIndex *) FFMS_ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize) {
	return ReadIndex(IndexFile, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_WriteIndex(const char *IndexFile, FFIndex *Index, char *ErrorMsg, unsigned MsgSize) {
	return WriteIndex(IndexFile, Index, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_GetPixFmt(const char *Name) {
	return avcodec_get_pix_fmt(Name);
}

FFMS_API(int) FFMS_DefaultAudioName(const char *SourceFile, int Track, const TAudioProperties *AP, char *FileName, unsigned FNSize) {

	return 0;
}
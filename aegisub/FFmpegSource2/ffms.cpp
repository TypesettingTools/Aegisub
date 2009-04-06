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

#include "ffms.h"
#include "ffvideosource.h"
#include "ffaudiosource.h"
#include "indexing.h"

#ifdef __UNIX__
#define _snprintf snprintf
#endif

FrameInfo::FrameInfo(int64_t DTS, bool KeyFrame) {
	this->DTS = DTS;
	this->SampleStart = 0;
	this->FilePos = 0;
	this->FrameSize = 0;
	this->KeyFrame = KeyFrame;
}

FrameInfo::FrameInfo(int64_t DTS, int64_t SampleStart, bool KeyFrame) {
	this->DTS = DTS;
	this->SampleStart = SampleStart;
	this->FilePos = 0;
	this->FrameSize = 0;
	this->KeyFrame = KeyFrame;
}

FrameInfo::FrameInfo(int64_t SampleStart, int64_t FilePos, unsigned int FrameSize, bool KeyFrame) {
	this->DTS = 0;
	this->SampleStart = SampleStart;
	this->FilePos = FilePos;
	this->FrameSize = FrameSize;
	this->KeyFrame = KeyFrame;
}

FFMS_API(void) FFMS_Init() {
	static bool InitDone = false;
	if (!InitDone)
		av_register_all();
	InitDone = true;
}

FFMS_API(void) FFMS_NoLog() {
	av_log_set_level(AV_LOG_QUIET);
}

FFMS_API(VideoBase *) FFMS_CreateVideoSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize) {
	try {
		switch (TrackIndices->Decoder) {
			case 0: return new FFVideoSource(SourceFile, Track, TrackIndices, PP, Threads, SeekMode, ErrorMsg, MsgSize);
			case 1: return new MatroskaVideoSource(SourceFile, Track, TrackIndices, PP, Threads, ErrorMsg, MsgSize);
#ifdef HAALITS
			case 2: return new HaaliTSVideoSource(SourceFile, Track, TrackIndices, PP, Threads, ErrorMsg, MsgSize);
#endif
			default: 
				_snprintf(ErrorMsg, MsgSize, "Unsupported format");
				return NULL;
		}
	} catch (...) {
		return NULL;
	}
}

FFMS_API(AudioBase *) FFMS_CreateAudioSource(const char *SourceFile, int Track, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize) {
	try {
		switch (TrackIndices->Decoder) {
			case 0: return new FFAudioSource(SourceFile, Track, TrackIndices, ErrorMsg, MsgSize);
			case 1: return new MatroskaAudioSource(SourceFile, Track, TrackIndices, ErrorMsg, MsgSize);
			default: 
				_snprintf(ErrorMsg, MsgSize, "Unsupported format");
				return NULL;
		}
	} catch (...) {
		return NULL;
	}
}

FFMS_API(void) FFMS_DestroyVideoSource(VideoBase *VB) {
	delete VB;
}

FFMS_API(void) FFMS_DestroyAudioSource(AudioBase *AB) {
	delete AB;
}

FFMS_API(const VideoProperties *) FFMS_GetVideoProperties(VideoBase *VB) {
	return &VB->GetVideoProperties();
}

FFMS_API(const AudioProperties *) FFMS_GetAudioProperties(AudioBase *AB) {
	return &AB->GetAudioProperties();
}

FFMS_API(const AVFrameLite *) FFMS_GetFrame(VideoBase *VB, int n, char *ErrorMsg, unsigned MsgSize) {
	return (AVFrameLite *)VB->GetFrame(n, ErrorMsg, MsgSize);
}

FFMS_API(const AVFrameLite *) FFMS_GetFrameByTime(VideoBase *VB, double Time, char *ErrorMsg, unsigned MsgSize) {
	return (AVFrameLite *)VB->GetFrameByTime(Time, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_GetAudio(AudioBase *AB, void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize) {
	return AB->GetAudio(Buf, Start, Count, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_SetOutputFormat(VideoBase *VB, int TargetFormat, int Width, int Height, char *ErrorMsg, unsigned MsgSize) {
	return VB->SetOutputFormat(TargetFormat, Width, Height, ErrorMsg, MsgSize);
}

FFMS_API(void) FFMS_ResetOutputFormat(VideoBase *VB) {
	VB->ResetOutputFormat();
}

FFMS_API(void) FFMS_DestroyFrameIndex(FrameIndex *FI) {
	delete FI;
}

FFMS_API(int) FFMS_GetFirstTrackOfType(FrameIndex *TrackIndices, int TrackType, char *ErrorMsg, unsigned MsgSize) {
	for (int i = 0; i < TrackIndices->size(); i++)
		if ((*TrackIndices)[i].TT == TrackType)
			return i;
	_snprintf(ErrorMsg, MsgSize, "No suitable track found");
	return -1;
}

FFMS_API(int) FFMS_GetNumTracks(FrameIndex *TrackIndices) {
	return TrackIndices->size();
}

FFMS_API(int) FFMS_GetTrackType(FrameInfoVector *FIV) {
	return FIV->TT;
}

FFMS_API(int) FFMS_GetNumFrames(FrameInfoVector *FIV) {
	return FIV->size();
}

FFMS_API(const FrameInfo *) FFMS_GetFrameInfo(FrameInfoVector *FIV, int Frame, char *ErrorMsg, unsigned MsgSize) {
	if (Frame < 0 || Frame >= FIV->size()) {
		_snprintf(ErrorMsg, MsgSize, "Invalid frame specified");
		return NULL;
	} else {
		return &(*FIV)[Frame];
	}	
}

FFMS_API(FrameInfoVector *) FFMS_GetTITrackIndex(FrameIndex *TrackIndices, int Track, char *ErrorMsg, unsigned MsgSize) {
	if (Track < 0 || Track >= TrackIndices->size()) {
		_snprintf(ErrorMsg, MsgSize, "Invalid track specified");
		return NULL;
	} else {
		return &(*TrackIndices)[Track];
	}	
}

FFMS_API(FrameInfoVector *) FFMS_GetVSTrackIndex(VideoBase *VB) {
	return VB->GetFrameInfoVector();
}

FFMS_API(int) FFMS_FindClosestKeyFrame(FrameInfoVector *FIV, int Frame, char *ErrorMsg, unsigned MsgSize) {
	if (Frame < 0 || Frame >= FIV->size()) {
		_snprintf(ErrorMsg, MsgSize, "Out of range frame specified");
		return -1;
	} else {
		return FIV->FindClosestKeyFrame(Frame);
	}
}

FFMS_API(int) FFMS_FrameFromDTS(FrameInfoVector *FIV, int64_t DTS) {
	return FIV->FrameFromDTS(DTS);
}

FFMS_API(int) FFMS_ClosestFrameFromDTS(FrameInfoVector *FIV, int64_t DTS) {
	return FIV->ClosestFrameFromDTS(DTS);
}

FFMS_API(const TrackTimeBase *) FFMS_GetTimeBase(FrameInfoVector *FIV) {
	return &FIV->TB;
}

FFMS_API(int) FFMS_WriteTimecodes(FrameInfoVector *FIV, const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize) {
	return FIV->WriteTimecodes(TimecodeFile, ErrorMsg, MsgSize);
}

FFMS_API(FrameIndex *) FFMS_MakeIndex(const char *SourceFile, int IndexMask, int DumpMask, const char *AudioFile, bool IgnoreDecodeErrors, IndexCallback IP, void *Private, char *ErrorMsg, unsigned MsgSize) {
	return MakeIndex(SourceFile, IndexMask, DumpMask, AudioFile, IgnoreDecodeErrors, IP, Private, ErrorMsg, MsgSize);
}

FFMS_API(FrameIndex *) FFMS_ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize) {
	return ReadIndex(IndexFile, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_WriteIndex(const char *IndexFile, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize) {
	return WriteIndex(IndexFile, TrackIndices, ErrorMsg, MsgSize);
}


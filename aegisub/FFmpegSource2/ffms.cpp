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

static int InitCount = 0;

FFMS_API(int) FFMS_Init() {
	if (!InitCount++) {
		av_register_all();
		av_log_set_level(AV_LOG_QUIET);
#ifdef HAALISOURCE
		if (::CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK)
			return 1;
#endif
	}
	return 0;
}


FFMS_API(void) FFMS_DeInit() {
	if (!--InitCount) {
#ifdef HAALISOURCE
		::CoUninitialize();
#endif
	}
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
			case 0: return new FFLAVFVideo(SourceFile, Track, Index, PP, Threads, SeekMode, ErrorMsg, MsgSize);
			case 1: return new FFMatroskaVideo(SourceFile, Track, Index, PP, Threads, ErrorMsg, MsgSize);
#ifdef HAALISOURCE
			case 2: return new FFHaaliVideo(SourceFile, Track, Index, PP, Threads, 0, ErrorMsg, MsgSize);
			case 3: return new FFHaaliVideo(SourceFile, Track, Index, PP, Threads, 1, ErrorMsg, MsgSize);
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
			case 0: return new FFLAVFAudio(SourceFile, Track, Index, ErrorMsg, MsgSize);
			case 1: return new FFMatroskaAudio(SourceFile, Track, Index, ErrorMsg, MsgSize);
#ifdef HAALISOURCE
			case 2: return new FFHaaliAudio(SourceFile, Track, Index, 0, ErrorMsg, MsgSize);
			case 3: return new FFHaaliAudio(SourceFile, Track, Index, 1, ErrorMsg, MsgSize);
#endif
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

FFMS_API(const TVideoProperties *) FFMS_GetVideoProperties(FFVideo *V) {
	return &V->GetTVideoProperties();
}

FFMS_API(const TAudioProperties *) FFMS_GetAudioProperties(FFAudio *A) {
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
	_snprintf(ErrorMsg, MsgSize, "No suitable, indexed track found");
	return -1;
}

FFMS_API(int) FFMS_GetFirstIndexedTrackOfType(FFIndex *Index, int TrackType, char *ErrorMsg, unsigned MsgSize) {
	for (int i = 0; i < static_cast<int>(Index->size()); i++)
		if ((*Index)[i].TT == TrackType && (*Index)[i].size() > 0)
			return i;
	_snprintf(ErrorMsg, MsgSize, "No suitable, indexed track found");
	return -1;
}

FFMS_API(int) FFMS_GetNumTracks(FFIndex *Index) {
	return Index->size();
}

FFMS_API(int) FFMS_GetNumTracksI(FFIndexer *Indexer, int Track) {
	return Indexer->GetNumberOfTracks();
}

FFMS_API(FFMS_TrackType) FFMS_GetTrackType(FFTrack *T) {
	return T->TT;
}

FFMS_API(FFMS_TrackType) FFMS_GetTrackTypeI(FFIndexer *Indexer, int Track) {
	return Indexer->GetTrackType(Track);
}

FFMS_API(int) FFMS_GetNumFrames(FFTrack *T) {
	return T->size();
}

FFMS_API(const FFFrameInfo *) FFMS_GetFrameInfo(FFTrack *T, int Frame, char *ErrorMsg, unsigned MsgSize) {
	if (Frame < 0 || Frame >= static_cast<int>(T->size())) {
		_snprintf(ErrorMsg, MsgSize, "Invalid frame specified");
		return NULL;
	} else {
		return reinterpret_cast<FFFrameInfo *>(&(*T)[Frame]);
	}	
}

FFMS_API(FFTrack *) FFMS_GetTrackFromIndex(FFIndex *Index, int Track, char *ErrorMsg, unsigned MsgSize) {
	if (Track < 0 || Track >= static_cast<int>(Index->size())) {
		_snprintf(ErrorMsg, MsgSize, "Invalid track specified");
		return NULL;
	} else {
		return &(*Index)[Track];
	}	
}

FFMS_API(FFTrack *) FFMS_GetTrackFromVideo(FFVideo *V) {
	return V->GetFFTrack();
}

FFMS_API(FFTrack *) FFMS_GetTrackFromAudio(FFAudio *A) {
	return A->GetFFTrack();
}

FFMS_API(const TTrackTimeBase *) FFMS_GetTimeBase(FFTrack *T) {
	return &T->TB;
}

FFMS_API(int) FFMS_WriteTimecodes(FFTrack *T, const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize) {
	return T->WriteTimecodes(TimecodeFile, ErrorMsg, MsgSize);
}

FFMS_API(FFIndex *) FFMS_MakeIndex(const char *SourceFile, int IndexMask, int DumpMask, TAudioNameCallback ANC, void *ANCPrivate, bool IgnoreDecodeErrors, TIndexCallback IC, void *ICPrivate, char *ErrorMsg, unsigned MsgSize) {
	FFIndexer *Indexer = FFMS_CreateIndexer(SourceFile, ErrorMsg, MsgSize);
	if (!Indexer)
		return NULL;
	return FFMS_DoIndexing(Indexer, IndexMask, DumpMask, ANC, ANCPrivate, IgnoreDecodeErrors, IC, ICPrivate, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_DefaultAudioFilename(const char *SourceFile, int Track, const TAudioProperties *AP, char *FileName, void *Private) {
	const char * FormatString = "%s.%d2.w64";
	if (FileName == NULL)
		return _snprintf(NULL, 0, FormatString, SourceFile, Track) + 1;
	else
		return _snprintf(FileName, 999999, FormatString, SourceFile, Track) + 1;
}

FFMS_API(FFIndexer *) FFMS_CreateIndexer(const char *SourceFile, char *ErrorMsg, unsigned MsgSize) {
	try {
		return FFIndexer::CreateFFIndexer(SourceFile, ErrorMsg, MsgSize);
	} catch (...) {
		return NULL;
	}
}

FFMS_API(FFIndex *) FFMS_DoIndexing(FFIndexer *Indexer, int IndexMask, int DumpMask, TAudioNameCallback ANC, void *ANCPrivate, bool IgnoreDecodeErrors, TIndexCallback IC, void *ICPrivate, char *ErrorMsg, unsigned MsgSize) {
	Indexer->SetIndexMask(IndexMask);
	Indexer->SetDumpMask(DumpMask);
	Indexer->SetIgnoreDecodeErrors(IgnoreDecodeErrors);
	Indexer->SetProgressCallback(IC, ICPrivate);
	Indexer->SetAudioNameCallback(ANC, ANCPrivate);
	FFIndex *Index = Indexer->DoIndexing(ErrorMsg, MsgSize);
	delete Indexer;
	return Index;
}

FFMS_API(void) FFMS_CancelIndexing(FFIndexer *Indexer) {
	delete Indexer;
}

FFMS_API(FFIndex *) FFMS_ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize) {
	FFIndex *Index = new FFIndex();
	if (Index->ReadIndex(IndexFile, ErrorMsg, MsgSize)) {
		delete Index;
		return NULL;
	} else {
		return Index;
	}
}

FFMS_API(int) FFMS_WriteIndex(const char *IndexFile, FFIndex *Index, char *ErrorMsg, unsigned MsgSize) {
	return Index->WriteIndex(IndexFile, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_GetPixFmt(const char *Name) {
	return avcodec_get_pix_fmt(Name);
}

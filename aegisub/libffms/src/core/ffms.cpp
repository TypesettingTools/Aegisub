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

#include <sstream>
#include <iomanip>
#include "ffms.h"
#include "ffvideosource.h"
#include "ffaudiosource.h"
#include "indexing.h"



static bool FFmpegInited = false;
int CPUFeatures = 0;

#ifdef FFMS_WIN_DEBUG

extern "C" int av_log_level;

void av_log_windebug_callback(void* ptr, int level, const char* fmt, va_list vl)
{
    static int print_prefix=1;
    static int count;
    static char line[1024], prev[1024];
    AVClass* avc= ptr ? *(AVClass**)ptr : NULL;
    if(level>av_log_level)
        return;
#undef fprintf
    if(print_prefix && avc) {
        snprintf(line, sizeof(line), "[%s @ %p]", avc->item_name(ptr), ptr);
    }else
        line[0]=0;

    vsnprintf(line + strlen(line), sizeof(line) - strlen(line), fmt, vl);

    print_prefix= line[strlen(line)-1] == '\n';
    if(print_prefix && !strcmp(line, prev)){
        count++;
        return;
    }
    if(count>0){
		fprintf(stderr, "    Last message repeated %d times\n", count);
        count=0;
    }
	OutputDebugStringA(line);
    strcpy(prev, line);
}

#endif

FFMS_API(void) FFMS_Init(int CPUFeatures) {
	if (!FFmpegInited) {
		av_register_all();
#ifdef FFMS_WIN_DEBUG
		av_log_set_callback(av_log_windebug_callback);
		av_log_set_level(AV_LOG_INFO);
#else
		av_log_set_level(AV_LOG_QUIET);
#endif
		::CPUFeatures = CPUFeatures;
		FFmpegInited = true;
	}
}

FFMS_API(int) FFMS_GetLogLevel() {
	return av_log_get_level();
}

FFMS_API(void) FFMS_SetLogLevel(int Level) {
	av_log_set_level(Level);
}

FFMS_API(FFVideo *) FFMS_CreateVideoSource(const char *SourceFile, int Track, FFIndex *Index, const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize) {
	if (Track < 0 || Track >= static_cast<int>(Index->size())) {
		snprintf(ErrorMsg, MsgSize, "Out of bounds track index selected");
		return NULL;
	}

	if (Index->at(Track).TT != FFMS_TYPE_VIDEO) {
		snprintf(ErrorMsg, MsgSize, "Not a video track");
		return NULL;
	}

	try {
		switch (Index->Decoder) {
			case 0: return new FFLAVFVideo(SourceFile, Track, Index, PP, Threads, SeekMode, ErrorMsg, MsgSize);
			case 1: return new FFMatroskaVideo(SourceFile, Track, Index, PP, Threads, ErrorMsg, MsgSize);
#ifdef HAALISOURCE
			case 2: return new FFHaaliVideo(SourceFile, Track, Index, PP, Threads, 0, ErrorMsg, MsgSize);
			case 3: return new FFHaaliVideo(SourceFile, Track, Index, PP, Threads, 1, ErrorMsg, MsgSize);
#endif
			default: 
				snprintf(ErrorMsg, MsgSize, "Unsupported format");
				return NULL;
		}
	} catch (...) {
		return NULL;
	}
}

FFMS_API(FFAudio *) FFMS_CreateAudioSource(const char *SourceFile, int Track, FFIndex *Index, char *ErrorMsg, unsigned MsgSize) {
	if (Track < 0 || Track >= static_cast<int>(Index->size())) {
		snprintf(ErrorMsg, MsgSize, "Out of bounds track index selected");
		return NULL;
	}

	if (Index->at(Track).TT != FFMS_TYPE_AUDIO) {
		snprintf(ErrorMsg, MsgSize, "Not an audio track");
		return NULL;
	}

	try {
		switch (Index->Decoder) {
			case 0: return new FFLAVFAudio(SourceFile, Track, Index, ErrorMsg, MsgSize);
			case 1: return new FFMatroskaAudio(SourceFile, Track, Index, ErrorMsg, MsgSize);
#ifdef HAALISOURCE
			case 2: return new FFHaaliAudio(SourceFile, Track, Index, 0, ErrorMsg, MsgSize);
			case 3: return new FFHaaliAudio(SourceFile, Track, Index, 1, ErrorMsg, MsgSize);
#endif
			default: 
				snprintf(ErrorMsg, MsgSize, "Unsupported format");
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

FFMS_API(const FFVideoProperties *) FFMS_GetVideoProperties(FFVideo *V) {
	return &V->GetFFVideoProperties();
}

FFMS_API(const FFAudioProperties *) FFMS_GetAudioProperties(FFAudio *A) {
	return &A->GetFFAudioProperties();
}

FFMS_API(const FFAVFrame *) FFMS_GetFrame(FFVideo *V, int n, char *ErrorMsg, unsigned MsgSize) {
	return (FFAVFrame *)V->GetFrame(n, ErrorMsg, MsgSize);
}

FFMS_API(const FFAVFrame *) FFMS_GetFrameByTime(FFVideo *V, double Time, char *ErrorMsg, unsigned MsgSize) {
	return (FFAVFrame *)V->GetFrameByTime(Time, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_GetAudio(FFAudio *A, void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize) {
	return A->GetAudio(Buf, Start, Count, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_SetOutputFormatV(FFVideo *V, int64_t TargetFormats, int Width, int Height, int Resizer, char *ErrorMsg, unsigned MsgSize) {
	return V->SetOutputFormat(TargetFormats, Width, Height, Resizer, ErrorMsg, MsgSize);
}

FFMS_API(void) FFMS_ResetOutputFormatV(FFVideo *V) {
	V->ResetOutputFormat();
}

FFMS_API(void) FFMS_DestroyIndex(FFIndex *Index) {
	delete Index;
}

FFMS_API(int) FFMS_GetFirstTrackOfType(FFIndex *Index, int TrackType, char *ErrorMsg, unsigned MsgSize) {
	for (int i = 0; i < static_cast<int>(Index->size()); i++)
		if ((*Index)[i].TT == TrackType)
			return i;
	snprintf(ErrorMsg, MsgSize, "No suitable, indexed track found");
	return -1;
}

FFMS_API(int) FFMS_GetFirstIndexedTrackOfType(FFIndex *Index, int TrackType, char *ErrorMsg, unsigned MsgSize) {
	for (int i = 0; i < static_cast<int>(Index->size()); i++)
		if ((*Index)[i].TT == TrackType && (*Index)[i].size() > 0)
			return i;
	snprintf(ErrorMsg, MsgSize, "No suitable, indexed track found");
	return -1;
}

FFMS_API(int) FFMS_GetNumTracks(FFIndex *Index) {
	return Index->size();
}

FFMS_API(int) FFMS_GetNumTracksI(FFIndexer *Indexer) {
	return Indexer->GetNumberOfTracks();
}

FFMS_API(int) FFMS_GetTrackType(FFTrack *T) {
	return T->TT;
}

FFMS_API(int) FFMS_GetTrackTypeI(FFIndexer *Indexer, int Track) {
	return Indexer->GetTrackType(Track);
}

FFMS_API(const char *) FFMS_GetCodecNameI(FFIndexer *Indexer, int Track) {
	return Indexer->GetTrackCodec(Track);
}

FFMS_API(int) FFMS_GetNumFrames(FFTrack *T) {
	return T->size();
}

FFMS_API(const FFFrameInfo *) FFMS_GetFrameInfo(FFTrack *T, int Frame) {
	return reinterpret_cast<FFFrameInfo *>(&(*T)[Frame]);
}

FFMS_API(FFTrack *) FFMS_GetTrackFromIndex(FFIndex *Index, int Track) {
	return &(*Index)[Track];
}

FFMS_API(FFTrack *) FFMS_GetTrackFromVideo(FFVideo *V) {
	return V->GetFFTrack();
}

FFMS_API(FFTrack *) FFMS_GetTrackFromAudio(FFAudio *A) {
	return A->GetFFTrack();
}

FFMS_API(const FFTrackTimeBase *) FFMS_GetTimeBase(FFTrack *T) {
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

/* Used by FFMS_DefaultAudioFilename */ 

static std::string IntToStr(int i, int zp = 0) {
	std::stringstream s;
	s.fill('0');
	s.width(zp);
	s << i;
	return s.str();
}

static void ReplaceString(std::string &s, std::string from, std::string to) {
	int idx;
	while ((idx = s.find(from)) != std::string::npos)
		s.replace(idx, from.length(), to);
}

FFMS_API(int) FFMS_DefaultAudioFilename(const char *SourceFile, int Track, const FFAudioProperties *AP, char *FileName, int FNSize, void *Private) {
	std::string s = static_cast<char *>(Private);

	ReplaceString(s, "%sourcefile%", SourceFile);
	ReplaceString(s, "%trackn%", IntToStr(Track));
	ReplaceString(s, "%trackzn%", IntToStr(Track, 2));
	ReplaceString(s, "%samplerate%", IntToStr(AP->SampleRate));
	ReplaceString(s, "%channels%", IntToStr(AP->Channels));
	ReplaceString(s, "%bps%", IntToStr(AP->BitsPerSample));
	ReplaceString(s, "%delay%", IntToStr(static_cast<int>(AP->FirstTime)));
	
	if (FileName == NULL) {
		return s.length() + 1;
	} else {
		strcpy(FileName, s.c_str());
		return 0;
	}
}

FFMS_API(FFIndexer *) FFMS_CreateIndexer(const char *SourceFile, char *ErrorMsg, unsigned MsgSize) {
	try {
		return FFIndexer::CreateFFIndexer(SourceFile, ErrorMsg, MsgSize);
	} catch (...) {
		return NULL;
	}
}

FFMS_API(FFIndex *) FFMS_DoIndexing(FFIndexer *Indexer, int IndexMask, int DumpMask, TAudioNameCallback ANC, void *ANCPrivate, bool IgnoreDecodeErrors, TIndexCallback IC, void *ICPrivate, char *ErrorMsg, unsigned MsgSize) {
	Indexer->SetIndexMask(IndexMask | DumpMask);
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

FFMS_API(int) FFMS_IndexBelongsToFile(FFIndex *Index, const char *SourceFile, char *ErrorMsg, unsigned MsgSize) {
	return Index->CompareFileSignature(SourceFile, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_WriteIndex(const char *IndexFile, FFIndex *Index, char *ErrorMsg, unsigned MsgSize) {
	return Index->WriteIndex(IndexFile, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_GetPixFmt(const char *Name) {
	return avcodec_get_pix_fmt(Name);
}

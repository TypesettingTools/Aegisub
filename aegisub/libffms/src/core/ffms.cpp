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



static bool FFmpegInited	= false;
bool HasHaaliMPEG = false;
bool HasHaaliOGG = false;
int CPUFeatures = 0;

#ifdef FFMS_WIN_DEBUG

extern "C" int av_log_level;

void av_log_windebug_callback(void* ptr, int level, const char* fmt, va_list vl) {
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
#ifdef HAALISOURCE
		CComPtr<IMMContainer> pMMC;
		HasHaaliMPEG = !FAILED(pMMC.CoCreateInstance(HAALI_MPEG_PARSER));
		pMMC = NULL;
		HasHaaliOGG = !FAILED(pMMC.CoCreateInstance(HAALI_OGG_PARSER));
		pMMC = NULL;
#endif
		FFmpegInited = true;
	}
}

FFMS_API(int) FFMS_GetLogLevel() {
	return av_log_get_level();
}

FFMS_API(void) FFMS_SetLogLevel(int Level) {
	av_log_set_level(Level);
}

FFMS_API(FFMS_VideoSource *) FFMS_CreateVideoSource(const char *SourceFile, int Track, FFMS_Index *Index, const char *PP, int Threads, int SeekMode, char *ErrorMsg, unsigned MsgSize) {
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
			case FFMS_SOURCE_LAVF:
				return new FFLAVFVideo(SourceFile, Track, Index, PP, Threads, SeekMode, ErrorMsg, MsgSize);
			case FFMS_SOURCE_MATROSKA:
				return new FFMatroskaVideo(SourceFile, Track, Index, PP, Threads, ErrorMsg, MsgSize);
#ifdef HAALISOURCE
			case FFMS_SOURCE_HAALIMPEG:
				if (HasHaaliMPEG)
					return new FFHaaliVideo(SourceFile, Track, Index, PP, Threads, 0, ErrorMsg, MsgSize);
				snprintf(ErrorMsg, MsgSize, "Haali MPEG/TS source unavailable");
				return NULL;
			case FFMS_SOURCE_HAALIOGG:
				if (HasHaaliOGG)
					return new FFHaaliVideo(SourceFile, Track, Index, PP, Threads, 1, ErrorMsg, MsgSize);
				snprintf(ErrorMsg, MsgSize, "Haali OGG/OGM source unavailable");
				return NULL;
#endif
			default: 
				snprintf(ErrorMsg, MsgSize, "Unsupported format");
				return NULL;
		}
	} catch (...) {
		return NULL;
	}
}

FFMS_API(FFMS_AudioSource *) FFMS_CreateAudioSource(const char *SourceFile, int Track, FFMS_Index *Index, char *ErrorMsg, unsigned MsgSize) {
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
			case FFMS_SOURCE_LAVF:
				return new FFLAVFAudio(SourceFile, Track, Index, ErrorMsg, MsgSize);
			case FFMS_SOURCE_MATROSKA:
				return new FFMatroskaAudio(SourceFile, Track, Index, ErrorMsg, MsgSize);
#ifdef HAALISOURCE
			case FFMS_SOURCE_HAALIMPEG:
				if (HasHaaliMPEG)
					return new FFHaaliAudio(SourceFile, Track, Index, 0, ErrorMsg, MsgSize);
				snprintf(ErrorMsg, MsgSize, "Haali MPEG/TS source unavailable");
				return NULL;
			case FFMS_SOURCE_HAALIOGG:
				if (HasHaaliOGG)
					return new FFHaaliAudio(SourceFile, Track, Index, 1, ErrorMsg, MsgSize);
				snprintf(ErrorMsg, MsgSize, "Haali OGG/OGM source unavailable");
				return NULL;
#endif
			default: 
				snprintf(ErrorMsg, MsgSize, "Unsupported format");
				return NULL;
		}
	} catch (...) {
		return NULL;
	}
}

FFMS_API(void) FFMS_DestroyVideoSource(FFMS_VideoSource *V) {
	delete V;
}

FFMS_API(void) FFMS_DestroyAudioSource(FFMS_AudioSource *A) {
	delete A;
}

FFMS_API(const FFMS_VideoProperties *) FFMS_GetVideoProperties(FFMS_VideoSource *V) {
	return &V->GetVideoProperties();
}

FFMS_API(const FFMS_AudioProperties *) FFMS_GetAudioProperties(FFMS_AudioSource *A) {
	return &A->GetAudioProperties();
}

FFMS_API(const FFMS_Frame *) FFMS_GetFrame(FFMS_VideoSource *V, int n, char *ErrorMsg, unsigned MsgSize) {
	return (FFMS_Frame *)V->GetFrame(n, ErrorMsg, MsgSize);
}

FFMS_API(const FFMS_Frame *) FFMS_GetFrameByTime(FFMS_VideoSource *V, double Time, char *ErrorMsg, unsigned MsgSize) {
	return (FFMS_Frame *)V->GetFrameByTime(Time, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_GetAudio(FFMS_AudioSource *A, void *Buf, int64_t Start, int64_t Count, char *ErrorMsg, unsigned MsgSize) {
	return A->GetAudio(Buf, Start, Count, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_SetOutputFormatV(FFMS_VideoSource *V, int64_t TargetFormats, int Width, int Height, int Resizer, char *ErrorMsg, unsigned MsgSize) {
	return V->SetOutputFormat(TargetFormats, Width, Height, Resizer, ErrorMsg, MsgSize);
}

FFMS_API(void) FFMS_ResetOutputFormatV(FFMS_VideoSource *V) {
	V->ResetOutputFormat();
}

FFMS_API(void) FFMS_DestroyIndex(FFMS_Index *Index) {
	delete Index;
}

FFMS_API(int) FFMS_GetFirstTrackOfType(FFMS_Index *Index, int TrackType, char *ErrorMsg, unsigned MsgSize) {
	for (int i = 0; i < static_cast<int>(Index->size()); i++)
		if ((*Index)[i].TT == TrackType)
			return i;
	snprintf(ErrorMsg, MsgSize, "No suitable, indexed track found");
	return -1;
}

FFMS_API(int) FFMS_GetFirstIndexedTrackOfType(FFMS_Index *Index, int TrackType, char *ErrorMsg, unsigned MsgSize) {
	for (int i = 0; i < static_cast<int>(Index->size()); i++)
		if ((*Index)[i].TT == TrackType && (*Index)[i].size() > 0)
			return i;
	snprintf(ErrorMsg, MsgSize, "No suitable, indexed track found");
	return -1;
}

FFMS_API(int) FFMS_GetNumTracks(FFMS_Index *Index) {
	return Index->size();
}

FFMS_API(int) FFMS_GetNumTracksI(FFMS_Indexer *Indexer) {
	return Indexer->GetNumberOfTracks();
}

FFMS_API(int) FFMS_GetTrackType(FFMS_Track *T) {
	return T->TT;
}

FFMS_API(int) FFMS_GetTrackTypeI(FFMS_Indexer *Indexer, int Track) {
	return Indexer->GetTrackType(Track);
}

FFMS_API(const char *) FFMS_GetCodecNameI(FFMS_Indexer *Indexer, int Track) {
	return Indexer->GetTrackCodec(Track);
}

FFMS_API(int) FFMS_GetNumFrames(FFMS_Track *T) {
	return T->size();
}

FFMS_API(const FFMS_FrameInfo *) FFMS_GetFrameInfo(FFMS_Track *T, int Frame) {
	return reinterpret_cast<FFMS_FrameInfo *>(&(*T)[Frame]);
}

FFMS_API(FFMS_Track *) FFMS_GetTrackFromIndex(FFMS_Index *Index, int Track) {
	return &(*Index)[Track];
}

FFMS_API(FFMS_Track *) FFMS_GetTrackFromVideo(FFMS_VideoSource *V) {
	return V->GetFFTrack();
}

FFMS_API(FFMS_Track *) FFMS_GetTrackFromAudio(FFMS_AudioSource *A) {
	return A->GetFFTrack();
}

FFMS_API(const FFMS_TrackTimeBase *) FFMS_GetTimeBase(FFMS_Track *T) {
	return &T->TB;
}

FFMS_API(int) FFMS_WriteTimecodes(FFMS_Track *T, const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize) {
	return T->WriteTimecodes(TimecodeFile, ErrorMsg, MsgSize);
}

FFMS_API(FFMS_Index *) FFMS_MakeIndex(const char *SourceFile, int IndexMask, int DumpMask, TAudioNameCallback ANC, void *ANCPrivate, bool IgnoreDecodeErrors, TIndexCallback IC, void *ICPrivate, char *ErrorMsg, unsigned MsgSize) {
	FFMS_Indexer *Indexer = FFMS_CreateIndexer(SourceFile, ErrorMsg, MsgSize);
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

FFMS_API(int) FFMS_DefaultAudioFilename(const char *SourceFile, int Track, const FFMS_AudioProperties *AP, char *FileName, int FNSize, void *Private) {
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

FFMS_API(FFMS_Indexer *) FFMS_CreateIndexer(const char *SourceFile, char *ErrorMsg, unsigned MsgSize) {
	try {
		return FFMS_Indexer::CreateIndexer(SourceFile, ErrorMsg, MsgSize);
	} catch (...) {
		return NULL;
	}
}

FFMS_API(FFMS_Index *) FFMS_DoIndexing(FFMS_Indexer *Indexer, int IndexMask, int DumpMask, TAudioNameCallback ANC, void *ANCPrivate, bool IgnoreDecodeErrors, TIndexCallback IC, void *ICPrivate, char *ErrorMsg, unsigned MsgSize) {
	Indexer->SetIndexMask(IndexMask | DumpMask);
	Indexer->SetDumpMask(DumpMask);
	Indexer->SetIgnoreDecodeErrors(IgnoreDecodeErrors);
	Indexer->SetProgressCallback(IC, ICPrivate);
	Indexer->SetAudioNameCallback(ANC, ANCPrivate);
	FFMS_Index *Index = Indexer->DoIndexing(ErrorMsg, MsgSize);
	delete Indexer;
	return Index;
}

FFMS_API(void) FFMS_CancelIndexing(FFMS_Indexer *Indexer) {
	delete Indexer;
}

FFMS_API(FFMS_Index *) FFMS_ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize) {
	FFMS_Index *Index = new FFMS_Index();
	if (Index->ReadIndex(IndexFile, ErrorMsg, MsgSize)) {
		delete Index;
		return NULL;
	} else {
		return Index;
	}
}

FFMS_API(int) FFMS_IndexBelongsToFile(FFMS_Index *Index, const char *SourceFile, char *ErrorMsg, unsigned MsgSize) {
	return Index->CompareFileSignature(SourceFile, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_WriteIndex(const char *IndexFile, FFMS_Index *Index, char *ErrorMsg, unsigned MsgSize) {
	return Index->WriteIndex(IndexFile, ErrorMsg, MsgSize);
}

FFMS_API(int) FFMS_GetPixFmt(const char *Name) {
	return avcodec_get_pix_fmt(Name);
}

FFMS_API(int) FFMS_GetPresentSources() {
	int Sources = FFMS_SOURCE_LAVF | FFMS_SOURCE_MATROSKA;
#ifdef HAALISOURCE
	Sources |= FFMS_SOURCE_HAALIMPEG | FFMS_SOURCE_HAALIOGG;
#endif
	return Sources;
}

FFMS_API(int) FFMS_GetEnabledSources() {
	if (!FFmpegInited)
		return 0;
	int Sources = FFMS_SOURCE_LAVF | FFMS_SOURCE_MATROSKA;
	if (HasHaaliMPEG)
		Sources |= FFMS_SOURCE_HAALIMPEG;
	if (HasHaaliOGG)
		Sources |= FFMS_SOURCE_HAALIOGG;
	return Sources;
}

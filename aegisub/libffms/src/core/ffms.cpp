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
#include "videosource.h"
#include "audiosource.h"
#include "indexing.h"

extern "C" {
#include <libavutil/pixdesc.h>
}


#ifdef FFMS_WIN_DEBUG
#	include <windows.h>
#endif

static bool FFmpegInited	= false;
bool HasHaaliMPEG = false;
bool HasHaaliOGG = false;
int CPUFeatures = 0;
bool GlobalUseUTF8Paths = false;


#ifdef FFMS_WIN_DEBUG

extern "C" int av_log_level;

void av_log_windebug_callback(void* ptr, int level, const char* fmt, va_list vl) {
	static int print_prefix=1;
	static int count;
	static char line[1024] = {0}, prev[1024] = {0};
	AVClass* avc = ptr ? *(AVClass**)ptr : NULL;
	if(level > av_log_level)
		return;

	int written = 0;
	if(print_prefix && avc) {
		written = snprintf(line, sizeof(line), "[%s @ %p]", avc->item_name(ptr), ptr);
	}

	written += vsnprintf(line + written, sizeof(line) - written, fmt, vl);

	print_prefix = line[written-1] == '\n';
	line[sizeof(line) - 1] = 0;
	if(print_prefix && !strcmp(line, prev)){
		count++;
		return;
	}
	if(count > 0){
		std::stringstream ss;
		ss << "    Last message repeated " << count << " times\n";
		OutputDebugStringA(ss.str().c_str());
		count = 0;
	}
	OutputDebugStringA(line);
	strcpy(prev, line);
}

#endif

FFMS_API(void) FFMS_Init(int CPUFeatures, int UseUTF8Paths) {
	if (!FFmpegInited) {
		av_register_all();
#ifdef _WIN32
		if (UseUTF8Paths) {
			ffms_patch_lavf_file_open();
			GlobalUseUTF8Paths = true;
		}
		else {
			GlobalUseUTF8Paths = false;
		}
#else
		GlobalUseUTF8Paths = false;
#endif
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

FFMS_API(FFMS_VideoSource *) FFMS_CreateVideoSource(const char *SourceFile, int Track, FFMS_Index *Index, int Threads, int SeekMode, FFMS_ErrorInfo *ErrorInfo) {
	if (Threads < 1)
		Threads = 1;
	try {
		switch (Index->Decoder) {
			case FFMS_SOURCE_LAVF:
				return new FFLAVFVideo(SourceFile, Track, Index, Threads, SeekMode);
			case FFMS_SOURCE_MATROSKA:
				return new FFMatroskaVideo(SourceFile, Track, Index, Threads);
#ifdef HAALISOURCE
			case FFMS_SOURCE_HAALIMPEG:
				if (HasHaaliMPEG)
					return new FFHaaliVideo(SourceFile, Track, Index, Threads, FFMS_SOURCE_HAALIMPEG);
				throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_NOT_AVAILABLE, "Haali MPEG/TS source unavailable");
			case FFMS_SOURCE_HAALIOGG:
				if (HasHaaliOGG)
					return new FFHaaliVideo(SourceFile, Track, Index, Threads, FFMS_SOURCE_HAALIOGG);
				throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_NOT_AVAILABLE, "Haali OGG/OGM source unavailable");
#endif
			default: 
				throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, "Unsupported format");
		}
	} catch (FFMS_Exception &e) {
		e.CopyOut(ErrorInfo);
		return NULL;
	}
}

FFMS_API(FFMS_AudioSource *) FFMS_CreateAudioSource(const char *SourceFile, int Track, FFMS_Index *Index, int DelayMode, FFMS_ErrorInfo *ErrorInfo) {
	try {
		switch (Index->Decoder) {
			case FFMS_SOURCE_LAVF:
				return new FFLAVFAudio(SourceFile, Track, *Index, DelayMode);
			case FFMS_SOURCE_MATROSKA:
				return new FFMatroskaAudio(SourceFile, Track, *Index, DelayMode);
#ifdef HAALISOURCE
			case FFMS_SOURCE_HAALIMPEG:
				if (HasHaaliMPEG)
					return new FFHaaliAudio(SourceFile, Track, *Index, FFMS_SOURCE_HAALIMPEG, DelayMode);
				throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_NOT_AVAILABLE, "Haali MPEG/TS source unavailable");
			case FFMS_SOURCE_HAALIOGG:
				if (HasHaaliOGG)
					return new FFHaaliAudio(SourceFile, Track, *Index, FFMS_SOURCE_HAALIOGG, DelayMode);
				throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_NOT_AVAILABLE, "Haali OGG/OGM source unavailable");
#endif
			default: 
				throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, "Unsupported format");
		}
	} catch (FFMS_Exception &e) {
		e.CopyOut(ErrorInfo);
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

FFMS_API(const FFMS_Frame *) FFMS_GetFrame(FFMS_VideoSource *V, int n, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);
	try {
		return V->GetFrame(n);
	} catch (FFMS_Exception &e) {
		e.CopyOut(ErrorInfo);
		return NULL;
	}
}

FFMS_API(const FFMS_Frame *) FFMS_GetFrameByTime(FFMS_VideoSource *V, double Time, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);
	try {
		return (FFMS_Frame *)V->GetFrameByTime(Time);
	} catch (FFMS_Exception &e) {
		e.CopyOut(ErrorInfo);
		return NULL;
	}
}

FFMS_API(int) FFMS_GetAudio(FFMS_AudioSource *A, void *Buf, int64_t Start, int64_t Count, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);
	try {
		A->GetAudio(Buf, Start, Count);
	} catch (FFMS_Exception &e) {
		return e.CopyOut(ErrorInfo);
	}
	return FFMS_ERROR_SUCCESS;
}

FFMS_API(int) FFMS_SetOutputFormatV(FFMS_VideoSource *V, int64_t TargetFormats, int Width, int Height, int Resizer, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);
	try {
		V->SetOutputFormat(TargetFormats, Width, Height, Resizer);
	} catch (FFMS_Exception &e) {
		return e.CopyOut(ErrorInfo);
	}
	return FFMS_ERROR_SUCCESS;
}

FFMS_API(void) FFMS_ResetOutputFormatV(FFMS_VideoSource *V) {
	V->ResetOutputFormat();
}

FFMS_API(int) FFMS_SetPP(FFMS_VideoSource *V, const char *PP, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);
	try {
		V->SetPP(PP);
	} catch (FFMS_Exception &e) {
		return e.CopyOut(ErrorInfo);
	}
	return FFMS_ERROR_SUCCESS;
}

FFMS_API(void) FFMS_ResetPP(FFMS_VideoSource *V) {
	V->ResetPP();
}

FFMS_API(void) FFMS_DestroyIndex(FFMS_Index *Index) {
	delete Index;
}

FFMS_API(int) FFMS_GetSourceType(FFMS_Index *Index) {
	return Index->Decoder;
}

FFMS_API(int) FFMS_GetFirstTrackOfType(FFMS_Index *Index, int TrackType, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);
	for (int i = 0; i < static_cast<int>(Index->size()); i++)
		if ((*Index)[i].TT == TrackType)
			return i;
	
	try {
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_NOT_AVAILABLE,
			"No suitable, indexed track found");
	} catch (FFMS_Exception &e) {
		e.CopyOut(ErrorInfo);
		return -1;
	}
}

FFMS_API(int) FFMS_GetFirstIndexedTrackOfType(FFMS_Index *Index, int TrackType, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);
	for (int i = 0; i < static_cast<int>(Index->size()); i++)
		if ((*Index)[i].TT == TrackType && (*Index)[i].size() > 0)
			return i;
	try {
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_NOT_AVAILABLE,
			"No suitable, indexed track found");
	} catch (FFMS_Exception &e) {
		e.CopyOut(ErrorInfo);
		return -1;
	}
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
	return V->GetTrack();
}

FFMS_API(FFMS_Track *) FFMS_GetTrackFromAudio(FFMS_AudioSource *A) {
	return A->GetTrack();
}

FFMS_API(const FFMS_TrackTimeBase *) FFMS_GetTimeBase(FFMS_Track *T) {
	return &T->TB;
}

FFMS_API(int) FFMS_WriteTimecodes(FFMS_Track *T, const char *TimecodeFile, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);
	try {
		T->WriteTimecodes(TimecodeFile);
	} catch (FFMS_Exception &e) {
		return e.CopyOut(ErrorInfo);
	}
	return FFMS_ERROR_SUCCESS;
}

FFMS_API(FFMS_Index *) FFMS_MakeIndex(const char *SourceFile, int IndexMask, int DumpMask, TAudioNameCallback ANC, void *ANCPrivate, int ErrorHandling, TIndexCallback IC, void *ICPrivate, FFMS_ErrorInfo *ErrorInfo) {
	FFMS_Indexer *Indexer = FFMS_CreateIndexer(SourceFile, ErrorInfo);
	if (!Indexer)
		return NULL;
	return FFMS_DoIndexing(Indexer, IndexMask, DumpMask, ANC, ANCPrivate, ErrorHandling, IC, ICPrivate, ErrorInfo);
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
	
	if (FileName != NULL)
		strcpy(FileName, s.c_str());

	return s.length() + 1;
}

FFMS_API(FFMS_Indexer *) FFMS_CreateIndexer(const char *SourceFile, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);
	try {
		return FFMS_Indexer::CreateIndexer(SourceFile);
	} catch (FFMS_Exception &e) {
		e.CopyOut(ErrorInfo);
		return NULL;
	}
}

FFMS_API(FFMS_Index *) FFMS_DoIndexing(FFMS_Indexer *Indexer, int IndexMask, int DumpMask, TAudioNameCallback ANC, void *ANCPrivate, int ErrorHandling, TIndexCallback IC, void *ICPrivate, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);

	Indexer->SetIndexMask(IndexMask | DumpMask);
	Indexer->SetDumpMask(DumpMask);
	Indexer->SetErrorHandling(ErrorHandling);
	Indexer->SetProgressCallback(IC, ICPrivate);
	Indexer->SetAudioNameCallback(ANC, ANCPrivate);

	FFMS_Index *Index = NULL;
	try {
		Index = Indexer->DoIndexing();
	} catch (FFMS_Exception &e) {
		e.CopyOut(ErrorInfo);
	}
	delete Indexer;
	return Index;
}

FFMS_API(void) FFMS_CancelIndexing(FFMS_Indexer *Indexer) {
	delete Indexer;
}

FFMS_API(FFMS_Index *) FFMS_ReadIndex(const char *IndexFile, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);
	FFMS_Index *Index = new FFMS_Index();
	try {
		Index->ReadIndex(IndexFile);
	} catch (FFMS_Exception &e) {
		delete Index;
		e.CopyOut(ErrorInfo);
		return NULL;
	}
	return Index;
}

FFMS_API(int) FFMS_IndexBelongsToFile(FFMS_Index *Index, const char *SourceFile, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);
	try {
		if (!Index->CompareFileSignature(SourceFile))
			throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_FILE_MISMATCH,
				"The index does not belong to the file");
	} catch (FFMS_Exception &e) {
		return e.CopyOut(ErrorInfo);
	}
	return FFMS_ERROR_SUCCESS;
}

FFMS_API(int) FFMS_WriteIndex(const char *IndexFile, FFMS_Index *Index, FFMS_ErrorInfo *ErrorInfo) {
	ClearErrorInfo(ErrorInfo);
	try {
		Index->WriteIndex(IndexFile);
	} catch (FFMS_Exception &e) {
		return e.CopyOut(ErrorInfo);
	}
	return FFMS_ERROR_SUCCESS;
}

FFMS_API(int) FFMS_GetPixFmt(const char *Name) {
	return av_get_pix_fmt(Name);
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

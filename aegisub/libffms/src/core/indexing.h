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

#ifndef INDEXING_H
#define	INDEXING_H

#include <memory>
#include "utils.h"
#include "wave64writer.h"

#ifdef HAALISOURCE
#	define WIN32_LEAN_AND_MEAN
#	define _WIN32_DCOM
#	include <windows.h>
#	include <tchar.h>
#	include <atlbase.h>
#	include <dshow.h>
#	include <initguid.h>
#	include "CoParser.h"
#	include "guids.h"
#endif



class SharedVideoContext {
private:
	bool FreeCodecContext;
public:
	AVCodecContext *CodecContext;
	AVCodecParserContext *Parser;
	CompressedStream *CS;

	SharedVideoContext(bool FreeCodecContext);
	~SharedVideoContext();
};

class SharedAudioContext {
private:
	bool FreeCodecContext;
public:
	AVCodecContext *CodecContext;
	Wave64Writer *W64Writer;
	int64_t CurrentSample;
	CompressedStream *CS;

	SharedAudioContext(bool FreeCodecContext);
	~SharedAudioContext();
};

struct TFrameInfo {
public:
	FFMS_FRAMEINFO_COMMON
	int64_t SampleStart;
	unsigned int SampleCount;
	int64_t FilePos;
	unsigned int FrameSize;
	size_t OriginalPos;

	TFrameInfo();
	static TFrameInfo VideoFrameInfo(int64_t PTS, int RepeatPict, bool KeyFrame, int64_t FilePos = 0, unsigned int FrameSize = 0);
	static TFrameInfo AudioFrameInfo(int64_t PTS, int64_t SampleStart, unsigned int SampleCount, bool KeyFrame, int64_t FilePos = 0, unsigned int FrameSize = 0);
private:
	TFrameInfo(int64_t PTS, int64_t SampleStart, unsigned int SampleCount, int RepeatPict, bool KeyFrame, int64_t FilePos, unsigned int FrameSize);
};

class FFMS_Track : public std::vector<TFrameInfo> {
public:
	FFMS_TrackType TT;
	FFMS_TrackTimeBase TB;

	int FindClosestVideoKeyFrame(int Frame);
	int FindClosestAudioKeyFrame(int64_t Sample);
	int FrameFromPTS(int64_t PTS);
	int ClosestFrameFromPTS(int64_t PTS);
	void WriteTimecodes(const char *TimecodeFile);

	FFMS_Track();
	FFMS_Track(int64_t Num, int64_t Den, FFMS_TrackType TT);
};

class FFMS_Index : public std::vector<FFMS_Track> {
public:
	static void CalculateFileSignature(const char *Filename, int64_t *Filesize, uint8_t Digest[20]);

	int Decoder;
	int64_t Filesize;
	uint8_t Digest[20];

	void Sort();
	bool CompareFileSignature(const char *Filename);
	void WriteIndex(const char *IndexFile);
	void ReadIndex(const char *IndexFile);

	FFMS_Index();
	FFMS_Index(int64_t Filesize, uint8_t Digest[20]);
};

class FFMS_Indexer {
protected:
	int IndexMask;
	int DumpMask;
	int ErrorHandling;
	TIndexCallback IC;
	void *ICPrivate;
	TAudioNameCallback ANC;
	void *ANCPrivate;
	const char *SourceFile;
	std::vector<int16_t> DecodingBuffer;

	int64_t Filesize;
	uint8_t Digest[20];

	void WriteAudio(SharedAudioContext &AudioContext, FFMS_Index *Index, int Track, int DBSize);
public:
	static FFMS_Indexer *CreateIndexer(const char *Filename);
	FFMS_Indexer(const char *Filename);
	virtual ~FFMS_Indexer();
	void SetIndexMask(int IndexMask);
	void SetDumpMask(int DumpMask);
	void SetErrorHandling(int ErrorHandling);
	void SetProgressCallback(TIndexCallback IC, void *ICPrivate);
	void SetAudioNameCallback(TAudioNameCallback ANC, void *ANCPrivate);
	virtual FFMS_Index *DoIndexing() = 0;
	virtual int GetNumberOfTracks() = 0;
	virtual FFMS_TrackType GetTrackType(int Track) = 0;
	virtual const char *GetTrackCodec(int Track) = 0;
};

class FFLAVFIndexer : public FFMS_Indexer {
private:
	AVFormatContext *FormatContext;
public:
	FFLAVFIndexer(const char *Filename, AVFormatContext *FormatContext);
	~FFLAVFIndexer();
	FFMS_Index *DoIndexing();
	int GetNumberOfTracks();
	FFMS_TrackType GetTrackType(int Track);
	const char *GetTrackCodec(int Track);
};

class FFMatroskaIndexer : public FFMS_Indexer {
private:
	MatroskaFile *MF;
	MatroskaReaderContext MC;
	AVCodec *Codec[32];
public:
	FFMatroskaIndexer(const char *Filename);
	~FFMatroskaIndexer();
	FFMS_Index *DoIndexing();
	int GetNumberOfTracks();
	FFMS_TrackType GetTrackType(int Track);
	const char *GetTrackCodec(int Track);
};

#ifdef HAALISOURCE

class FFHaaliIndexer : public FFMS_Indexer {
private:
	int SourceMode;
	CComPtr<IMMContainer> pMMC;
	int NumTracks;
	FFMS_TrackType TrackType[32];
	AVCodec *Codec[32];
	std::vector<uint8_t> CodecPrivate[32];
	int CodecPrivateSize[32];
	CComQIPtr<IPropertyBag> PropertyBags[32];
	int64_t Duration;
public:
	FFHaaliIndexer(const char *Filename, enum FFMS_Sources SourceMode);
	FFMS_Index *DoIndexing();
	int GetNumberOfTracks();
	FFMS_TrackType GetTrackType(int Track);
	const char *GetTrackCodec(int Track);
};

#endif // HAALISOURCE

#endif

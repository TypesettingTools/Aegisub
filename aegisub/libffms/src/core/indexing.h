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

#define INDEXVERSION 28
#define INDEXID 0x53920873

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
	int64_t FilePos;
	unsigned int FrameSize;

	static TFrameInfo VideoFrameInfo(int64_t DTS, int RepeatPict, bool KeyFrame, int64_t FilePos = 0, unsigned int FrameSize = 0);
	static TFrameInfo AudioFrameInfo(int64_t DTS, int64_t SampleStart, bool KeyFrame, int64_t FilePos = 0, unsigned int FrameSize = 0);
private:
	TFrameInfo(int64_t DTS, int64_t SampleStart, int RepeatPict, bool KeyFrame, int64_t FilePos, unsigned int FrameSize);
};

class FFTrack : public std::vector<TFrameInfo> {
public:
	FFMS_TrackType TT;
	FFTrackTimeBase TB;

	int FindClosestVideoKeyFrame(int Frame);
	int FindClosestAudioKeyFrame(int64_t Sample);
	int FrameFromDTS(int64_t DTS);
	int ClosestFrameFromDTS(int64_t DTS);
	int WriteTimecodes(const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize);

	FFTrack();
	FFTrack(int64_t Num, int64_t Den, FFMS_TrackType TT);
};

class FFIndex : public std::vector<FFTrack> {
public:
	static int CalculateFileSignature(const char *Filename, int64_t *Filesize, uint8_t Digest[20], char *ErrorMsg, unsigned MsgSize);

	int Decoder;
	int64_t Filesize;
	uint8_t Digest[20];

	void Sort();
	int CompareFileSignature(const char *Filename, char *ErrorMsg, unsigned MsgSize);
	int WriteIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize);
	int ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize);

	FFIndex();
	FFIndex(int64_t Filesize, uint8_t Digest[20]);
};

class FFIndexer {
protected:
	int IndexMask;
	int DumpMask;
	bool IgnoreDecodeErrors;
	TIndexCallback IC;
	void *ICPrivate;
	TAudioNameCallback ANC;
	void *ANCPrivate;
	const char *SourceFile;
	std::vector<int16_t> DecodingBuffer;

	int64_t Filesize;
	uint8_t Digest[20];

	bool WriteAudio(SharedAudioContext &AudioContext, FFIndex *Index, int Track, int DBSize, char *ErrorMsg, unsigned MsgSize);
public:
	static FFIndexer *CreateFFIndexer(const char *Filename, char *ErrorMsg, unsigned MsgSize);
	FFIndexer(const char *Filename, char *ErrorMsg, unsigned MsgSize);
	virtual ~FFIndexer();
	void SetIndexMask(int IndexMask);
	void SetDumpMask(int DumpMask);
	void SetIgnoreDecodeErrors(bool IgnoreDecodeErrors);
	void SetProgressCallback(TIndexCallback IC, void *ICPrivate);
	void SetAudioNameCallback(TAudioNameCallback ANC, void *ANCPrivate);
	virtual FFIndex *DoIndexing(char *ErrorMsg, unsigned MsgSize) = 0;
	virtual int GetNumberOfTracks() = 0;
	virtual FFMS_TrackType GetTrackType(int Track) = 0;
	virtual const char *GetTrackCodec(int Track) = 0;
};

class FFLAVFIndexer : public FFIndexer {
private:
	AVFormatContext *FormatContext;
public:
	FFLAVFIndexer(const char *Filename, AVFormatContext *FormatContext, char *ErrorMsg, unsigned MsgSize);
	~FFLAVFIndexer();
	FFIndex *DoIndexing(char *ErrorMsg, unsigned MsgSize);
	int GetNumberOfTracks();
	FFMS_TrackType GetTrackType(int Track);
	const char *GetTrackCodec(int Track);
};

class FFMatroskaIndexer : public FFIndexer {
private:
	MatroskaFile *MF;
	MatroskaReaderContext MC;
	AVCodec *Codec[32];
public:
	FFMatroskaIndexer(const char *Filename, char *ErrorMsg, unsigned MsgSize);
	~FFMatroskaIndexer();
	FFIndex *DoIndexing(char *ErrorMsg, unsigned MsgSize);
	int GetNumberOfTracks();
	FFMS_TrackType GetTrackType(int Track);
	const char *GetTrackCodec(int Track);
};

#ifdef HAALISOURCE

class FFHaaliIndexer : public FFIndexer {
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
	FFHaaliIndexer(const char *Filename, int SourceMode, char *ErrorMsg, unsigned MsgSize);
	FFIndex *DoIndexing(char *ErrorMsg, unsigned MsgSize);
	int GetNumberOfTracks();
	FFMS_TrackType GetTrackType(int Track);
	const char *GetTrackCodec(int Track);
};

#endif // HAALISOURCE

#endif

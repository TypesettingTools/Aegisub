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

#include "utils.h"
#include "ffms.h"

#define INDEXVERSION 24
#define INDEXID 0x53920873

struct IndexHeader {
	uint32_t Id;
	uint32_t Version;
	uint32_t Tracks;
	uint32_t Decoder;
	uint32_t LAVUVersion;
	uint32_t LAVFVersion;
	uint32_t LAVCVersion;
	uint32_t LSWSVersion;
	uint32_t LPPVersion;
};

class FFIndexer {
protected:
	int IndexMask;
	int DumpMask;
	bool IgnoreDecodeErrors;
	TIndexCallback IC;
	void *Private;
public:
	static FFIndexer *CreateFFIndexer(const char *Filename, char *ErrorMsg, unsigned MsgSize);
	virtual ~FFIndexer() { }
	void SetIndexMask(int IndexMask) { this->IndexMask = IndexMask; }
	void SetDumpMask(int DumpMask) { this->DumpMask = DumpMask; }
	void SetIgnoreDecodeErrors(bool IgnoreDecodeErrors) { this->IgnoreDecodeErrors = IgnoreDecodeErrors; }
	void SetProgressCallback(TIndexCallback IC, void *Private) { this->IC = IC; this->Private = Private; }
	virtual FFIndex *DoIndexing(const char *AudioFile, char *ErrorMsg, unsigned MsgSize) = 0;
	virtual int GetNumberOfTracks() = 0;
	virtual FFMS_TrackType GetTrackType(int Track) = 0;
	virtual const char *GetTrackCodec(int Track) = 0;
};

class FFLAVFIndexer : public FFIndexer {
private:
	bool IsIndexing;
	AVFormatContext *FormatContext;
public:
	FFLAVFIndexer(AVFormatContext *FormatContext, char *ErrorMsg, unsigned MsgSize);
	~FFLAVFIndexer();
	FFIndex *DoIndexing(const char *AudioFile, char *ErrorMsg, unsigned MsgSize);
	int GetNumberOfTracks() { return FormatContext->nb_streams; }
	FFMS_TrackType GetTrackType(int Track) { return static_cast<FFMS_TrackType>(FormatContext->streams[Track]->codec->codec_type); }
	const char *GetTrackCodec(int Track) { return FormatContext->streams[Track]->codec->codec_name; }
};

class FFMatroskaIndexer : public FFIndexer {
private:
	MatroskaFile *MF;
	MatroskaReaderContext MC;
public:
	FFMatroskaIndexer(const char *Filename, char *ErrorMsg, unsigned MsgSize);
	FFIndex *DoIndexing(const char *AudioFile, char *ErrorMsg, unsigned MsgSize);
	int GetNumberOfTracks() { return mkv_GetNumTracks(MF); }
	FFMS_TrackType GetTrackType(int Track) { return HaaliTrackTypeToFFTrackType(mkv_GetTrackInfo(MF, Track)->Type); }
	const char *GetTrackCodec(int Track) { return mkv_GetTrackInfo(MF, Track)->CodecID; }
};

class FFHaaliIndexer : public FFIndexer {
private:
	int SourceMode;
	CComPtr<IMMContainer> pMMC;
	int NumTracks;
	FFMS_TrackType TrackType[32];
	AVCodec *Codec[32];
	uint8_t *CodecPrivate[32];
	int CodecPrivateSize[32];
public:
	FFHaaliIndexer(const char *Filename, int SourceMode, char *ErrorMsg, unsigned MsgSize);
	~FFHaaliIndexer() { for (int i = 0; i < 32; i++) delete[] CodecPrivate[i]; }
	FFIndex *DoIndexing(const char *AudioFile, char *ErrorMsg, unsigned MsgSize);
	int GetNumberOfTracks() { return NumTracks; }
	FFMS_TrackType GetTrackType(int Track) { return TrackType[Track]; }
	const char *GetTrackCodec(int Track) { if (Codec[Track]) return Codec[Track]->name; else return "Unsupported codec/Unknown codec name"; }
};

#endif

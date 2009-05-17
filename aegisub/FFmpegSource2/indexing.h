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

#include <vector>
#include "utils.h"
#include "ffms.h"

#define INDEXVERSION 21
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

struct FFTrack : public std::vector<TFrameInfo> {
public:
	int TT;
	TTrackTimeBase TB;

	int FindClosestKeyFrame(int Frame);
	int FrameFromDTS(int64_t DTS);
	int ClosestFrameFromDTS(int64_t DTS);
	int WriteTimecodes(const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize);

	FFTrack();
	FFTrack(int64_t Num, int64_t Den, int TT);
};

struct FFIndex : public std::vector<FFTrack> {
public:
	int Decoder;
	int WriteIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize);
};

FFIndex *MakeIndex(const char *SourceFile, int IndexMask, int DumpMask, const char *AudioFile, bool IgnoreDecodeErrors, TIndexCallback IP, void *Private, char *ErrorMsg, unsigned MsgSize);
FFIndex *ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize);

#endif

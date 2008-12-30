//  Copyright (c) 2007-2008 Fredrik Mellbin
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

#define INDEXVERSION 11
#define INDEXID 0x53920873

struct IndexHeader {
	uint32_t Id;
	uint32_t Version;
	uint32_t Tracks;
	uint32_t Decoder;
};

class FrameInfoVector : public std::vector<FrameInfo> {
public:
	int TT;
	TrackTimeBase TB;

	int FindClosestKeyFrame(int Frame);
	int FrameFromDTS(int64_t DTS);
	int ClosestFrameFromDTS(int64_t DTS);
	int WriteTimecodes(const char *TimecodeFile, char *ErrorMsg, unsigned MsgSize);

	FrameInfoVector();
	FrameInfoVector(int Num, int Den, int TT);
};

class FrameIndex : public std::vector<FrameInfoVector> {
public:
	int Decoder;
};

FrameIndex *MakeIndex(const char *SourceFile, int IndexMask, int DumpMask, const char *AudioFile, bool IgnoreDecodeErrors, IndexCallback IP, void *Private, char *ErrorMsg, unsigned MsgSize);
FrameIndex *ReadIndex(const char *IndexFile, char *ErrorMsg, unsigned MsgSize);
int WriteIndex(const char *IndexFile, FrameIndex *TrackIndices, char *ErrorMsg, unsigned MsgSize);

#endif

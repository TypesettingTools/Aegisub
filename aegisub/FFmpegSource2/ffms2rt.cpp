//  Copyright (c) 2009 Fredrik Mellbin
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
#include <libavutil/log.h>
#include <libavutil/md5.h>
}

#include <cassert>
#include <iostream>
#include <fstream>
#include "ffms.h"

using namespace std;

#define VERBOSE

static int FFMS_CC UpdateProgress(int64_t Current, int64_t Total, void *Private) {

	int LastPercentage = (int)Private;
	int Percentage = int((double(Current)/double(Total)) * 100);

	if (Percentage <= LastPercentage)
		return 0;

	Private = (void *)Percentage;

#ifdef VERBOSE
	cout << "Indexing, please wait... " << Percentage << "% \r" << flush;
#endif

	return 0;
}

void TestFullDump1(char *SrcFile, bool WithAudio) {
	char ErrorMsg[2000];
	int ret = FFMS_Init();
	assert(!ret);

	FFIndexer *FIdx = FFMS_CreateIndexer(SrcFile, ErrorMsg, sizeof(ErrorMsg));
	assert(FIdx);
	FFMS_CancelIndexing(FIdx);
	FIdx = FFMS_CreateIndexer(SrcFile, ErrorMsg, sizeof(ErrorMsg));
	assert(FIdx);

	FFIndex *FI = FFMS_DoIndexing(FIdx, -1, -1, FFMS_DefaultAudioFilename, NULL, false, UpdateProgress, NULL, ErrorMsg, sizeof(ErrorMsg));
	assert(FI);

	int vtrack = FFMS_GetFirstTrackOfType(FI, FFMS_TYPE_VIDEO, ErrorMsg, sizeof(ErrorMsg));
	assert(vtrack >= 0);
	int atrack = FFMS_GetFirstTrackOfType(FI, FFMS_TYPE_AUDIO, ErrorMsg, sizeof(ErrorMsg));
	assert(atrack >= 0);

	FFVideo *V = FFMS_CreateVideoSource(SrcFile, vtrack, FI, "", 1, 1, ErrorMsg, sizeof(ErrorMsg));
	assert(V);

	if (WithAudio) {
		uint8_t *DB = new uint8_t[100000];
		FFAudio *A = FFMS_CreateAudioSource(SrcFile, atrack, FI, ErrorMsg, sizeof(ErrorMsg));
		assert(A);

		const TAudioProperties *AP = FFMS_GetAudioProperties(A);
		for (int i = 0; i < AP->NumSamples / 1000; i++) {
			ret = FFMS_GetAudio(A, DB, i * 1000, 1000, ErrorMsg, sizeof(ErrorMsg));
			assert(!ret);
		}

		FFMS_DestroyAudioSource(A);
		delete[] DB;
	}

	const TVideoProperties *VP = FFMS_GetVideoProperties(V);
	for (int i = 0; i < VP->NumFrames; i++) {
		const TAVFrameLite *AVF =  FFMS_GetFrame(V, i, ErrorMsg, sizeof(ErrorMsg));
		assert(AVF);
	}

	FFMS_DestroyFFIndex(FI);
	FFMS_DestroyVideoSource(V);
	FFMS_DeInit();
}

int main(int argc, char *argv[]) {
	char *TestFiles1[10];
	TestFiles1[0] = "[FLV1]_The_Melancholy_of_Haruhi_Suzumiya_-_Full_Clean_Ending.flv";
	TestFiles1[1] = "jra_jupiter.avi";
	TestFiles1[2] = "h264_16-bframes_16-references_pyramid_crash-indexing.mkv";
	TestFiles1[3] = "pyramid-adaptive-10-bframes.mkv";
	TestFiles1[4] = "negative-timecodes.avi";
	TestFiles1[5] = "Zero1_ITV2_TS_Test.ts";

	for (int i = 0; i < 5; i++)
		TestFullDump1(TestFiles1[5], true);
/*
	TestFullDump1(TestFiles1[5], false);

	for (int i = 0; i < 5; i++)
		TestFullDump1(TestFiles1[i], true);
	TestFullDump1(TestFiles1[5], false);
*/
	return 0;
}

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
#include <libavutil/md5.h>
}

#include <iostream>
#include <fstream>
#include "ffms.h"

using namespace std;

//#define VERBOSE

static int FFMS_CC UpdateProgress(int State, int64_t Current, int64_t Total, void *Private) {

	static int LastPercentage = -1;
	int Percentage = int((double(Current)/double(Total)) * 100);

	if (Percentage <= LastPercentage)
		return 0;

	LastPercentage = Percentage;

#ifdef VERBOSE
	cout << "Indexing " << static_cast<char *>(Private) << ", please wait... " << Percentage << "% \r" << flush;
#endif

	return 0;
}

int main(int argc, char *argv[]) {
	char ErrorMsg[2000];
	AVMD5 *ctx = reinterpret_cast<AVMD5 *>(new uint8_t[av_md5_size]);
	uint8_t md5sum[16];

	if (argc != 2)
		return -1;

	FFMS_Init();
#ifndef VERBOSE
	FFMS_NoLog();
#endif
	
	av_md5_init(ctx);
	FrameIndex *FI = FFMS_MakeIndex(argv[1], -1, 0, NULL, false, UpdateProgress, argv[1], ErrorMsg, sizeof(ErrorMsg));
	if (!FI) {
		cout << "Indexing error: " << ErrorMsg << endl;
		return 1;
	}

	int track = FFMS_GetFirstTrackOfType(FI, FFMS_TYPE_VIDEO, ErrorMsg, sizeof(ErrorMsg));
	if (track < 0) {
		cout << "No usable track error: " << ErrorMsg << endl;
		return 2;
	}

	VideoBase *FV = FFMS_CreateVideoSource(argv[1], track, FI, "", 1, 1, ErrorMsg, sizeof(ErrorMsg));
	FFMS_DestroyFrameIndex(FI);
	if (!FV) {
		cout << "Video source error: " << ErrorMsg << endl;
		return 3;
	}

	const VideoProperties *VP = FFMS_GetVideoProperties(FV);
	for (int i = 0; i < VP->NumFrames; i++) {
		const AVFrameLite *AVF =  FFMS_GetFrame(FV, i, ErrorMsg, sizeof(ErrorMsg));
		if (!AVF) {
			cout << "Frame request error: " << ErrorMsg << " at frame " << i << endl;
			return 4;
		}

#ifdef VERBOSE
	int LastPercentage = -1;
	int Percentage = int((double(i)/double(VP->NumFrames)) * 100);

	if (Percentage > LastPercentage) {
		LastPercentage = Percentage;
		cout << "Requesting frames " << argv[1] << ", please wait... " << Percentage << "% \r" << flush;
	}
#endif

		uint8_t *Data[3];
		Data[0] = AVF->Data[0];
		Data[1] = AVF->Data[1];
		Data[2] = AVF->Data[2];

		switch (VP->VPixelFormat) {
			case PIX_FMT_YUV420P:
			case PIX_FMT_YUVJ420P:
				for (int j = 0; j < VP->Height / 2; j++) {
					av_md5_update(ctx, Data[0], VP->Width);
					Data[0] += AVF->Linesize[0];
					av_md5_update(ctx, Data[0], VP->Width);
					Data[0] += AVF->Linesize[0];
					av_md5_update(ctx, Data[1], VP->Width / 2);
					Data[1] += AVF->Linesize[1];
					av_md5_update(ctx, Data[2], VP->Width / 2);
					Data[2] += AVF->Linesize[2];
				}
				break;
			case PIX_FMT_YUV422P:
				for (int j = 0; j < VP->Height / 2; j++) {
					av_md5_update(ctx, Data[0], VP->Width);
					Data[0] += AVF->Linesize[0];
					av_md5_update(ctx, Data[0], VP->Width);
					Data[0] += AVF->Linesize[0];
					av_md5_update(ctx, Data[1], VP->Width / 2);
					Data[1] += AVF->Linesize[1];
					av_md5_update(ctx, Data[2], VP->Width / 2);
					Data[2] += AVF->Linesize[2];
				}
				break;
		}
	}

	FFMS_DestroyVideoSource(FV);
	av_md5_final(ctx, md5sum);

	delete[] reinterpret_cast<uint8_t *>(ctx);

	cout << "Test successful, MD5: " << hex;
	for (int i = 0; i < sizeof(md5sum); i++)
		cout << static_cast<unsigned>(md5sum[i]);
	cout << endl;
//	cin >> argc;
	return 0;
}
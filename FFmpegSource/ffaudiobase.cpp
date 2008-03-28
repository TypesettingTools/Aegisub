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

#include "ffmpegsource.h"

FFAudioBase::FFAudioBase() {
	memset(&VI, 0, sizeof(VI));
	DecodingBuffer = new uint8_t[AVCODEC_MAX_AUDIO_FRAME_SIZE];
};

FFAudioBase::~FFAudioBase() {
	delete[] DecodingBuffer;
};

size_t FFAudioBase::FindClosestAudioKeyFrame(int64_t Sample) {
	for (size_t i = 0; i < SI.size(); i++) {
		if (SI[i].SampleStart == Sample && SI[i].KeyFrame)
			return i;
		else if (SI[i].SampleStart > Sample && SI[i].KeyFrame)
			return i  - 1;
	}
	return SI.size() - 1;
}

bool FFAudioBase::LoadSampleInfoFromFile(const char *AAudioCacheFile, const char *ASource, int AAudioTrack) {
	char DefaultCacheFilename[1024];
	sprintf(DefaultCacheFilename, "%s.ffas%dcache", ASource, AAudioTrack);
	if (!strcmp(AAudioCacheFile, ""))
		AAudioCacheFile = DefaultCacheFilename;

	FILE *CacheFile = fopen(AAudioCacheFile, "r");
	if (!CacheFile)
		return false;

	size_t AudioBlocks = 0;

	if (fscanf(CacheFile, "%lld %u\r\n", &VI.num_audio_samples, &AudioBlocks) <= 0 || VI.num_audio_samples <= 0 || AudioBlocks <= 0) {
		VI.num_audio_samples = 0;
		fclose(CacheFile);
		return false;
	}

	for (size_t i = 0; i < AudioBlocks; i++) {
		int64_t SampleStart;
		int64_t FilePos;
		unsigned int FrameSize;
		int Flags;

		fscanf(CacheFile, "%lld %lld %u %d\r\n", &SampleStart, &FilePos, &FrameSize, &Flags);
		SI.push_back(SampleInfo(SampleStart, FilePos, FrameSize, (Flags & 1) != 0));
	}

	fclose(CacheFile);
	return true;
}

bool FFAudioBase::SaveSampleInfoToFile(const char *AAudioCacheFile, const char *ASource, int AAudioTrack) {
	char DefaultCacheFilename[1024];
	sprintf(DefaultCacheFilename, "%s.ffas%dcache", ASource, AAudioTrack);
	if (!strcmp(AAudioCacheFile, ""))
		AAudioCacheFile = DefaultCacheFilename;

	FILE *CacheFile = fopen(AAudioCacheFile, "wb");
	if (!CacheFile)
		return false;

	fprintf(CacheFile, "%lld %u\r\n", VI.num_audio_samples, SI.size());
	for (size_t i = 0; i < SI.size(); i++) {
		int Flags = SI[i].KeyFrame ? 1 : 0;
		fprintf(CacheFile, "%lld %lld %u %d\r\n", SI[i].SampleStart, SI[i].FilePos, SI[i].FrameSize, Flags);
	}

	fclose(CacheFile);
	return true;
}
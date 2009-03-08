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

#ifndef WAVE64WRITER_H
#define	WAVE64WRITER_H

#include <stdint.h>
#include <iostream>
#include <fstream>

// this is to avoid depending on windows.h etc.
typedef struct FFMS_WAVEFORMATEX { 
	uint16_t wFormatTag; 
	uint16_t nChannels; 
	uint32_t nSamplesPerSec; 
	uint32_t nAvgBytesPerSec; 
	uint16_t nBlockAlign; 
	uint16_t wBitsPerSample; 
	uint16_t cbSize; 
} FFMS_WAVEFORMATEX;

class Wave64Writer : private std::ofstream {
public:
	Wave64Writer(const char *Filename, uint16_t BitsPerSample, uint16_t Channels, uint32_t SamplesPerSec, bool IsFloat);
	~Wave64Writer();
	void WriteData(void *Data, std::streamsize Length);
private:
	int32_t BitsPerSample;
	int32_t Channels;
	uint32_t SamplesPerSec;
	uint64_t BytesWritten;
	uint32_t HeaderSize;
	bool IsFloat;

	void WriteHeader(bool Initial, bool IsFloat);
};

#endif

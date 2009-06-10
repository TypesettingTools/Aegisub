// Copyright (c) 2005-2006, Rodrigo Braz Monteiro, Fredrik Mellbin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#ifdef WITH_FFMPEG

#ifdef WIN32
#define EMULATE_INTTYPES
#endif
#include <wx/wxprec.h>

/* avcodec.h uses INT64_C in a *single* place. This prolly breaks on Win32,
 * but, well. Let's at least fix it for Linux.
 *
#define __STDC_CONSTANT_MACROS 1
#include <stdint.h>
 * - done in posix/defines.h
 */

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include "mkv_wrap.h"
#include "lavc_file.h"
#include "include/aegisub/audio_provider.h"


///////////////////////
// LAVC Audio Provider
class LAVCAudioProvider : public AudioProvider {
private:
	LAVCFile *lavcfile;

	AVCodecContext *codecContext;
	ReSampleContext *rsct;
	float resample_ratio;
	AVStream *stream;
	int audStream;

	int16_t *buffer;
	int16_t *overshoot_buffer;

	int64_t last_output_sample;

	int leftover_samples;

	void Destroy();

public:
	LAVCAudioProvider(Aegisub::String _filename);
	virtual ~LAVCAudioProvider();

	// Supposedly lavc always returns machine endian samples
	bool AreSamplesNativeEndian() { return true; }

	virtual void GetAudio(void *buf, int64_t start, int64_t count);
};


///////////
// Factory
class LAVCAudioProviderFactory : public AudioProviderFactory {
public:
	AudioProvider *CreateProvider(Aegisub::String file) { return new LAVCAudioProvider(file); }
};

#endif

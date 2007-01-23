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
#define EMULATE_INTTYPES
#include <wx/wxprec.h>
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include "mkv_wrap.h"
#include "lavc_file.h"
#include "audio_provider.h"
#include "lavc_file.h"
#include "utils.h"
#include "options.h"


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

	void Destroy();

public:
	LAVCAudioProvider(wxString _filename);
	virtual ~LAVCAudioProvider();
	virtual void GetAudio(void *buf, __int64 start, __int64 count);
};


///////////
// Factory
class LAVCAudioProviderFactory : public AudioProviderFactory {
public:
	AudioProvider *CreateProvider(wxString file) { return new LAVCAudioProvider(file); }
	AvisynthAudioProviderFactory() : AudioProviderFactory(_T("lavc")) {}
} registerLAVCaudio;


///////////////
// Constructor
LAVCAudioProvider::LAVCAudioProvider(wxString _filename)
	: lavcfile(NULL), codecContext(NULL), rsct(NULL), buffer(NULL)
{
	try {
#if 0
	/* since seeking currently is likely to be horribly broken with two
	 * providers accessing the same stream, this is disabled for now.
	 */
	LAVCVideoProvider *vpro_lavc = dynamic_cast<LAVCVideoProvider *>(vpro);
	if (vpro_lavc) {
		lavcfile = vpro->lavcfile->AddRef();
		filename = vpro_lavc->GetFilename();
	} else {
#endif
		lavcfile = LAVCFile::Create(_filename);
		filename = _filename;
#if 0
	}
#endif
	audStream = -1;
	for (int i = 0; i < lavcfile->fctx->nb_streams; i++) {
		codecContext = lavcfile->fctx->streams[i]->codec;
		if (codecContext->codec_type == CODEC_TYPE_AUDIO) {
			stream = lavcfile->fctx->streams[i];
			audStream = i;
			break;
		}
	}
	if (audStream == -1)
		throw _T("Could not find an audio stream");
	AVCodec *codec = avcodec_find_decoder(codecContext->codec_id);
	if (!codec)
		throw _T("Could not find a suitable audio decoder");
	if (avcodec_open(codecContext, codec) < 0)
		throw _T("Failed to open audio decoder");

	/* aegisub currently supports mono only, so always resample */

	sample_rate = Options.AsInt(_T("Audio Sample Rate"));
	if (!sample_rate)
		sample_rate = codecContext->sample_rate;

	channels = 1;
	bytes_per_sample = 2;

	rsct = audio_resample_init(1, codecContext->channels, sample_rate, codecContext->sample_rate);
	if (!rsct)
		throw _T("Failed to initialize resampling");

	resample_ratio = (float)sample_rate / (float)codecContext->sample_rate;

	double length = (double)stream->duration * av_q2d(stream->time_base);
	num_samples = (__int64)(length * sample_rate);

	buffer = (int16_t *)malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE);
	if (!buffer)
		throw _T("Out of memory");

	} catch (...) {
		Destroy();
		throw;
	}
}


LAVCAudioProvider::~LAVCAudioProvider()
{
	Destroy();
}

void LAVCAudioProvider::Destroy()
{
	if (buffer)
		free(buffer);
	if (rsct)
		audio_resample_close(rsct);
	if (codecContext)
		avcodec_close(codecContext);
	if (lavcfile)
		lavcfile->Release();
}

void LAVCAudioProvider::GetAudio(void *buf, __int64 start, __int64 count)
{
	int16_t *_buf = (int16_t *)buf;
	__int64 _count = num_samples - start;
	if (count < _count)
		_count = count;
	if (_count < 0)
		_count = 0;

	memset(_buf + _count, 0, (count - _count) << 1);

	AVPacket packet;
	while (_count > 0 && av_read_frame(lavcfile->fctx, &packet) >= 0) {
		while (packet.stream_index == audStream) {
			int bytesout = 0, samples;
			if (avcodec_decode_audio(codecContext, buffer, &bytesout, packet.data, packet.size) < 0)
				throw _T("Failed to decode audio");
			if (bytesout == 0)
				break;

			samples = bytesout >> 1;
			if (rsct) {
				if ((__int64)(samples * resample_ratio / codecContext->channels) > _count)
					samples = (__int64)(_count / resample_ratio * codecContext->channels);
				samples = audio_resample(rsct, _buf, buffer, samples / codecContext->channels);

				assert(samples <= _count);
			} else {
				/* currently dead code, rsct != NULL because we're resampling for mono */
				if (samples > _count)
					samples = _count;
				memcpy(_buf, buffer, samples << 1);
			}

			_buf += samples;
			_count -= samples;
			break;
		}

		av_free_packet(&packet);
	}
}

#endif

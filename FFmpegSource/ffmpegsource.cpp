//  Copyright (c) 2007 Fredrik Mellbin
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

static DWORD WINAPI AVFindStreamInfoExecute(AVFormatContext *FormatContext) {
	return av_find_stream_info(FormatContext);
}

int FFmpegSource::GetTrackIndex(int Index, CodecType ATrackType, IScriptEnvironment *Env) {
	if (Index == -1)
		for (unsigned int i = 0; i < FormatContext->nb_streams; i++)
			if (FormatContext->streams[i]->codec->codec_type == ATrackType) {
				Index = i;
				break;
			}

	if (Index == -1)
		Env->ThrowError("FFmpegSource: No %s track found", (ATrackType == CODEC_TYPE_VIDEO) ? "video" : "audio");
	if (Index <= -2)
		return -2;

	if (Index >= (int)FormatContext->nb_streams)
		Env->ThrowError("FFmpegSource: Invalid %s track number", (ATrackType ==  CODEC_TYPE_VIDEO) ? "video" : "audio");

	if (FormatContext->streams[Index]->codec->codec_type != ATrackType)
		Env->ThrowError("FFmpegSource: Selected track is not %s", (ATrackType == CODEC_TYPE_VIDEO) ? "video" : "audio");

	return Index;
}

FFmpegSource::FFmpegSource(const char *ASource, int AVideoTrack, int AAudioTrack, const char *ATimecodes, bool AVCache, const char *AVideoCache, const char *AAudioCache, int AACCompression, const char *APPString, int AQuality, int ASeekMode, IScriptEnvironment* Env) {
	CurrentFrame = 0;
	SeekMode = ASeekMode;

	AVCodecContext *AudioCodecContext = NULL;
	AVCodec *AudioCodec;
	AVCodec *VideoCodec;

	FormatContext = NULL;
	VideoCodecContext = NULL;
	VideoCodec = NULL;

	if (av_open_input_file(&FormatContext, ASource, NULL, 0, NULL) != 0)
		Env->ThrowError("FFmpegSource: Couldn't open '%s'", ASource);
	
	if (av_find_stream_info(FormatContext) < 0)
		Env->ThrowError("FFmpegSource: Couldn't find stream information");

	VideoTrack = GetTrackIndex(AVideoTrack, CODEC_TYPE_VIDEO, Env);
	int AudioTrack = GetTrackIndex(AAudioTrack, CODEC_TYPE_AUDIO, Env);

	bool VCacheIsValid = true;
	bool ACacheIsValid = true;

	if (VideoTrack >= 0) {
		VCacheIsValid = LoadFrameInfoFromFile(AVideoCache, ASource, VideoTrack);

		VideoCodecContext = FormatContext->streams[VideoTrack]->codec;

		VideoCodec = avcodec_find_decoder(VideoCodecContext->codec_id);
		if (VideoCodec == NULL)
			Env->ThrowError("FFmpegSource: Video codec not found");

		if (avcodec_open(VideoCodecContext, VideoCodec) < 0)
			Env->ThrowError("FFmpegSource: Could not open video codec");

		VI.image_type = VideoInfo::IT_TFF;
		VI.width = VideoCodecContext->width;
		VI.height = VideoCodecContext->height;
		VI.fps_denominator = FormatContext->streams[VideoTrack]->time_base.num;
		VI.fps_numerator = FormatContext->streams[VideoTrack]->time_base.den;

		// sanity check framerate
		if (VI.fps_denominator > VI.fps_numerator || VI.fps_denominator <= 0 || VI.fps_numerator <= 0) {
			VI.fps_denominator = 1;
			VI.fps_numerator = 30;
		}
		
		SetOutputFormat(VideoCodecContext->pix_fmt, Env);
		InitPP(VI.width, VI.height, APPString, AQuality, VideoCodecContext->pix_fmt, Env);
	}

	if (AudioTrack >= 0) {
		AudioCodecContext = FormatContext->streams[AudioTrack]->codec;

		AudioCodec = avcodec_find_decoder(AudioCodecContext->codec_id);
		if (AudioCodec == NULL)
			Env->ThrowError("FFmpegSource: Audio codec not found");

		if (avcodec_open(AudioCodecContext, AudioCodec) < 0)
			Env->ThrowError("FFmpegSource: Could not open audio codec");

		switch (AudioCodecContext->sample_fmt) {
			case SAMPLE_FMT_U8: VI.sample_type = SAMPLE_INT8; AACCompression = -1; break;
			case SAMPLE_FMT_S16: VI.sample_type = SAMPLE_INT16; break;
			case SAMPLE_FMT_S24: VI.sample_type = SAMPLE_INT24; AACCompression = -1; break;
			case SAMPLE_FMT_S32: VI.sample_type = SAMPLE_INT32; AACCompression = -1; break;
			case SAMPLE_FMT_FLT: VI.sample_type = SAMPLE_FLOAT; AACCompression = -1; break;
			default:
				Env->ThrowError("FFmpegSource: Unsupported/unknown sample format");
		}

		VI.nchannels = AudioCodecContext->channels;
		VI.audio_samples_per_second = AudioCodecContext->sample_rate;

		ACacheIsValid = OpenAudioCache(AAudioCache, ASource, AudioTrack, Env);
	}

	// Needs to be indexed?
	if (!ACacheIsValid || !VCacheIsValid) {

		FLAC__StreamEncoder *FSE = NULL;
		FILE *RawCache = NULL;
		if (!ACacheIsValid)
			if (AACCompression >= 0)
				AudioCacheType = acFLAC;
			else
				AudioCacheType = acRaw;	

		switch (AudioCacheType) {
			case acFLAC: FSE = NewFLACCacheWriter(AAudioCache, ASource, AudioTrack, AACCompression, Env); break;
			case acRaw: RawCache = NewRawCacheWriter(AAudioCache, ASource, AudioTrack, Env); break;
		}

		AVPacket Packet;
		while (av_read_frame(FormatContext, &Packet) >= 0) {
			if (Packet.stream_index == VideoTrack && !VCacheIsValid) {
				FrameToDTS.push_back(FrameInfo(Packet.dts, (Packet.flags & PKT_FLAG_KEY) ? 1 : 0));
				VI.num_frames++;
			} else if (Packet.stream_index == AudioTrack && !ACacheIsValid) {
				int Size = Packet.size;
				uint8_t *Data = Packet.data;

				while (Size > 0) {
					int TempOutputBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
					int Ret = avcodec_decode_audio2(AudioCodecContext, (int16_t *)DecodingBuffer, &TempOutputBufSize, Data, Size);
					if (Ret < 0)
						Env->ThrowError("FFmpegSource: Audio decoding error");

					int DecodedSamples = VI.AudioSamplesFromBytes(TempOutputBufSize);

					Size -= Ret;
					Data += Ret;
					VI.num_audio_samples += DecodedSamples;

					if (AudioCacheType == acFLAC) {
						for (int i = 0; i < DecodedSamples * VI.nchannels; i++)
							FLACBuffer[i] = ((int16_t *)DecodingBuffer)[i];
						FLAC__stream_encoder_process_interleaved(FSE, FLACBuffer, DecodedSamples); 
					} else if (AudioCacheType == acRaw) {
						fwrite(DecodingBuffer, 1, TempOutputBufSize, RawCache);
					}
				}
			}

			av_free_packet(&Packet);
		}

		if (!ACacheIsValid) {
			switch (AudioCacheType) {
				case acFLAC: CloseFLACCacheWriter(FSE); break;
				case acRaw: CloseRawCacheWriter(RawCache); break;
			}

			ACacheIsValid = OpenAudioCache(AAudioCache, ASource, AudioTrack, Env);
			if (!ACacheIsValid)
				Env->ThrowError("FFmpegSource: Failed to open newly created audio cache for reading");
		}

		if (VideoTrack >= 0 && VI.num_frames == 0)
			Env->ThrowError("FFmpegSource: Video track contains no frames");

		if (AudioTrack >= 0 && VI.num_audio_samples == 0)
			Env->ThrowError("FFmpegSource: Audio track contains no samples");

		if (VideoTrack >= 0)
			av_seek_frame(FormatContext, VideoTrack, FrameToDTS.front().DTS, AVSEEK_FLAG_BACKWARD);

		if (AVCache && !VCacheIsValid)
			if (!SaveFrameInfoToFile(AVideoCache, ASource, VideoTrack))
				Env->ThrowError("FFmpegSource: Failed to write video cache info");
	}

	if (AudioTrack >= 0)
		avcodec_close(AudioCodecContext);

	if (VideoTrack >= 0) {
		if (!SaveTimecodesToFile(ATimecodes, FormatContext->streams[VideoTrack]->time_base.num * 1000, FormatContext->streams[VideoTrack]->time_base.den))
			Env->ThrowError("FFmpegSource: Failed to write timecodes");

		// Adjust framerate to match the duration of the first frame
		if (FrameToDTS.size() >= 2) {
			int64_t DTSDiff = (double)(FrameToDTS[1].DTS - FrameToDTS[0].DTS);
			VI.fps_denominator *= DTSDiff;
		}
	}
}

FFmpegSource::~FFmpegSource() {
	if (VideoTrack >= 0)
		avcodec_close(VideoCodecContext);
	av_close_input_file(FormatContext);
}


int FFmpegSource::DecodeNextFrame(AVFrame *AFrame, int64_t *AStartTime) {
	AVPacket Packet;
	int FrameFinished = 0;
	int Ret = -1;
	*AStartTime = -1;

	while (av_read_frame(FormatContext, &Packet) >= 0) {
        if (Packet.stream_index == VideoTrack) {
			Ret = avcodec_decode_video(VideoCodecContext, AFrame, &FrameFinished, Packet.data, Packet.size);

			if (*AStartTime < 0)
				*AStartTime = Packet.dts;
        }

        av_free_packet(&Packet);

		if (FrameFinished)
			goto Done;
	}

	// Flush the last frame
	if (CurrentFrame == VI.num_frames - 1  && VideoCodecContext->has_b_frames)
		Ret = avcodec_decode_video(VideoCodecContext, AFrame, &FrameFinished, NULL, 0);

Done:
	return Ret;
}

PVideoFrame __stdcall FFmpegSource::GetFrame(int n, IScriptEnvironment* Env) {
	bool HasSeeked = false;
	int ClosestKF = FindClosestKeyFrame(n);

	if (SeekMode == 0) {
		if (n < CurrentFrame) {
			av_seek_frame(FormatContext, VideoTrack, FrameToDTS.front().DTS, AVSEEK_FLAG_BACKWARD);
			avcodec_flush_buffers(VideoCodecContext);
			CurrentFrame = 0;
		}
	} else {
		// 10 frames is used as a margin to prevent excessive seeking since the predicted best keyframe isn't always selected by avformat
		if (n < CurrentFrame || ClosestKF > CurrentFrame + 10 || (SeekMode == 3 && n > CurrentFrame + 10)) {
			av_seek_frame(FormatContext, VideoTrack, (SeekMode == 3) ? FrameToDTS[n].DTS : FrameToDTS[ClosestKF].DTS, AVSEEK_FLAG_BACKWARD);
			avcodec_flush_buffers(VideoCodecContext);
			HasSeeked = true;
		}
	}

	do {
		int64_t StartTime;
		DecodeNextFrame(DecodeFrame, &StartTime);

		if (HasSeeked) {
			HasSeeked = false;

			// Is the seek destination time known? Does it belong to a frame?
			if (StartTime < 0 || (CurrentFrame = FrameFromDTS(StartTime)) < 0) {
				switch (SeekMode) {
					case 1:
						Env->ThrowError("FFmpegSource: Frame accurate seeking is not possible in this file");
					case 2:
					case 3:
						CurrentFrame = ClosestFrameFromDTS(StartTime);
						break;
					default:
						Env->ThrowError("FFmpegSource: Failed assertion");
				}	
			}
		}

		CurrentFrame++;
	} while (CurrentFrame <= n);

	return OutputFrame(DecodeFrame, Env);
}

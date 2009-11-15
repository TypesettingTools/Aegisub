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

#include "indexing.h"



FFLAVFIndexer::FFLAVFIndexer(const char *Filename, AVFormatContext *FormatContext) : FFMS_Indexer(Filename) {
	SourceFile = Filename;
	this->FormatContext = FormatContext;
	
	if (av_find_stream_info(FormatContext) < 0) {
		av_close_input_file(FormatContext);
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ,
			"Couldn't find stream information");
	}
}

FFLAVFIndexer::~FFLAVFIndexer() {
	av_close_input_file(FormatContext);
}

FFMS_Index *FFLAVFIndexer::DoIndexing() {
	std::vector<SharedAudioContext> AudioContexts(FormatContext->nb_streams, SharedAudioContext(false));
	std::vector<SharedVideoContext> VideoContexts(FormatContext->nb_streams, SharedVideoContext(false));

	std::auto_ptr<FFMS_Index> TrackIndices(new FFMS_Index(Filesize, Digest));
	TrackIndices->Decoder = FFMS_SOURCE_LAVF;

	for (unsigned int i = 0; i < FormatContext->nb_streams; i++) {
		TrackIndices->push_back(FFMS_Track((int64_t)FormatContext->streams[i]->time_base.num * 1000, 
			FormatContext->streams[i]->time_base.den,
			static_cast<FFMS_TrackType>(FormatContext->streams[i]->codec->codec_type)));

		if (static_cast<FFMS_TrackType>(FormatContext->streams[i]->codec->codec_type) == FFMS_TYPE_VIDEO &&
			(VideoContexts[i].Parser = av_parser_init(FormatContext->streams[i]->codec->codec_id))) {

			AVCodec *VideoCodec = avcodec_find_decoder(FormatContext->streams[i]->codec->codec_id);
			if (VideoCodec == NULL)	
				throw FFMS_Exception(FFMS_ERROR_CODEC, FFMS_ERROR_UNSUPPORTED,
					"Video codec not found");

			if (avcodec_open(FormatContext->streams[i]->codec, VideoCodec) < 0)	
				throw FFMS_Exception(FFMS_ERROR_CODEC, FFMS_ERROR_DECODING,
					"Could not open video codec");

			VideoContexts[i].CodecContext = FormatContext->streams[i]->codec;
			VideoContexts[i].Parser->flags = PARSER_FLAG_COMPLETE_FRAMES;
		}

		if (IndexMask & (1 << i) && FormatContext->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO) {
			AVCodecContext *AudioCodecContext = FormatContext->streams[i]->codec;

			AVCodec *AudioCodec = avcodec_find_decoder(AudioCodecContext->codec_id);
			if (AudioCodec == NULL)
				throw FFMS_Exception(FFMS_ERROR_CODEC, FFMS_ERROR_UNSUPPORTED,
					"Audio codec not found");

			if (avcodec_open(AudioCodecContext, AudioCodec) < 0)
				throw FFMS_Exception(FFMS_ERROR_CODEC, FFMS_ERROR_DECODING,
					"Could not open audio codec");

			AudioContexts[i].CodecContext = AudioCodecContext;
		} else {
			IndexMask &= ~(1 << i);
		}
	}

	//

	AVPacket Packet, TempPacket;
	InitNullPacket(Packet);
	InitNullPacket(TempPacket);
	while (av_read_frame(FormatContext, &Packet) >= 0) {
		// Update progress
		if (IC) {
			if ((*IC)(FormatContext->pb->pos, FormatContext->file_size, ICPrivate))	
				throw FFMS_Exception(FFMS_ERROR_CANCELLED, FFMS_ERROR_USER,
					"Cancelled by user");
		}

		// Only create index entries for video for now to save space
		if (FormatContext->streams[Packet.stream_index]->codec->codec_type == CODEC_TYPE_VIDEO) {
			uint8_t *OB;
			int OBSize;
			int RepeatPict = -1;

			if (VideoContexts[Packet.stream_index].Parser) {
				av_parser_parse2(VideoContexts[Packet.stream_index].Parser, VideoContexts[Packet.stream_index].CodecContext, &OB, &OBSize, Packet.data, Packet.size, Packet.pts, Packet.dts, Packet.pos);
				RepeatPict = VideoContexts[Packet.stream_index].Parser->repeat_pict;
			}

			(*TrackIndices)[Packet.stream_index].push_back(TFrameInfo::VideoFrameInfo(Packet.dts, RepeatPict, (Packet.flags & AV_PKT_FLAG_KEY) ? 1 : 0));
		} else if (FormatContext->streams[Packet.stream_index]->codec->codec_type == CODEC_TYPE_AUDIO && (IndexMask & (1 << Packet.stream_index))) {
			int64_t StartSample = AudioContexts[Packet.stream_index].CurrentSample;
			AVCodecContext *AudioCodecContext = FormatContext->streams[Packet.stream_index]->codec;
			TempPacket.data = Packet.data;
			TempPacket.size = Packet.size;
			TempPacket.flags = Packet.flags;

			while (TempPacket.size > 0) {
				int dbsize = AVCODEC_MAX_AUDIO_FRAME_SIZE*10;
				int Ret = avcodec_decode_audio3(AudioCodecContext, &DecodingBuffer[0], &dbsize, &TempPacket);
				if (Ret < 0) {
					if (ErrorHandling == FFMS_IEH_ABORT) {
						throw FFMS_Exception(FFMS_ERROR_CODEC, FFMS_ERROR_DECODING,
							"Audio decoding error");
					} else if (ErrorHandling == FFMS_IEH_CLEAR_TRACK) {
						(*TrackIndices)[Packet.stream_index].clear();
						IndexMask &= ~(1 << Packet.stream_index);
						break;
					} else if (ErrorHandling == FFMS_IEH_STOP_TRACK) {
						IndexMask &= ~(1 << Packet.stream_index);
						break;
					} else if (ErrorHandling == FFMS_IEH_IGNORE) {
						break;
					}
				}

				if (Ret > 0) {
					TempPacket.size -= Ret;
					TempPacket.data += Ret;
				}

				if (dbsize > 0)
					AudioContexts[Packet.stream_index].CurrentSample += (dbsize * 8) / (av_get_bits_per_sample_format(AudioCodecContext->sample_fmt) * AudioCodecContext->channels);

				if (DumpMask & (1 << Packet.stream_index))
					WriteAudio(AudioContexts[Packet.stream_index], TrackIndices.get(), Packet.stream_index, dbsize);
			}

			(*TrackIndices)[Packet.stream_index].push_back(TFrameInfo::AudioFrameInfo(Packet.dts, StartSample,
				static_cast<unsigned int>(AudioContexts[Packet.stream_index].CurrentSample - StartSample), (Packet.flags & AV_PKT_FLAG_KEY) ? 1 : 0));
		}

		av_free_packet(&Packet);
	}

	TrackIndices->Sort();
	return TrackIndices.release();
}

int FFLAVFIndexer::GetNumberOfTracks() {
	return FormatContext->nb_streams;
}

FFMS_TrackType FFLAVFIndexer::GetTrackType(int Track) {
	return static_cast<FFMS_TrackType>(FormatContext->streams[Track]->codec->codec_type);
}

const char *FFLAVFIndexer::GetTrackCodec(int Track) { 
	return (avcodec_find_decoder(FormatContext->streams[Track]->codec->codec_id))->name;
}

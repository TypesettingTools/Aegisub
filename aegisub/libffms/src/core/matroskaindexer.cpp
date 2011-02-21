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
#include "matroskaparser.h"



FFMatroskaIndexer::FFMatroskaIndexer(const char *Filename) : FFMS_Indexer(Filename) {
	SourceFile = Filename;
	char ErrorMessage[256];
	for (int i = 0; i < 32; i++) {
		Codec[i] = NULL;
	}

	InitStdIoStream(&MC.ST);
	MC.ST.fp = ffms_fopen(SourceFile, "rb");
	if (MC.ST.fp == NULL) {
		std::ostringstream buf;
		buf << "Can't open '" << SourceFile << "': " << strerror(errno);
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}

	setvbuf(MC.ST.fp, NULL, _IOFBF, CACHESIZE);

	MF = mkv_OpenEx(&MC.ST.base, 0, 0, ErrorMessage, sizeof(ErrorMessage));
	if (MF == NULL) {
		std::ostringstream buf;
		buf << "Can't parse Matroska file: " << ErrorMessage;
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}

	for (unsigned int i = 0; i < mkv_GetNumTracks(MF); i++) {
		TrackInfo *TI = mkv_GetTrackInfo(MF, i);
		Codec[i] = avcodec_find_decoder(MatroskaToFFCodecID(TI->CodecID, TI->CodecPrivate, 0, TI->AV.Audio.BitDepth));
	}
}

FFMatroskaIndexer::~FFMatroskaIndexer() {
	mkv_Close(MF);
}

FFMS_Index *FFMatroskaIndexer::DoIndexing() {
	std::vector<SharedAudioContext> AudioContexts(mkv_GetNumTracks(MF), SharedAudioContext(true));
	std::vector<SharedVideoContext> VideoContexts(mkv_GetNumTracks(MF), SharedVideoContext(true));

	std::auto_ptr<FFMS_Index> TrackIndices(new FFMS_Index(Filesize, Digest));
	TrackIndices->Decoder = FFMS_SOURCE_MATROSKA;

	for (unsigned int i = 0; i < mkv_GetNumTracks(MF); i++) {
		TrackInfo *TI = mkv_GetTrackInfo(MF, i);
		TrackIndices->push_back(FFMS_Track(mkv_TruncFloat(mkv_GetTrackInfo(MF, i)->TimecodeScale), 1000000, HaaliTrackTypeToFFTrackType(mkv_GetTrackInfo(MF, i)->Type)));

		if (!Codec[i]) continue;

		AVCodecContext *CodecContext = avcodec_alloc_context();
		InitializeCodecContextFromMatroskaTrackInfo(TI, CodecContext);

		try {
			if (TI->Type == TT_VIDEO && (VideoContexts[i].Parser = av_parser_init(Codec[i]->id))) {
				if (avcodec_open(CodecContext, Codec[i]) < 0)
					throw FFMS_Exception(FFMS_ERROR_CODEC, FFMS_ERROR_DECODING,
						"Could not open video codec");

				if (TI->CompEnabled)
					VideoContexts[i].TCC = new TrackCompressionContext(MF, TI, i);

				VideoContexts[i].CodecContext = CodecContext;
				VideoContexts[i].Parser->flags = PARSER_FLAG_COMPLETE_FRAMES;
			}
			else if (IndexMask & (1 << i) && TI->Type == TT_AUDIO) {
				if (avcodec_open(CodecContext, Codec[i]) < 0)
					throw FFMS_Exception(FFMS_ERROR_CODEC, FFMS_ERROR_DECODING,
						"Could not open audio codec");

				if (TI->CompEnabled)
					AudioContexts[i].TCC = new TrackCompressionContext(MF, TI, i);

				AudioContexts[i].CodecContext = CodecContext;
			} else {
				IndexMask &= ~(1 << i);
				av_freep(&CodecContext);
			}
		}
		catch (...) {
			av_freep(&CodecContext);
			throw;
		}
	}

	ulonglong StartTime, EndTime, FilePos;
	unsigned int Track, FrameFlags, FrameSize;
	AVPacket TempPacket;
	InitNullPacket(TempPacket);

	while (mkv_ReadFrame(MF, 0, &Track, &StartTime, &EndTime, &FilePos, &FrameSize, &FrameFlags) == 0) {
		// Update progress
		if (IC && (*IC)(ftello(MC.ST.fp), Filesize, ICPrivate))
			throw FFMS_Exception(FFMS_ERROR_CANCELLED, FFMS_ERROR_USER, "Cancelled by user");

		if (mkv_GetTrackInfo(MF, Track)->Type == TT_VIDEO) {
			uint8_t *OB;
			int OBSize;
			int RepeatPict = -1;

			if (VideoContexts[Track].Parser) {
				av_parser_parse2(VideoContexts[Track].Parser, VideoContexts[Track].CodecContext, &OB, &OBSize, TempPacket.data, TempPacket.size, ffms_av_nopts_value, ffms_av_nopts_value, ffms_av_nopts_value);
				RepeatPict = VideoContexts[Track].Parser->repeat_pict;
			}

			(*TrackIndices)[Track].push_back(TFrameInfo::VideoFrameInfo(StartTime, RepeatPict, (FrameFlags & FRAME_KF) != 0, FilePos, FrameSize));
		} else if (mkv_GetTrackInfo(MF, Track)->Type == TT_AUDIO && (IndexMask & (1 << Track))) {
			TrackCompressionContext *TCC = AudioContexts[Track].TCC;
			unsigned int CompressedFrameSize = FrameSize;
			ReadFrame(FilePos, FrameSize, TCC, MC);
			TempPacket.data = MC.Buffer;
			TempPacket.size = (TCC && TCC->CompressionMethod == COMP_PREPEND) ? FrameSize + TCC->CompressedPrivateDataSize : FrameSize;
			TempPacket.flags = FrameFlags & FRAME_KF ? AV_PKT_FLAG_KEY : 0;

			int64_t StartSample = AudioContexts[Track].CurrentSample;
			int64_t SampleCount = IndexAudioPacket(Track, &TempPacket, AudioContexts[Track], *TrackIndices);

			if (SampleCount != 0)
				(*TrackIndices)[Track].push_back(TFrameInfo::AudioFrameInfo(StartTime, StartSample,
					SampleCount, (FrameFlags & FRAME_KF) != 0, FilePos, CompressedFrameSize));
		}
	}

	TrackIndices->Sort();
	return TrackIndices.release();
}

int FFMatroskaIndexer::GetNumberOfTracks() {
	return mkv_GetNumTracks(MF);
}

FFMS_TrackType FFMatroskaIndexer::GetTrackType(int Track) {
	return HaaliTrackTypeToFFTrackType(mkv_GetTrackInfo(MF, Track)->Type);
}

const char *FFMatroskaIndexer::GetTrackCodec(int Track) {
	return Codec[Track] ? Codec[Track]->name : NULL;
}

const char *FFMatroskaIndexer::GetFormatName() {
	return "matroska";
}

FFMS_Sources FFMatroskaIndexer::GetSourceType() {
	return FFMS_SOURCE_MATROSKA;
}


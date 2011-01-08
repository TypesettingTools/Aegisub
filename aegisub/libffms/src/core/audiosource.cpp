//  Copyright (c) 2010 Thomas Goyne <tgoyne@gmail.com>
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

#include "audiosource.h"

#include <algorithm>
#include <cassert>

FFMS_AudioSource::FFMS_AudioSource(const char *SourceFile, FFMS_Index &Index, int Track)
: Delay(0)
, MaxCacheBlocks(50)
, BytesPerSample(0)
, Decoded(0)
, CurrentSample(-1)
, PacketNumber(0)
, CurrentFrame(NULL)
, TrackNumber(Track)
, SeekOffset(0)
, DecodingBuffer(AVCODEC_MAX_AUDIO_FRAME_SIZE * 10)
{
	if (Track < 0 || Track >= static_cast<int>(Index.size()))
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_INVALID_ARGUMENT,
			"Out of bounds track index selected");

	if (Index[Track].TT != FFMS_TYPE_AUDIO)
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_INVALID_ARGUMENT,
			"Not an audio track");

	if (Index[Track].empty())
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_INVALID_ARGUMENT,
			"Audio track contains no audio frames");

	Frames = Index[Track];

	if (!Index.CompareFileSignature(SourceFile))
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_FILE_MISMATCH,
			"The index does not match the source file");
}
void FFMS_AudioSource::Init(FFMS_Index &Index, int DelayMode) {
	// The first packet after a seek is often decoded incorrectly, which
	// makes it impossible to ever correctly seek back to the beginning, so
	// store the first block now

	// In addition, anything with the same PTS as the first packet can't be
	// distinguished from the first packet and so can't be seeked to, so
	// store those as well

	// Some of LAVF's splitters don't like to seek to the beginning of the
	// file (ts and?), so cache a few blocks even if PTSes are unique
	// Packet 7 is the last packet I've had be unseekable to, so cache up to
	// 10 for a bit of an extra buffer
	CacheIterator end = Cache.end();
	while (PacketNumber < Frames.size() &&
		((Frames[0].PTS != ffms_av_nopts_value && Frames[PacketNumber].PTS == Frames[0].PTS) ||
		 Cache.size() < 10)) {

		DecodeNextBlock();
		if (Decoded)
			CacheBlock(end, CurrentSample, Decoded, &DecodingBuffer[0]);
	}
	// Store the iterator to the last element of the cache which is used for
	// correctness rather than speed, so that when looking for one to delete
	// we know how much to skip
	CacheNoDelete = Cache.end();
	--CacheNoDelete;

	// Read properties of the audio which may not be available until the first
	// frame has been decoded
	FillAP(AP, CodecContext, Frames);

	if (AP.SampleRate <= 0 || AP.BitsPerSample <= 0)
		throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_CODEC,
			"Codec returned zero size audio");

	if (DelayMode < FFMS_DELAY_NO_SHIFT)
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_INVALID_ARGUMENT,
			"Bad audio delay compensation mode");

	if (DelayMode == FFMS_DELAY_NO_SHIFT) return;

	if (DelayMode > (signed)Index.size())
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_INVALID_ARGUMENT,
			"Out of bounds track index selected for audio delay compensation");

	if (DelayMode >= 0 && Index[DelayMode].TT != FFMS_TYPE_VIDEO)
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_INVALID_ARGUMENT,
			"Audio delay compensation must be relative to a video track");

	double AdjustRelative = 0;
	if (DelayMode != FFMS_DELAY_TIME_ZERO) {
		if (DelayMode == FFMS_DELAY_FIRST_VIDEO_TRACK) {
			for (size_t i = 0; i < Index.size(); ++i) {
				if (Index[i].TT == FFMS_TYPE_VIDEO) {
					DelayMode = i;
					break;
				}
			}
		}

		if (DelayMode >= 0) {
			const FFMS_Track &VTrack = Index[DelayMode];
			AdjustRelative = VTrack[0].PTS * VTrack.TB.Num / (double)VTrack.TB.Den / 1000.;
		}
	}

	Delay = static_cast<int64_t>((AdjustRelative - Frames[0].PTS) * AP.SampleRate + .5);
	AP.NumSamples -= Delay;
}

void FFMS_AudioSource::CacheBlock(CacheIterator &pos, int64_t Start, size_t Samples, uint8_t *SrcData) {
	Cache.insert(pos, AudioBlock(Start, Samples, SrcData, Samples * BytesPerSample));

	if (Cache.size() >= MaxCacheBlocks) {
		// Kill the oldest one
		CacheIterator min = CacheNoDelete;
		// Never drop the first one as the first packet decoded after a seek
		// is often decoded incorrectly and we can't seek to before the first one
		++min;
		for (CacheIterator it = min; it != Cache.end(); ++it)
			if (it->Age < min->Age) min = it;
		if (min == pos) ++pos;
		Cache.erase(min);
	}
}

void FFMS_AudioSource::DecodeNextBlock() {
	if (BytesPerSample == 0) BytesPerSample = (av_get_bits_per_sample_fmt(CodecContext->sample_fmt) * CodecContext->channels) / 8;

	CurrentFrame = &Frames[PacketNumber];

	AVPacket Packet;
	if (!ReadPacket(&Packet))
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_UNKNOWN, "ReadPacket unexpectedly failed to read a packet");

	// ReadPacket may have changed the packet number
	CurrentFrame = &Frames[PacketNumber];
	CurrentSample = CurrentFrame->SampleStart;
	++PacketNumber;

	uint8_t *Buf = &DecodingBuffer[0];
	uint8_t *Data = Packet.data;
	while (Packet.size > 0) {
		int TempOutputBufSize = AVCODEC_MAX_AUDIO_FRAME_SIZE * 10 - (Buf - &DecodingBuffer[0]);
		int Ret = avcodec_decode_audio3(CodecContext, (int16_t *)Buf, &TempOutputBufSize, &Packet);

		// Should only ever happen if the user chose to ignore decoding errors
		// during indexing, so continue to just ignore decoding errors
		if (Ret < 0) break;

		if (Ret > 0) {
			Packet.size -= Ret;
			Packet.data += Ret;
			Buf += TempOutputBufSize;
		}
	}
	Packet.data = Data;
	FreePacket(&Packet);

	Decoded = (Buf - &DecodingBuffer[0]) / BytesPerSample;
	if (Decoded == 0) {
		// zero sample packets aren't included in the index so we didn't
		// actually move to the next packet
		--PacketNumber;
	}
}

static bool SampleStartComp(const TFrameInfo &a, const TFrameInfo &b) {
	return a.SampleStart < b.SampleStart;
}

void FFMS_AudioSource::GetAudio(void *Buf, int64_t Start, int64_t Count) {
	if (Start < 0 || Start + Count > AP.NumSamples || Count < 0)
		throw FFMS_Exception(FFMS_ERROR_DECODING, FFMS_ERROR_INVALID_ARGUMENT,
			"Out of bounds audio samples requested");

	uint8_t *Dst = static_cast<uint8_t*>(Buf);

	// Apply audio delay (if any) and fill any samples before the start time with zero
	Start -= Delay;
	if (Start < 0) {
		size_t Bytes = static_cast<size_t>(BytesPerSample * FFMIN(-Start, Count));
		memset(Dst, 0, Bytes);

		Count += Start;
		// Entire request was before the start of the audio
		if (Count <= 0) return;

		Start = 0;
		Dst += Bytes;
	}

	CacheIterator it = Cache.begin();

	while (Count > 0) {
		// Find first useful cache block
		while (it != Cache.end() && it->Start + it->Samples <= Start) ++it;

		// Cache has the next block we want
		if (it != Cache.end() && it->Start <= Start) {
			int64_t SrcOffset = FFMAX(0, Start - it->Start);
			int64_t DstOffset = FFMAX(0, it->Start - Start);
			int64_t CopySamples = FFMIN(it->Samples - SrcOffset, Count - DstOffset);
			size_t Bytes = static_cast<size_t>(CopySamples * BytesPerSample);

			memcpy(Dst + DstOffset * BytesPerSample, &it->Data[SrcOffset * BytesPerSample], Bytes);
			Start += CopySamples;
			Count -= CopySamples;
			Dst += Bytes;
			++it;
		}
		// Decode another block
		else {
			if (Start < CurrentSample && SeekOffset == -1)
				throw FFMS_Exception(FFMS_ERROR_SEEKING, FFMS_ERROR_CODEC, "Audio stream is not seekable");

			if (SeekOffset >= 0 && (Start < CurrentSample || Start > CurrentSample + Decoded * 5)) {
				TFrameInfo f;
				f.SampleStart = Start;
				int NewPacketNumber = std::distance(Frames.begin(), std::lower_bound(Frames.begin(), Frames.end(), f, SampleStartComp));
				NewPacketNumber = FFMAX(0, NewPacketNumber - SeekOffset - 15);
				while (NewPacketNumber > 0 && !Frames[NewPacketNumber].KeyFrame) --NewPacketNumber;

				// Only seek forward if it'll actually result in moving forward
				if (Start < CurrentSample || NewPacketNumber > PacketNumber) {
					PacketNumber = NewPacketNumber;
					Decoded = 0;
					CurrentSample = -1;
					avcodec_flush_buffers(CodecContext);
					Seek();
				}
			}

			// Decode everything between the last keyframe and the block we want
			while (CurrentSample + Decoded <= Start) DecodeNextBlock();
			if (CurrentSample > Start)
				throw FFMS_Exception(FFMS_ERROR_SEEKING, FFMS_ERROR_CODEC, "Seeking is severely broken");

			CacheBlock(it, CurrentSample, Decoded, &DecodingBuffer[0]);

			size_t FirstSample = static_cast<size_t>(Start - CurrentSample);
			size_t Samples = static_cast<size_t>(Decoded - FirstSample);
			size_t Bytes = FFMIN(Samples, static_cast<size_t>(Count)) * BytesPerSample;

			memcpy(Dst, &DecodingBuffer[FirstSample * BytesPerSample], Bytes);

			Start += Samples;
			Count -= Samples;
			Dst += Bytes;
		}
	}
}

size_t GetSeekablePacketNumber(FFMS_Track const& Frames, size_t PacketNumber) {
	// Packets don't always have unique PTSes, so we may not be able to
	// uniquely identify the packet we want. This function attempts to find
	// a PTS we can seek to which will let us figure out which packet we're
	// on before we get to the packet we actually wanted

	// MatroskaAudioSource doesn't need this, as it seeks by byte offset
	// rather than PTS. LAVF theoretically can seek by byte offset, but we
	// don't use it as not all demuxers support it and it's broken in some of
	// those that claim to support it

	// However much we might wish to, we can't seek to before packet zero
	if (PacketNumber == 0) return PacketNumber;

	// Desired packet's PTS is unique, so don't do anything
	if (Frames[PacketNumber].PTS != Frames[PacketNumber - 1].PTS &&
		(PacketNumber + 1 == Frames.size() || Frames[PacketNumber].PTS != Frames[PacketNumber + 1].PTS))
		return PacketNumber;

	// When decoding, we only reliably know what packet we're at when the
	// newly parsed packet has a different PTS from the previous one. As such,
	// we walk backwards until we hit a different PTS and then seek to there,
	// so that we can then decode until we hit the PTS group we actually wanted
	// (and thereby know that we're at the first packet in the group rather
	// than whatever the splitter happened to choose)

	// This doesn't work if our desired packet has the same PTS as the first
	// packet, but this scenario should never come up anyway; we permanently
	// cache the decoded results from those packets, so there's no need to ever
	// seek to them
	int64_t PTS = Frames[PacketNumber].PTS;
	while (PacketNumber > 0 && PTS == Frames[PacketNumber].PTS)
		--PacketNumber;
	return PacketNumber;
}

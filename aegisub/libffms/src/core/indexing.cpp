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
#include <iostream>
#include <fstream>
#include <algorithm>

extern "C" {
#include <libavutil/sha1.h>
}

#define INDEXID 0x53920873

extern bool HasHaaliMPEG;
extern bool HasHaaliOGG;

struct IndexHeader {
	uint32_t Id;
	uint32_t Version;
	uint32_t Tracks;
	uint32_t Decoder;
	uint32_t LAVUVersion;
	uint32_t LAVFVersion;
	uint32_t LAVCVersion;
	uint32_t LSWSVersion;
	uint32_t LPPVersion;
	int64_t FileSize;
	uint8_t FileSignature[20];
};

struct TrackHeader {
		uint32_t TT;
		uint32_t Frames;
		int64_t Num;
		int64_t Den;
};

SharedVideoContext::SharedVideoContext(bool FreeCodecContext) {
	CodecContext = NULL;
	Parser = NULL;
	CS = NULL;
	this->FreeCodecContext = FreeCodecContext;
}

SharedVideoContext::~SharedVideoContext() {
	if (CodecContext) {
		avcodec_close(CodecContext);
		if (FreeCodecContext)
			av_freep(&CodecContext);
	}

	if (Parser)
		av_parser_close(Parser);

	if (CS)
		cs_Destroy(CS);
}

SharedAudioContext::SharedAudioContext(bool FreeCodecContext) {
	W64Writer = NULL;
	CodecContext = NULL;
	CurrentSample = 0;
	CS = NULL;
	this->FreeCodecContext = FreeCodecContext;
}

SharedAudioContext::~SharedAudioContext() {
	delete W64Writer;
	if (CodecContext) {
		avcodec_close(CodecContext);
		if (FreeCodecContext)
			av_freep(&CodecContext);
	}

	if (CS)
		cs_Destroy(CS);
}

TFrameInfo::TFrameInfo() {
}

TFrameInfo::TFrameInfo(int64_t PTS, int64_t SampleStart, unsigned int SampleCount, int RepeatPict, bool KeyFrame, int64_t FilePos, unsigned int FrameSize) {
	this->PTS = PTS;
	this->RepeatPict = RepeatPict;
	this->KeyFrame = KeyFrame;
	this->SampleStart = SampleStart;
	this->SampleCount = SampleCount;
	this->FilePos = FilePos;
	this->FrameSize = FrameSize;
	this->OriginalPos = 0;
}

TFrameInfo TFrameInfo::VideoFrameInfo(int64_t PTS, int RepeatPict, bool KeyFrame, int64_t FilePos, unsigned int FrameSize) {
	return TFrameInfo(PTS, 0, 0, RepeatPict, KeyFrame, FilePos, FrameSize);
}

TFrameInfo TFrameInfo::AudioFrameInfo(int64_t PTS, int64_t SampleStart, unsigned int SampleCount, bool KeyFrame, int64_t FilePos, unsigned int FrameSize) {
	return TFrameInfo(PTS, SampleStart, SampleCount, 0, KeyFrame, FilePos, FrameSize);
}

void FFMS_Track::WriteTimecodes(const char *TimecodeFile) {
	ffms_fstream Timecodes(TimecodeFile, std::ios::out | std::ios::trunc);

	if (!Timecodes.is_open()) {
		std::ostringstream buf;
		buf << "Failed to open '" << TimecodeFile << "' for writing";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}

	Timecodes << "# timecode format v2\n";

	for (iterator Cur=begin(); Cur!=end(); Cur++)
		Timecodes << std::fixed << ((Cur->PTS * TB.Num) / (double)TB.Den) << "\n";
}

int FFMS_Track::FrameFromPTS(int64_t PTS) {
	for (int i = 0; i < static_cast<int>(size()); i++)
		if (at(i).PTS == PTS)
			return i;
	return -1;
}

int FFMS_Track::ClosestFrameFromPTS(int64_t PTS) {
	int Frame = 0; 
	int64_t BestDiff = 0xFFFFFFFFFFFFFFLL; // big number
	for (int i = 0; i < static_cast<int>(size()); i++) {
		int64_t CurrentDiff = FFABS(at(i).PTS - PTS);
		if (CurrentDiff < BestDiff) {
			BestDiff = CurrentDiff;
			Frame = i;
		}
	}

	return Frame;
}

int FFMS_Track::FindClosestVideoKeyFrame(int Frame) {
	Frame = FFMIN(FFMAX(Frame, 0), static_cast<int>(size()) - 1);
	for (int i = Frame; i > 0; i--)
		if (at(i).KeyFrame)
			return i;
	return 0;
}

int FFMS_Track::FindClosestAudioKeyFrame(int64_t Sample) {
	for (size_t i = 0; i < size(); i++) {
		if (at(i).SampleStart == Sample && at(i).KeyFrame)
			return i;
		else if (at(i).SampleStart > Sample && at(i).KeyFrame)
			return i  - 1;
	}
	return size() - 1;
}

FFMS_Track::FFMS_Track() {
	this->TT = FFMS_TYPE_UNKNOWN;
	this->TB.Num = 0; 
	this->TB.Den = 0;
}

FFMS_Track::FFMS_Track(int64_t Num, int64_t Den, FFMS_TrackType TT) {
	this->TT = TT;
	this->TB.Num = Num; 
	this->TB.Den = Den;
}

void FFMS_Index::CalculateFileSignature(const char *Filename, int64_t *Filesize, uint8_t Digest[20]) {
	FILE *SFile = ffms_fopen(Filename,"rb");

	if (SFile == NULL) {
		std::ostringstream buf;
		buf << "Failed to open '" << Filename << "' for hashing";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}

	const int BlockSize = 1024*1024;
	std::vector<uint8_t> FileBuffer(BlockSize);
	std::vector<uint8_t> ctxmem(av_sha1_size);
	AVSHA1 *ctx = (AVSHA1 *)&ctxmem[0];
	av_sha1_init(ctx);
	
	memset(&FileBuffer[0], 0, BlockSize);
	fread(&FileBuffer[0], 1, BlockSize, SFile);
	if (ferror(SFile) && !feof(SFile)) {
		av_sha1_final(ctx, Digest);
		fclose(SFile);
		std::ostringstream buf;
		buf << "Failed to read '" << Filename << "' for hashing";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}
	av_sha1_update(ctx, &FileBuffer[0], BlockSize);

	fseeko(SFile, -BlockSize, SEEK_END);
	memset(&FileBuffer[0], 0, BlockSize);
	fread(&FileBuffer[0], 1, BlockSize, SFile);
	if (ferror(SFile) && !feof(SFile)) {
		av_sha1_final(ctx, Digest);
		fclose(SFile);
		std::ostringstream buf;
		buf << "Failed to seek with offset " << BlockSize << " from file end in '" << Filename << "' for hashing";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}
	av_sha1_update(ctx, &FileBuffer[0], BlockSize);

	fseeko(SFile, 0, SEEK_END);
	if (ferror(SFile)) {
		av_sha1_final(ctx, Digest);
		fclose(SFile);
		std::ostringstream buf;
		buf << "Failed to seek to end of '" << Filename << "' for hashing";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}
	*Filesize = ftello(SFile);
	fclose(SFile);

	av_sha1_final(ctx, Digest);
}

static bool PTSComparison(TFrameInfo FI1, TFrameInfo FI2) {
	return FI1.PTS < FI2.PTS;
}

void FFMS_Index::Sort() {
	for (FFMS_Index::iterator Cur=begin(); Cur!=end(); Cur++) {

		for (size_t i = 0; i < Cur->size(); i++)
			Cur->at(i).OriginalPos = i;

		std::sort(Cur->begin(), Cur->end(), PTSComparison);

		std::vector<size_t> ReorderTemp;
		ReorderTemp.resize(Cur->size());

		for (size_t i = 0; i < Cur->size(); i++)
			ReorderTemp[i] = Cur->at(i).OriginalPos;

		for (size_t i = 0; i < Cur->size(); i++)
			Cur->at(ReorderTemp[i]).OriginalPos = i;
	}
}

bool FFMS_Index::CompareFileSignature(const char *Filename) {
	int64_t CFilesize;
	uint8_t CDigest[20];
	CalculateFileSignature(Filename, &CFilesize, CDigest);
	return (CFilesize == Filesize && !memcmp(CDigest, Digest, sizeof(Digest)));
}

void FFMS_Index::WriteIndex(const char *IndexFile) {
	ffms_fstream IndexStream(IndexFile, std::ios::out | std::ios::binary | std::ios::trunc);

	if (!IndexStream.is_open()) {
		std::ostringstream buf;
		buf << "Failed to open '" << IndexFile << "' for writing";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}

	// Write the index file header
	IndexHeader IH;
	IH.Id = INDEXID;
	IH.Version = FFMS_VERSION;
	IH.Tracks = size();
	IH.Decoder = Decoder;
	IH.LAVUVersion = avutil_version();
	IH.LAVFVersion = avformat_version();
	IH.LAVCVersion = avcodec_version();
	IH.LSWSVersion = swscale_version();
	IH.LPPVersion = postproc_version();
	IH.FileSize = Filesize;
	memcpy(IH.FileSignature, Digest, sizeof(Digest));

	IndexStream.write(reinterpret_cast<char *>(&IH), sizeof(IH));
	
	for (unsigned int i = 0; i < IH.Tracks; i++) {
		TrackHeader TH;
		TH.TT = at(i).TT;
		TH.Frames = at(i).size();
		TH.Num = at(i).TB.Num;;
		TH.Den = at(i).TB.Den;

		IndexStream.write(reinterpret_cast<char *>(&TH), sizeof(TH));
		IndexStream.write(reinterpret_cast<char *>(FFMS_GET_VECTOR_PTR(at(i))), TH.Frames * sizeof(TFrameInfo));
	}
}

void FFMS_Index::ReadIndex(const char *IndexFile) {
	ffms_fstream Index(IndexFile, std::ios::in | std::ios::binary);

	if (!Index.is_open()) {
		std::ostringstream buf;
		buf << "Failed to open '" << IndexFile << "' for reading";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}
	
	// Read the index file header
	IndexHeader IH;
	Index.read(reinterpret_cast<char *>(&IH), sizeof(IH));
	if (IH.Id != INDEXID) {
		std::ostringstream buf;
		buf << "'" << IndexFile << "' is not a valid index file";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}

	if (IH.Version != FFMS_VERSION) {
		std::ostringstream buf;
		buf << "'" << IndexFile << "' is not the expected index version";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}

	if (IH.LAVUVersion != avutil_version() || IH.LAVFVersion != avformat_version() ||
		IH.LAVCVersion != avcodec_version() || IH.LSWSVersion != swscale_version() ||
		IH.LPPVersion != postproc_version()) {
		std::ostringstream buf;
		buf << "A different FFmpeg build was used to create '" << IndexFile << "'";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}

	if (!(IH.Decoder & FFMS_GetEnabledSources()))
		throw FFMS_Exception(FFMS_ERROR_INDEX, FFMS_ERROR_NOT_AVAILABLE,
			"The source which this index was created with is not available");

	Decoder = IH.Decoder;
	Filesize = IH.FileSize;
	memcpy(Digest, IH.FileSignature, sizeof(Digest));

	try {
		for (unsigned int i = 0; i < IH.Tracks; i++) {
			TrackHeader TH;
			Index.read(reinterpret_cast<char *>(&TH), sizeof(TH));
			push_back(FFMS_Track(TH.Num, TH.Den, static_cast<FFMS_TrackType>(TH.TT)));

			if (TH.Frames) {
				at(i).resize(TH.Frames);
				Index.read(reinterpret_cast<char *>(FFMS_GET_VECTOR_PTR(at(i))), TH.Frames * sizeof(TFrameInfo));
			}
		}

	} catch (...) {
		std::ostringstream buf;
		buf << "Unknown error while reading index information in '" << IndexFile << "'";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}
}

FFMS_Index::FFMS_Index() {
	// this comment documents nothing
}

FFMS_Index::FFMS_Index(int64_t Filesize, uint8_t Digest[20]) {
	this->Filesize = Filesize;
	memcpy(this->Digest, Digest, sizeof(this->Digest));
}

void FFMS_Indexer::SetIndexMask(int IndexMask) {
	this->IndexMask = IndexMask;
}

void FFMS_Indexer::SetDumpMask(int DumpMask) {
	this->DumpMask = DumpMask;
}

void FFMS_Indexer::SetErrorHandling(int ErrorHandling) {
	if (ErrorHandling != FFMS_IEH_ABORT && ErrorHandling != FFMS_IEH_CLEAR_TRACK &&
		ErrorHandling != FFMS_IEH_STOP_TRACK && ErrorHandling != FFMS_IEH_IGNORE)
		throw FFMS_Exception(FFMS_ERROR_INDEXING, FFMS_ERROR_INVALID_ARGUMENT,
			"Invalid error handling mode specified");
	this->ErrorHandling = ErrorHandling;
}

void FFMS_Indexer::SetProgressCallback(TIndexCallback IC, void *ICPrivate) {
	this->IC = IC; 
	this->ICPrivate = ICPrivate;
}

void FFMS_Indexer::SetAudioNameCallback(TAudioNameCallback ANC, void *ANCPrivate) {
	this->ANC = ANC;
	this->ANCPrivate = ANCPrivate;
}

FFMS_Indexer *FFMS_Indexer::CreateIndexer(const char *Filename) {
	AVFormatContext *FormatContext = NULL;

	if (av_open_input_file(&FormatContext, Filename, NULL, 0, NULL) != 0) {
		std::ostringstream buf;
		buf << "Can't open '" << Filename << "'";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}

	// Do matroska indexing instead?
	if (!strcmp(FormatContext->iformat->name, "matroska")) {
		av_close_input_file(FormatContext);
		return new FFMatroskaIndexer(Filename);
	}

#ifdef HAALISOURCE
	// Do haali ts indexing instead?
	if (HasHaaliMPEG && (!strcmp(FormatContext->iformat->name, "mpeg") || !strcmp(FormatContext->iformat->name, "mpegts"))) {
		av_close_input_file(FormatContext);
		return new FFHaaliIndexer(Filename, FFMS_SOURCE_HAALIMPEG);
	}

	if (HasHaaliOGG && !strcmp(FormatContext->iformat->name, "ogg")) {
		av_close_input_file(FormatContext);
		return new FFHaaliIndexer(Filename, FFMS_SOURCE_HAALIOGG);
	}
#endif

	return new FFLAVFIndexer(Filename, FormatContext);
}

FFMS_Indexer::FFMS_Indexer(const char *Filename) : DecodingBuffer(AVCODEC_MAX_AUDIO_FRAME_SIZE * 5) {
	IndexMask = 0;
	DumpMask = 0;
	ErrorHandling = FFMS_IEH_CLEAR_TRACK;
	IC = NULL;
	ICPrivate = NULL;
	ANC = NULL;
	ANCPrivate = NULL;

	FFMS_Index::CalculateFileSignature(Filename, &Filesize, Digest);
}

FFMS_Indexer::~FFMS_Indexer() {

}

void FFMS_Indexer::WriteAudio(SharedAudioContext &AudioContext, FFMS_Index *Index, int Track, int DBSize) {
	// Delay writer creation until after an audio frame has been decoded. This ensures that all parameters are known when writing the headers.
	if (DBSize > 0) {
		if (!AudioContext.W64Writer) {
			FFMS_AudioProperties AP;
			FillAP(AP, AudioContext.CodecContext, (*Index)[Track]);
			int FNSize = (*ANC)(SourceFile, Track, &AP, NULL, 0, ANCPrivate);
			std::vector<char> WName(FNSize);
			(*ANC)(SourceFile, Track, &AP, &WName[0], FNSize, ANCPrivate);
			std::string WN(&WName[0]);
			try {
				AudioContext.W64Writer = new Wave64Writer(WN.c_str(), av_get_bits_per_sample_format(AudioContext.CodecContext->sample_fmt),
					AudioContext.CodecContext->channels, AudioContext.CodecContext->sample_rate, (AudioContext.CodecContext->sample_fmt == SAMPLE_FMT_FLT) || (AudioContext.CodecContext->sample_fmt == SAMPLE_FMT_DBL));
			} catch (...) {
				throw FFMS_Exception(FFMS_ERROR_WAVE_WRITER, FFMS_ERROR_FILE_WRITE,
					"Failed to write wave data");
			}
		}

		AudioContext.W64Writer->WriteData(&DecodingBuffer[0], DBSize);
	}
}

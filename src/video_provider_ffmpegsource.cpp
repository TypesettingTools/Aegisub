// Copyright (c) 2008-2009, Karl Blomster
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
// Aegisub Project http://www.aegisub.org/

/// @file video_provider_ffmpegsource.cpp
/// @brief FFmpegSource2-based video provider
/// @ingroup video_input ffms
///

#ifdef WITH_FFMS2
#include "ffmpegsource_common.h"
#include "include/aegisub/video_provider.h"

#include "options.h"
#include "utils.h"
#include "video_frame.h"

#include <libaegisub/fs.h>
#include <libaegisub/make_unique.h>

namespace {
typedef enum AGI_ColorSpaces {
	AGI_CS_RGB = 0,
	AGI_CS_BT709 = 1,
	AGI_CS_UNSPECIFIED = 2,
	AGI_CS_FCC = 4,
	AGI_CS_BT470BG = 5,
	AGI_CS_SMPTE170M = 6,
	AGI_CS_SMPTE240M = 7,
	AGI_CS_YCOCG = 8,
	AGI_CS_BT2020_NCL = 9,
	AGI_CS_BT2020_CL = 10,
	AGI_CS_SMPTE2085 = 11,
	AGI_CS_CHROMATICITY_DERIVED_NCL = 12,
	AGI_CS_CHROMATICITY_DERIVED_CL = 13,
	AGI_CS_ICTCP = 14
} AGI_ColorSpaces;

/// @class FFmpegSourceVideoProvider
/// @brief Implements video loading through the FFMS library.
class FFmpegSourceVideoProvider final : public VideoProvider, FFmpegSourceProvider {
	/// video source object
	agi::scoped_holder<FFMS_VideoSource*, void (FFMS_CC*)(FFMS_VideoSource*)> VideoSource;
	const FFMS_VideoProperties *VideoInfo = nullptr; ///< video properties

	int Width = -1;                 ///< width in pixels
	int Height = -1;                ///< height in pixels
	int CS = -1;                    ///< Reported colorspace of first frame
	int CR = -1;                    ///< Reported colorrange of first frame
	double DAR;                     ///< display aspect ratio
	std::vector<int> KeyFramesList; ///< list of keyframes
	agi::vfr::Framerate Timecodes;  ///< vfr object
	std::string ColorSpace;         ///< Colorspace name
	std::string RealColorSpace;     ///< Colorspace name

	char FFMSErrMsg[1024];          ///< FFMS error message
	FFMS_ErrorInfo ErrInfo;         ///< FFMS error codes/messages
	bool has_audio = false;

	void LoadVideo(agi::fs::path const& filename, std::string const& colormatrix);

public:
	FFmpegSourceVideoProvider(agi::fs::path const& filename, std::string const& colormatrix, agi::BackgroundRunner *br);

	void GetFrame(int n, VideoFrame &out) override;

	void SetColorSpace(std::string const& matrix) override {
		if (matrix == ColorSpace) return;
		if (matrix == RealColorSpace)
			FFMS_SetInputFormatV(VideoSource, CS, CR, FFMS_GetPixFmt(""), nullptr);
		else if (matrix == "TV.601")
			FFMS_SetInputFormatV(VideoSource, AGI_CS_BT470BG, CR, FFMS_GetPixFmt(""), nullptr);
		else
			return;
		ColorSpace = matrix;
	}

	int GetFrameCount() const override             { return VideoInfo->NumFrames; }

#if FFMS_VERSION >= ((2 << 24) | (24 << 16) | (0 << 8) | 0)
	int GetWidth() const override  { return (VideoInfo->Rotation % 180 == 90 || VideoInfo->Rotation % 180 == -90) ? Height : Width; }
	int GetHeight() const override { return (VideoInfo->Rotation % 180 == 90 || VideoInfo->Rotation % 180 == -90) ? Width : Height; }
	double GetDAR() const override { return (VideoInfo->Rotation % 180 == 90 || VideoInfo->Rotation % 180 == -90) ? 1 / DAR : DAR; }
#else
	int GetWidth() const override                  { return Width; }
	int GetHeight() const override                 { return Height; }
	double GetDAR() const override                 { return DAR; }
#endif

	agi::vfr::Framerate GetFPS() const override    { return Timecodes; }
	std::string GetColorSpace() const override     { return ColorSpace; }
	std::string GetRealColorSpace() const override { return RealColorSpace; }
	std::vector<int> GetKeyFrames() const override { return KeyFramesList; };
	std::string GetDecoderName() const override    { return "FFmpegSource"; }
	bool WantsCaching() const override             { return true; }
	bool HasAudio() const override                 { return has_audio; }
};

std::string colormatrix_description(int cs, int cr) {
	// Assuming TV for unspecified
	std::string str = cr == FFMS_CR_JPEG ? "PC" : "TV";

	switch (cs) {
		case AGI_CS_RGB:
			return "None";
		case AGI_CS_BT709:
			return str + ".709";
		case AGI_CS_FCC:
			return str + ".FCC";
		case AGI_CS_BT470BG:
		case AGI_CS_SMPTE170M:
			return str + ".601";
		case AGI_CS_SMPTE240M:
			return str + ".240M";
		default:
			throw VideoOpenError("Unknown video color space");
	}
}

FFmpegSourceVideoProvider::FFmpegSourceVideoProvider(agi::fs::path const& filename, std::string const& colormatrix, agi::BackgroundRunner *br) try
: FFmpegSourceProvider(br)
, VideoSource(nullptr, FFMS_DestroyVideoSource)
{
	ErrInfo.Buffer		= FFMSErrMsg;
	ErrInfo.BufferSize	= sizeof(FFMSErrMsg);
	ErrInfo.ErrorType	= FFMS_ERROR_SUCCESS;
	ErrInfo.SubType		= FFMS_ERROR_SUCCESS;

	SetLogLevel();

	LoadVideo(filename, colormatrix);
}
catch (agi::EnvironmentError const& err) {
	throw VideoOpenError(err.GetMessage());
}

void FFmpegSourceVideoProvider::LoadVideo(agi::fs::path const& filename, std::string const& colormatrix) {
	FFMS_Indexer *Indexer = FFMS_CreateIndexer(filename.string().c_str(), &ErrInfo);
	if (!Indexer) {
		if (ErrInfo.SubType == FFMS_ERROR_FILE_READ)
			throw agi::fs::FileNotFound(std::string(ErrInfo.Buffer));
		else
			throw VideoNotSupported(ErrInfo.Buffer);
	}

	std::map<int, std::string> TrackList = GetTracksOfType(Indexer, FFMS_TYPE_VIDEO);
	if (TrackList.size() <= 0)
		throw VideoNotSupported("no video tracks found");

	int TrackNumber = -1;
	if (TrackList.size() > 1) {
		auto Selection = AskForTrackSelection(TrackList, FFMS_TYPE_VIDEO);
		if (Selection == TrackSelection::None)
			throw agi::UserCancelException("video loading cancelled by user");
		TrackNumber = static_cast<int>(Selection);
	}

	// generate a name for the cache file
	auto CacheName = GetCacheFilename(filename);

	// try to read index
	agi::scoped_holder<FFMS_Index*, void (FFMS_CC*)(FFMS_Index*)>
		Index(FFMS_ReadIndex(CacheName.string().c_str(), &ErrInfo), FFMS_DestroyIndex);

	if (Index && FFMS_IndexBelongsToFile(Index, filename.string().c_str(), &ErrInfo))
		Index = nullptr;

	// time to examine the index and check if the track we want is indexed
	// technically this isn't really needed since all video tracks should always be indexed,
	// but a bit of sanity checking never hurt anyone
	if (Index && TrackNumber >= 0) {
		FFMS_Track *TempTrackData = FFMS_GetTrackFromIndex(Index, TrackNumber);
		if (FFMS_GetNumFrames(TempTrackData) <= 0)
			Index = nullptr;
	}

	// moment of truth
	if (!Index) {
		auto TrackMask = TrackSelection::None;
		if (OPT_GET("Provider/FFmpegSource/Index All Tracks")->GetBool() || OPT_GET("Video/Open Audio")->GetBool())
			TrackMask = TrackSelection::All;
		Index = DoIndexing(Indexer, CacheName, TrackMask, GetErrorHandlingMode());
	}
	else {
		FFMS_CancelIndexing(Indexer);
	}

	// update access time of index file so it won't get cleaned away
	agi::fs::Touch(CacheName);

	// we have now read the index and may proceed with cleaning the index cache
	CleanCache();

	// track number still not set?
	if (TrackNumber < 0) {
		// just grab the first track
		TrackNumber = FFMS_GetFirstIndexedTrackOfType(Index, FFMS_TYPE_VIDEO, &ErrInfo);
		if (TrackNumber < 0)
			throw VideoNotSupported(std::string("Couldn't find any video tracks: ") + ErrInfo.Buffer);
	}

	// Check if there's an audio track
	has_audio = FFMS_GetFirstTrackOfType(Index, FFMS_TYPE_AUDIO, nullptr) != -1;

	// set thread count
	int Threads = OPT_GET("Provider/Video/FFmpegSource/Decoding Threads")->GetInt();
#if FFMS_VERSION < ((2 << 24) | (30 << 16) | (0 << 8) | 0)
	if (FFMS_GetVersion() < ((2 << 24) | (17 << 16) | (2 << 8) | 1) && FFMS_GetSourceType(Index) == FFMS_SOURCE_LAVF)
		Threads = 1;
#endif

	// set seekmode
	// TODO: give this its own option?
	int SeekMode;
	if (OPT_GET("Provider/Video/FFmpegSource/Unsafe Seeking")->GetBool())
		SeekMode = FFMS_SEEK_UNSAFE;
	else
		SeekMode = FFMS_SEEK_NORMAL;

	VideoSource = FFMS_CreateVideoSource(filename.string().c_str(), TrackNumber, Index, Threads, SeekMode, &ErrInfo);
	if (!VideoSource)
		throw VideoOpenError(std::string("Failed to open video track: ") + ErrInfo.Buffer);

	// load video properties
	VideoInfo = FFMS_GetVideoProperties(VideoSource);

	const FFMS_Frame *TempFrame = FFMS_GetFrame(VideoSource, 0, &ErrInfo);
	if (!TempFrame)
		throw VideoOpenError(std::string("Failed to decode first frame: ") + ErrInfo.Buffer);

	Width  = TempFrame->EncodedWidth;
	Height = TempFrame->EncodedHeight;
	if (VideoInfo->SARDen > 0 && VideoInfo->SARNum > 0)
		DAR = double(Width) * VideoInfo->SARNum / ((double)Height * VideoInfo->SARDen);
	else
		DAR = double(Width) / Height;

	int VideoCS = CS = TempFrame->ColorSpace;
	CR = TempFrame->ColorRange;

	if (CS == AGI_CS_UNSPECIFIED)
		CS = Width > 1024 || Height >= 600 ? AGI_CS_BT709 : AGI_CS_BT470BG;
	RealColorSpace = ColorSpace = colormatrix_description(CS, CR);

#if FFMS_VERSION >= ((2 << 24) | (17 << 16) | (1 << 8) | 0)
	if (CS != AGI_CS_RGB && CS != AGI_CS_BT470BG && ColorSpace != colormatrix && colormatrix == "TV.601") {
		CS = AGI_CS_BT470BG;
		ColorSpace = colormatrix_description(AGI_CS_BT470BG, CR);
	}

	if (CS != VideoCS) {
		if (FFMS_SetInputFormatV(VideoSource, CS, CR, FFMS_GetPixFmt(""), &ErrInfo))
			throw VideoOpenError(std::string("Failed to set input format: ") + ErrInfo.Buffer);
	}

	const int TargetFormat[] = { FFMS_GetPixFmt("bgra"), -1 };
	if (FFMS_SetOutputFormatV2(VideoSource, TargetFormat, Width, Height, FFMS_RESIZER_BICUBIC, &ErrInfo))
		throw VideoOpenError(std::string("Failed to set output format: ") + ErrInfo.Buffer);

	// get frame info data
	FFMS_Track *FrameData = FFMS_GetTrackFromVideo(VideoSource);
	if (FrameData == nullptr)
		throw VideoOpenError("failed to get frame data");
	const FFMS_TrackTimeBase *TimeBase = FFMS_GetTimeBase(FrameData);
	if (TimeBase == nullptr)
		throw VideoOpenError("failed to get track time base");

	// build list of keyframes and timecodes
	std::vector<int> TimecodesVector;
	for (int CurFrameNum = 0; CurFrameNum < VideoInfo->NumFrames; CurFrameNum++) {
		const FFMS_FrameInfo *CurFrameData = FFMS_GetFrameInfo(FrameData, CurFrameNum);
		if (!CurFrameData)
			throw VideoOpenError("Couldn't get info about frame " + std::to_string(CurFrameNum));

		// keyframe?
		if (CurFrameData->KeyFrame)
			KeyFramesList.push_back(CurFrameNum);

		// calculate timestamp and add to timecodes vector
		int Timestamp = (int)((CurFrameData->PTS * TimeBase->Num) / TimeBase->Den);
		TimecodesVector.push_back(Timestamp);
	}
	if (TimecodesVector.size() < 2)
		Timecodes = 25.0;
	else
		Timecodes = agi::vfr::Framerate(TimecodesVector);
}

void FFmpegSourceVideoProvider::GetFrame(int n, VideoFrame &out) {
	n = mid(0, n, GetFrameCount() - 1);

	auto frame = FFMS_GetFrame(VideoSource, n, &ErrInfo);
	if (!frame)
		throw VideoDecodeError(std::string("Failed to retrieve frame: ") +  ErrInfo.Buffer);

	out.data.assign(frame->Data[0], frame->Data[0] + frame->Linesize[0] * Height);
	out.flipped = false;
	out.width = Width;
	out.height = Height;
	out.pitch = frame->Linesize[0];
#if FFMS_VERSION >= ((2 << 24) | (31 << 16) | (0 << 8) | 0)
	// Handle flip
	if (VideoInfo->Flip > 0)
		for (int x = 0; x < Height; ++x)
			for (int y = 0; y < Width / 2; ++y)
				for (int ch = 0; ch < 4; ++ch)
					std::swap(out.data[frame->Linesize[0] * x + 4 * y + ch], out.data[frame->Linesize[0] * x + 4 * (Width - 1 - y) + ch]);

	else if (VideoInfo->Flip < 0)
		for (int x = 0; x < Height / 2; ++x)
			for (int y = 0; y < Width; ++y)
				for (int ch = 0; ch < 4; ++ch)
					std::swap(out.data[frame->Linesize[0] * x + 4 * y + ch], out.data[frame->Linesize[0] * (Height - 1 - x) + 4 * y + ch]);
#endif
#if FFMS_VERSION >= ((2 << 24) | (24 << 16) | (0 << 8) | 0)
	// Handle rotation
	if (VideoInfo->Rotation % 360 == 180 || VideoInfo->Rotation % 360 == -180) {
		std::vector<unsigned char> data(std::move(out.data));
		out.data.resize(Width * Height * 4);
		for (int x = 0; x < Height; ++x)
			for (int y = 0; y < Width; ++y)
				for (int ch = 0; ch < 4; ++ch)
					out.data[4 * (Width * x + y) + ch] = data[frame->Linesize[0] * (Height - 1 - x) + 4 * (Width - 1 - y) + ch];
		out.pitch = 4 * Width;
	}
	else if (VideoInfo->Rotation % 180 == 90 || VideoInfo->Rotation % 360 == -270) {
		std::vector<unsigned char> data(std::move(out.data));
		out.data.resize(Width * Height * 4);
		for (int x = 0; x < Width; ++x)
			for (int y = 0; y < Height; ++y)
				for (int ch = 0; ch < 4; ++ch)
					out.data[4 * (Height * x + y) + ch] = data[frame->Linesize[0] * y + 4 * (Width - 1 - x) + ch];
		out.width = Height;
		out.height = Width;
		out.pitch = 4 * Height;
	}
	else if (VideoInfo->Rotation % 180 == 270 || VideoInfo->Rotation % 360 == -90) {
		std::vector<unsigned char> data(std::move(out.data));
		out.data.resize(Width * Height * 4);
		for (int x = 0; x < Width; ++x)
			for (int y = 0; y < Height; ++y)
				for (int ch = 0; ch < 4; ++ch)
					out.data[4 * (Height * x + y) + ch] = data[frame->Linesize[0] * (Height - 1 - y) + 4 * x + ch];
		out.width = Height;
		out.height = Width;
		out.pitch = 4 * Height;
	}
#endif
}
}

std::unique_ptr<VideoProvider> CreateFFmpegSourceVideoProvider(agi::fs::path const& path, std::string const& colormatrix, agi::BackgroundRunner *br) {
	return agi::make_unique<FFmpegSourceVideoProvider>(path, colormatrix, br);
}

#endif /* WITH_FFMS2 */

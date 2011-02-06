// Copyright (c) 2008-2009, Karl Blomster <thefluff@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @file ffms_video.h
/// @brief FFmpegSource Video support.
/// @ingroup fmms video

#ifndef AGI_PRE
#include <vector>
#endif

#include <libaegisub/exception.h>

#include "../../libffms/include/ffms.h"
#include "libaegisub/vfr.h"
#include "../common/ffms_common.h"
#include "libmedia/video.h"

namespace media {
	namespace ffms {

/// @class FFmpegSourceVideoProvider
/// @brief Implements video loading through the FFMS library.
class Video : public VideoProvider, FFmpegSourceProvider {
private:
	FFMS_VideoSource *VideoSource;			/// video source object
	const FFMS_VideoProperties *VideoInfo;	/// video properties

	int Width;					/// width in pixels
	int Height;					/// height in pixels
	int FrameNumber;			/// current framenumber
	std::vector<int> KeyFramesList;	/// list of keyframes
	agi::vfr::Framerate Timecodes;	/// vfr object
	bool COMInited;				/// COM initialization state

	media::AegiVideoFrame CurFrame;	/// current video frame

	char FFMSErrMsg[1024];		/// FFMS error message
	FFMS_ErrorInfo ErrInfo;		/// FFMS error codes/messages

	void LoadVideo(std::string filename);
	void Close();

public:
	Video(std::string filename);
	~Video();

	const media::AegiVideoFrame GetFrame(int n);

	int GetPosition() const { return FrameNumber; }
	int GetFrameCount() const { return VideoInfo->NumFrames; }
	int GetWidth() const { return Width; }
	int GetHeight() const { return Height; }
	agi::vfr::Framerate GetFPS() const { return Timecodes; }

	/// @brief Gets a list of keyframes
	/// @return	Returns a vector<int> of keyframes.
	std::vector<int> GetKeyFrames() const { return KeyFramesList; };
	/// @brief Gets the desired cache behavior.
	/// @return Returns true.
	bool WantsCaching() const { return true; }
};


	} // namespace ffms
} // namespace media

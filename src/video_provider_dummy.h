// Copyright (c) 2007, Niels Martin Hansen
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

/// @file video_provider_dummy.h
/// @see video_provider_dummy.cpp
/// @ingroup video_input
///

#include "include/aegisub/video_provider.h"

namespace agi { struct Color; }

/// @class DummyVideoProvider
/// @brief A dummy video provider for when opening a file is just too much effort
///
/// This simply returns a single constant frame, which can either be a flat
/// color or a checkerboard pattern
class DummyVideoProvider : public VideoProvider {
	int framecount;          ///< Length of the dummy video in frames
	agi::vfr::Framerate fps; ///< Frame rate to use
	int width;               ///< Width in pixels
	int height;              ///< Height in pixels

	/// The data for the image returned for all frames
	std::vector<unsigned char> data;

	/// Create the dummy frame from the given parameters
	/// @param fps Frame rate of the dummy video
	/// @param frames Length in frames of the dummy video
	/// @param width Width in pixels of the dummy video
	/// @param height Height in pixels of the dummy video
	/// @param red Red component of the primary colour of the dummy video
	/// @param green Green component of the primary colour of the dummy video
	/// @param blue Blue component of the primary colour of the dummy video
	/// @param pattern Use a checkerboard pattern rather than a solid colour
	void Create(double fps, int frames, int width, int height, unsigned char red, unsigned char green, unsigned char blue, bool pattern);

public:
	/// Create a dummy video from a string returned from MakeFilename
	DummyVideoProvider(agi::fs::path const& filename, std::string const& colormatix);

	/// Create a dummy video from separate parameters
	/// @param fps Frame rate of the dummy video
	/// @param frames Length in frames of the dummy video
	/// @param width Width in pixels of the dummy video
	/// @param height Height in pixels of the dummy video
	/// @param colour Primary colour of the dummy video
	/// @param pattern Use a checkerboard pattern rather than a solid colour
	DummyVideoProvider(double fps, int frames, int width, int height, agi::Color colour, bool pattern);

	/// Make a fake filename which when passed to the constructor taking a
	/// string will result in a video with the given parameters
	static std::string MakeFilename(double fps, int frames, int width, int height, agi::Color colour, bool pattern);

	std::shared_ptr<VideoFrame> GetFrame(int n) override;

	int GetFrameCount()             const override { return framecount; }
	int GetWidth()                  const override { return width; }
	int GetHeight()                 const override { return height; }
	double GetDAR()                 const override { return 0; }
	agi::vfr::Framerate GetFPS()    const override { return fps; }
	std::vector<int> GetKeyFrames() const override { return {}; }
	std::string GetColorSpace()     const override { return "None"; }
	std::string GetDecoderName()    const override { return "Dummy Video Provider"; }
	bool ShouldSetVideoProperties() const override { return false; }
};

// Copyright (c) 2010, Niels Martin Hansen
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
//
// $Id$

/// @brief Styles audio may be rendered in
///
/// The constants are ordered by priority:
/// Selected has highest priority and should overlap active, which should
/// overlap inactive, which should overlap normal regions.
enum AudioRenderingStyle {
	/// Regular audio with no special properties
	AudioStyle_Normal,
	/// Audio belonging to objects that can not be manipulated currently
	AudioStyle_Inactive,
	/// Audio that may be manipulated indirectly, usually part of selected lines
	AudioStyle_Active,
	/// Primary selection for work, usually coinciding with the primary playback range
	AudioStyle_Selected,
	/// Number of audio styles
	AudioStyle_MAX
};


/// @class AudioRenderingStyleRanges
/// @brief Abstract container for ranges of audio rendering styles
///
/// Interface for producers of audio rendering ranges, consumers should
/// implement this interface for objects to pass to producers.
class AudioRenderingStyleRanges {
protected:
	~AudioRenderingStyleRanges() { }
public:
	/// @brief Add a range to the line
	/// @param start First sample index in range
	/// @param end   One past last sample index in range
	/// @param style Style of the range added
	virtual void AddRange(int64_t start, int64_t end, AudioRenderingStyle style) = 0;
};

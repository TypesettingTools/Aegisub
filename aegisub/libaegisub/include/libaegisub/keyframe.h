// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file keyframe.h
/// @see keyframe.cpp
/// @ingroup libaegisub
///

#if !defined(AGI_PRE) && !defined(LAGI_PRE)
#include <vector>
#endif

#include "exception.h"

namespace agi {
	namespace vfr { class Framerate; }
	namespace keyframe {
		/// @brief Load a keyframe file
		/// @param filename File to load
		/// @return Pair of frame numbers which are keyframes and fps
		std::pair<std::vector<int>, double> Load(std::string const& filename);
		/// @brief Save keyframes to a file
		/// @param filename File to save to
		/// @param keyframes List of keyframes to save
		/// @param fps Current fps that goes with the keyframes
		void Save(std::string const& filename, std::vector<int> const& keyframes, vfr::Framerate const& fps);

		DEFINE_SIMPLE_EXCEPTION_NOINNER(Error, Exception, "keyframe/error")
	}
}

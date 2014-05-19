// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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
// Aegisub Project http://www.aegisub.org/

#include <string>
#include <vector>

class AssFile;

enum class ResampleARMode {
	Stretch,
	AddBorder,
	RemoveBorder,
	Manual
};

enum class YCbCrMatrix : int {
	rgb,
	tv_601,
	pc_601,
	tv_709,
	pc_709,
	tv_fcc,
	pc_fcc,
	tv_240m,
	pc_240m
};

YCbCrMatrix MatrixFromString(std::string const& str);
std::string MatrixToString(YCbCrMatrix mat);
std::vector<std::string> MatrixNames();

/// Configuration parameters for a resample
struct ResampleSettings {
	int margin[4];  ///< Amount to add to each margin
	int source_x;   ///< Original  X resolution
	int source_y;   ///< Original Y resolution
	int dest_x;     ///< New X resolution
	int dest_y;     ///< New Y resolution
	ResampleARMode ar_mode; ///< What to do if the old AR and new AR don't match
	YCbCrMatrix source_matrix;
	YCbCrMatrix dest_matrix;
};

/// Resample the subtitles in the project
/// @param file Subtitles to resample
/// @param settings Resample configuration settings
void ResampleResolution(AssFile *file, ResampleSettings settings);

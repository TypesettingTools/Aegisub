// Copyright (c) 2011 Niels Martin Hansen <nielsm@aegisub.org>
// Copyright (c) 2012 Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file dialog_export_ebu3264.h
/// @see dialog_export_ebu3264.cpp
/// @ingroup subtitle_io export

#include <libaegisub/vfr.h>

#include <memory>

class wxWindow;
namespace agi {
namespace charset {
class IconvWrapper;
}
} // namespace agi

#pragma pack(push, 1)
/// A binary timecode representation, packed
struct EbuTimecode {
	uint8_t h, m, s, f;
};
#pragma pack(pop)

/// User configuration for EBU Tech 3264-1991 export
class EbuExportSettings {
	/// Prefix used for saving to/loading from options
	std::string prefix;

  public:
	/// Frame rate + timecode format
	enum TvStandard {
		STL23 = 0,     ///< 23.976 fps (non-dropframe) (marked as 24)
		STL24 = 1,     ///< 24 fps (film)
		STL25 = 2,     ///< 25 fps (PAL)
		STL29 = 3,     ///< 29.97 fps (non-dropframe) (marked as 30)
		STL29drop = 4, ///< 29.97 fps (dropframe) (marked as 30)
		STL30 = 5,     ///< 30 fps (NTSC monochrome)
	};

	/// Character sets for subtitle data
	enum TextEncoding {
		iso6937_2 = 0, ///< latin multibyte
		iso8859_5 = 1, ///< cyrillic
		iso8859_6 = 2, ///< arabic
		iso8859_7 = 3, ///< greek
		iso8859_8 = 4, ///< hebrew
		utf8 = 5,      ///< nonstandard
	};

	/// Modes for handling lines over the maximum width
	enum LineWrappingMode {
		AutoWrap = 0,        ///< Wrap overly-long lines ASS-style
		AutoWrapBalance = 1, ///< Wrap overly-long lines with balanced lines
		AbortOverLength = 2, ///< Fail if there are overly-long lines
		IgnoreOverLength = 3 ///< Skip overly-long lines
	};

	/// Types of subtitles/captions that can be stored in STL files
	enum DisplayStandard {
		DSC_Open = 0,   ///< Open subtitles
		DSC_Level1 = 1, ///< Level-1 teletext closed captions
		DSC_Level2 = 2  ///< Level-2 teletext closed captions
	};

	/// Which TV standard (frame rate + timecode encoding) to use
	TvStandard tv_standard;

	/// How to encode subtitle text
	TextEncoding text_encoding;

	/// Maximum length of rows in subtitles (in characters)
	int max_line_length;

	/// How to deal with over-length rows
	LineWrappingMode line_wrapping_mode;

	/// Translate SSA alignments?
	bool translate_alignments;

	/// Timecode which time 0 in Aegisub corresponds to
	EbuTimecode timecode_offset;

	/// Are end timecodes inclusive or exclusive?
	bool inclusive_end_times;

	/// Save as subtitles, or as closed captions?
	DisplayStandard display_standard;

	/// Get the frame rate for the current TV Standard
	agi::vfr::Framerate GetFramerate() const;

	/// Get a charset encoder for the current text encoding
	std::unique_ptr<agi::charset::IconvWrapper> GetTextEncoder() const;

	/// Load saved export settings from options
	/// @param prefix Option name prefix
	EbuExportSettings(std::string const& prefix);

	/// Save export settings to options
	void Save() const;
};

/// Show a dialog box for getting an export configuration for EBU Tech 3264-1991
/// @param owner Parent window of the dialog
/// @param s Struct with initial values and to fill with the chosen settings
int ShowEbuExportConfigurationDialog(wxWindow* owner, EbuExportSettings& s);

// Copyright (c) 2011 Niels Martin Hansen <nielsm@aegisub.org>
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


// This implements support for the EBU tech 3264 (1991) subtitling data exchange format.
// Work on support for this format was sponsored by Bandai.
// Based on specifications obtained at <http://tech.ebu.ch/docs/tech/tech3264.pdf>

	/*
	  todo:
	  - gui configuration (80% - missing store/load of last used)
	  - row length enforcing (90%)
	*/


#include "config.h"

#include <wx/datetime.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/radiobox.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/button.h>
#include <wx/valgen.h>
#include <wx/valtext.h>

#include "include/aegisub/exception.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "ass_override.h"
#include "subtitle_format_ebu3264.h"
#include "text_file_writer.h"
#include "aegisub_endian.h"
#include "options.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


namespace {

#pragma pack(push, 1)
	// A binary timecode representation, packed
	struct Timecode {
		uint8_t h, m, s, f;
	};
	// General Subtitle Information block as it appears in the file
	struct BlockGSI {
		char cpn[3];    // code page number
		char dfc[8];    // disk format code
		char dsc;       // display standard code
		char cct[2];    // character code table number
		char lc[2];     // language code
		char opt[32];   // original programme title
		char oet[32];   // original episode title
		char tpt[32];   // translated programme title
		char tet[32];   // translated episode title
		char tn[32];    // translator name
		char tcd[32];   // translator contact details;
		char slr[16];   // subtitle list reference code
		char cd[6];     // creation date
		char rd[6];     // revision date
		char rn[2];     // revision number
		char tnb[5];    // total number of TTI blocks
		char tns[5];    // total number of subtitles
		char tng[3];    // total number of subtitle groups
		char mnc[2];    // maximum number of displayable characters in a row
		char mnr[2];    // maximum number of displayable rows
		char tcs;       // time code: status
		char tcp[8];    // time code: start of programme
		char tcf[8];    // time code: first in-cue
		char tnd;       // total number of disks
		char dsn;       // disk sequence number
		char co[3];     // country of origin
		char pub[32];   // publisher
		char en[32];    // editor's name
		char ecd[32];   // editor's contact details
		char unused[75];
		char uda[576];  // user defined area
	};
	// Text and Timing Information block as it appears in the file
	struct BlockTTI {
		uint8_t  sgn;   // subtitle group number
		uint16_t sn;    // subtitle number
		uint8_t  ebn;   // extension block number
		uint8_t  cs;    // cumulative status
		Timecode tci;   // time code in
		Timecode tco;   // time code out
		uint8_t  vp;    // vertical position
		uint8_t  jc;    // justification code
		uint8_t  cf;    // comment flag
		char tf[112];   // text field
	};
#pragma pack(pop)

	// user configuration for export
	struct EbuExportSettings {
		enum TvStandard {
			STL23     = 0, // 23.976 fps (non-dropframe) (marked as 24)
			STL24     = 1, // 24 fps (film)
			STL25     = 2, // 25 fps (PAL)
			STL29     = 3, // 29.97 fps (non-dropframe) (marked as 30)
			STL29drop = 4, // 29.97 fps (dropframe) (marked as 30)
			STL30     = 5, // 30 fps (NTSC monochrome)
		};
		enum TextEncoding {
			iso6937_2 = 0, // latin multibyte
			iso8859_5 = 1, // cyrillic
			iso8859_6 = 2, // arabic
			iso8859_7 = 3, // greek
			iso8859_8 = 4, // hebrew
			utf8      = 5, // nonstandard
		};
		/*enum LineWrappingMode {
			AutoWrap         = 0,
			AbortOverLength  = 1,
			IgnoreOverLength = 2,
		};*/
		// Which TV standard (frame rate + timecode encoding) to use
		TvStandard tv_standard;
		SubtitleFormat::FPSRational GetFramerate() const;
		// How to encode subtitle text
		TextEncoding text_encoding;
		std::auto_ptr<wxMBConv> GetTextEncoder() const;
		// Maximum length of rows in subtitles (in characters)
		int max_line_length;
		// Perform automatic line wrapping or ignore over-length lines?
		bool do_line_wrapping;
		// How to deal with over-length rows
		//LineWrappingMode line_wrapping_mode;
		// translate SSA alignments?
		bool translate_alignments;
		// timecode of first frame the subs were timed to
		Timecode timecode_offset;
		// are end timecodes inclusive or exclusive?
		bool inclusive_end_times;

		// Produce a string storing the settings
		wxString Serialize() const;
		// Reset settings to those stored in string
		void Deserialize(const wxString &settings);
	};

	// a block of text with really basic formatting
	struct EbuFormattedText {
		wxString text;
		bool underline;
		bool italic;
		bool wordstart;
		EbuFormattedText(const wxString &t = wxString(), bool u = false, bool i = false, bool ws = true) : text(t), underline(u), italic(i), wordstart(ws) { }
	};
	typedef std::vector<EbuFormattedText> EbuTextRow;

	// intermediate format
	struct EbuSubtitle {
		enum CumulativeStatus {
			NotCumulative,
			CumulativeStart,
			CulumativeMiddle,
			CumulativeEnd
		};
		enum JustificationCode {
			UnchangedPresentation,
			JustifyLeft,
			JustifyCentre,
			JustifyRight
		};
		enum VerticalPosition {
			PositionTop,
			PositionMiddle,
			PositionBottom
		};
		int group_number; // always 0 for compat
		// subtitle number is assigned when generating blocks
		CumulativeStatus cumulative_status; // always NotCumulative for compat
		int time_in;  // frame number
		int time_out; // frame number
		bool comment_flag; // always false for compat
		JustificationCode justification_code; // never Unchanged presentation for compat
		VerticalPosition vertical_position; // translated to row on tti conversion
		std::vector<EbuTextRow> text_rows; // text split into rows, still unicode

		EbuSubtitle();
		void SetAlignment(int ass_alignment);
		void SetTextFromAss(AssDialogue *line, bool style_underline, bool style_italic);
		void SplitLines(int max_width, int split_type);
		bool CheckLineLengths(int max_width);
	};

	// formatting character constants
	const char EBU_FORMAT_ITALIC_ON     = '\x80';
	const char EBU_FORMAT_ITALIC_OFF    = '\x81';
	const char EBU_FORMAT_UNDERLINE_ON  = '\x82';
	const char EBU_FORMAT_UNDERLINE_OFF = '\x83';
	const char EBU_FORMAT_BOXING_ON     = '\x84';
	const char EBU_FORMAT_BOXING_OFF    = '\x85';
	const char EBU_FORMAT_LINEBREAK     = '\x8a';
	const char EBU_FORMAT_UNUSED_SPACE  = '\x8f';

	// dialog box for getting an export configuration
	class EbuExportConfigurationDialog : public wxDialog {
		wxRadioBox *tv_standard_box;
		wxRadioBox *text_encoding_box;
		wxSpinCtrl *max_line_length_ctrl;
		wxCheckBox *auto_wrap_check;
		wxCheckBox *translate_alignments_check;
		wxCheckBox *inclusive_end_times_check;
		wxTextCtrl *timecode_offset_entry;
		DECLARE_EVENT_TABLE()
		void OnOk(wxCommandEvent &evt);
		void OnCancel(wxCommandEvent &evt);
	public:
		EbuExportConfigurationDialog(wxWindow *owner);
		static EbuExportSettings GetExportConfiguration(wxWindow *owner = 0);
	};
}


class TimecodeValidator : public wxValidator {
	wxRegEx re;
	Timecode *value;

	static const wchar_t *timecode_regex;// = L"([[:digit:]]{2}):([[:digit:]]{2}):([[:digit:]]{2}):([[:digit:]]{2})";

	wxTextCtrl *GetCtrl() const
	{
		return dynamic_cast<wxTextCtrl*>(GetWindow());
	}

public:
	TimecodeValidator(Timecode *target)
	: re(timecode_regex)
	, value(target)
	{
		wxASSERT(target != NULL);
	}

	TimecodeValidator(const TimecodeValidator &other)
	: re(timecode_regex)
	, value(other.value)
	{
	}

	virtual bool TransferToWindow()
	{
		wxTextCtrl *ctrl = GetCtrl();
		if (!ctrl) return false;
		ctrl->SetValue(wxString::Format(L"%02d:%02d:%02d:%02d", (int)value->h, (int)value->m, (int)value->s, (int)value->f));
		return true;
	}

	virtual bool TransferFromWindow()
	{
		wxTextCtrl *ctrl = GetCtrl();
		if (!ctrl) return false;

		wxString str = ctrl->GetValue();

		if (re.Matches(str))
		{
			long h, m, s, f;
			re.GetMatch(str, 1).ToLong(&h);
			re.GetMatch(str, 2).ToLong(&m);
			re.GetMatch(str, 3).ToLong(&s);
			re.GetMatch(str, 4).ToLong(&f);
			value->h = h;
			value->m = m;
			value->s = s;
			value->f = f;
			return true;
		}
		else
		{
			return false;
		}
	}

	virtual bool Validate(wxWindow *parent)
	{
		wxTextCtrl *ctrl = GetCtrl();
		if (!ctrl)
			return false;

		if (!re.Matches(ctrl->GetValue()))
		{
			wxMessageBox(_("Time code offset in incorrect format. Ensure it is entered as four groups of two digits separated by colons."), _("EBU STL export"), wxICON_EXCLAMATION);
			return false;
		}
		return true;
	}

	virtual wxObject * Clone() const
	{
		return new TimecodeValidator(*this);
	}
};
const wchar_t *TimecodeValidator::timecode_regex = L"([[:digit:]]{2}):([[:digit:]]{2}):([[:digit:]]{2}):([[:digit:]]{2})";


// On Unixy systems we can use iconv, but Windows doesn't have a useful
// ISO 6937-2 conversion available. (It has something, but it fails on
// common cases, making it useless.)
// To avoid pulling in iconv let's roll our own!
#ifdef _WIN32
class ConvISO6937 : public wxMBConv {
	static const int WINDOWS_ISO6937_CODEPAGE = 20269;

	struct Mapping {
		wchar_t          unicode;
		unsigned char    prefix;
		unsigned char    base;
	};
	static const Mapping unicode_iso6937_map[];

	static std::map<wchar_t, Mapping> map;
public:
	ConvISO6937()
	{
		if (!map.empty())
			return;

		const Mapping *c = unicode_iso6937_map;
		while (c->unicode != 0)
		{
			map[c->unicode] = *c;
			++c;
		}
	}

	virtual size_t ToWChar(wchar_t *dst, size_t dstLen, const char *src, size_t srcLen) const
	{
		// not really used since we only write.
		// if windows' mb2wc also happens to fail,
		// use the table the other way as well.
		return MultiByteToWideChar(
			WINDOWS_ISO6937_CODEPAGE,
			MB_ERR_INVALID_CHARS,
			src,
			srcLen,
			dst,
			dstLen
			);
	}

	virtual size_t FromWChar(char *dst, size_t dstLen, const wchar_t *src, size_t srcLen) const
	{
		// To keep things simple, first normalise the string into
		// using precomposed characters, instead of having to muck
		// about with Unicode combining diacritical marks.
		int bufNeeded = FoldStringW(
			MAP_PRECOMPOSED,
			src, srcLen,
			NULL, 0
			);
		if (bufNeeded <= 0)
			return 0;
		std::vector<wchar_t> buf(bufNeeded);
		bufNeeded = FoldStringW(
			MAP_PRECOMPOSED,
			src, srcLen,
			&buf[0], bufNeeded
			);
		// Then, go custom encoding converter!
		if (dstLen == 0)
		{
			// check buffer size required
			size_t res = 0;
			for (size_t i = 0; i < (size_t)bufNeeded; ++i)
			{
				if (buf[i] < 128)
				{
					// low ascii requires one byte;
					res += 1;
				}
				else if (buf[i] < 0x0200)
				{
					// look up the codepoint in our map
					std::map<wchar_t, Mapping>::iterator m = map.find(buf[i]);
					if (m != map.end() && m->second.prefix != 0)
						// found and has a prefix
						res += 2;
					else
						// has no prefix or was not found
						// (non-mapping characters still require 1 byte.)
						res += 1;
				}
				else
				{
					// even though some codepoints above 0x200 also map into
					// iso6937, they map into a single byte, just like any
					// non-mapping character does.
					// (non-mapping characters become question marks.)
					res += 1;
				}
			}
			return res;
		}
		else
		{
			// convert as much as possible into the target buffer
			// don't write half characters
			size_t written = 0;
			for (size_t i = 0; i < (size_t)bufNeeded && written < dstLen; ++i)
			{
				if (buf[i] < 128)
				{
					// low ascii
					dst[written++] = (char)buf[i];
				}
				else if (buf[i] < 0x3000)
				{
					// reasonably low codepoint, try to map it
					std::map<wchar_t, Mapping>::iterator m = map.find(buf[i]);
					if (m != map.end())
					{
						if (m->second.prefix == 0)
						{
							// simple mapping
							dst[written++] = m->second.base;
						}
						else if (dstLen-written >= 2)
						{
							// prefix mapping and there is room for it
							dst[written++] = m->second.prefix;
							dst[written++] = m->second.base;
						}
						else
						{
							// prefix mapping but not room for both prefix and base
							// stop here
							return written;
						}
					}
					else
					{
						// did not map, write mark of failure
						dst[written++] = '?';
					}
				}
				else
				{
					// too high codepoint, fails
					dst[written++] = '?';
				}
			}
			return written;
		}
	}

	virtual size_t GetMBNulLen() const
	{
		return 1;
	}

	virtual wxMBConv *Clone() const { return new ConvISO6937(*this); }

	bool IsOk() const { return true; }
};
// Table based on <http://en.wikipedia.org/wiki/ISO/IEC_6937>
// and code charts from the Unicode corsortium
const ConvISO6937::Mapping ConvISO6937::unicode_iso6937_map[] = {
	//unicode  pfx  base
	{ 0x00a0, 0x00, 0xa0 }, // single byte characters
	{ 0x00a1, 0x00, 0xa1 },
	{ 0x00a2, 0x00, 0xa2 },
	{ 0x00a3, 0x00, 0xa3 },
	{ 0x00a5, 0x00, 0xa5 },
	{ 0x00a7, 0x00, 0xa7 },
	{ 0x00a4, 0x00, 0xa8 },
	{ 0x2018, 0x00, 0xa9 },
	{ 0x201c, 0x00, 0xaa },
	{ 0x00ab, 0x00, 0xab },
	{ 0x2190, 0x00, 0xac },
	{ 0x2191, 0x00, 0xad },
	{ 0x2192, 0x00, 0xae },
	{ 0x2193, 0x00, 0xaf },
	{ 0x00b0, 0x00, 0xb0 },
	{ 0x00b1, 0x00, 0xb1 },
	{ 0x00b2, 0x00, 0xb2 },
	{ 0x00b3, 0x00, 0xb3 },
	{ 0x00d7, 0x00, 0xb4 },
	{ 0x00b5, 0x00, 0xb5 },
	{ 0x00b6, 0x00, 0xb6 },
	{ 0x00b7, 0x00, 0xb7 },
	{ 0x00f7, 0x00, 0xb8 },
	{ 0x2019, 0x00, 0xb9 },
	{ 0x201d, 0x00, 0xba },
	{ 0x00bb, 0x00, 0xbb },
	{ 0x00bc, 0x00, 0xbc },
	{ 0x00bd, 0x00, 0xbd },
	{ 0x00be, 0x00, 0xbe },
	{ 0x00bf, 0x00, 0xbf },
	{ 0x2015, 0x00, 0xd0 },
	{ 0x00b9, 0x00, 0xd1 },
	{ 0x00ae, 0x00, 0xd2 },
	{ 0x00a9, 0x00, 0xd3 },
	{ 0x2122, 0x00, 0xd4 },
	{ 0x266a, 0x00, 0xd5 },
	{ 0x00ac, 0x00, 0xd6 },
	{ 0x00a6, 0x00, 0xd7 },
	{ 0x215b, 0x00, 0xdc },
	{ 0x215c, 0x00, 0xdd },
	{ 0x215d, 0x00, 0xde },
	{ 0x215e, 0x00, 0xdf },
	{ 0x2126, 0x00, 0xe0 },
	{ 0x00c6, 0x00, 0xe1 },
	{ 0x00d0, 0x00, 0xe2 }, // Eth/D with stroke/African D
	{ 0x0189, 0x00, 0xe2 },
	{ 0x0110, 0x00, 0xe2 },
	{ 0x00aa, 0x00, 0xe3 },
	{ 0x0126, 0x00, 0xe4 },
	{ 0x0132, 0x00, 0xe6 },
	{ 0x013f, 0x00, 0xe7 },
	{ 0x0141, 0x00, 0xe8 },
	{ 0x00d8, 0x00, 0xe9 },
	{ 0x0152, 0x00, 0xea },
	{ 0x00ba, 0x00, 0xeb },
	{ 0x00de, 0x00, 0xec },
	{ 0x0166, 0x00, 0xed },
	{ 0x014a, 0x00, 0xee },
	{ 0x0149, 0x00, 0xef },
	{ 0x0138, 0x00, 0xf0 },
	{ 0x00e6, 0x00, 0xf1 },
	{ 0x0111, 0x00, 0xf2 },
	{ 0x00f0, 0x00, 0xf3 },
	{ 0x0127, 0x00, 0xf4 },
	{ 0x0131, 0x00, 0xf5 },
	{ 0x0133, 0x00, 0xf6 },
	{ 0x0140, 0x00, 0xf7 },
	{ 0x0142, 0x00, 0xf8 },
	{ 0x00f8, 0x00, 0xf9 },
	{ 0x0153, 0x00, 0xfa },
	{ 0x00df, 0x00, 0xfb },
	{ 0x00fe, 0x00, 0xfc },
	{ 0x0167, 0x00, 0xfd },
	{ 0x014b, 0x00, 0xfe },
	{ 0x00ad, 0x00, 0xff },
	{ 0x00c0, 0xc1, 'A'  }, // multibyte: grave accent
	{ 0x00c8, 0xc1, 'E'  },
	{ 0x00cc, 0xc1, 'I'  },
	{ 0x00d2, 0xc1, 'O'  },
	{ 0x00d9, 0xc1, 'U'  },
	{ 0x00e0, 0xc1, 'a'  },
	{ 0x00e8, 0xc1, 'e'  },
	{ 0x00ec, 0xc1, 'i'  },
	{ 0x00f2, 0xc1, 'o'  },
	{ 0x00f9, 0xc1, 'u'  },
	{ 0x00c1, 0xc2, 'A'  }, // multibyte: acute accent
	{ 0x0106, 0xc2, 'C'  },
	{ 0x00c9, 0xc2, 'E'  },
	{ 0x00cd, 0xc2, 'I'  },
	{ 0x0139, 0xc2, 'L'  },
	{ 0x0143, 0xc2, 'N'  },
	{ 0x00d3, 0xc2, 'O'  },
	{ 0x0154, 0xc2, 'R'  },
	{ 0x015a, 0xc2, 'S'  },
	{ 0x00da, 0xc2, 'U'  },
	{ 0x00dd, 0xc2, 'Y'  },
	{ 0x0179, 0xc2, 'Z'  },
	{ 0x00e1, 0xc2, 'a'  },
	{ 0x0107, 0xc2, 'c'  },
	{ 0x00e9, 0xc2, 'e'  },
	{ 0x0123, 0xc2, 'g'  },
	{ 0x00ed, 0xc2, 'i'  },
	{ 0x013a, 0xc2, 'l'  },
	{ 0x0144, 0xc2, 'n'  },
	{ 0x00f3, 0xc2, 'o'  },
	{ 0x0155, 0xc2, 'r'  },
	{ 0x015b, 0xc2, 's'  },
	{ 0x00fa, 0xc2, 'u'  },
	{ 0x00fd, 0xc2, 'y'  },
	{ 0x017a, 0xc2, 'z'  },
	{ 0x00c2, 0xc3, 'A'  }, // multibyte: circumflex
	{ 0x0108, 0xc3, 'C'  },
	{ 0x00ca, 0xc3, 'E'  },
	{ 0x011c, 0xc3, 'G'  },
	{ 0x0124, 0xc3, 'H'  },
	{ 0x00ce, 0xc3, 'I'  },
	{ 0x0134, 0xc3, 'J'  },
	{ 0x00d4, 0xc3, 'O'  },
	{ 0x015c, 0xc3, 'S'  },
	{ 0x00db, 0xc3, 'U'  },
	{ 0x0174, 0xc3, 'W'  },
	{ 0x0176, 0xc3, 'Y'  },
	{ 0x00e2, 0xc3, 'a'  },
	{ 0x0109, 0xc3, 'c'  },
	{ 0x00ea, 0xc3, 'e'  },
	{ 0x011d, 0xc3, 'g'  },
	{ 0x0125, 0xc3, 'h'  },
	{ 0x00ee, 0xc3, 'i'  },
	{ 0x0135, 0xc3, 'j'  },
	{ 0x00f4, 0xc3, 'o'  },
	{ 0x015d, 0xc3, 's'  },
	{ 0x00fb, 0xc3, 'u'  },
	{ 0x0175, 0xc3, 'w'  },
	{ 0x0177, 0xc3, 'y'  },
	{ 0x00c3, 0xc4, 'A'  }, // multibyte: tilde
	{ 0x0128, 0xc4, 'I'  },
	{ 0x00d1, 0xc4, 'N'  },
	{ 0x00d5, 0xc4, 'O'  },
	{ 0x0168, 0xc4, 'U'  },
	{ 0x00e3, 0xc4, 'a'  },
	{ 0x0129, 0xc4, 'i'  },
	{ 0x00f1, 0xc4, 'n'  },
	{ 0x00f5, 0xc4, 'o'  },
	{ 0x0169, 0xc4, 'u'  },
	{ 0x0100, 0xc5, 'A'  }, // multibyte: macron
	{ 0x0112, 0xc5, 'E'  },
	{ 0x012a, 0xc5, 'I'  },
	{ 0x014c, 0xc5, 'O'  },
	{ 0x016a, 0xc5, 'U'  },
	{ 0x0101, 0xc5, 'a'  },
	{ 0x0113, 0xc5, 'e'  },
	{ 0x012b, 0xc5, 'i'  },
	{ 0x014d, 0xc5, 'o'  },
	{ 0x016b, 0xc5, 'u'  },
	{ 0x0102, 0xc6, 'A'  }, // multibyte: accent breve
	{ 0x011e, 0xc6, 'G'  },
	{ 0x016c, 0xc6, 'U'  },
	{ 0x0103, 0xc6, 'a'  },
	{ 0x011f, 0xc6, 'g'  },
	{ 0x016d, 0xc6, 'u'  },
	{ 0x010a, 0xc7, 'C'  }, // multibyte: dot above
	{ 0x0116, 0xc7, 'E'  },
	{ 0x0120, 0xc7, 'G'  },
	{ 0x0130, 0xc7, 'I'  },
	{ 0x017b, 0xc7, 'Z'  },
	{ 0x010b, 0xc7, 'c'  },
	{ 0x0117, 0xc7, 'e'  },
	{ 0x0121, 0xc7, 'g'  },
	{ 0x017c, 0xc7, 'z'  },
	{ 0x00c4, 0xc8, 'A'  }, // multibyte: diaeresis/umlaut
	{ 0x00cb, 0xc8, 'E'  },
	{ 0x00cf, 0xc8, 'I'  },
	{ 0x00d6, 0xc8, 'O'  },
	{ 0x00dc, 0xc8, 'U'  },
	{ 0x0178, 0xc8, 'Y'  },
	{ 0x00e4, 0xc8, 'a'  },
	{ 0x00eb, 0xc8, 'e'  },
	{ 0x00ef, 0xc8, 'i'  },
	{ 0x00f6, 0xc8, 'o'  },
	{ 0x00fc, 0xc8, 'u'  },
	{ 0x00ff, 0xc8, 'y'  },
	{ 0x00c5, 0xca, 'A'  }, // multibyte: ring above
	{ 0x016e, 0xca, 'U'  },
	{ 0x00e5, 0xca, 'a'  },
	{ 0x016f, 0xca, 'u'  },
	{ 0x00c7, 0xcb, 'C'  }, // multibyte: cedilla
	{ 0x0122, 0xcb, 'G'  },
	{ 0x0136, 0xcb, 'K'  },
	{ 0x013b, 0xcb, 'L'  },
	{ 0x0145, 0xcb, 'N'  },
	{ 0x0156, 0xcb, 'R'  },
	{ 0x015e, 0xcb, 'S'  },
	{ 0x0162, 0xcb, 'T'  },
	{ 0x00e7, 0xcb, 'c'  },
	{ 0x0137, 0xcb, 'k'  },
	{ 0x013c, 0xcb, 'l'  },
	{ 0x0146, 0xcb, 'n'  },
	{ 0x0157, 0xcb, 'r'  },
	{ 0x015f, 0xcb, 's'  },
	{ 0x0163, 0xcb, 't'  },
	{ 0x0150, 0xcd, 'O'  }, // multibyte: double acute
	{ 0x0170, 0xcd, 'U'  },
	{ 0x0151, 0xcd, 'o'  },
	{ 0x0171, 0xcd, 'u'  },
	{ 0x0104, 0xce, 'A'  }, // multibyte: ogonek
	{ 0x0118, 0xce, 'E'  },
	{ 0x012e, 0xce, 'I'  },
	{ 0x0172, 0xce, 'U'  },
	{ 0x0105, 0xce, 'a'  },
	{ 0x0119, 0xce, 'e'  },
	{ 0x012f, 0xce, 'i'  },
	{ 0x0173, 0xce, 'u'  },
	{ 0x010c, 0xcf, 'C'  }, // multibyte: caron
	{ 0x010d, 0xcf, 'D'  },
	{ 0x011a, 0xcf, 'E'  },
	{ 0x013d, 0xcf, 'L'  },
	{ 0x0147, 0xcf, 'N'  },
	{ 0x0158, 0xcf, 'R'  },
	{ 0x0160, 0xcf, 'S'  },
	{ 0x0164, 0xcf, 'T'  },
	{ 0x017d, 0xcf, 'Z'  },
	{ 0x010d, 0xcf, 'c'  },
	{ 0x010e, 0xcf, 'd'  },
	{ 0x011b, 0xcf, 'e'  },
	{ 0x013e, 0xcf, 'l'  },
	{ 0x0148, 0xcf, 'n'  },
	{ 0x0159, 0xcf, 'r'  },
	{ 0x0161, 0xcf, 's'  },
	{ 0x0165, 0xcf, 't'  },
	{ 0x017e, 0xcf, 'z'  },
	{ 0,      0,    0    }  // ending sentinel
};
std::map<wchar_t, ConvISO6937::Mapping> ConvISO6937::map;
#endif


SubtitleFormat::FPSRational EbuExportSettings::GetFramerate() const
{
	typedef SubtitleFormat::FPSRational fps;
	switch (tv_standard)
	{
	case STL24:
		return fps(24);
	case STL25:
		return fps(25);
	case STL30:
		return fps(30);
	case STL23:
		return fps(24000, 1001, false);
	case STL29:
		return fps(30000, 1001, false);
	case STL29drop:
		return fps(30000, 1001, true);
	default:
		return fps(25);
	}
}

std::auto_ptr<wxMBConv> EbuExportSettings::GetTextEncoder() const
{
	wxMBConv *res = 0;
	switch (text_encoding)
	{
	case iso6937_2:
#ifdef _WIN32
		res = new ConvISO6937();
#else
		res = new wxCSConv(_T("ISO-6937-2"));
		if (!static_cast<wxCSConv*>(res)->IsOk())
			wxLogWarning(_T("Could not create ISO 6937-2 encoder, will use ISO-8859-1 instead."));
#endif
		break;
	case iso8859_5:
		res = new wxCSConv(wxFONTENCODING_ISO8859_5);
		break;
	case iso8859_6:
		res = new wxCSConv(wxFONTENCODING_ISO8859_6);
		break;
	case iso8859_7:
		res = new wxCSConv(wxFONTENCODING_ISO8859_7);
		break;
	case iso8859_8:
		res = new wxCSConv(wxFONTENCODING_ISO8859_8);
		break;
	case utf8:
		res = new wxMBConvUTF8();
		break;
	default:
		res = new wxCSConv(wxFONTENCODING_ISO8859_1);
		break;
	}
	return std::auto_ptr<wxMBConv>(res);
}

wxString EbuExportSettings::Serialize() const
{
	return wxString::Format(
		L"tvstd=%d;enc=%d;maxlen=%d;wrap=%d;usealign=%d;incend=%d;tcofs=%d",
		tv_standard,
		text_encoding,
		max_line_length,
		do_line_wrapping?1:0,
		translate_alignments?1:0,
		inclusive_end_times?1:0,
		((int)(timecode_offset.h)<<24)|((int)(timecode_offset.m)<<16)|((int)(timecode_offset.s)<<8)|(int)(timecode_offset.f)
		);
}

void EbuExportSettings::Deserialize(const wxString &settings)
{
	wxRegEx keyval(L"^([[:alpha:]]+)=(.*)$");
	wxStringTokenizer tk(settings, L";", wxTOKEN_DEFAULT);
	while (tk.HasMoreTokens())
	{
		wxString setting = tk.GetNextToken();
		if (keyval.Matches(setting))
		{
			wxString key = keyval.GetMatch(setting, 1);
			wxString val = keyval.GetMatch(setting, 2);
			long tmp = 0;
			if (key == L"tvstd")
			{
				if (val.ToLong(&tmp))
					tv_standard = (TvStandard)tmp;
			}
			else if (key == L"enc")
			{
				if (val.ToLong(&tmp))
					text_encoding = (TextEncoding)tmp;
			}
			else if (key == L"maxlen")
			{
				if (val.ToLong(&tmp))
					max_line_length = tmp;
			}
			else if (key == L"wrap")
			{
				do_line_wrapping = (val != L"0");
			}
			else if (key == L"usealign")
			{
				translate_alignments = (val != L"0");
			}
			else if (key == L"incend")
			{
				inclusive_end_times = (val != L"0");
			}
			else if (key == L"tcofs")
			{
				if (val.ToLong(&tmp))
				{
					Timecode tc;
					tc.h = (tmp>>24)&0xff;
					tc.m = (tmp>>16)&0xff;
					tc.s = (tmp>> 8)&0xff;
					tc.f = (tmp    )&0xff;
					timecode_offset = tc;
				}
			}
		}
	}
}




EbuSubtitle::EbuSubtitle()
: group_number(0)
, cumulative_status(NotCumulative)
, time_in(0)
, time_out(0)
, comment_flag(false)
, justification_code(JustifyCentre)
, vertical_position(PositionBottom)
, text_rows()
{
}

void EbuSubtitle::SetAlignment(int an)
{
	switch (an)
	{
	case 1:
	case 2:
	case 3:
		vertical_position = PositionBottom;
		break;
	case 4:
	case 5:
	case 6:
		vertical_position = PositionMiddle;
		break;
	case 7:
	case 8:
	case 9:
		vertical_position = PositionTop;
		break;
	}
	switch (an)
	{
	case 1:
	case 4:
	case 7:
		justification_code = JustifyLeft;
		break;
	case 2:
	case 5:
	case 8:
		justification_code = JustifyCentre;
		break;
	case 3:
	case 6:
	case 9:
		justification_code = JustifyRight;
		break;
	}
}

void EbuSubtitle::SetTextFromAss(AssDialogue *line, bool style_underline, bool style_italic)
{
	// Helper for finding special characters
	wxRegEx special_char_search(L"\\\\[nNh]|[ \t]", wxRE_ADVANCED);

	line->ParseASSTags();

	// current row being worked on
	EbuTextRow *cur_row;
	text_rows.clear();
	text_rows.push_back(EbuTextRow());
	cur_row = &text_rows.back();

	// create initial text part
	cur_row->push_back(EbuFormattedText(_T(""), style_underline, style_italic, true));

	for (std::vector<AssDialogueBlock*>::iterator bl = line->Blocks.begin(); bl != line->Blocks.end(); ++bl)
	{
		AssDialogueBlock *b = *bl;
		switch (b->GetType())
		{
		case BLOCK_PLAIN:
			// find special characters and convert them
			{
				wxString text = b->GetText();
				while (special_char_search.Matches(text))
				{
					size_t start, len;
					special_char_search.GetMatch(&start, &len);

					// add first part of text to current part
					cur_row->back().text.append(text.Left(start));

					// process special character
					wxString substr = text.Mid(start, len);
					if (substr == _T("\\N") || substr == _T("\\n"))
					{
						// create a new row with current style
						bool underline = cur_row->back().underline, italic = cur_row->back().italic;
						text_rows.push_back(EbuTextRow());
						cur_row = &text_rows.back();
						cur_row->push_back(EbuFormattedText(_T(""), underline, italic, true));
					}
					else if (substr == _T("\\h"))
					{
						// replace hard spaces with regular ones
						cur_row->back().text.append(_T(" "));
					}
					else if (substr == _T(" ") || substr == _T("\t"))
					{
						// space character, create new word
						cur_row->back().text.append(_T(" ")); // tabs are replaced with spaces too
						cur_row->push_back(EbuFormattedText(_T(""), cur_row->back().underline, cur_row->back().italic, true));
					}

					text = text.Mid(start+len);
				}
				// add the remaining text
				cur_row->back().text.append(text);
			}
			break;

		case BLOCK_OVERRIDE:
			// find relevant tags and process them
			{
				AssDialogueBlockOverride *ob = AssDialogueBlock::GetAsOverride(b);
				ob->ParseTags();
				bool underline = cur_row->back().underline, italic = cur_row->back().italic;
				for (std::vector<AssOverrideTag*>::iterator tag = ob->Tags.begin(); tag != ob->Tags.end(); ++tag)
				{
					AssOverrideTag *t = *tag;
					if (t->Name == _T("\\u"))
					{
						if (t->Params.size() == 0 || t->Params[0]->ommited)
						{
							underline = style_underline;
						}
						else
						{
							underline = (t->Params[0]->AsInt() != 0);
						}
					}
					else if (t->Name == _T("\\i"))
					{
						if (t->Params.size() == 0 || t->Params[0]->ommited)
						{
							italic = style_italic;
						}
						else
						{
							italic = (t->Params[0]->AsInt() != 0);
						}
					}
					else if (t->Name == _T("\\a") && t->Params.size() > 0 && !t->Params[0]->ommited)
					{
						switch (t->Params[0]->AsInt())
						{
						case  1: SetAlignment(1); break;
						case  2: SetAlignment(2); break;
						case  3: SetAlignment(3); break;
						case  5: SetAlignment(7); break;
						case  6: SetAlignment(8); break;
						case  7: SetAlignment(9); break;
						case  9: SetAlignment(4); break;
						case 10: SetAlignment(5); break;
						case 11: SetAlignment(6); break;
						}
					}
					else if (t->Name == _T("\\an") && t->Params.size() > 0 && !t->Params[0]->ommited)
					{
						SetAlignment(t->Params[0]->AsInt());
					}
				}
				// apply any changes
				if (underline != cur_row->back().underline || italic != cur_row->back().italic)
				{
					if (cur_row->back().text.IsEmpty())
					{
						// current part is empty, we can safely change formatting on it
						cur_row->back().underline = underline;
						cur_row->back().italic = italic;
					}
					else
					{
						// create a new empty part with new style
						cur_row->push_back(EbuFormattedText(_T(""), underline, italic, false));
					}
				}
			}
			break;

		default:
			// ignore block, we don't want to output it (drawing or unknown)
			break;
		}
	}

	line->ClearBlocks();
}

void EbuSubtitle::SplitLines(int max_width, int split_type)
{
	// split_type is an SSA wrap style number
	if (split_type == 2) return; // no wrapping here!
	if (split_type < 0) return;
	if (split_type > 3) return;
	std::vector<EbuTextRow> new_text;
	for (std::vector<EbuTextRow>::iterator row = text_rows.begin(); row != text_rows.end(); ++row)
	{
		int len = 0;
		for (EbuTextRow::iterator block = row->begin(); block != row->end(); ++block)
		{
			len += block->text.length();
		}
		if (len > max_width)
		{
			// line needs splitting
			// formatting handling has already split line into words!
			int lines_required = (len + max_width - 1) / max_width;
			int line_max_chars = len / lines_required;
			if (split_type == 1)
				line_max_chars = max_width;
			// cur_block is the walking iterator, cur_word keeps track of the last word start
			EbuTextRow::iterator cur_block, cur_word;
			cur_block = row->begin();
			cur_word = cur_block;
			int cur_word_len = 0;
			int cur_line_len = 0;
			new_text.push_back(EbuTextRow());
			while (cur_block != row->end())
			{
				cur_word_len += cur_block->text.length();
				cur_block++;
				if (cur_block == row->end() || cur_block->wordstart)
				{
					// next block starts a new word, we may have to break here
					if (cur_line_len + cur_word_len > line_max_chars)
					{
						if (split_type == 0)
						{
							// include word on this row, create new blank row
							new_text.back().insert(new_text.back().end(), cur_word, cur_block);
							new_text.push_back(EbuTextRow());
							cur_line_len = 0;
							// if this was the first row in the sub, make short lines from now on
							if (new_text.size() == 2) split_type = 3;
						}
						else // (split_type == 1 || split_type == 3)
						{
							// create new row, add word there
							new_text.push_back(EbuTextRow());
							new_text.back().insert(new_text.back().end(), cur_word, cur_block);
							cur_line_len = cur_word_len;
							// if this was the first row in the row and we are doing first line short,
							// switch the rest of the lines to long
							if (new_text.size() == 2 && split_type == 3) split_type = 0;
						}
					}
					else
					{
						// no need to split, just add word to current row
						new_text.back().insert(new_text.back().end(), cur_word, cur_block);
						cur_line_len += cur_word_len;
					}
					cur_word_len = 0;
					cur_word = cur_block;
				}
			}
		}
		else
		{
			// doesn't need splitting, copy straight over
			new_text.push_back(*row);
		}
	}
	// replace old text
	text_rows = new_text;
}

bool EbuSubtitle::CheckLineLengths(int max_width)
{
	for (std::vector<EbuTextRow>::iterator row = text_rows.begin(); row != text_rows.end(); ++row)
	{
		int len = 0;
		for (std::vector<EbuFormattedText>::iterator block = row->begin(); block != row->end(); ++block)
		{
			len += block->text.length();
		}
		if (len > max_width)
			// early return as soon as any line is over length
			return false;
	}
	// no lines failed
	return true;
}




EbuExportSettings EbuExportConfigurationDialog::GetExportConfiguration(wxWindow *owner)
{
	EbuExportConfigurationDialog dlg(owner);
	EbuExportSettings s;

	// initial values
	s.tv_standard = EbuExportSettings::STL29drop;
	s.timecode_offset.h = 0;
	s.timecode_offset.m = 0;
	s.timecode_offset.s = 0;
	s.timecode_offset.f = 0;
	s.inclusive_end_times = true;
	s.text_encoding = EbuExportSettings::iso6937_2;
	s.max_line_length = 42;
	s.do_line_wrapping = true;
	s.translate_alignments = true;

	s.Deserialize(Options.AsText(_T("Export EBU STL Settings")));

	// set up validators to move data in and out
	dlg.tv_standard_box->SetValidator(wxGenericValidator((int*)&s.tv_standard));
	dlg.text_encoding_box->SetValidator(wxGenericValidator((int*)&s.text_encoding));
	dlg.translate_alignments_check->SetValidator(wxGenericValidator(&s.translate_alignments));
	dlg.max_line_length_ctrl->SetValidator(wxGenericValidator(&s.max_line_length));
	dlg.auto_wrap_check->SetValidator(wxGenericValidator(&s.do_line_wrapping));
	dlg.inclusive_end_times_check->SetValidator(wxGenericValidator(&s.inclusive_end_times));
	dlg.timecode_offset_entry->SetValidator(TimecodeValidator(&s.timecode_offset));

	if (dlg.ShowModal() == wxID_OK)
	{
		Options.SetText(_T("Export EBU STL Settings"), s.Serialize());
		return s;
	}
	else
	{
		throw new Aegisub::UserCancelException(_T("EBU/STL export"));
	}
}

EbuExportConfigurationDialog::EbuExportConfigurationDialog(wxWindow *owner)
: wxDialog(owner, -1, _("Export to EBU STL format"))
{
	wxString tv_standards[] = {
		_("23.976 fps (non-standard, STL24.01)"),
		_("24 fps (non-standard, STL24.01)"),
		_("25 fps (STL25.01)"),
		_("29.97 fps (non-dropframe, STL30.01)"),
		_("29.97 fps (dropframe, STL30.01)"),
		_("30 fps (STL30.01)")
	};
	tv_standard_box = new wxRadioBox(this, -1, _("TV standard"), wxDefaultPosition, wxDefaultSize, 6, tv_standards, 0, wxRA_SPECIFY_ROWS);

	wxStaticBox *timecode_control_box = new wxStaticBox(this, -1, _("Time codes"));
	timecode_offset_entry = new wxTextCtrl(this, -1, _T("00:00:00:00"));
	inclusive_end_times_check = new wxCheckBox(this, -1, _("Out-times are inclusive"));

	wxString text_encodings[] = {
		_("ISO 6937-2 (Latin/Western Europe)"),
		_("ISO 8859-5 (Cyrillic)"),
		_("ISO 8859-6 (Arabic)"),
		_("ISO 8859-7 (Greek)"),
		_("ISO 8859-8 (Hebrew)"),
		_("UTF-8 Unicode (non-standard)")
	};
	text_encoding_box = new wxRadioBox(this, -1, _("Text encoding"), wxDefaultPosition, wxDefaultSize, 6, text_encodings, 0, wxRA_SPECIFY_ROWS);

	wxStaticBox *text_formatting_box = new wxStaticBox(this, -1, _("Text formatting"));
	max_line_length_ctrl = new wxSpinCtrl(this, -1, wxString(), wxDefaultPosition, wxSize(65, -1));
	auto_wrap_check = new wxCheckBox(this, -1, _("Automatic wrapping"));
	translate_alignments_check = new wxCheckBox(this, -1, _("Translate alignments"));

	// Default values
	// TODO: Remember last used settings
	max_line_length_ctrl->SetRange(10, 99);

	wxSizer *max_line_length_labelled = new wxBoxSizer(wxHORIZONTAL);
	max_line_length_labelled->Add(new wxStaticText(this, -1, _("Max. line length:")), 1, wxALIGN_CENTRE|wxRIGHT, 12);
	max_line_length_labelled->Add(max_line_length_ctrl, 0, 0, 0);

	wxSizer *timecode_offset_labelled = new wxBoxSizer(wxHORIZONTAL);
	timecode_offset_labelled->Add(new wxStaticText(this, -1, _("Time code offset:")), 1, wxALIGN_CENTRE|wxRIGHT, 12);
	timecode_offset_labelled->Add(timecode_offset_entry, 0, 0, 0);

	wxSizer *text_formatting_sizer = new wxStaticBoxSizer(text_formatting_box, wxVERTICAL);
	text_formatting_sizer->Add(max_line_length_labelled, 0, wxEXPAND|wxALL&~wxTOP, 6);
	text_formatting_sizer->Add(auto_wrap_check, 0, wxEXPAND|wxALL&~wxTOP, 6);
	text_formatting_sizer->Add(translate_alignments_check, 0, wxEXPAND|wxALL&~wxTOP, 6);

	wxSizer *timecode_control_sizer = new wxStaticBoxSizer(timecode_control_box, wxVERTICAL);
	timecode_control_sizer->Add(timecode_offset_labelled, 0, wxEXPAND|wxALL&~wxTOP, 6);
	timecode_control_sizer->Add(inclusive_end_times_check, 0, wxEXPAND|wxALL&~wxTOP, 6);

	wxSizer *left_column = new wxBoxSizer(wxVERTICAL);
	left_column->Add(tv_standard_box, 0, wxEXPAND|wxBOTTOM, 6);
	left_column->Add(timecode_control_sizer, 0, wxEXPAND, 0);

	wxSizer *right_column = new wxBoxSizer(wxVERTICAL);
	right_column->Add(text_encoding_box, 0, wxEXPAND|wxBOTTOM, 6);
	right_column->Add(text_formatting_sizer, 0, wxEXPAND, 0);

	wxSizer *vertical_split_sizer = new wxBoxSizer(wxHORIZONTAL);
	vertical_split_sizer->Add(left_column, 0, wxRIGHT, 6);
	vertical_split_sizer->Add(right_column, 0, 0, 0);

	wxSizer *buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
	// Developers are requested to leave this message in! Intentionally not translateable.
	wxStaticText *sponsor_label = new wxStaticText(this, -1, _T("EBU STL format writing sponsored by Bandai"));
	sponsor_label->Enable(false);
	buttons_sizer->Add(sponsor_label, 1, wxALIGN_BOTTOM, 0);
	wxButton *ok_button = new wxButton(this, wxID_OK);
	buttons_sizer->Add(ok_button, 0, wxLEFT, 6);
	buttons_sizer->Add(new wxButton(this, wxID_CANCEL), 0, wxLEFT, 6);

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(vertical_split_sizer, 0, wxEXPAND|wxALL, 12);
	main_sizer->Add(buttons_sizer, 0, wxEXPAND|wxALL&~wxTOP, 12);

	SetAffirmativeId(wxID_OK);
	ok_button->SetDefault();

	SetSizerAndFit(main_sizer);
	CenterOnParent();
}

BEGIN_EVENT_TABLE(EbuExportConfigurationDialog, wxDialog)
	EVT_BUTTON(wxID_OK, EbuExportConfigurationDialog::OnOk)
	EVT_BUTTON(wxID_CANCEL, EbuExportConfigurationDialog::OnCancel)
END_EVENT_TABLE()

void EbuExportConfigurationDialog::OnOk(wxCommandEvent &evt)
{
	// TODO: Store settings chosen for use as defaults later
	if (Validate() && TransferDataFromWindow())
	{
		EndModal(wxID_OK);
	}
}

void EbuExportConfigurationDialog::OnCancel(wxCommandEvent &evt)
{
	EndModal(wxID_CANCEL);
}



wxString Ebu3264SubtitleFormat::GetName()
{
	return _T("EBU subtitling data exchange format (EBU tech 3264, 1991)");
}


wxArrayString Ebu3264SubtitleFormat::GetWriteWildcards()
{
	wxArrayString formats;
	formats.Add(_T("stl"));
	return formats;
}


bool Ebu3264SubtitleFormat::CanWriteFile(wxString filename)
{
	return (filename.Right(4).Lower() == _T(".stl"));
}


void Ebu3264SubtitleFormat::WriteFile(wxString filename,wxString encoding)
{
	// collect data from user
	EbuExportSettings export_settings = EbuExportConfigurationDialog::GetExportConfiguration();

	// apply split/merge algorithm
	CreateCopy();
	StripComments();
	SortLines();
	RecombineOverlaps();
	MergeIdentical();

	int line_wrap_type = GetAssFile()->GetScriptInfoAsInt(_T("WrapStyle"));

	// time->frame conversion
	// set a separator just to humor FractionalTime
	// (we do our own format string stuff and use binary representation most of the time anyway)
	FPSRational fps = export_settings.GetFramerate();
	FractionalTime fractime(_T(":"), fps.num, fps.den, fps.smpte_dropframe);
	Timecode &tcofs = export_settings.timecode_offset;
	int timecode_bias = fractime.FramenumberFromFields(tcofs.h, tcofs.m, tcofs.s, tcofs.s);

	// convert to intermediate format
	std::vector<EbuSubtitle> subs_list;
	for (entryIter orgline = Line->begin(); orgline != Line->end(); ++orgline)
	{
		AssDialogue *line = AssEntry::GetAsDialogue(*orgline);
		if (line != 0 && !line->Comment)
		{
			// add a new subtitle and work on it
			subs_list.push_back(EbuSubtitle());
			EbuSubtitle &imline = subs_list.back();
			// some defaults for compatibility
			imline.group_number = 0;
			imline.comment_flag = false;
			imline.cumulative_status = EbuSubtitle::NotCumulative;
			// convert times
			imline.time_in = fractime.FramenumberFromMillisecs(line->Start.GetMS()) + timecode_bias;
			imline.time_out = fractime.FramenumberFromMillisecs(line->End.GetMS()) + timecode_bias;
			if (export_settings.inclusive_end_times)
				// cheap and possibly wrong way to ensure exclusive times, subtract one frame from end time
				imline.time_out -= 1;
			// convert alignment from style
			AssStyle *style = GetAssFile()->GetStyle(line->Style);
			if (style != 0)
			{
				imline.SetAlignment(style->alignment);
			}
			// add text, translate formatting
			if (style != 0)
				imline.SetTextFromAss(line, style->underline, style->italic);
			else
				imline.SetTextFromAss(line, false, false);
			// line breaking handling
			if (export_settings.do_line_wrapping)
				imline.SplitLines(export_settings.max_line_length, line_wrap_type);
			/*if (export_settings.line_wrapping_mode == EbuExportSettings::AutoWrap)
			{
				imline.SplitLines(export_settings.max_line_length, line_wrap_type);
			}
			else if (export_settings.line_wrapping_mode == EbuExportSettings::AbortOverLength)
			{
				wxLogError(_("Line over maximum length: %s"), line->Text.c_str());
				throw new Aegisub::UserCancelException(_T("Line over length limit")); // fixme: should be something else
			}*/
		}
	}

	// produce an empty line if there are none
	// (it still has to contain a space to not get ignored)
	if (subs_list.size() == 0)
	{
		subs_list.push_back(EbuSubtitle());
		subs_list.back().text_rows.push_back(EbuTextRow());
		subs_list.back().text_rows.back().push_back(EbuFormattedText(_T(" ")));
	}

	// for later use!
	wxString scriptinfo_title = GetAssFile()->GetScriptInfo(_T("Title"));
	wxString scriptinfo_translation = GetAssFile()->GetScriptInfo(_T("Original Translation"));
	wxString scriptinfo_editing = GetAssFile()->GetScriptInfo(_T("Original Editing"));
	// because it's silly to keep a copy of the file around for too
	ClearCopy();

	// line breaking/length checking
	// skip for now

	// prepare a text encoder
	// fixme: should use unique_ptr<> instead in C++11
	std::auto_ptr<wxMBConv> encoder = export_settings.GetTextEncoder();

	// produce blocks
	uint16_t subtitle_number = 0;
	std::vector<BlockTTI> tti;
	tti.reserve(subs_list.size());
	for (std::vector<EbuSubtitle>::iterator sub = subs_list.begin(); sub != subs_list.end(); ++sub)
	{
		std::string fullstring;
		for (std::vector<EbuTextRow>::iterator row = sub->text_rows.begin(); row != sub->text_rows.end(); ++row)
		{
			// formatting is reset at the start of every row, so keep track per row
			bool underline = false, italic = false;
			for (std::vector<EbuFormattedText>::iterator block = row->begin(); block != row->end(); ++block)
			{
				// insert codes for changed formatting
				if (underline != block->underline)
				{
					fullstring += (block->underline ? EBU_FORMAT_UNDERLINE_ON : EBU_FORMAT_UNDERLINE_OFF);
					underline = block->underline;
				}
				if (italic != block->italic)
				{
					fullstring += (block->italic ? EBU_FORMAT_ITALIC_ON : EBU_FORMAT_ITALIC_OFF);
					italic = block->italic;
				}
				// convert text to specified encoding
				fullstring += block->text.mb_str(*encoder);
			}
			// check that this is not the last row
			if (row+1 != sub->text_rows.end())
			{
				// insert linebreak for non-final lines
				fullstring += EBU_FORMAT_LINEBREAK;
			}
		}

		// construct a base block that can be copied and filled
		BlockTTI base;
		base.sgn = sub->group_number;
		base.sn = Endian::MachineToLittle(subtitle_number);
		base.ebn = 255;
		base.cf = sub->comment_flag;
		memset(base.tf, EBU_FORMAT_UNUSED_SPACE, sizeof(base.tf));
		// time codes
		{
			int h=0, m=0, s=0, f=0;
			fractime.FieldsFromFramenumber(sub->time_in,  h, m, s, f);
			base.tci.h = h;
			base.tci.m = m;
			base.tci.s = s;
			base.tci.f = f;
			fractime.FieldsFromFramenumber(sub->time_out, h, m, s, f);
			base.tco.h = h;
			base.tco.m = m;
			base.tco.s = s;
			base.tco.f = f;
		}
		// cumulative status
		if (sub->cumulative_status == EbuSubtitle::CumulativeStart)
			base.cs = 1;
		else if (sub->cumulative_status == EbuSubtitle::CulumativeMiddle)
			base.cs = 2;
		else if (sub->cumulative_status == EbuSubtitle::CumulativeEnd)
			base.cs = 3;
		else //if (sub->cumulative_status == EbuSubtitle::NotCumulative)
			base.cs = 0;
		// vertical position
		if (sub->vertical_position == EbuSubtitle::PositionTop)
			base.vp = 0;
		else if (sub->vertical_position == EbuSubtitle::PositionMiddle)
			base.vp = std::min<size_t>(0, 50 - (10 * sub->text_rows.size()));
		else //if (sub->vertical_position == EbuSubtitle::PositionBottom)
			base.vp = 99;
		// justification code
		if (sub->justification_code == EbuSubtitle::JustifyLeft)
			base.jc = 1;
		else if (sub->justification_code == EbuSubtitle::JustifyCentre)
			base.jc = 2;
		else if (sub->justification_code == EbuSubtitle::JustifyRight)
			base.jc = 3;
		else //if (sub->justification_code == EbuSubtitle::UnchangedPresentation)
			base.jc = 0;

		// produce blocks from string
		uint8_t num_blocks = 0;
		size_t bytes_remaining = fullstring.size();
		std::string::iterator pos = fullstring.begin();
		while (bytes_remaining > 0)
		{
			tti.push_back(base);
			BlockTTI &block = tti.back();
			if (bytes_remaining > sizeof(block.tf))
				// the text does not end in this block, write an extension block number
				block.ebn = num_blocks;
			else
				// this was the last block for the subtitle
				block.ebn = 255;
			size_t max_writeable = std::min(sizeof(block.tf), bytes_remaining);
			std::copy(pos, pos+max_writeable, block.tf);
			bytes_remaining -= max_writeable;
			pos += max_writeable;
		}

		// increment subtitle number for next
		subtitle_number += 1;
	}

	// build header
	wxCSConv gsi_encoder(wxFONTENCODING_CP850);
	if (!gsi_encoder.IsOk())
		wxLogWarning(_T("Could not get an IBM 850 text encoder, EBU/STL header may contain incorrectly encoded data."));
	BlockGSI gsi;
	memset(&gsi, 0x20, sizeof(gsi)); // fill with spaces
	memcpy(gsi.cpn, "850", 3);
	{
		const char *dfcstr;
		switch (export_settings.tv_standard)
		{
		case EbuExportSettings::STL23:
		case EbuExportSettings::STL24:
			dfcstr = "STL24.01";
			break;
		case EbuExportSettings::STL29:
		case EbuExportSettings::STL29drop:
		case EbuExportSettings::STL30:
			dfcstr = "STL30.01";
			break;
		case EbuExportSettings::STL25:
		default:
			dfcstr = "STL25.01";
			break;
		}
		memcpy(gsi.dfc, dfcstr, 8);
	}
	gsi.dsc = '0'; // open subtitling
	gsi.cct[0] = '0';
	if (export_settings.text_encoding == EbuExportSettings::iso6937_2)
		gsi.cct[1] = '0';
	else if (export_settings.text_encoding == EbuExportSettings::iso8859_5)
		gsi.cct[1] = '1';
	else if (export_settings.text_encoding == EbuExportSettings::iso8859_6)
		gsi.cct[1] = '2';
	else if (export_settings.text_encoding == EbuExportSettings::iso8859_7)
		gsi.cct[1] = '3';
	else if (export_settings.text_encoding == EbuExportSettings::iso8859_8)
		gsi.cct[1] = '4';
	else if (export_settings.text_encoding == EbuExportSettings::utf8)
		memcpy(gsi.cct, "U8", 2);
	memcpy(gsi.lc, "00", 2);
	gsi_encoder.FromWChar(gsi.opt, 32, scriptinfo_title.wc_str(), scriptinfo_title.length());
	gsi_encoder.FromWChar(gsi.tn, 32, scriptinfo_translation.wc_str(), scriptinfo_translation.length());
	{
		char buf[20];
		time_t now;
		time(&now);
		tm *thetime = localtime(&now);
		strftime(buf, 20, "AGI-%y%m%d%H%M%S", thetime);
		memcpy(gsi.slr, buf, 16);
		strftime(buf, 20, "%y%m%d", thetime);
		memcpy(gsi.cd, buf, 6);
		memcpy(gsi.rd, buf, 6);
		memcpy(gsi.rn, "00", 2);
		sprintf(buf, "%5u", (unsigned int)tti.size());
		memcpy(gsi.tnb, buf, 5);
		sprintf(buf, "%5u", (unsigned int)subs_list.size());
		memcpy(gsi.tns, buf, 5);
		memcpy(gsi.tng, "001", 3);
		sprintf(buf, "%02u", (unsigned int)export_settings.max_line_length);
		memcpy(gsi.mnc, buf, 2);
		memcpy(gsi.mnr, "99", 2);
		gsi.tcs = '1';
		int h, m, s, f;
		fractime.FieldsFromFramenumber(timecode_bias, h, m, s, f);
		sprintf(buf, "%02u%02u%02u%02u", h, m, s, f);
		memcpy(gsi.tcp, buf, 8);
		BlockTTI &block0 = tti.front();
		sprintf(buf, "%02u%02u%02u%02u", (unsigned int)block0.tci.h, (unsigned int)block0.tci.m, (unsigned int)block0.tci.s, (unsigned int)block0.tci.f);
		memcpy(gsi.tcf, buf, 8);
	}
	gsi.tnd = '1';
	gsi.dsn = '1';
	memcpy(gsi.co, "NTZ", 3); // neutral zone!
	gsi_encoder.FromWChar(gsi.en, 32, scriptinfo_editing.wc_str(), scriptinfo_editing.length());
	if (export_settings.text_encoding == EbuExportSettings::utf8)
	{
		char warning_str[] = "This file was exported by Aegisub using non-standard UTF-8 encoding for the subtitle blocks. The TTI.TF field contains UTF-8-encoded text interspersed with the standard formatting codes, which are not encoded. GSI.CCT is set to 'U8' to signify this.";
		memcpy(gsi.uda, warning_str, sizeof(warning_str));
	}
	
	// write file
	wxFile f(filename, wxFile::write);
	if (!f.IsOpened())
	{
		wxLogError(_T("Could not open file for writing: %s"), filename.c_str());
		return;
	}
	f.Write(&gsi, sizeof(gsi));
	for (std::vector<BlockTTI>::iterator block = tti.begin(); block != tti.end(); ++block)
	{
		f.Write(&*block, sizeof(*block));
	}
	f.Close();
}


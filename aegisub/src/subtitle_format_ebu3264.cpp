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

/// @file subtitle_format_ebu3264.cpp
/// @see subtitle_format_ebu3264.h
/// @ingroup subtitle_io

// This implements support for the EBU tech 3264 (1991) subtitling data exchange format.
// Work on support for this format was sponsored by Bandai.

#include "config.h"

#include "subtitle_format_ebu3264.h"

#include <wx/regex.h>

#include <libaegisub/charset_conv.h>
#include <libaegisub/exception.h>
#include <libaegisub/io.h>
#include <libaegisub/line_wrap.h>
#include <libaegisub/of_type_adaptor.h>
#include <libaegisub/scoped_ptr.h>

#include "aegisub_endian.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "compat.h"
#include "dialog_export_ebu3264.h"
#include "main.h"
#include "text_file_writer.h"

namespace
{
#pragma pack(push, 1)
	/// General Subtitle Information block as it appears in the file
	struct BlockGSI
	{
		char cpn[3];    ///< code page number
		char dfc[8];    ///< disk format code
		char dsc;       ///< display standard code
		char cct[2];    ///< character code table number
		char lc[2];     ///< language code
		char opt[32];   ///< original programme title
		char oet[32];   ///< original episode title
		char tpt[32];   ///< translated programme title
		char tet[32];   ///< translated episode title
		char tn[32];    ///< translator name
		char tcd[32];   ///< translator contact details
		char slr[16];   ///< subtitle list reference code
		char cd[6];     ///< creation date
		char rd[6];     ///< revision date
		char rn[2];     ///< revision number
		char tnb[5];    ///< total number of TTI blocks
		char tns[5];    ///< total number of subtitles
		char tng[3];    ///< total number of subtitle groups
		char mnc[2];    ///< maximum number of displayable characters in a row
		char mnr[2];    ///< maximum number of displayable rows
		char tcs;       ///< time code: status
		char tcp[8];    ///< time code: start of programme
		char tcf[8];    ///< time code: first in-cue
		char tnd;       ///< total number of disks
		char dsn;       ///< disk sequence number
		char co[3];     ///< country of origin
		char pub[32];   ///< publisher
		char en[32];    ///< editor's name
		char ecd[32];   ///< editor's contact details
		char unused[75];
		char uda[576];  ///< user defined area
	};

	/// Text and Timing Information block as it appears in the file
	struct BlockTTI
	{
		uint8_t     sgn; ///< subtitle group number
		uint16_t    sn;  ///< subtitle number
		uint8_t     ebn; ///< extension block number
		uint8_t     cs;  ///< cumulative status
		EbuTimecode tci; ///< time code in
		EbuTimecode tco; ///< time code out
		uint8_t     vp;  ///< vertical position
		uint8_t     jc;  ///< justification code
		uint8_t     cf;  ///< comment flag
		char    tf[112]; ///< text field
	};
#pragma pack(pop)

	/// A block of text with basic formatting information
	struct EbuFormattedText
	{
		wxString text;   ///< Text in this block
		bool underline;  ///< Is this block underlined?
		bool italic;     ///< Is this block italic?
		bool word_start; ///< Is it safe to line-wrap between this block and the previous one?
		EbuFormattedText(wxString const& t, bool u = false, bool i = false, bool ws = true) : text(t), underline(u), italic(i), word_start(ws) { }
	};
	typedef std::vector<EbuFormattedText> EbuTextRow;

	/// Formatting character constants
	const unsigned char EBU_FORMAT_ITALIC[]     = "\x81\x80";
	const unsigned char EBU_FORMAT_UNDERLINE[]  = "\x83\x82";
	const unsigned char EBU_FORMAT_BOXING[]     = "\x85\x84";
	const unsigned char EBU_FORMAT_LINEBREAK    = '\x8a';
	const unsigned char EBU_FORMAT_UNUSED_SPACE = '\x8f';

	/// intermediate format
	class EbuSubtitle
	{
		void ProcessOverrides(AssDialogueBlockOverride *ob, bool &underline, bool &italic, int &align, bool style_underline, bool style_italic)
		{
			for (auto const& t : ob->Tags)
			{
				if (t.Name == "\\u")
					underline = t.Params[0].Get<bool>(style_underline);
				else if (t.Name == "\\i")
					italic = t.Params[0].Get<bool>(style_italic);
				else if (t.Name == "\\an")
					align = t.Params[0].Get<int>(align);
				else if (t.Name == "\\a" && !t.Params[0].omitted)
					align = AssStyle::SsaToAss(t.Params[0].Get<int>());
			}
		}

		void SetAlignment(int ass_alignment)
		{
			if (ass_alignment < 1 || ass_alignment > 9)
				ass_alignment = 2;

			vertical_position = static_cast<VerticalPosition>(ass_alignment / 3);
			justification_code = static_cast<JustificationCode>((ass_alignment - 1) % 3 + 1);
		}

	public:
		enum CumulativeStatus
		{
			NotCumulative    = 0,
			CumulativeStart  = 1,
			CulumativeMiddle = 2,
			CumulativeEnd    = 3
		};

		enum JustificationCode
		{
			UnchangedPresentation = 0,
			JustifyLeft           = 1,
			JustifyCentre         = 2,
			JustifyRight          = 3
		};

		// note: not set to constants from spec
		enum VerticalPosition
		{
			PositionTop    = 2,
			PositionMiddle = 1,
			PositionBottom = 0
		};

		int group_number; ///< always 0 for compat
		/// subtitle number is assigned when generating blocks
		CumulativeStatus cumulative_status; ///< always NotCumulative for compat
		int time_in;       ///< frame number
		int time_out;      ///< frame number
		bool comment_flag; ///< always false for compat
		JustificationCode justification_code; ///< never Unchanged presentation for compat
		VerticalPosition vertical_position;   ///< translated to row on tti conversion
		std::vector<EbuTextRow> text_rows;    ///< text split into rows, still unicode

		EbuSubtitle()
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

		void SplitLines(int max_width, int split_type)
		{
			// split_type is an SSA wrap style number
			if (split_type == 2) return; // no wrapping here!
			if (split_type < 0) return;
			if (split_type > 4) return;

			std::vector<EbuTextRow> new_text;
			new_text.reserve(text_rows.size());

			for (auto const& row : text_rows)
			{
				// Get lengths of each word
				std::vector<size_t> word_lengths;
				for (auto const& cur_block : row)
				{
					if (cur_block.word_start)
						word_lengths.push_back(0);
					word_lengths.back() += cur_block.text.size();
				}

				std::vector<size_t> split_points = agi::get_wrap_points(word_lengths, (size_t)max_width, (agi::WrapMode)split_type);

				if (split_points.empty())
				{
					// Line doesn't need splitting, so copy straight over
					new_text.push_back(row);
					continue;
				}

				// Apply the splits
				new_text.emplace_back();
				size_t cur_word = 0;
				size_t split_point = 0;
				for (auto const& cur_block : row)
				{
					if (cur_block.word_start && split_point < split_points.size())
					{
						if (split_points[split_point] == cur_word)
						{
							new_text.emplace_back();
							++split_point;
						}
						++cur_word;
					}

					new_text.back().push_back(cur_block);
				}
			}

			// replace old text
			swap(text_rows, new_text);
		}

		bool CheckLineLengths(int max_width) const
		{
			for (auto const& row : text_rows)
			{
				int line_length = 0;
				for (auto const& block : row)
					line_length += block.text.size();

				if (line_length > max_width)
					// early return as soon as any line is over length
					return false;
			}
			// no lines failed
			return true;
		}

		void SetTextFromAss(AssDialogue *line, bool style_underline, bool style_italic, int align, int wrap_mode)
		{
			// Helper for finding special characters
			wxRegEx special_char_search("\\\\[nN]| ", wxRE_ADVANCED);

			boost::ptr_vector<AssDialogueBlock> blocks(line->ParseTags());

			text_rows.clear();
			text_rows.emplace_back();

			// current row being worked on
			EbuTextRow *cur_row = &text_rows.back();

			// create initial text part
			cur_row->emplace_back("", style_underline, style_italic, true);

			bool underline = style_underline, italic = style_italic;

			for (auto& b : blocks)
			{
				switch (b.GetType())
				{
					case BLOCK_PLAIN:
					// find special characters and convert them
					{
						wxString text = b.GetText();

						// Skip comments
						if (text.size() > 1 && text[0] =='{' && text.Last() == '}')
							continue;

						text.Replace("\\t", " ");

						while (special_char_search.Matches(text))
						{
							size_t start, len;
							special_char_search.GetMatch(&start, &len);

							// add first part of text to current part
							cur_row->back().text.append(text.Left(start));

							// process special character
							wxString substr = text.Mid(start, len);
							if (substr == "\\N" || (wrap_mode == 1 && substr == "\\n"))
							{
								// create a new row with current style
								text_rows.emplace_back();
								cur_row = &text_rows.back();
								cur_row->emplace_back("", underline, italic, true);
							}
							else // if (substr == " " || substr == "\\h" || substr == "\\n")
							{
								cur_row->back().text.append(" ");
								cur_row->emplace_back("", underline, italic, true);
							}

							text = text.Mid(start+len);
						}

						// add the remaining text
						cur_row->back().text.append(text);

						// convert \h to regular spaces
						// done after parsing so that words aren't split on \h
						cur_row->back().text.Replace("\\h", " ");
					}
					break;

					case BLOCK_OVERRIDE:
					// find relevant tags and process them
					{
						AssDialogueBlockOverride *ob = static_cast<AssDialogueBlockOverride*>(&b);
						ob->ParseTags();
						ProcessOverrides(ob, underline, italic, align, style_underline, style_italic);

						// apply any changes
						if (underline != cur_row->back().underline || italic != cur_row->back().italic)
						{
							if (!cur_row->back().text)
							{
								// current part is empty, we can safely change formatting on it
								cur_row->back().underline = underline;
								cur_row->back().italic = italic;
							}
							else
							{
								// create a new empty part with new style
								cur_row->push_back(EbuFormattedText("", underline, italic, false));
							}
						}
					}
					break;

				default:
					// ignore block, we don't want to output it (drawing or unknown)
					break;
				}
			}

			SetAlignment(align);
		}
	};

	std::vector<EbuSubtitle> convert_subtitles(AssFile &copy, EbuExportSettings const& export_settings)
	{
		SubtitleFormat::StripComments(copy);
		copy.Sort();
		SubtitleFormat::RecombineOverlaps(copy);
		SubtitleFormat::MergeIdentical(copy);

		int line_wrap_type = copy.GetScriptInfoAsInt("WrapStyle");

		agi::vfr::Framerate fps = export_settings.GetFramerate();
		EbuTimecode tcofs = export_settings.timecode_offset;
		int timecode_bias = fps.FrameAtSmpte(tcofs.h, tcofs.m, tcofs.s, tcofs.s);

		AssStyle default_style;
		std::vector<EbuSubtitle> subs_list;
		subs_list.reserve(copy.Line.size());

		// convert to intermediate format
		for (auto line : copy.Line | agi::of_type<AssDialogue>())
		{
			// add a new subtitle and work on it
			subs_list.emplace_back();
			EbuSubtitle &imline = subs_list.back();

			// some defaults for compatibility
			imline.group_number = 0;
			imline.comment_flag = false;
			imline.cumulative_status = EbuSubtitle::NotCumulative;

			// convert times
			imline.time_in = fps.FrameAtTime(line->Start) + timecode_bias;
			imline.time_out = fps.FrameAtTime(line->End) + timecode_bias;
			if (export_settings.inclusive_end_times)
				// cheap and possibly wrong way to ensure exclusive times, subtract one frame from end time
				imline.time_out -= 1;

			// convert alignment from style
			AssStyle *style = copy.GetStyle(line->Style);
			if (!style)
				style = &default_style;

			// add text, translate formatting
			imline.SetTextFromAss(line, style->underline, style->italic, style->alignment, line_wrap_type);

			// line breaking handling
			if (export_settings.line_wrapping_mode == EbuExportSettings::AutoWrap)
				imline.SplitLines(export_settings.max_line_length, line_wrap_type);
			else if (export_settings.line_wrapping_mode == EbuExportSettings::AutoWrapBalance)
				imline.SplitLines(export_settings.max_line_length, agi::Wrap_Balanced);
			else if (!imline.CheckLineLengths(export_settings.max_line_length))
			{
				if (export_settings.line_wrapping_mode == EbuExportSettings::AbortOverLength)
					throw Ebu3264SubtitleFormat::ConversionFailed(from_wx(wxString::Format(_("Line over maximum length: %s"), line->Text.get())), 0);
				else // skip over-long lines
					subs_list.pop_back();
			}
		}

		// produce an empty line if there are none
		// (it still has to contain a space to not get ignored)
		if (subs_list.empty())
		{
			subs_list.emplace_back();
			subs_list.back().text_rows.emplace_back();
			subs_list.back().text_rows.back().emplace_back(" ");
		}

		return subs_list;
	}

	inline size_t buffer_size(wxString const& str)
	{
#if wxUSE_UNICODE_UTF8
		return str.utf8_length();
#else
		return str.length() * sizeof(wxStringCharType);
#endif
	}

	inline const char *wx_str(wxString const& str)
	{
		return reinterpret_cast<const char *>(str.wx_str());
	}

	std::string convert_subtitle_line(EbuSubtitle const& sub, agi::charset::IconvWrapper *encoder, bool enable_formatting)
	{
		std::string fullstring;
		for (auto const& row : sub.text_rows)
		{
			if (!fullstring.empty())
				fullstring += EBU_FORMAT_LINEBREAK;

			// formatting is reset at the start of every row, so keep track per row
			bool underline = false, italic = false;
			for (auto const& block : row)
			{
				if (enable_formatting)
				{
					// insert codes for changed formatting
					if (underline != block.underline)
						fullstring += EBU_FORMAT_UNDERLINE[block.underline];
					if (italic != block.italic)
						fullstring += EBU_FORMAT_ITALIC[block.italic];

					underline = block.underline;
					italic = block.italic;
				}

				// convert text to specified encoding
				fullstring += encoder->Convert(std::string(wx_str(block.text), buffer_size(block.text)));
			}
		}
		return fullstring;
	}

	void smpte_at_frame(agi::vfr::Framerate const& fps, int frame, EbuTimecode &tc)
	{
		int h=0, m=0, s=0, f=0;
		fps.SmpteAtFrame(frame, &h, &m, &s, &f);
		tc.h = h;
		tc.m = m;
		tc.s = s;
		tc.f = f;
	}

	std::vector<BlockTTI> create_blocks(std::vector<EbuSubtitle> const& subs_list, EbuExportSettings const& export_settings)
	{
		agi::scoped_ptr<agi::charset::IconvWrapper> encoder(export_settings.GetTextEncoder());
		agi::vfr::Framerate fps = export_settings.GetFramerate();

		// Teletext captions are 1-23; Open subtitles are 0-99
		uint8_t min_row = 0;
		uint8_t max_row = 100;
		if (export_settings.display_standard != EbuExportSettings::DSC_Open) {
			min_row = 1;
			max_row = 24;
		}

		uint16_t subtitle_number = 0;

		std::vector<BlockTTI> tti;
		tti.reserve(subs_list.size());
		for (auto const& sub : subs_list)
		{
			std::string fullstring = convert_subtitle_line(sub, encoder.get(),
				export_settings.display_standard == EbuExportSettings::DSC_Open);

			// construct a base block that can be copied and filled
			BlockTTI base;
			base.sgn = sub.group_number;
			base.sn = Endian::MachineToLittle(subtitle_number++);
			base.ebn = 255;
			base.cf = sub.comment_flag;
			memset(base.tf, EBU_FORMAT_UNUSED_SPACE, sizeof(base.tf));
			smpte_at_frame(fps, sub.time_in, base.tci);
			smpte_at_frame(fps, sub.time_out, base.tco);
			base.cs = sub.cumulative_status;

			if (export_settings.translate_alignments)
			{
				// vertical position
				if (sub.vertical_position == EbuSubtitle::PositionTop)
					base.vp = min_row;
				else if (sub.vertical_position == EbuSubtitle::PositionMiddle)
					base.vp = std::min<uint8_t>(min_row, max_row / 2 - (max_row / 5 * sub.text_rows.size()));
				else //if (sub.vertical_position == EbuSubtitle::PositionBottom)
					base.vp = max_row - 1;

				base.jc = sub.justification_code;
			}
			else
			{
				base.vp = max_row - 1;
				base.jc = EbuSubtitle::JustifyCentre;
			}

			// produce blocks from string
			static const size_t block_size = sizeof(((BlockTTI*)0)->tf);
			uint8_t num_blocks = 0;
			for (size_t pos = 0; pos < fullstring.size(); pos += block_size)
			{
				size_t bytes_remaining = fullstring.size() - pos;

				tti.push_back(base);
				// write an extension block number if the remaining text doesn't fit in the block
				tti.back().ebn = bytes_remaining >= block_size ? num_blocks++ : 255;

				std::copy(&fullstring[pos], &fullstring[pos + std::min(block_size, bytes_remaining)], tti.back().tf);

				// Write another block for the terminator if we exactly used up
				// the last block
				if (bytes_remaining == block_size)
					tti.push_back(base);
			}
		}

		return tti;
	}

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

	BlockGSI create_header(AssFile const& copy, EbuExportSettings const& export_settings)
	{
		wxString scriptinfo_title = copy.GetScriptInfo("Title");
		wxString scriptinfo_translation = copy.GetScriptInfo("Original Translation");
		wxString scriptinfo_editing = copy.GetScriptInfo("Original Editing");

		agi::charset::IconvWrapper gsi_encoder(wxSTRING_ENCODING, "CP850");

		BlockGSI gsi;
		memset(&gsi, 0x20, sizeof(gsi)); // fill with spaces
		memcpy(gsi.cpn, "850", 3);
		switch (export_settings.tv_standard)
		{
			case EbuExportSettings::STL23:
			case EbuExportSettings::STL24:
				memcpy(gsi.dfc, "STL24.01", 8);
				break;
			case EbuExportSettings::STL29:
			case EbuExportSettings::STL29drop:
			case EbuExportSettings::STL30:
				memcpy(gsi.dfc, "STL30.01", 8);
				break;
			case EbuExportSettings::STL25:
			default:
				memcpy(gsi.dfc, "STL25.01", 8);
				break;
		}
		gsi.dsc = '0' + (int)export_settings.display_standard;
		gsi.cct[0] = '0';
		gsi.cct[1] = '0' + (int)export_settings.text_encoding;
		if (export_settings.text_encoding == EbuExportSettings::utf8)
			memcpy(gsi.cct, "U8", 2);
		memcpy(gsi.lc, "00", 2);
		gsi_encoder.Convert(wx_str(scriptinfo_title), buffer_size(scriptinfo_title), gsi.opt, 32);
		gsi_encoder.Convert(wx_str(scriptinfo_translation), buffer_size(scriptinfo_translation), gsi.tn, 32);
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
			memcpy(gsi.tng, "001", 3);
			snprintf(gsi.mnc, 2, "%02u", (unsigned int)export_settings.max_line_length);
			memcpy(gsi.mnr, "99", 2);
			gsi.tcs = '1';
			EbuTimecode tcofs = export_settings.timecode_offset;
			snprintf(gsi.tcp, 8, "%02u%02u%02u%02u", (unsigned int)tcofs.h, (unsigned int)tcofs.m, (unsigned int)tcofs.s, (unsigned int)tcofs.s);
		}
		gsi.tnd = '1';
		gsi.dsn = '1';
		memcpy(gsi.co, "NTZ", 3); // neutral zone!
		gsi_encoder.Convert(wx_str(scriptinfo_editing), buffer_size(scriptinfo_editing), gsi.en, 32);
		if (export_settings.text_encoding == EbuExportSettings::utf8)
			strncpy(gsi.uda, "This file was exported by Aegisub using non-standard UTF-8 encoding for the subtitle blocks. The TTI.TF field contains UTF-8-encoded text interspersed with the standard formatting codes, which are not encoded. GSI.CCT is set to 'U8' to signify this.", sizeof(gsi.uda));

		return gsi;
	}

	EbuExportSettings get_export_config(wxWindow *parent)
	{
		EbuExportSettings s("Subtitle Format/EBU STL");

		// Disable the busy cursor set by the exporter while the dialog is visible
		wxEndBusyCursor();
		int res = EbuExportConfigurationDialog(parent, s).ShowModal();
		wxBeginBusyCursor();

		if (res != wxID_OK)
			throw agi::UserCancelException("EBU/STL export");

		s.Save();
		return s;
	}

} // namespace {

Ebu3264SubtitleFormat::Ebu3264SubtitleFormat()
: SubtitleFormat("EBU subtitling data exchange format (EBU tech 3264, 1991)")
{
}

wxArrayString Ebu3264SubtitleFormat::GetWriteWildcards() const
{
	wxArrayString formats;
	formats.Add("stl");
	return formats;
}

void Ebu3264SubtitleFormat::WriteFile(const AssFile *src, wxString const& filename, wxString const& encoding) const
{
	// collect data from user
	EbuExportSettings export_settings = get_export_config(0);
	AssFile copy(*src);

	std::vector<EbuSubtitle> subs_list = convert_subtitles(copy, export_settings);
	std::vector<BlockTTI> tti = create_blocks(subs_list, export_settings);
	BlockGSI gsi = create_header(copy, export_settings);

	BlockTTI &block0 = tti.front();
	snprintf(gsi.tcf, 8, "%02u%02u%02u%02u", (unsigned int)block0.tci.h, (unsigned int)block0.tci.m, (unsigned int)block0.tci.s, (unsigned int)block0.tci.f);
	snprintf(gsi.tnb, 5, "%5u", (unsigned int)tti.size());
	snprintf(gsi.tns, 5, "%5u", (unsigned int)subs_list.size());

	// write file
	agi::io::Save f(from_wx(filename), true);
	f.Get().write((const char *)&gsi, sizeof(gsi));
	for (auto const& block : tti)
		f.Get().write((const char *)&block, sizeof(block));
}

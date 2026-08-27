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

#include "resolution_resampler.h"

#include "ass_dialogue.h"
#include "project_document.h"
#include "ass_style.h"
#include "utils.h"

#include <libaegisub/exception.h>
#include <libaegisub/of_type_adaptor.h>
#include <libaegisub/split.h>
#include <libaegisub/util.h>
#include <libaegisub/ycbcr_conv.h>

#include <algorithm>
#include <cmath>
#include <wx/intl.h>

enum {
	LEFT = 0,
	RIGHT = 1,
	TOP = 2,
	BOTTOM = 3
};

namespace {
	std::string transform_drawing(std::string const& drawing, int shift_x, int shift_y, double scale_x, double scale_y) {
		bool is_x = true;
		std::string final;
		final.reserve(drawing.size());

		for (auto cur : agi::Split(drawing, ' ')) {
			double val;
			if (agi::util::try_parse(cur, &val)) {
				if (is_x)
					val = (val + shift_x) * scale_x;
				else
					val = (val + shift_y) * scale_y;
				final += float_to_string(val);
				final += ' ';
				is_x = !is_x;
			}
			else if (cur.size() == 1) {
				char c = tolower(cur[0]);
				if (c == 'm' || c == 'n' || c == 'l' || c == 'b' || c == 's' || c == 'p' || c == 'c') {
					is_x = true;
					final += c;
					final += ' ';
				}
			}
		}

		if (final.size())
			final.pop_back();
		return final;
	}

	struct resample_state {
		const int *margin;
		double rx;
		double ry;
		double rm;
		double ar;
		std::optional<agi::ycbcr_converter> conv;
	};

	void resample_tags(std::string const&, AssOverrideParameter *cur, void *ud) {
		resample_state *state = static_cast<resample_state *>(ud);

		double resizer = 1.0;
		int shift = 0;

		switch (cur->classification) {
			case AssParameterClass::ABSOLUTE_SIZE_X:
				resizer = state->rx;
				break;

			case AssParameterClass::ABSOLUTE_SIZE_Y:
				resizer = state->ry;
				break;

			case AssParameterClass::ABSOLUTE_SIZE_XY:
				resizer = state->rm;
				break;

			case AssParameterClass::ABSOLUTE_POS_X:
				resizer = state->rx;
				shift = state->margin[LEFT];
				break;

			case AssParameterClass::ABSOLUTE_POS_Y:
				resizer = state->ry;
				shift = state->margin[TOP];
				break;

			case AssParameterClass::RELATIVE_SIZE_X:
				resizer = state->ar;
				break;

			case AssParameterClass::RELATIVE_SIZE_Y:
				break;

			case AssParameterClass::DRAWING: {
				cur->Set(transform_drawing(
					cur->Get<std::string>(),
					state->margin[LEFT], state->margin[TOP], state->rx, state->ry));
				return;
			}

			case AssParameterClass::COLOR:
				if (state->conv)
					cur->Set<std::string>(state->conv->rgb_to_rgb(agi::Color{cur->Get<std::string>()}).GetAssOverrideFormatted());
				return;

			default:
				return;
		}

		VariableDataType curType = cur->GetType();
		if (curType == VariableDataType::FLOAT)
			cur->Set((cur->Get<double>() + shift) * resizer);
		else if (curType == VariableDataType::INT)
			cur->Set<int>((cur->Get<int>() + shift) * resizer + 0.5);
	}

	void resample_line(resample_state *state, AssDialogue &diag) {
		if (diag.Comment && (diag.Effect.get().starts_with("template") || diag.Effect.get().starts_with("code")))
			return;

		auto blocks = diag.ParseTags();

		for (auto block : blocks | agi::of_type<AssDialogueBlockOverride>())
			block->ProcessParameters(resample_tags, state);

		for (auto drawing : blocks | agi::of_type<AssDialogueBlockDrawing>())
			drawing->text = transform_drawing(drawing->text, 0, 0, state->rx / state->ar, state->ry);

		for (size_t i = 0; i < 3; ++i) {
			if (diag.Margin[i])
				diag.Margin[i] = int((diag.Margin[i] + state->margin[i]) * (i < 2 ? state->rx : state->ry) + 0.5);
		}

		diag.UpdateText(blocks);
	}

	void resample_style(resample_state *state, AssStyle &style) {
		style.fontsize = int(style.fontsize * state->ry + 0.5);
		style.outline_w *= state->ry;
		style.shadow_w *= state->ry;
		style.spacing *= state->ry;  // gets multiplied by scalex (and hence by ar) during rendering
		style.scalex *= state->ar;
		for (int i = 0; i < 3; i++)
			style.Margin[i] = int((style.Margin[i] + state->margin[i]) * (i < 2 ? state->rx : state->ry) + 0.5);
		if (state->conv) {
			style.primary = state->conv->rgb_to_rgb(style.primary);
			style.secondary = state->conv->rgb_to_rgb(style.secondary);
			style.outline = state->conv->rgb_to_rgb(style.outline);
			style.shadow = state->conv->rgb_to_rgb(style.shadow);
		}
		style.UpdateData();
	}
}

void ResampleResolution(ProjectDocument *ass, ResampleSettings settings) {
	auto horizontal_stretch = 1.0;
	auto old_ar = double(settings.source_x) / settings.source_y;
	auto new_ar = double(settings.dest_x) / settings.dest_y;
	bool border_horizontally = new_ar > old_ar;
	// Don't convert aspect ratio if it's very close to correct
	// (for reference, 848x480 <-> 1280x720 is .006)
	if (std::abs(old_ar - new_ar) / new_ar > .01) {
		switch (settings.ar_mode) {
		case ResampleARMode::RemoveBorder:
			border_horizontally = !border_horizontally;
			[[fallthrough]];
		case ResampleARMode::AddBorder:
			if (border_horizontally) // Wider/Shorter
				settings.margin[LEFT] = settings.margin[RIGHT] = (settings.source_y * new_ar - settings.source_x) / 2;
			else // Taller/Narrower
				settings.margin[TOP] = settings.margin[BOTTOM] = (settings.source_x / new_ar - settings.source_y) / 2;
			break;
		case ResampleARMode::Stretch:
			horizontal_stretch = new_ar / old_ar;
			break;
		case ResampleARMode::Manual:
			old_ar =
				double(settings.source_x + settings.margin[LEFT] + settings.margin[RIGHT]) /
				double(settings.source_y + settings.margin[TOP] + settings.margin[BOTTOM]);

			if (std::abs(old_ar - new_ar) / new_ar > .01)
				horizontal_stretch = new_ar / old_ar;
			break;
		}
	}

	// Compute the updated LayoutRes. When cropping or stretching, LayoutRes needs to be cropped/stretched accordingly,
	// so that:
	// 1. The ratio of the LayoutRes AR to the PlayRes AR is not changed by the resampling process:
	//    This ratio controls how much text is stretched relative to drawings, so it must stay the same.
	//
	//    (Note that this is not technically required, see the TODO note below, but it's the logic that's consistent with
	//    the rest of the resolution resampler's current behavior.)
	// 2. LayoutResY stays unchanged when stretching the video, but is changed proportionally when adding or removing top or bottom margins.
	//    More precisely, the ratio between (newPlayResY / newLayoutResY) and (oldPlayResY / oldLayoutResY) should match
	//    the vertical stretching factor (newPlayResY / (oldPlayResY + marginTop + marginBottom)).
	//
	//    This is because vertical \blur and the perspective focal length are given in LayoutResY units
	//    (i.e. their values are converted to PlayRes units by multiplying them by PlayResY / LayoutResY), and
	//    the values in PlayRes units should grow proportionally with the other values.
	//
	// Adjusting the LayoutRes according to these constraints allow preserving rendering exactly when adding or removing borders,
	// or when stretching proportionally. However, when subtitles are stretched anamorphically, this is not possible:
	// There, horizontal blur and perspective transformations will change in appearance.
	//
	// TODO: In theory, it should be possible to exactly preserve rendering even when stretching anamorphically. This would involve:
	// - Scaling LayoutResX the same way LayoutResY is being scaled
	// - Leaving \fscx unchanged in exchange
	// - Scaling drawings horizontally by the stretching ratio
	// - Splitting all \bord and \shad tags and into \xbord\ybord / \xshad\yshad and scaling \xbord/\xshad accordingly
	//       (Note that this also requires injecting bord/shad tags into lines where the bord/shad is set via a style, as well as after \r)
	// However, this also has the downside of generating files whose LayoutRes AR does not match the PlayRes AR (i.e. sacrificing point 1. above),
	// which will result in unintuitive behavior and is probably badly supported in tooling.
	// Since there is arguably no more actual use case for stretching PlayRes anyway, this is not implemented for now.
	int lrx = 0, lry = 0;
	int new_lrx = 0, new_lry = 0;
	ass->GetLayoutResolution(lrx, lry);

	if (lrx != 0 && lry != 0) {
		new_lry = lry + std::round(lry * (settings.margin[TOP] + settings.margin[BOTTOM]) / double(settings.source_x));

		new_lrx = std::round(lrx * (double(new_lry) / double(lry)) * (double(settings.dest_x) / double(settings.dest_y)) / (double(settings.source_x) / double(settings.source_y)));
	}

	// Add margins to original resolution
	settings.source_x += settings.margin[LEFT] + settings.margin[RIGHT];
	settings.source_y += settings.margin[TOP] + settings.margin[BOTTOM];

	double rx = double(settings.dest_x) / double(settings.source_x);
	double ry = double(settings.dest_y) / double(settings.source_y);

	resample_state state = {
		settings.margin,
		rx,
		ry,
		rx == ry ? rx : std::sqrt(rx * ry),
		horizontal_stretch,
		settings.matrix_conversion && settings.matrix_conversion->first != settings.matrix_conversion->second ?	    // FIXME: Use transform once C++23 can be used
		std::make_optional(agi::ycbcr_converter{
			settings.matrix_conversion->first,
			settings.matrix_conversion->second,
		}) : std::nullopt,
	};

	for (auto& line : ass->Styles)
		resample_style(&state, line);
	for (auto& line : ass->Events)
		resample_line(&state, line);

	ass->SetScriptInfo("PlayResX", std::to_string(settings.dest_x));
	ass->SetScriptInfo("PlayResY", std::to_string(settings.dest_y));
	if (settings.matrix_conversion)
		ass->SetScriptInfo("YCbCr Matrix", agi::ycbcr::Header(settings.matrix_conversion->second).to_best_practice_string());

	if (lrx != 0 && lry != 0) {
		ass->SetScriptInfo("LayoutResX", std::to_string(new_lrx));
		ass->SetScriptInfo("LayoutResY", std::to_string(new_lry));
	}

	ass->Commit(_("resolution resampling"), ProjectDocument::COMMIT_SCRIPTINFO | ProjectDocument::COMMIT_DIAG_FULL);
}

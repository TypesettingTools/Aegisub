/*****************************************************************************
 * csri: common subtitle renderer interface
 *****************************************************************************
 * Copyright (C) 2007  David Lamparter
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 ****************************************************************************/

/** libass csri wrapper.
 * Indirectly based on code from aegisub,
 * (c) 2006-2007, Rodrigo Braz Monteiro, Evgeniy Stepanov
 */

#ifdef HAVE_CONFIG_H
#include "acconf.h"
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <ass/ass.h>

#ifdef _WIN32
# define CSRIAPI 	__declspec(dllexport)
#else
# ifdef HAVE_GCC_VISIBILITY
#  define CSRIAPI	__attribute__((visibility ("default")))
# else
#  define CSRIAPI
# endif
#endif

#define CSRI_OWN_HANDLES
typedef struct csri_libass_rend {
	ass_library_t* ass_library;
} csri_rend;

typedef struct csri_asa_inst {
	ass_renderer_t* ass_renderer;
	ass_track_t* ass_track;
} csri_inst;

#include <csri/csri.h>
#include <csri/stream.h>
#include <subhelp.h>

static struct csri_libass_rend csri_libass = { NULL };

csri_inst *csri_open_file(csri_rend *renderer,
	const char *filename, struct csri_openflag *flags)
{
	return subhelp_open_file(renderer, csri_open_mem, filename, flags);
}

csri_inst *csri_open_mem(csri_rend *renderer,
	const void *data, size_t length, struct csri_openflag *flags)
{
	csri_inst *rv;
	if (renderer != &csri_libass)
		return NULL;

	rv = (csri_inst *)malloc(sizeof(csri_inst));
	if (!rv)
		return NULL;

	rv->ass_renderer = ass_renderer_init(renderer->ass_library);
	if (!rv->ass_renderer) {
		free(rv);
		return NULL;
	}
	ass_set_font_scale(rv->ass_renderer, 1.);
	ass_set_fonts(rv->ass_renderer, NULL, "Sans");
	rv->ass_track = ass_read_memory(csri_libass.ass_library,
		(void *)data, length, "UTF-8");
	if (!rv->ass_track) {
		ass_renderer_done(rv->ass_renderer);
		free(rv);
		return NULL;
	}
	return rv;
}

void csri_close(csri_inst *inst)
{
	ass_free_track(inst->ass_track);
	ass_renderer_done(inst->ass_renderer);
	free(inst);
}

int csri_request_fmt(csri_inst *inst, const struct csri_fmt *fmt)
{
	if (!csri_is_rgb(fmt->pixfmt) || csri_has_alpha(fmt->pixfmt))
		return -1;
	ass_set_frame_size(inst->ass_renderer, fmt->width, fmt->height);
	return 0;
}

void csri_render(csri_inst *inst, struct csri_frame *frame, double time)
{
	ass_image_t *img = ass_render_frame(inst->ass_renderer,
		inst->ass_track, (int)(time * 1000), NULL);

	while (img) {
		unsigned bpp, alpha = 256 - (img->color && 0xFF);
		int src_d, dst_d;
		unsigned char *src, *dst, *endy, *endx;
		unsigned char c[3] = {
			(img->color >> 8) & 0xFF,	/* B */
			(img->color >> 16) & 0xFF,	/* G */
			img->color >> 24		/* R */
		};
		if ((frame->pixfmt | 1) == CSRI_F__RGB
			|| frame->pixfmt == CSRI_F_RGB) {
			unsigned char tmp = c[2];
			c[2] = c[0]; 
			c[0] = tmp;
		}
		bpp = frame->pixfmt >= 0x200 ? 3 : 4;

		dst = frame->planes[0]
			+ img->dst_y * frame->strides[0]
			+ img->dst_x * bpp;
		if (frame->pixfmt & 1)
			dst++;
		src = img->bitmap;

		src_d = img->stride - img->w;
		dst_d = frame->strides[0] - img->w * bpp;
		endy = src + img->h * img->stride;

		while (src != endy) {
			endx = src + img->w;
			while (src != endx) {
				/* src[x]: 0..255, alpha: 1..256 (see above)
				 * -> src[x]*alpha: 0<<8..255<<8
				 * -> need 1..256 for mult => +1
				 */
				unsigned s = ((*src++ * alpha) >> 8) + 1;
				unsigned d = 257 - s;
				/* c[0]: 0.255, s/d: 1..256 */
				dst[0] = (s*c[0] + d*dst[0]) >> 8;
				dst[1] = (s*c[1] + d*dst[1]) >> 8;
				dst[2] = (s*c[2] + d*dst[2]) >> 8;
				dst += bpp;
			}
			dst += dst_d;
			src += src_d;
		}
		img = img->next;
	}
}

static csri_inst *libass_init_stream(csri_rend *renderer,
	const void *header, size_t headerlen,
	struct csri_openflag *flags)
{
	csri_inst *rv;

	if (renderer != &csri_libass)
		return NULL;

	rv = (csri_inst *)malloc(sizeof(csri_inst));
	if (!rv)
		return NULL;

	rv->ass_renderer = ass_renderer_init(renderer->ass_library);
	if (!rv->ass_renderer) {
		free(rv);
		return NULL;
	}
	ass_set_font_scale(rv->ass_renderer, 1.);
	ass_set_fonts(rv->ass_renderer, NULL, "Sans");
	rv->ass_track = ass_new_track(csri_libass.ass_library);
	if (!rv->ass_track) {
		ass_renderer_done(rv->ass_renderer);
		free(rv);
		return NULL;
	}
	ass_process_codec_private(rv->ass_track, (void *)header, headerlen);
	return rv;
}

static void libass_push_packet(csri_inst *inst,
	const void *packet, size_t packetlen,
	double pts_start, double pts_end)
{
	ass_process_chunk(inst->ass_track, (void *)packet, packetlen,
		(int)(pts_start * 1000), (int)((pts_end - pts_start) * 1000));
}

static struct csri_stream_ext streamext = {
	libass_init_stream,
	libass_push_packet,
	NULL
};

void *csri_query_ext(csri_rend *rend, csri_ext_id extname)
{
	if (!rend)
		return NULL;
	if (!strcmp(extname, CSRI_EXT_STREAM_ASS))
		return &streamext;
	return NULL;
}

static struct csri_info csri_libass_info = {
	"libass",
	"0.9.x",
	"libass (the MPlayer SSA/ASS renderer, 0.9.x API)",
	"Evgeniy Stepanov",
	"Copyright (c) 2006, 2007 by Evgeniy Stepanov"
};

struct csri_info *csri_renderer_info(csri_rend *rend)
{
	return &csri_libass_info;
}

csri_rend *csri_renderer_byname(const char *name,
	const char *specific)
{
	if (strcmp(name, csri_libass_info.name))
		return NULL;
	if (specific && strcmp(specific, csri_libass_info.specific))
		return NULL;
	return &csri_libass;
}

csri_rend *csri_renderer_default()
{
	csri_libass.ass_library = ass_library_init();
	if (!csri_libass.ass_library)
		return NULL;

	ass_set_fonts_dir(csri_libass.ass_library, "");
	ass_set_extract_fonts(csri_libass.ass_library, 0);
	ass_set_style_overrides(csri_libass.ass_library, NULL);
	return &csri_libass;
}

csri_rend *csri_renderer_next(csri_rend *prev)
{
	return NULL;
}


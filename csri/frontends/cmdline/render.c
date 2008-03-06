/*****************************************************************************
 * asa: portable digital subtitle renderer
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

#ifdef HAVE_CONFIG_H
#include "acconf.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <csri/csri.h>
#include <csri/logging.h>

#include "render.h"

extern csri_rend *r;

#ifdef HAVE_LIBPNG
#include <png.h>

struct csri_frame *png_load(const char *filename,
	uint32_t *width, uint32_t *height, enum csri_pixfmt fmt)
{
	struct csri_frame *frame;
	int bit_depth, color_type;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *rows;
	unsigned char *imgdata;
	FILE *fp;

	if (!csri_is_rgb(fmt)) {
		fprintf(stderr, "PNG loader: can't load non-RGB.\n");
		return NULL;
	}

	frame = (struct csri_frame *)malloc(sizeof(struct csri_frame));
	if (!frame)
		return NULL;
	memset(frame, 0, sizeof(*frame));
	frame->pixfmt = fmt;

	fp = fopen(filename, "rb");
	if (!fp) {
		fprintf(stderr, "Error opening \"%s\": %s (%d)\n",
			filename, strerror(errno), errno);
		return NULL;
	}

	assert(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL));
	assert(info_ptr = png_create_info_struct(png_ptr));
	//keepalpha ? CSRI_F_RGBA : CSRI_F_RGB_;

	png_init_io(png_ptr, fp);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr,
		(png_uint_32 *)width, (png_uint_32 *)height,
		&bit_depth, &color_type, NULL, NULL, NULL);
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);
	if (bit_depth < 8)
		png_set_packing(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);
	if (bit_depth == 16)
		png_set_strip_16(png_ptr);

	if ((fmt < 0x200 && (fmt & 2)) || fmt == CSRI_F_BGR)
		png_set_bgr(png_ptr);
	if (csri_has_alpha(fmt)) {
		int before = fmt == CSRI_F_ARGB || fmt == CSRI_F_ABGR;
		if (color_type == PNG_COLOR_TYPE_RGB)
			png_set_filler(png_ptr, 0xff, before
				? PNG_FILLER_BEFORE : PNG_FILLER_AFTER);
		else {
			png_set_invert_alpha(png_ptr);
			if (before)
				png_set_swap_alpha(png_ptr);
		}
	} else {
		if (color_type & PNG_COLOR_MASK_ALPHA)
			png_set_strip_alpha(png_ptr);
		if (fmt != CSRI_F_RGB && fmt != CSRI_F_BGR) {
			int before = fmt == CSRI_F__RGB || fmt == CSRI_F__BGR;
			png_set_filler(png_ptr, 0xff, before
				? PNG_FILLER_BEFORE : PNG_FILLER_AFTER);
		}
	}

	rows = (png_bytep *)malloc(sizeof(png_bytep) * *height);
	assert(rows);
	imgdata = (unsigned char *)malloc(4 * *height * *width);
	assert(imgdata);
	for (uint32_t y = 0; y < *height; y++)
		rows[y] = imgdata + 4 * *width * y;
	png_read_image(png_ptr, rows);
	png_read_end(png_ptr, NULL);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	free(rows);

	frame->planes[0] = imgdata;
	frame->strides[0] = *width * 4;
	printf("\033[32;1mloaded %ux%u\033[m\n", *width, *height);
	return frame;
}

void png_store(struct csri_frame *frame, const char *filename,
	uint32_t width, uint32_t height)
{
	enum csri_pixfmt fmt = frame->pixfmt;
	int xforms = 0, before = 0, after = 0;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *rows;
	FILE *fp;

	fp = fopen(filename, "wb");
	if (!fp) {
		fprintf(stderr, "Error opening \"%s\": %s (%d)\n",
			filename, strerror(errno), errno);
		return;
	}

	rows = (png_bytep *)malloc(sizeof(png_bytep) * height);
	assert(rows);

	after = fmt == CSRI_F_RGB_ || fmt == CSRI_F_BGR_;
	before = fmt == CSRI_F__RGB || fmt == CSRI_F__BGR;
	for (uint32_t y = 0; y < height; y++) {
		rows[y] = frame->planes[0] + frame->strides[0] * y;
		if (before || after) {
			unsigned char *d = rows[y], *s = rows[y],
				*e = d + frame->strides[0];
			if (before)
				s++;
			while (d < e) {
				*d++ = *s++;
				*d++ = *s++;
				*d++ = *s++;
				s++;
			}
		}
	}

	assert(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL));
	assert(info_ptr = png_create_info_struct(png_ptr));
	png_init_io(png_ptr, fp);
	if (csri_has_alpha(fmt)
		&& (fmt == CSRI_F_ARGB || fmt == CSRI_F_ABGR))
		xforms |= PNG_TRANSFORM_SWAP_ALPHA;
	if ((fmt < 0x200 && (fmt & 2)) || fmt == CSRI_F_BGR)
		xforms |= PNG_TRANSFORM_BGR;
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, csri_has_alpha(fmt)
		? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_set_rows(png_ptr, info_ptr, rows);
	png_write_png(png_ptr, info_ptr, xforms, NULL);
	fflush(fp);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

#else
struct csri_frame *png_load(const char *filename,
 	uint32_t *width, uint32_t *height, enum csri_pixfmt fmt)
{
	fprintf(stderr, "PNG support not compiled in.\n");
	return NULL;
}

void png_store(struct csri_frame *frame, const char *filename,
 	uint32_t width, uint32_t height)
{
	fprintf(stderr, "PNG support not compiled in.\n");
	return;
}
#endif

struct csri_frame *frame_alloc(uint32_t width, uint32_t height,
	enum csri_pixfmt fmt)
{
	int bpp;
	unsigned char *d;
	struct csri_frame *frame;

	if (!csri_is_rgb(fmt))
		return NULL;

	bpp = fmt < CSRI_F_RGB ? 4 : 3;
	d = (unsigned char *)malloc(width * height * bpp);
	frame = (struct csri_frame *)malloc(sizeof(struct csri_frame));
	if (!frame || !d)
		return NULL;
	memset(frame, 0, sizeof(*frame));
	frame->pixfmt = fmt;
	frame->strides[0] = width * bpp;
	memset(d, csri_has_alpha(fmt) ? 0x00 : 0x80, width * height * bpp);
	frame->planes[0] = d;
	return frame;
}

void frame_free(struct csri_frame *frame)
{
	int c;
	for (c = 0; c < 4; c++)
		if (frame->planes[c])
			free(frame->planes[c]);

	free(frame);
}

void frame_copy(struct csri_frame *dst, struct csri_frame *src,
	uint32_t width, uint32_t height)
{
	memcpy(dst->planes[0], src->planes[0], height * src->strides[0]);
}

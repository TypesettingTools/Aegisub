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

#ifndef _RENDER_H
#define _RENDER_H

extern struct csri_frame *png_load(const char *filename,
 	uint32_t *width, uint32_t *height, enum csri_pixfmt fmt);
extern void png_store(struct csri_frame *frame, const char *filename,
 	uint32_t width, uint32_t height);
extern struct csri_frame *frame_alloc(uint32_t width, uint32_t height,
	enum csri_pixfmt fmt);
extern void frame_free(struct csri_frame *frame);
extern void frame_copy(struct csri_frame *dst, struct csri_frame *src,
	uint32_t width, uint32_t height);

#endif

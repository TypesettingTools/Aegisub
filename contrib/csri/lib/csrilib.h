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

#ifndef _CSRILIB_H
#define _CSRILIB_H

#ifdef HAVE_CONFIG_H
#include "acconf.h"
#endif

#include "visibility.h"

#define CSRIAPI export

#include <csri/csri.h>
#include <csri/stream.h>
#include "csrilib_os.h"

struct csri_wrap_rend {
	struct csri_wrap_rend *next;

	csri_rend *rend;
	csri_inst *(*open_file)(csri_rend *renderer,
		const char *filename, struct csri_openflag *flags);
	csri_inst *(*open_mem)(csri_rend *renderer,
		const void *data, size_t length,
		struct csri_openflag *flags);
	void (*close)(csri_inst *inst);
	int (*request_fmt)(csri_inst *inst, const struct csri_fmt *fmt);
	void (*render)(csri_inst *inst, struct csri_frame *frame,
		double time);
	void *(*query_ext)(csri_rend *rend, csri_ext_id extname);
	struct csri_stream_ext stream_ass, stream_text;
	csri_inst *(*init_stream_ass)(csri_rend *renderer,
		const void *header, size_t headerlen,
		struct csri_openflag *flags);
	csri_inst *(*init_stream_text)(csri_rend *renderer,
		const void *header, size_t headerlen,
		struct csri_openflag *flags);

	struct csri_info *info;
	struct csrilib_os os;
};

struct csri_wrap_inst {
	struct csri_wrap_inst *next;

	csri_inst *inst;
	struct csri_wrap_rend *wrend;
	void (*close)(csri_inst *inst);
	int (*request_fmt)(csri_inst *inst, const struct csri_fmt *fmt);
	void (*render)(csri_inst *inst, struct csri_frame *frame,
		double time);
};

extern struct csri_wrap_rend *wraprends;

extern struct csri_wrap_rend *csrilib_rend_lookup(csri_rend *rend);
extern void csrilib_rend_initadd(struct csri_wrap_rend *wrend);

extern struct csri_wrap_inst *csrilib_inst_lookup(csri_inst *inst);
extern csri_inst *csrilib_inst_initadd(struct csri_wrap_rend *wrend,
	csri_inst *inst);
extern void csrilib_inst_remove(struct csri_wrap_inst *winst);

extern void csrilib_os_init();

#endif /*_CSRILIB_H */

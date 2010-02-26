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

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "csrilib.h"

struct csri_wrap_rend *wraprends = NULL;

struct csri_wrap_rend *csrilib_rend_lookup(csri_rend *rend)
{
	struct csri_wrap_rend *wrap = wraprends;
	for (; wrap; wrap = wrap->next)
		if (wrap->rend == rend)
			return wrap;
	return NULL;
}

#define INSTHASHSZ 256
#define HASH(x) (((intptr_t)(x) & 0xff00) >> 8)
static struct csri_wrap_inst *wrapinsts[INSTHASHSZ];

struct csri_wrap_inst *csrilib_inst_lookup(csri_inst *inst)
{
	struct csri_wrap_inst *ent = wrapinsts[HASH(inst)];
	while (ent && ent->inst != inst)
		ent = ent->next;
	return ent;
}

csri_inst *csrilib_inst_initadd(struct csri_wrap_rend *wrend,
	csri_inst *inst)
{
	struct csri_wrap_inst *winst = (struct csri_wrap_inst *)
		malloc(sizeof(struct csri_wrap_inst)),
		**pnext;
	if (!winst) {
		wrend->close(inst);
		return NULL;
	}
	winst->wrend = wrend;
	winst->inst = inst;
	winst->close = wrend->close;
	winst->request_fmt = wrend->request_fmt;
	winst->render = wrend->render;
	winst->next = NULL;
	pnext = &wrapinsts[HASH(inst)];
	while (*pnext)
		pnext = &(*pnext)->next;
	*pnext = winst;
	return inst;
}

void csrilib_inst_remove(struct csri_wrap_inst *winst)
{
	struct csri_wrap_inst **pnext = &wrapinsts[HASH(winst->inst)];
	while (*pnext && *pnext != winst)
		pnext = &(*pnext)->next;
	if (!*pnext)
		return;
	*pnext = (*pnext)->next;
	free(winst);
}

void csrilib_rend_initadd(struct csri_wrap_rend *wrend)
{
	wrend->next = wraprends;
	wraprends = wrend;
}

static int initialized = 0;

csri_rend *csri_renderer_default()
{
	if (!initialized) {
		csrilib_os_init();
		initialized = 1;
	}
	if (!wraprends)
		return NULL;
	return wraprends->rend;
}

csri_rend *csri_renderer_next(csri_rend *prev)
{
	struct csri_wrap_rend *wrend = csrilib_rend_lookup(prev);
	if (!wrend || !wrend->next)
		return NULL;
	return wrend->next->rend;
}

csri_rend *csri_renderer_byname(const char *name, const char *specific)
{
	struct csri_wrap_rend *wrend;
	if (!initialized) {
		csrilib_os_init();
		initialized = 1;
	}
	if (!name)
		return NULL;
	for (wrend = wraprends; wrend; wrend = wrend->next) {
		if (strcmp(wrend->info->name, name))
			continue;
		if (specific && strcmp(wrend->info->specific, specific))
			continue;
		return wrend->rend;
	}
	return NULL;
}

csri_rend *csri_renderer_byext(unsigned n_ext, csri_ext_id *ext)
{
	struct csri_wrap_rend *wrend;
	unsigned i;
	if (!initialized) {
		csrilib_os_init();
		initialized = 1;
	}
	for (wrend = wraprends; wrend; wrend = wrend->next) {
		for (i = 0; i < n_ext; i++) {
			if (!wrend->query_ext(wrend->rend, ext[i]))
				break;
		}
		if (i == n_ext)
			return wrend->rend;
	}
	return NULL;
}


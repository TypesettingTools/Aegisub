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

#define _POSIX_C_SOURCE 200112L		/* for PATH_MAX */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <dlfcn.h>

#include "../csrilib.h"
#include "subhelp.h"

static const char csri_path[] = CSRI_PATH;

static void csrilib_enum_dir(const char *dir);

static void csrilib_add(csri_rend *rend,
	const struct csri_wrap_rend *tmp, struct csri_info *info)
{
	struct csri_wrap_rend *wrend = (struct csri_wrap_rend *)
		malloc(sizeof(struct csri_wrap_rend));
	if (!wrend)
		return;
	memcpy(wrend, tmp, sizeof(struct csri_wrap_rend));
	wrend->rend = rend;
	wrend->info = info;
	csrilib_rend_initadd(wrend);
}

static void csrilib_do_load(const char *filename, dev_t device, ino_t inode)
{
	void *dlhandle = dlopen(filename, RTLD_NOW);
	struct csri_wrap_rend tmp;
	csri_rend *rend;
	struct csri_info *(*renderer_info)(csri_rend *rend);
	csri_rend *(*renderer_default)();
	csri_rend *(*renderer_next)(csri_rend *prev);
	const char *sym;


	if (!dlhandle) {
		subhelp_log(CSRI_LOG_WARNING, "dlopen(\"%s\") says: %s",
			filename, dlerror());
		return;
	}
	if (dlsym(dlhandle, "csri_library")) {
		subhelp_log(CSRI_LOG_WARNING, "ignoring library %s",
			filename);
		return;
	}
	subhelp_log(CSRI_LOG_INFO, "loading %s", filename);

	tmp.os.dlhandle = dlhandle;
	tmp.os.device = device;
	tmp.os.inode = inode;

/* okay, this is uber-ugly. either I end up casting from void *
 * to a fptr (which yields a cast warning), or I do a *(void **)&tmp.x
 * (which yields a strict-aliasing warning).
 * casting via char* works because char* can alias anything.
 */
#define _dl_map_function(x, dst) do { \
	char *t1 = (char *)&dst; \
	union x { void *ptr; } *ptr = (union x *)t1; \
	sym = "csri_" # x; \
	ptr->ptr = dlsym(dlhandle, sym);\
	if (!ptr->ptr) goto out_dlfail; } while (0)
#define dl_map_function(x) _dl_map_function(x, tmp.x)
	dl_map_function(query_ext);
	subhelp_logging_pass((struct csri_logging_ext *)
		tmp.query_ext(NULL, CSRI_EXT_LOGGING));
	dl_map_function(open_file);
	dl_map_function(open_mem);
	dl_map_function(close);
	dl_map_function(request_fmt);
	dl_map_function(render);
#define dl_map_local(x) _dl_map_function(x, x)
	dl_map_local(renderer_info);
	dl_map_local(renderer_default);
	dl_map_local(renderer_next);

	rend = renderer_default();
	while (rend) {
		csrilib_add(rend, &tmp, renderer_info(rend));
		rend = renderer_next(rend);
	}
	return;

out_dlfail:
	subhelp_log(CSRI_LOG_WARNING, "%s: symbol %s not found (%s)",
		filename, sym, dlerror());
	dlclose(dlhandle);
}

static void csrilib_load(const char *filename)
{
	struct csri_wrap_rend *rend;
	struct stat st;
	if (stat(filename, &st))
		return;

	if (S_ISDIR(st.st_mode)) {
		csrilib_enum_dir(filename);
		return;
	}
	if (!S_ISREG(st.st_mode))
		return;
	if (access(filename, X_OK))
		return;

	for (rend = wraprends; rend; rend = rend->next)
		if (rend->os.device == st.st_dev
			&& rend->os.inode == st.st_ino)
			return;

	csrilib_do_load(filename, st.st_dev, st.st_ino);
}

static void csrilib_enum_dir(const char *dir)
{
	DIR *dirh;
	struct dirent *e;
	char buf[PATH_MAX];

	dirh = opendir(dir);
	if (!dirh) {
		subhelp_log(CSRI_LOG_WARNING, "ignoring directory \"%s\":"
			" %s (%d)", dir, strerror(errno), errno);
		return;
	}

	subhelp_log(CSRI_LOG_INFO, "scanning directory \"%s\"", dir);
	while ((e = readdir(dirh))) {
		if (e->d_name[0] == '.')
			continue;
		snprintf(buf, sizeof(buf), "%s/%s", dir, e->d_name);
		csrilib_load(buf);
	}
		
	closedir(dirh);
}

static void csrilib_expand_enum_dir(const char *dir)
{
	if (dir[0] == '~' && (dir[1] == '\0' || dir[1] == '/')) {
		char buf[PATH_MAX], *home = getenv("HOME");
		if (!home)
			home = "";
		snprintf(buf, sizeof(buf), "%s%s", home, dir + 1);
		csrilib_enum_dir(buf);
	} else
		csrilib_enum_dir(dir);
}

void csrilib_os_init()
{
	char buf[4096];
	char *envpath = getenv("CSRI_PATH");
	char *pos, *next = buf;

	if (envpath)
		snprintf(buf, sizeof(buf), "%s:%s", csri_path, envpath);
	else {
		strncpy(buf, csri_path, sizeof(buf));
		buf[sizeof(buf) - 1] = '\0';
	}
	do {
		pos = next;
		next = strchr(pos, ':');
		if (next)
			*next++ = '\0';
		csrilib_expand_enum_dir(pos);
	} while (next);
}


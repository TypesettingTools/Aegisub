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

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../csrilib.h"
#include "subhelp.h"

static void csrilib_enum_dir(const wchar_t *dir);

static const char *get_errstr()
{
	static char msg[2048];
	DWORD err = GetLastError();
	
	if (!FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, 0, msg, sizeof(msg), NULL))
		strcpy(msg, "Unknown Error");
	else {
		int msglen = strlen(msg) - 1;
		if (msg[msglen] == '\n')
			msg[msglen] = '\0';
	}
	return msg;
}

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

static void csrilib_do_load(const wchar_t *filename)
{
	HMODULE dlhandle = LoadLibraryExW(filename, NULL,
		LOAD_WITH_ALTERED_SEARCH_PATH);
	struct csri_wrap_rend tmp;
	csri_rend *rend;
	struct csri_info *(*renderer_info)(csri_rend *rend);
	csri_rend *(*renderer_default)();
	csri_rend *(*renderer_next)(csri_rend *prev);
	const char *sym;

	if (!dlhandle) {
		subhelp_log(CSRI_LOG_WARNING, "LoadLibraryEx(\"%ls\") failed: "
			"%s", filename, get_errstr());
		return;
	}
	if (GetProcAddress(dlhandle, "csri_library")) {
		subhelp_log(CSRI_LOG_WARNING, "ignoring library %ls",
			filename);
		goto out_freelib;
	}
	subhelp_log(CSRI_LOG_INFO, "loading %ls", filename);

	tmp.os.dlhandle = dlhandle;

/* okay, this is uber-ugly. either I end up casting from void *
 * to a fptr (which yields a cast warning), or I do a *(void **)&tmp.x
 * (which yields a strict-aliasing warning).
 * casting via char* works because char* can alias anything.
 */
#define _dl_map_function(x, dst) do { \
	char *t1 = (char *)&dst; \
	union x { FARPROC ptr; } *ptr = (union x *)t1; \
	sym = "csri_" # x; \
	ptr->ptr = GetProcAddress(dlhandle, sym);\
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
	subhelp_log(CSRI_LOG_WARNING, "%ls: symbol %s not found (%s)",
		filename, sym, get_errstr());
out_freelib:
	FreeLibrary(dlhandle);
}

static void csrilib_load(const wchar_t *filename)
{
	DWORD attr = GetFileAttributesW(filename);
	if (attr == INVALID_FILE_ATTRIBUTES)
		return;

	if (attr & FILE_ATTRIBUTE_DIRECTORY) {
		csrilib_enum_dir(filename);
		return;
	}
	csrilib_do_load(filename);
}

static void csrilib_enum_dir(const wchar_t *dir)
{
	WIN32_FIND_DATAW data;
	HANDLE res;
	wchar_t buf[MAX_PATH];

	_snwprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%ls\\*", dir);
	res = FindFirstFileW(buf, &data);
	if (res == INVALID_HANDLE_VALUE) {
		subhelp_log(CSRI_LOG_WARNING, "ignoring directory \"%ls\": %s",
			dir, get_errstr());
		return;
	}

	subhelp_log(CSRI_LOG_INFO, "scanning directory \"%ls\"", dir);
	do {
		if (data.cFileName[0] == '.')
			continue;
		_snwprintf(buf, sizeof(buf) / sizeof(buf[0]),
			L"%ls\\%ls", dir, data.cFileName);
		csrilib_load(buf);
	} while (FindNextFileW(res, &data));
	FindClose(res);
}

void csrilib_os_init()
{
	wchar_t filename[MAX_PATH], *slash;
	DWORD rv = GetModuleFileNameW(NULL, filename, MAX_PATH);
	if (!rv)
		*filename = L'\0';
	slash = wcsrchr(filename, L'\\');
	slash = slash ? slash + 1 : filename;
	*slash = L'\0';
	wcsncpy(slash, L"csri", filename + MAX_PATH - slash);
	csrilib_enum_dir(filename);
}


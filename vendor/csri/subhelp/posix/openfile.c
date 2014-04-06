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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "subhelp.h"
#include <csri/openerr.h>

csri_inst *subhelp_open_file(csri_rend *renderer,
	csri_inst *(*memopenfunc)(csri_rend *renderer, const void *data,
		size_t length, struct csri_openflag *flags),
	const char *filename, struct csri_openflag *flags)
{
	csri_inst *rv = NULL;
	void *data;
	int fd;
	struct stat st;
	struct csri_openflag *err;

	fd = open(filename, O_RDONLY);
	if (fd == -1)
		goto out_err;
	if (fstat(fd, &st)
		|| !(data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE,
			fd, 0))) {
		close(fd);
		goto out_err;
	}

	rv = memopenfunc(renderer, data, st.st_size, flags);

	munmap(data, st.st_size);
	close(fd);
	return rv;

out_err:
	for (err = flags; err; err = err->next)
		if (!strcmp(err->name, CSRI_EXT_OPENERR))
			break;
	if (err) {
		struct csri_openerr_flag *errd = (struct csri_openerr_flag *)
			err->data.otherval;
		errd->flags = CSRI_OPENERR_FILLED | CSRI_OPENERR_ERRNO;
		errd->posixerrno = errno;
	}
	return NULL;
}


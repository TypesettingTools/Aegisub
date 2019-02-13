/*****************************************************************************
 * csri: common subtitle renderer interface
 *****************************************************************************
 * Copyright (C) 2007  David Lamparter
 * All rights reserved.
 * 	
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 ****************************************************************************/

/** \file openerr.h - csri.openerr - error return extension.
 * $Id$ */

#ifndef _CSRI_OPENERR_H
/** \cond */
#define _CSRI_OPENERR_H 20070119
/** \endcond */

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup openerr csri.openerr extension. */
/*@{*/

/** extension identifier */
#define CSRI_EXT_OPENERR (csri_ext_id)"csri.openerr"

/** flag field describing which fields of #csri_openerr_flag are valid */
enum csri_openerr_flags {
	/** support indicator.
	 * set if the structure was filled with any meaningful data
	 */
	CSRI_OPENERR_FILLED = (1 << 0),
	/** csri_openerr_flag.posixerrno valid */
	CSRI_OPENERR_ERRNO = (1 << 1),
	/** csri_openerr_flag.winerr valid */
	CSRI_OPENERR_WINERR = (1 << 2),
	/** csri_openerr_flag.custerr valid */
	CSRI_OPENERR_CUSTERR = (1 << 3),
	/** csri_openerr_flag.warncount valid */
	CSRI_OPENERR_WARNCOUNT = (1 << 4)
};

/** returned error information.
 * to be passed via csri_vardata.otherval as an openflag with
 * extension ID #CSRI_EXT_OPENERR.
 *
 * Memory management by caller.
 *
 * The three error fields should only be filled when csri_open_file()
 * / csri_open_mem() returned NULL. The warning counter can indicate
 * information on successfully loaded scripts.
 */
struct csri_openerr_flag {
	/** bitfield of valid information */
	enum csri_openerr_flags flags;
	/** posix errno value indicating the error occured */
	int posixerrno;
	/** Windows GetLastError value */
	unsigned winerr;
	/** renderer-specific custom error value.
	 * should be string-lookupable via csri_openerr_ext.strerror
	 * (csri_query_ext())
	 */
	union csri_vardata custerr;
	/** warning count.
	 * The number of (renderer-specific) warnings that occured
	 * during loading the script. The content of these warnings
	 * should be retrievable via a renderer-specific extension.
	 */
	unsigned warncount;
};

/** openerr extension information structure */
struct csri_openerr_ext {
	/** csri_openerr_flag.custerr to string lookup.
	 * \param renderer the renderer handle.
	 * \param custerr renderer-specific error
	 * \param buffer buffer to fill with an UTF-8 error message.
	 *   may be NULL; in that case only size is filled in
	 * \param size buffer size.\n
	 *   in: maximum bytes to write to  buffer, in bytes,
	 *   including terminating \\0.\nthe renderer MUST always
	 *   zero-terminate this, even when the space is not sufficient
	 *   \n\n
	 *   out: number of bytes (including terminating \\0) needed
	 *   to return the full error message
	 *
	 * This function pointer may be NULL if the renderer does not
	 * return custom error codes. #CSRI_OPENERR_CUSTERR must not be
	 * used in that case.
	 */
	void (*strerror)(csri_rend *renderer, union csri_vardata custerr,
		char *buffer, size_t *size);
};

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _CSRI_OPENERR_H */

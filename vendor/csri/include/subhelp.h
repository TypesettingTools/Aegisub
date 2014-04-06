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

/** \file subhelp.h - subtitle helper API.
 * $Id$ */

#ifndef _SUBHELP_H
#define _SUBHELP_H

#include <stdarg.h>
#include <csri/csri.h>
#include <csri/logging.h>

/** \defgroup subhelp subtitle filter helper API. */
/*@{*/

/** file opening wrapper.
 * can be used to implement csri_open_file() by using csri_open_mem().
 * \param renderer the renderer handle
 * \param memopenfunc function pointer to a csri_open_mem() implementation
 * \param filename name of file to open
 * \param flags pointer #csri_openflag.\n
 *   subhelp_open_file will fill in csri.openerr if present
 * \return return value from memopenfunc or NULL on fs error
 */
extern csri_inst *subhelp_open_file(csri_rend *renderer,
	csri_inst *(*memopenfunc)(csri_rend *renderer, const void *data,
		size_t length, struct csri_openflag *flags),
	const char *filename, struct csri_openflag *flags);


/** logging extension query function.
 * call from csri_query_ext BEFORE checking whether the renderer.
 * \code
 *   void *csri_query_ext(csri_rend *rend, csri_ext_id extname) {
 *     void *rv;
 *     if ((rv = subhelp_query_ext_logging(extname)))
 *       return rv;
 *     if (!rend)
 *       return NULL;
 *     ...
 * \endcode
 * \param extname the extension name. compared to "csri.logging" by
 *   this function.
 * \return logging extension pointer, if the extension name matched.
 *   NULL otherwise.
 */
extern void *subhelp_query_ext_logging(csri_ext_id extname);

/** configure other renderer with our settings.
 * \param logext csri.logging from configuree.
 */
extern void subhelp_logging_pass(struct csri_logging_ext *logext);

/** logging function.
 * \param severity severity of this message, as defined by csri.logging
 * \param msg log message, one line, without \\n at the end.
 */
extern void subhelp_log(enum csri_logging_severity severity,
	const char *msg, ...)
#ifdef __GNUC__
	__attribute__((format(printf, 2, 3)))
#endif
	;

/** logging function, varargs version.
 * \param severity severity of this message, as defined by csri.logging
 * \param msg log message, one line, without \\n at the end.
 * \param args argument list
 */
extern void subhelp_vlog(enum csri_logging_severity severity,
	const char *msg, va_list args)
#ifdef __GNUC__
	__attribute__((format(printf, 2, 0)))
#endif
	;

/** logging function, fixed string version.
 * \param severity severity of this message, as defined by csri.logging
 * \param msg log message, one line, without \\n at the end.
 */
extern void subhelp_slog(enum csri_logging_severity severity, const char *msg);

/*@}*/

#endif /* _SUBHELP_H */


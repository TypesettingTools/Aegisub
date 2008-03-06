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

/** \file logging.h - csri.logging - logging for renderers via callback.
 * $Id$
 *
 * <b>THE SPECIFICATION OF THIS EXTENSION IS TENTATIVE
 * AND NOT FINALIZED YET</b>
 */

#ifndef _CSRI_LOGGING_H
/** \cond */
#define _CSRI_LOGGING_H 20070119
/** \endcond */

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup logging csri.logging extension.
 * <b>THE SPECIFICATION OF THIS EXTENSION IS TENTATIVE
 * AND NOT FINALIZED YET</b>
 */
/*@{*/

/** extension identifier */
#define CSRI_EXT_LOGGING (csri_ext_id)"csri.logging"

/** -. TODO: add scope? */
enum csri_logging_severity {
	CSRI_LOG_DEBUG = 0,
	CSRI_LOG_INFO,
	CSRI_LOG_NOTICE,
	CSRI_LOG_WARNING,
	CSRI_LOG_ERROR
};

typedef void csri_logging_func(void *appdata,
		enum csri_logging_severity sev,
		const char *message);

struct csri_logging_ext {
	void (*set_logcallback)(csri_logging_func *func, void *appdata);
};

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _CSRI_LOGGING_H */

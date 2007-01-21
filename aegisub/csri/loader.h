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

/** \file loader.h - CSRI helper library functions.
 * $Id: loader.h 3 2007-01-19 12:04:43Z equinox $ */

#ifndef _CSRI_HELPER_H
/** \cond */
#define _CSRI_HELPER_H 20070119
/** \endcond */


#if _CSRI_HELPER_H != _CSRI_H
#error CSRI helper API header doesn't match CSRI header
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup help CSRI helper/loader API.
 *
 * These functions locate renderers based on given parameters.
 *
 * <b>Renderers must implement these functions as well.</b>
 *
 * They are used by the library to grab renderer information
 * from a shared object; and also this way a single renderer
 * can be linked directly into an appliaction.
 */
/*@{*/

/** try to load a given renderer
 * \param name the name of the renderer, as in csri_info.name
 * \param specific the specific version of the renderer,
 *   as in csri_info.specific;\n
 *   alternatively NULL if any version of the renderer is ok.
 * \return a handle to the renderer if it was successfully loaded,
 *   NULL otherwise.
 */
CSRIAPI csri_rend *csri_renderer_byname(const char *name,
	const char *specific);

/** get the default (highest priority) renderer
 * \return a handle to the default renderer, or NULL if
 *   no renderer is installed.
 * 
 * Together with csri_renderer_next(), this can be used
 * to enumerate all installed renderers.
 */
CSRIAPI csri_rend *csri_renderer_default();

/** get the next lower priority renderer
 * \param prev the current renderer
 * \return the renderer with the next lower priority than
 *   the one named in prev, or NULL if prev is the last
 *   renderer installed.
 */
CSRIAPI csri_rend *csri_renderer_next(csri_rend *prev);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _CSRI_HELPER_H */


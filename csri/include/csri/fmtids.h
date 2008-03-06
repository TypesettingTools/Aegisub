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

/** \file fmtids.h - csri.format - extension identifiers for subtitle formats.
 * $Id$ */

#ifndef _CSRI_FMTIDS_H
/** \cond */
#define _CSRI_FMTIDS_H 20070119
/** \endcond */

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup fmtids csri.format identifiers.
 * only includes the most important ones for now, more to be added. */
/*@{*/

/** SSA / ASS. Sub Station Alpha - versions 2, 3, 4, 4+ and 4++ */
#define CSRI_EXT_FMT_SSA	(csri_ext_id)"csri.format.ssa"
/** SRT. SubRip Text, SubRip Titles or something similar. */
#define CSRI_EXT_FMT_SRT	(csri_ext_id)"csri.format.srt"
/** MicroDVD */
#define CSRI_EXT_FMT_MICRODVD	(csri_ext_id)"csri.format.microdvd"
/** SAMI. Microsoft Synchronized Accessible Media Interchange */
#define CSRI_EXT_FMT_SAMI	(csri_ext_id)"csri.format.sami"
/** SMIL. W3C Synchronized Multimedia Integration Language.
 * NB: this format uses separate files for text streams */
#define CSRI_EXT_FMT_SMIL	(csri_ext_id)"csri.format.smil"

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _CSRI_OPENERR_H */

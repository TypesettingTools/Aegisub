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

/** \file stream.h - subtitle streaming (MKV & co).
 * $Id$
 *
 * <b>THE SPECIFICATION OF THIS EXTENSION IS TENTATIVE
 * AND NOT FINALIZED YET</b>
 */

#ifndef _CSRI_STREAM_H
/** \cond */
#define _CSRI_STREAM_H 20070119
/** \endcond */

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup stream csri.stream.* extensions. */
/*@{*/

/** stream extension group.
 * note: you cannot query for csri.stream, you need
 * to query for one of the streaming formats instead,
 * which will return a csri_stream_ext pointer.
 */
#define CSRI_EXT_STREAM (csri_ext_id)"csri.stream"

/** Matroska-style ASS streaming.
 * header contains standard SSA stuff, packet contains
 * ReadOrder,Layer,Style,Speaker,MarginL,R,V,Effect,Text
 */
#define CSRI_EXT_STREAM_ASS CSRI_EXT_STREAM ".ass"
/** Simple text + timestamp streams */
#define CSRI_EXT_STREAM_TEXT CSRI_EXT_STREAM ".text"
/* missing: USF, MPEG-4 TT */

/** stream extension information structure */
struct csri_stream_ext {
	/** create streaming renderer instance.
	 * \param renderer the renderer handle.
	 * \param header codec-private stream header.
	 * \param headerlen byte length of header data.
	 * \param flags openflags.
	 *
	 * not NULL if this extension is supported.
	 * may take various flags like csri_openerr_flag.
	 *
	 * the returned instance can be used with
	 * csri_request_fmt, csri_render and csri_close.
	 */
	csri_inst *(*init_stream)(csri_rend *renderer,
		const void *header, size_t headerlen,
		struct csri_openflag *flags);

	/** process a streamed packet.
	 * \param inst instance created with init_stream.
	 * \param packet stream packet data.
	 * \param packetlen byte length of packet.
	 * \param pts_start start timestamp from container.
	 * \param pts_end end timestamp from container.
	 *
	 * add a single packet to the renderer instance.
	 */
	void (*push_packet)(csri_inst *inst,
		const void *packet, size_t packetlen,
		double pts_start, double pts_end);

	/** discard processed packets.
	 * \param inst instance created with init_stream.
	 * \param all discard possibly-active packets too.\n
	 *   a possibly-active packet is a packet which
	 *   has not seen a csri_render call with a pts
	 *   beyond its end timestamp yet.
	 *
	 * frees up memory, or can force a clean renderer.
	 * may be NULL if unsupported, check before calling!
	 */
	void (*discard)(csri_inst *inst, int all);
};

/** streaming openflag ext for controlling subtitle lifetime */
#define CSRI_EXT_STREAM_DISCARD CSRI_EXT_STREAM ".discard"

/** subtitle packet lifetime */
enum csri_stream_discard {
	/** lifetime: timestamp expiry.
	 * delete packets from csri_render if the current
	 * timestamp is beyond the packet's end timestamp.
	 * this should be the default
	 */
	CSRI_STREAM_DISCARD_TSEXPIRE = 0,
	/** lifetime: discard immediately.
	 * discard all packets on returning from csri_render.
	 */
	CSRI_STREAM_DISCARD_IMMEDIATELY,
	/** lifetime: discard explicitly.
	 * never discard packets, use csri_stream_ext.discard
	 */
	CSRI_STREAM_DISCARD_EXPLICIT
};

/** openflag for csri_stream_ext.init_stream */
struct csri_stream_discard_flag {
	/** the lifetime to be used for subtitle packets */
	enum csri_stream_discard lifetime;
};

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _CSRI_STREAM_H */

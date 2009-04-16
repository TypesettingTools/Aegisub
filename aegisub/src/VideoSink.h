/*
 * Copyright (c) 2004-2006 Mike Matsnev.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice immediately at the beginning of the file, without modification,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Absolutely no warranty of function or purpose is made by the author
 *    Mike Matsnev.
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
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * $Id: VideoSink.h,v 1.5 2007/01/17 23:40:51 mike Exp $
 * 
 */

#ifndef VIDEOSINK_H
#define	VIDEOSINK_H

// callback, invoked when a new frame is ready
[uuid("8D9F1DA8-10DB-42fe-8CAC-94A02497B3DD")]
interface IVideoSinkNotify : public IUnknown {
  // this may be called on a worker thread!
  STDMETHOD(FrameReady)() = 0;
};

// supported bitmap types
#define	IVS_RGB24 1
#define	IVS_RGB32 2
#define	IVS_YUY2  4
#define	IVS_YV12  8

typedef void  (*ReadFrameFunc)(__int64 timestamp, unsigned format, unsigned bpp,
			       const unsigned char *frame, unsigned width, unsigned height, int stride,
			       unsigned arx, unsigned ary,
			       void *arg);

[uuid("6B9EFC3E-3841-42ca-ABE5-0F963C638249")]
interface IVideoSink : public IUnknown {
  STDMETHOD(SetAllowedTypes)(unsigned types) = 0;
  STDMETHOD(GetAllowedTypes)(unsigned *types) = 0;

  STDMETHOD(NotifyFrame)(IVideoSinkNotify *notify) = 0;

  // failure return means format is not negotiated yet
  STDMETHOD(GetFrameFormat)(unsigned *type, unsigned *width, unsigned *height, unsigned *arx, unsigned *ary) = 0;

  // S_FALSE return means end of file was reached
  STDMETHOD(ReadFrame)(ReadFrameFunc f, void *arg) = 0;
};

[uuid("80CADA0E-DFA5-4fcc-99DD-52F7C1B0E575")]
interface IVideoSink2 : public IUnknown {
  STDMETHOD(NotifyFrame)(HANDLE hEvent) = 0;
  STDMETHOD(GetFrameFormat)(unsigned *type, unsigned *width, unsigned *height, unsigned *arx, unsigned *ary, __int64 *def_duration) = 0;
};

HRESULT CreateVideoSink(IBaseFilter **pVS);


#endif
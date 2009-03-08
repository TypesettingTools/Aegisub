/*
 * Copyright (c) 2004-2006 Mike Matsnev.  All Rights Reserved.
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
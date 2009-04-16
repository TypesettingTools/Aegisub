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
 * $Id: VideoSink.cpp,v 1.8 2007/01/17 23:40:51 mike Exp $
 * 
 */


#include "config.h"

#ifdef WITH_DIRECTSHOW
#include <windows.h>
#ifdef __WXDEBUG__
#undef __WXDEBUG__
#endif
typedef TCHAR* PTCHAR;
#include <streams.h>
#include <dvdmedia.h>
#include "VideoSink.h"
#include "initguid.h"

class CVideoSink;
// CLSID for videosink: {F13D3732-96BD-4108-AFEB-E85F68FF64DC}
//DEFINE_GUID(CLSID_AegiVideoSink, 0xf13d3732, 0x96bd, 0x4108, 0xaf, 0xeb, 0xe8, 0x5f, 0x68, 0xff, 0x64, 0xdc);

// {E9C80780-4C07-4b36-87D4-5241CD0C6FE2}
DEFINE_GUID(CLSID_AegiVideoSink, 0xe9c80780, 0x4c07, 0x4b36, 0x87, 0xd4, 0x52, 0x41, 0xcd, 0xc, 0x6f, 0xe2);

static int  GetBPP(const BITMAPINFOHEADER& h) {
  switch (h.biCompression) {
    case MAKEFOURCC('Y','U','Y','2'): return 16;
    case MAKEFOURCC('Y','V','1','2'): return 12;
    case 0: return h.biBitCount;
  }
  return 0;
}

[uuid("2EE04A02-4AF5-43f8-B05B-5DEB66473419")]
interface IVSAllocator : public IUnknown {
  STDMETHOD(SetNextMT)(const AM_MEDIA_TYPE *pMT) = 0;
};

class CVSAllocator : public CMemAllocator, public IVSAllocator {
  CMediaType  *m_nextmt;
public:
  CVSAllocator(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) : CMemAllocator(pName, pUnk, phr), m_nextmt(NULL) { }
  ~CVSAllocator() {
    delete m_nextmt;
  }

  STDMETHOD(SetNextMT)(const AM_MEDIA_TYPE *pMT) {
    CMediaType  *newMT = new CMediaType(*pMT);
    newMT = (CMediaType *)InterlockedExchangePointer((void **)&m_nextmt, newMT);
    if (newMT != NULL)
      delete pMT;
    return S_OK;
  }

  STDMETHOD(GetBuffer)(IMediaSample **ppS, REFERENCE_TIME *pStart, REFERENCE_TIME *pStop, DWORD dwFlags) {
    CMediaType  *pMT = (CMediaType *)InterlockedExchangePointer((void **)&m_nextmt, NULL);
    if (pMT != NULL) {
      BITMAPINFOHEADER  *bmh = NULL;

      if (pMT->formattype == FORMAT_VideoInfo)
        bmh = &((VIDEOINFOHEADER *)pMT->pbFormat)->bmiHeader;
      else if (pMT->formattype == FORMAT_VideoInfo2)
        bmh = &((VIDEOINFOHEADER2 *)pMT->pbFormat)->bmiHeader;

      if (bmh != NULL) {
        ALLOCATOR_PROPERTIES  ap, act;

        Decommit();
        GetProperties(&ap);

        long  newsize = (bmh->biWidth * abs(bmh->biHeight) * GetBPP(*bmh)) >> 3;

        if (ap.cbBuffer < newsize)
          ap.cbBuffer = newsize;

        SetProperties(&ap, &act);
        Commit();
      }
    }

    HRESULT hr = CMemAllocator::GetBuffer(ppS, pStart, pStop, dwFlags);
    if (SUCCEEDED(hr) && pMT != NULL)
      (*ppS)->SetMediaType(pMT);

    if (pMT != NULL)
      delete pMT;

    return hr;
  }

  DECLARE_IUNKNOWN;
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv) {
    if (riid == __uuidof(IVSAllocator))
      return GetInterface((IVSAllocator *)this, ppv);

    return CMemAllocator::NonDelegatingQueryInterface(riid, ppv);
  }
};

class CVideoSinkPin : public CRenderedInputPin {
  bool        m_changedmt;
  unsigned    m_types;
  CVideoSink  *m_sink;

  HRESULT CheckMediaType(const CMediaType *pMT) {
    if (pMT->majortype != MEDIATYPE_Video ||
	(pMT->formattype != FORMAT_VideoInfo && pMT->formattype != FORMAT_VideoInfo2))
      return VFW_E_TYPE_NOT_ACCEPTED;

    if (pMT->subtype == MEDIASUBTYPE_RGB24 && m_types & IVS_RGB24)
      return S_OK;
    if (pMT->subtype == MEDIASUBTYPE_RGB32 && m_types & IVS_RGB32)
      return S_OK;
    if (pMT->subtype == MEDIASUBTYPE_YUY2 && m_types & IVS_YUY2)
      return S_OK;
    if (pMT->subtype == MEDIASUBTYPE_YV12 && m_types & IVS_YV12)
      return S_OK;

    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  STDMETHOD(Receive)(IMediaSample *pS);
  STDMETHOD(EndOfStream)();
  STDMETHOD(BeginFlush)();

public:
  CVideoSinkPin(CVideoSink *sink, HRESULT *phr);

  STDMETHOD(GetAllocator)(IMemAllocator **ppAllocator) {
    CAutoLock cObjectLock(m_pLock);

    if (m_pAllocator == NULL) {
      HRESULT hr = S_OK;
      m_pAllocator = new CVSAllocator(NAME("CVSAllocator"), NULL, &hr);
      if (FAILED(hr)) {
        delete m_pAllocator;
        m_pAllocator = NULL;
        return hr;
      }
      m_pAllocator->AddRef();
    }
    ASSERT(m_pAllocator != NULL);
    *ppAllocator = m_pAllocator;
    m_pAllocator->AddRef();
    return NOERROR;
  }
  STDMETHOD(NotifyAllocator)(IMemAllocator *pAlloc, BOOL bReadOnly) {
    CAutoLock cObjectLock(m_pLock);

    CComQIPtr<IVSAllocator> pVSA(pAlloc);
    if (!pVSA)
      return E_NOINTERFACE;
    if (m_changedmt) {
      m_changedmt = false;
      pVSA->SetNextMT(&m_mt);
    }
    return CRenderedInputPin::NotifyAllocator(pAlloc, bReadOnly);
  }

  HRESULT SetMediaType(const CMediaType *pMT) {
    HRESULT hr = CRenderedInputPin::SetMediaType(pMT);
    if (FAILED(hr))
      return hr;

    unsigned  type, width, height, bpp, arx, ary;
    int       stride;
    if (FAILED(hr = GetFrameFormat(&type, &width, &height, &stride, &bpp, &arx, &ary, NULL)))
      return hr;

    if ((stride & 15) != 0) { // extend
      CMediaType  newMT(m_mt);

      if (newMT.formattype == FORMAT_VideoInfo) {
        VIDEOINFOHEADER *vh = (VIDEOINFOHEADER *)newMT.pbFormat;

        vh->bmiHeader.biWidth = ((abs(stride) + 15) & ~15) / bpp;
        vh->rcTarget.left = vh->rcTarget.top = 0;
        vh->rcTarget.right = width;
        vh->rcTarget.bottom = height;
        vh->rcSource = vh->rcTarget;
      } else if (newMT.formattype == FORMAT_VideoInfo2) {
        VIDEOINFOHEADER2 *vh = (VIDEOINFOHEADER2 *)newMT.pbFormat;

        vh->bmiHeader.biWidth = ((abs(stride) + 15) & ~15) / bpp;
        vh->rcTarget.left = vh->rcTarget.top = 0;
        vh->rcTarget.right = width;
        vh->rcTarget.bottom = height;
        vh->rcSource = vh->rcTarget;
      } else
        return E_FAIL;

      hr = m_Connected->QueryAccept(&newMT);
      if (SUCCEEDED(hr)) {
        hr = CRenderedInputPin::SetMediaType(&newMT);
        if (FAILED(hr))
          return hr;

        CComQIPtr<IVSAllocator> pVSA(m_pAllocator);
        if (pVSA)
          pVSA->SetNextMT(&newMT);
        else
          m_changedmt = true;
      }
    }

    return S_OK;
  }

  BOOL		  AtEOF() { return m_bAtEndOfStream; }
  REFERENCE_TIME  SegStartTime() { return m_tStart; }
  unsigned	  GetTypes() { return m_types; }
  void		  SetTypes(unsigned t) { m_types = t; }

  HRESULT GetFrameFormat(unsigned *type, unsigned *width, unsigned *height, int *stride,
			 unsigned *pbpp, unsigned *arx, unsigned *ary, __int64 *def_duration)
  {
    if (!IsConnected())
      return VFW_E_NOT_CONNECTED;

    unsigned  bpp;

    if (m_mt.subtype == MEDIASUBTYPE_RGB24)
      *type = IVS_RGB24, bpp = 3;
    else if (m_mt.subtype == MEDIASUBTYPE_RGB32)
      *type = IVS_RGB32, bpp = 4;
    else if (m_mt.subtype == MEDIASUBTYPE_YUY2)
      *type = IVS_YUY2, bpp = 2;
    else if (m_mt.subtype == MEDIASUBTYPE_YV12)
      *type = IVS_YV12, bpp = 1;
    else
      return VFW_E_INVALID_MEDIA_TYPE;

    if (pbpp)
      *pbpp = bpp;

    BITMAPINFOHEADER  *bmh;
    RECT	      rct;

    if (m_mt.formattype == FORMAT_VideoInfo && m_mt.FormatLength() >= sizeof(VIDEOINFOHEADER)) {
      VIDEOINFOHEADER *vh = (VIDEOINFOHEADER *)m_mt.Format();
      bmh = &vh->bmiHeader;
      rct = vh->rcTarget;
      if (arx)
	*arx = 1;
      if (*ary)
	*ary = 1;
      if (def_duration)
        *def_duration = vh->AvgTimePerFrame;
    } else if (m_mt.formattype == FORMAT_VideoInfo2 && m_mt.FormatLength() >= sizeof(VIDEOINFOHEADER2)) {
      VIDEOINFOHEADER2 *vh = (VIDEOINFOHEADER2 *)m_mt.Format();
      bmh = &vh->bmiHeader;
      rct = vh->rcTarget;
      if (arx)
	*arx = vh->dwPictAspectRatioX;
      if (ary)
	*ary = vh->dwPictAspectRatioY;
      if (def_duration)
        *def_duration = vh->AvgTimePerFrame;
    } else
      return VFW_E_INVALID_MEDIA_TYPE;

    if (stride)
      *stride = (bmh->biHeight > 0 && bmh->biCompression == 0 ? -1 : 1) * (int)bmh->biWidth * (int)bpp;

    if (rct.right != 0)
      *width = rct.right - rct.left;
    else
      *width = bmh->biWidth;
    if (rct.bottom != 0)
      *height = rct.bottom - rct.top;
    else
      *height = abs(bmh->biHeight);

    return S_OK;
  }

  DECLARE_IUNKNOWN;
};

class CVideoSink :
  public CBaseFilter,
  public IVideoSink,
  public IVideoSink2,
  public IAMFilterMiscFlags
{
  CVideoSinkPin		    *m_pin;
  CRendererPosPassThru      *m_rpp;
  CCritSec		    m_lock;

  int	    GetPinCount() { return 1; }
  CBasePin  *GetPin(int n) { return n == 0 ? m_pin : NULL; }

  CComPtr<IMediaSample>	    m_sample;
  HANDLE		    m_hEvent1, m_hEvent2, m_hNotify;
  CComPtr<IVideoSinkNotify> m_notify;

public:
  CVideoSink(IUnknown *pUnk, HRESULT *phr) :
    CBaseFilter(_T("CVideoSink"), pUnk, &m_lock, CLSID_AegiVideoSink),
    m_pin(NULL),
    m_rpp(NULL)
  {
    m_pin = new CVideoSinkPin(this, phr);
    if (FAILED(*phr))
      return;
    m_rpp = new CRendererPosPassThru(NAME("CVideoSink PosPassThru"), CBaseFilter::GetOwner(), phr, m_pin);
    if (FAILED(*phr))
      return;

    m_hEvent1 = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hEvent2 = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hNotify = NULL;
  }
  ~CVideoSink() {
    m_sample = NULL;
    delete m_rpp;
    delete m_pin;
    CloseHandle(m_hEvent1);
    CloseHandle(m_hEvent2);
  }

  CCritSec  *pStateLock() { return m_pLock; }

  // called when lock is held
  HRESULT   Receive(IMediaSample *pS) {
    if (pS == NULL)
      m_rpp->EOS();
    else
      m_rpp->RegisterMediaTime(pS);

    // notify callback
    CComPtr<IVideoSinkNotify> notify = m_notify;
    HANDLE  hNotify = m_hNotify;

    if (notify || hNotify) {
      // save our sample
      m_sample = pS;

      // notify receiver
      SetEvent(m_hEvent1);
    }

    pStateLock()->Unlock();

    if (notify || hNotify) {
      if (notify)
        notify->FrameReady();
      if (hNotify)
        SetEvent(hNotify);

      // wait until the thing is processed
      WaitForSingleObject(m_hEvent2, INFINITE);
    }

    if (pS == NULL)
      NotifyEvent(EC_COMPLETE, 0, (LONG_PTR)static_cast<IBaseFilter*>(this));

    return S_OK;
  }

  HRESULT   BeginFlush() {
    CAutoLock lock(pStateLock());
    ResetEvent(m_hEvent1);
    m_sample = NULL;
    SetEvent(m_hEvent2);
    return S_OK;
  }

  STDMETHOD(Stop)() {
    BeginFlush();
    return CBaseFilter::Stop();
  }

  // IVideoSink
  STDMETHOD(SetAllowedTypes)(unsigned types) {
    CAutoLock lock(pStateLock());
    m_pin->SetTypes(types);
    return S_OK;
  }
  STDMETHOD(GetAllowedTypes)(unsigned *types) {
    CheckPointer(types, E_POINTER);
    CAutoLock lock(pStateLock());
    *types = m_pin->GetTypes();
    return S_OK;
  }
  STDMETHOD(NotifyFrame)(IVideoSinkNotify *notify) {
    CAutoLock lock(pStateLock());
    m_notify = notify;
    return S_OK;
  }
  STDMETHOD(GetFrameFormat)(unsigned *type, unsigned *width, unsigned *height, unsigned *arx, unsigned *ary) {
    CheckPointer(type, E_POINTER);
    CheckPointer(width, E_POINTER);
    CheckPointer(height, E_POINTER);
    CAutoLock lock(pStateLock());
    return m_pin->GetFrameFormat(type, width, height, NULL, NULL, arx, ary, NULL);
  }
  STDMETHOD(ReadFrame)(ReadFrameFunc f, void *arg) {
    {
      CAutoLock	lock(pStateLock());
      if (m_pin->AtEOF()) {
        if (WaitForSingleObject(m_hEvent1, 0) == WAIT_OBJECT_0)
          SetEvent(m_hEvent2);
        return S_FALSE;
      }
    }

    WaitForSingleObject(m_hEvent1, INFINITE);

    HRESULT hr = S_OK;
    {
      CAutoLock	lock(pStateLock());

      CComPtr<IMediaSample>   pS(m_sample);
      m_sample = NULL;

      if (!pS)
        hr = S_FALSE;
      else {
        REFERENCE_TIME	rtS, rtE;
        if (SUCCEEDED(pS->GetTime(&rtS, &rtE)))
          rtS += m_pin->SegStartTime();
        else
          rtS = -1;

	if (f) {
	  unsigned  type, srcW, srcH, arx, ary, srcBPP;
          int       srcS;
	  BYTE	  *srcP;
	  if (FAILED(m_pin->GetFrameFormat(&type, &srcW, &srcH, &srcS, &srcBPP, &arx, &ary, NULL)) ||
	      FAILED(pS->GetPointer(&srcP)))
	    hr = E_FAIL;
	  else {
            if (srcS < 0)
              srcP += abs(srcS) * (srcH - 1);
	    f(rtS, type, srcBPP, srcP, srcW, srcH, srcS, arx, ary, arg);
          }
	}
      }
    }

    SetEvent(m_hEvent2);

    return hr;
  }

  // IVideoSink2
  STDMETHOD(NotifyFrame)(HANDLE hEvent) {
    m_hNotify = hEvent;
    return S_OK;
  }
  STDMETHOD(GetFrameFormat)(unsigned *type, unsigned *width, unsigned *height, unsigned *arx, unsigned *ary, __int64 *def_duration) {
    CheckPointer(type, E_POINTER);
    CheckPointer(width, E_POINTER);
    CheckPointer(height, E_POINTER);
    CAutoLock lock(pStateLock());
    return m_pin->GetFrameFormat(type, width, height, NULL, NULL, arx, ary, def_duration);
  }

  // IAMFilterMiscFlags
  STDMETHOD_(ULONG, GetMiscFlags)() { return AM_FILTER_MISC_FLAGS_IS_RENDERER; }

  // COM
  DECLARE_IUNKNOWN;
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv) {
    CAutoLock lock(pStateLock());

    if (riid == __uuidof(IVideoSink))
      return GetInterface((IVideoSink *)this, ppv);
    if (riid == __uuidof(IVideoSink2))
      return GetInterface((IVideoSink2 *)this, ppv);
    if (riid == __uuidof(IAMFilterMiscFlags))
      return GetInterface((IAMFilterMiscFlags *)this, ppv);

    if (riid == IID_IMediaSeeking || riid == IID_IMediaPosition)
      return m_rpp->NonDelegatingQueryInterface(riid, ppv);

    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
  }
};

CVideoSinkPin::CVideoSinkPin(CVideoSink *sink, HRESULT *phr) :
  CRenderedInputPin(_T("CVideoSinkPin"), sink, sink->pStateLock(), phr, L"Input"),
  m_types(IVS_RGB32),
  m_sink(sink),
  m_changedmt(false)
{
}

HRESULT CVideoSinkPin::Receive(IMediaSample *pS) {
  m_pLock->Lock();

  if (m_bFlushing) {
    m_pLock->Unlock();
    return S_FALSE;
  }

  CMediaType	  MT;
  AM_MEDIA_TYPE *pMT;
  if (SUCCEEDED(pS->GetMediaType(&pMT)) && pMT != NULL) {
    MT.Set(*pMT); DeleteMediaType(pMT);

    HRESULT hr = CheckMediaType(&MT);
    if (FAILED(hr)) {
      m_pLock->Unlock();
      return hr;
    }

    SetMediaType(&MT);
  }

  if (pS->IsPreroll() == S_OK) {
    m_pLock->Unlock();
    return S_OK;
  }

  return m_sink->Receive(pS);
}

HRESULT	CVideoSinkPin::EndOfStream() {
  HRESULT hr1, hr2;

  m_pLock->Lock();
  hr1 = CRenderedInputPin::EndOfStream();
  if (m_bFlushing) {
    m_pLock->Unlock();
    return hr1;
  }

  hr2 = m_sink->Receive(NULL);
  if (FAILED(hr1))
    return hr1;
  return hr2;
}

HRESULT	CVideoSinkPin::BeginFlush() {
  HRESULT hr = CRenderedInputPin::BeginFlush();
  m_sink->BeginFlush();
  return hr;
}

//CUnknown * WINAPI CreateVideoSink(IUnknown *pUnk, HRESULT *phr) {
//  CVideoSink  *vs = new CVideoSink(pUnk, phr);
//  if (vs == NULL)
//    *phr = E_OUTOFMEMORY;
//  else if (FAILED(*phr)) {
//    delete vs;
//    vs = NULL;
//  }
//  return vs;
//}

HRESULT CreateVideoSink(IBaseFilter **pVS) {
	HRESULT hr = S_OK;
	CVideoSink *vs = new CVideoSink(NULL,&hr);
	if (vs == NULL) hr = E_OUTOFMEMORY;
	else if (FAILED(hr)) {
		delete vs;
		vs = NULL;
	}
	vs->AddRef();
	*pVS = vs;
	return hr;
}

#endif // WITH_DIRECTSHOW

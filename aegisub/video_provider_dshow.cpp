// Copyright (c) 2006, Rodrigo Braz Monteiro, Mike Matsnev
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers

#ifdef WITH_DIRECTSHOW

#pragma warning(disable: 4995)
#include <wx/wxprec.h>
#ifdef __WINDOWS__
#include <wx/image.h>
#include <dshow.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <windows.h>
#include <tchar.h>
#include <initguid.h>
#include "utils.h"
#include "vfr.h"
#include "videosink.h"
#include "gl_wrap.h"
#include "options.h"
#include "video_provider_dshow.h"


///////////////
// Constructor
// Based on Haali's code for DirectShowSource2
DirectShowVideoProvider::DirectShowVideoProvider(wxString _filename, double _fps) {
	fps = _fps;
	m_registered = false;
	m_hFrameReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	HRESULT hr = OpenVideo(_filename);
	if (FAILED(hr)) throw _T("Failed opening DirectShow content.");
}


//////////////
// Destructor
DirectShowVideoProvider::~DirectShowVideoProvider() {
	CloseVideo();
}


////////////
// Get pin
// Code by Haali
#define ENUM_FILTERS(graph, var) for (CComPtr<IEnumFilters> __pEF__; !__pEF__ && SUCCEEDED(graph->EnumFilters(&__pEF__)); ) for (CComPtr<IBaseFilter> var; __pEF__->Next(1, &var, NULL) == S_OK; var.Release())
#define ENUM_PINS(filter, var) for (CComPtr<IEnumPins> __pEP__; !__pEP__ && SUCCEEDED(filter->EnumPins(&__pEP__)); ) for (CComPtr<IPin> var; __pEP__->Next(1, &var, NULL) == S_OK; var.Release())

class MTPtr {
  AM_MEDIA_TYPE *pMT;

  MTPtr(const MTPtr&);
  MTPtr& operator=(const MTPtr&);
public:
  MTPtr() : pMT(NULL) { }
  ~MTPtr() { DeleteMediaType(pMT); }

  AM_MEDIA_TYPE *operator->() { return pMT; }
  const AM_MEDIA_TYPE *operator->() const { return pMT; }
  operator AM_MEDIA_TYPE *() { return pMT; }
  AM_MEDIA_TYPE **operator&() { DeleteMediaType(pMT); pMT = NULL; return &pMT; }

  static void FreeMediaType(AM_MEDIA_TYPE *pMT) {
    if (pMT == NULL)
      return;
    if (pMT->cbFormat > 0) {
      CoTaskMemFree(pMT->pbFormat);
      pMT->pbFormat = NULL;
      pMT->cbFormat = 0;
    }
    if (pMT->pUnk) {
      pMT->pUnk->Release();
      pMT->pUnk = NULL;
    }
  }

  static void  DeleteMediaType(AM_MEDIA_TYPE *pMT) {
    if (pMT == NULL)
      return;
    if (pMT->cbFormat > 0)
      CoTaskMemFree(pMT->pbFormat);
    if (pMT->pUnk)
      pMT->pUnk->Release();
    CoTaskMemFree(pMT);
  }
};

#define ENUM_MT(pin, var) for (CComPtr<IEnumMediaTypes> __pEMT__; !__pEMT__ && SUCCEEDED(pin->EnumMediaTypes(&__pEMT__)); ) for (MTPtr var; __pEMT__->Next(1, &var, NULL) == S_OK; )

CComPtr<IPin> GetPin(IBaseFilter *pF, bool include_connected, PIN_DIRECTION dir, const GUID *pMT = NULL) {
  if (pF == NULL)
    return CComPtr<IPin>();

  ENUM_PINS(pF, pP) {
    PIN_DIRECTION     pd;
    if (FAILED(pP->QueryDirection(&pd)))
      continue;
    if (pd == dir) {
      if (!include_connected) {
	CComPtr<IPin> pQ;
	if (SUCCEEDED(pP->ConnectedTo(&pQ)))
	  continue;
      }
      if (pMT == NULL)
	return pP;

      ENUM_MT(pP, MT)
	if (MT->majortype == *pMT)
          return pP;
    }
  }

  return CComPtr<IPin>();
}


////////////////////
// More Haali stuff
void DirectShowVideoProvider::RegROT() {
	if (!m_pGC || m_registered)
	  return;

	CComPtr<IRunningObjectTable>  rot;
	if (FAILED(GetRunningObjectTable(0, &rot)))
	  return;

	CStringA  name;
	name.Format("FilterGraph %08p pid %08x (avss)", m_pGC.p, GetCurrentProcessId());

	CComPtr<IMoniker>		  mk;
	if (FAILED(CreateItemMoniker(L"!", CA2W(name), &mk)))
	  return;

	if (SUCCEEDED(rot->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, m_pGC, mk, &m_rot_cookie)))
	  m_registered = true;
}

void DirectShowVideoProvider::UnregROT() {
	if (!m_registered)
	  return;

	CComPtr<IRunningObjectTable>  rot;
	if (FAILED(GetRunningObjectTable(0, &rot)))
	  return;

	if (SUCCEEDED(rot->Revoke(m_rot_cookie)))
	  m_registered = false;
}



//////////////
// Open video
HRESULT DirectShowVideoProvider::OpenVideo(wxString _filename) {
	HRESULT hr;

	// Create an instance of the Filter Graph
	CComPtr<IGraphBuilder> pG;
	if (FAILED(hr = pG.CoCreateInstance(CLSID_FilterGraph))) return hr;

	// Create an Instance of the Video Sink
	//CComPtr<IBaseFilter> pR;
	//CLSID CLSID_VideoSink;
	//CLSIDFromString(L"{F13D3732-96BD-4108-AFEB-E85F68FF64DC}",&CLSID_VideoSink);
	//if (FAILED(hr = pR.CoCreateInstance(CLSID_VideoSink))) return hr;

	CComPtr<IBaseFilter>	pR;
	hr = CreateVideoSink(&pR);

	// Add VideoSink to graph
	pG->AddFilter(pR, L"VideoSink");

	// Query interface from sink
	CComQIPtr<IVideoSink> sink(pR);
	if (!sink) return E_NOINTERFACE;
	CComQIPtr<IVideoSink2> sink2(pR);
	if (!sink2) return E_NOINTERFACE;

	// Set allowed types for sink
	unsigned int types = IVS_RGB24 | IVS_RGB32;
	if (OpenGLWrapper::UseShaders()) types = types | IVS_YV12;
	sink->SetAllowedTypes(types);

	// Pass the event to sink, so it gets set when a frame is available
	ResetEvent(m_hFrameReady);
	sink2->NotifyFrame(m_hFrameReady);
	// Create source filter and add it to graph
	CComPtr<IBaseFilter> pS;
	if (FAILED(hr = pG->AddSourceFilter(_filename.wc_str(), NULL, &pS))) return hr;

	// Property bag? The heck is this?
	// Is this supposed to make it "interactive", enabling some actions?
	// I have no clue.
	CComQIPtr<IPropertyBag> pPB(pS);
	if (pPB) pPB->Write(L"ui.interactive", &CComVariant(0u, VT_UI4));

	// Get source's output pin
	CComPtr<IPin> pO(GetPin(pS, false, PINDIR_OUTPUT, &MEDIATYPE_Video));
	if (!pO) pO = GetPin(pS, false, PINDIR_OUTPUT, &MEDIATYPE_Stream);

	// Get sink's input pin
	CComPtr<IPin> pI(GetPin(pR, false, PINDIR_INPUT));

	// Check if pins are ok
	if (!pO || !pI) return E_FAIL;

	// Connect pins
	if (FAILED(hr = pG->Connect(pO, pI))) return hr;

	// Query the control interfaces from the graph
	CComQIPtr<IMediaControl>  mc(pG);
	CComQIPtr<IMediaSeeking>  ms(pG);

	// See if they were created correctly
	if (!mc || !ms) return E_NOINTERFACE;

	// Run MediaControl, initiating the data flow through it
	if (FAILED(hr = mc->Run())) return hr;

	// Get state from media seeking (??)
	OAFilterState fs;
	if (FAILED(hr = mc->GetState(2000, &fs))) return hr;

	// Wait up to 5 seconds for the first frame to arrive
	if (WaitForSingleObject(m_hFrameReady, 5000) != WAIT_OBJECT_0) return E_FAIL;

	// Get frame format
	unsigned type, arx, ary;
	if (FAILED(hr = sink2->GetFrameFormat(&type, &width, &height, &arx, &ary, &defd))) return hr;

	// Get video duration
	if (FAILED(hr = ms->GetDuration(&duration))) return hr;

	// Set pixel type
	//switch (type) {
	//	case IVS_RGB32: m_vi.pixel_type = VideoInfo::CS_BGR32; break;
	//	case IVS_YUY2: m_vi.pixel_type = VideoInfo::CS_YUY2; break;
	//	case IVS_YV12: m_vi.pixel_type = VideoInfo::CS_YV12; break;
	//	default: return E_FAIL;
	//}

	// Set FPS and frame duration
	if (defd == 0) defd = 417083;
	if (fps != 0.0) defd = int64_t (10000000.0 / fps) + 1;
	else fps = 10000000.0 / double(++defd);

	// Set number of frames
	last_fnum = 0;
	num_frames = duration / defd;

	// Store filters
	m_pR = sink;
	m_pGC = mc;
	m_pGS = ms;

	// Flag frame as ready?
	SetEvent(m_hFrameReady);

	// Register graph with Running Objects Table for remote graphedit connection
	RegROT();

	//NextFrame();

	// Set frame count
	//m_f.SetCount(m_vi.num_frames);
	return hr;
}


///////////////
// Close video
void DirectShowVideoProvider::CloseVideo() {
	rdf.frame.Clear();
	CComQIPtr<IVideoSink2>  pVS2(m_pR);
	if (pVS2) pVS2->NotifyFrame(NULL);

	UnregROT();

	m_pR.Release();
	m_pGC.Release();
	m_pGS.Release();
	ResetEvent(m_hFrameReady);
	CloseHandle(m_hFrameReady);
}


/////////////////////////
// Read DirectShow frame
void DirectShowVideoProvider::ReadFrame(int64_t timestamp, unsigned format, unsigned bpp, const unsigned char *frame, unsigned width, unsigned height, int stride, unsigned arx, unsigned ary, void *arg) {
	// Set frame
	DF *df = (DF*) arg;
	df->timestamp = timestamp;

	// Create frame
	const unsigned char * src = frame;
	if (stride < 0) {
		src += stride*(height-1);
		stride = -stride;
		df->frame.flipped = true;
	}
	else df->frame.flipped = false;
	df->frame.w = width;
	df->frame.h = height;
	df->frame.pitch[0] = stride;
	if (format == IVS_YV12) {
		df->frame.pitch[1] = stride/2;
		df->frame.pitch[2] = stride/2;
	}
	df->frame.cppAlloc = false;
	df->frame.invertChannels = true;

	// Set format
	if (format == IVS_RGB24) df->frame.format = FORMAT_RGB24;
	else if (format == IVS_RGB32) df->frame.format = FORMAT_RGB32;
	else if (format == IVS_YV12) {
		df->frame.format = FORMAT_YV12;
		df->frame.invertChannels = true;
	}
	else if (format == IVS_YUY2) df->frame.format = FORMAT_YUY2;

	// Allocate and copy data
	df->frame.Allocate();
	memcpy(df->frame.data[0],src,df->frame.pitch[0]*height + (df->frame.pitch[1]+df->frame.pitch[2])*height/2);
}


/////////////////////
// Get Next DS Frame
int DirectShowVideoProvider::NextFrame(DF &df,int &_fn) {
	// Keep reading until it gets a good frame
	while (true) {
		// Set object and receive data
		if (WaitForSingleObject(m_hFrameReady, INFINITE) != WAIT_OBJECT_0) return 1;

		// Read frame
		HRESULT hr = m_pR->ReadFrame(ReadFrame, &df);
		if (FAILED(hr)) {
			//df.frame.Clear();
			return 2;
		}

		// End of file
		if (hr == S_FALSE) {
			//df.frame.Clear();
			return 3;
		}

		// Valid timestamp
		if (df.timestamp >= 0) {
			// CFR frame number
			int frameno = -1;
			if (frameTime.Count() == 0) frameno = (int)((double)df.timestamp / defd + 0.5);

			// VFR
			else {
				for (unsigned int i=0;i<frameTime.Count();i++) {
					if (df.timestamp < (int64_t) frameTime[i] * 10000) {
						frameno = i-1;
						break;
					}
				}
				if (frameno == -1) frameno = frameTime.Count()-1;
			}

			// Got a good one
			if (frameno >= 0) {
				_fn = frameno;
				//_df = df;
				return 0;
			}
		}

		//df.frame.Clear();
	}
}


/////////////
// Get frame
const AegiVideoFrame DirectShowVideoProvider::GetFrame(int n,int formatMask) {
	// Normalize frame number
	if (n >= (signed) num_frames) n = num_frames-1;
	if (n < 0) n = 0;

	// Variables
	//DF df;
	int fn;

	// Time to seek to
	REFERENCE_TIME cur;
	cur = defd * n + 10001;
	if (frameTime.Count() > (unsigned) n) cur = frameTime[n] * 10000 + 10001;
	if (cur < 0) cur = 0;

	// Is next
	if (n == (signed)last_fnum + 1) {
		//rdf.frame.Clear();
		NextFrame(rdf,fn);
		last_fnum = n;
		return rdf.frame;
	}

	// Not the next, reset and seek first
seek:
	ResetEvent(m_hFrameReady);

	// Seek
	if (FAILED(m_pGS->SetPositions(&cur, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning))) return AegiVideoFrame(width,height);

	// Set time
	REFERENCE_TIME timestamp = -1;

	// Actually get data
	while (true) {
		// Get frame
		int fn = -1;
		int result = NextFrame(rdf,fn);

		// Preroll
		if (result == 0 && fn < n) {
			continue;
		}

		// Right frame
		else if (fn == n) {
			// we want this frame, compare timestamps to account for decimation
			// we see this for the first time
			if (timestamp < 0) timestamp = rdf.timestamp;

			// early, ignore
			if (rdf.timestamp < timestamp) {
				continue;
			}

			// this is the frame we want
			last_fnum = n;
			//rdf.frame.Clear();
			//rdf.frame = df.frame;
			return rdf.frame;
		}

		// Passed or end of file, seek back and try again
		else if (result == 0 || result == 3) {
			cur -= defd;
			goto seek;
		}

		// Failed
		else {
			return AegiVideoFrame(width,height);
		}
	}
}


////////////////
// Refresh subs
void DirectShowVideoProvider::RefreshSubtitles() {
}


///////////////////
// Get float frame
void DirectShowVideoProvider::GetFloatFrame(float* Buffer, int n) {
}


////////////////////////
// Override frame times
void DirectShowVideoProvider::OverrideFrameTimeList(wxArrayInt list) {
	frameTime = list;
	num_frames = frameTime.Count();
}

#endif
#endif // WITH_DIRECTSHOW

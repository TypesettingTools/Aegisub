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
#include "setup.h"
#if USE_DIRECTSHOW == 1
#pragma warning(disable: 4995)
#include <wx/wxprec.h>
#include <wx/image.h>
#include <windows.h>
#include <tchar.h>
#include <initguid.h>
#include "video_provider_dshow.h"
#include "utils.h"
#include "vfr.h"

// CLSID for videosink: {F13D3732-96BD-4108-AFEB-E85F68FF64DC}
DEFINE_GUID(CLSID_VideoSink, 0xf13d3732, 0x96bd, 0x4108, 0xaf, 0xeb, 0xe8, 0x5f, 0x68, 0xff, 0x64, 0xdc);


///////////////
// Constructor
// Based on Haali's code for DirectShowSource2
DirectShowVideoProvider::DirectShowVideoProvider(wxString _filename, wxString _subfilename) {
	zoom = 1.0;
	dar = 4.0/3.0;
	m_hFrameReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	OpenVideo(_filename);
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
	CComPtr<IBaseFilter> pR;
	CLSID CLSID_VideoSink;
	CLSIDFromString(L"{F13D3732-96BD-4108-AFEB-E85F68FF64DC}",&CLSID_VideoSink);
	if (FAILED(hr = pR.CoCreateInstance(CLSID_VideoSink))) return hr;

	// Add VideoSink to graph
	pG->AddFilter(pR, L"VideoSink");

	// Create instance of sink (??)
	CComQIPtr<IVideoSink> sink(pR);
	if (!sink) return E_NOINTERFACE;

	// Create another instance of sink (??)
	CComQIPtr<IVideoSink2> sink2(pR);
	if (!sink2) return E_NOINTERFACE;

	// Set allowed types for sink
	//sink->SetAllowedTypes(IVS_RGB32|IVS_YV12|IVS_YUY2);
	sink->SetAllowedTypes(IVS_RGB24);

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

	// Length of each frame? (??)
	if (defd == 0) defd = 417083;

	// No clue, either
	int avgf = 0;
	if (avgf > 0) defd = avgf;

	// Set pixel type
	//switch (type) {
	//	case IVS_RGB32: m_vi.pixel_type = VideoInfo::CS_BGR32; break;
	//	case IVS_YUY2: m_vi.pixel_type = VideoInfo::CS_YUY2; break;
	//	case IVS_YV12: m_vi.pixel_type = VideoInfo::CS_YV12; break;
	//	default: return E_FAIL;
	//}

	// Set number of frames and fps
	last_fnum = -1;
	num_frames = duration / defd;
	fps = 10000000.0 / double(defd);

	// Set frame length
	//m_avgframe = defd;

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
	CComQIPtr<IVideoSink2>  pVS2(m_pR);
	if (pVS2) pVS2->NotifyFrame(NULL);

	UnregROT();

	m_pR.Release();
	m_pGC.Release();
	m_pGS.Release();
}


/////////////////////////
// Read DirectShow frame
void DirectShowVideoProvider::ReadFrame(long long timestamp, unsigned format, unsigned bpp, const unsigned char *frame, unsigned width, unsigned height, unsigned stride, unsigned arx, unsigned ary, void *arg) {
	// Set frame
	DF *df = (DF*) arg;
	df->timestamp = timestamp;
	unsigned int w_cp = width;
	unsigned int h_cp = height;

	// Create data
	unsigned char *data;
	//data = new unsigned char[width*height*bpp];
	data = (unsigned char *) malloc(width*height*bpp);
	unsigned int dstride = width*bpp;

	// Read RGB24 data
	if (format == IVS_RGB24) {
		unsigned char *dst = data + h_cp*dstride;
		const unsigned char *src = frame;
		//unsigned char t1,t2;
		w_cp *= bpp;
		for (int y=h_cp; --y>=0;) {
			dst -= dstride;
			for (int x=width; --x>=0;) {
				//t1 = *src++;
				//t2 = *src++;
				*dst++ = *(src+2);
				*dst++ = *(src+1);
				*dst++ = *src;
				src += 3;
			}
			dst -= dstride;
		}
	}

	// Create bitmap out of data
	//df->frame = wxBitmap((const char*) data, width, height, bpp*8);
	//delete data;
	df->frame = wxImage(width,height,data,false);
	
	//else if (format == IVS_YV12 && vi->pixel_type == VideoInfo::CS_YV12) {
	//  // plane Y
	//  BYTE                *dp = df->frame->GetWritePtr(PLANAR_Y);
	//  const unsigned char *sp = frame;
	//  int                 dstride = df->frame->GetPitch(PLANAR_Y);

	//  for (int y = 0; y < h_cp; ++y) {
	//	memcpy(dp, sp, w_cp);
	//	sp += stride;
	//	dp += dstride;
	//  }

	//  // UV
	//  dstride >>= 1;
	//  stride >>= 1;
	//  w_cp >>= 1;
	//  h_cp >>= 1;

	//  // plane V
	//  dp = df->frame->GetWritePtr(PLANAR_V);
	//  sp = frame + height * stride * 2;
	//  dstride = df->frame->GetPitch(PLANAR_V);

	//  for (int y = 0; y < h_cp; ++y) {
	//	memcpy(dp, sp, w_cp);
	//	sp += stride;
	//	dp += dstride;
	//  }

	//  // plane U
	//  dp = df->frame->GetWritePtr(PLANAR_U);
	//  sp = frame + height * stride * 2 + (height >> 1) * stride;
	//  dstride = df->frame->GetPitch(PLANAR_U);

	//  for (int y = 0; y < h_cp; ++y) {
	//	memcpy(dp, sp, w_cp);
	//	sp += stride;
	//	dp += dstride;
	//  }
	//}
}


/////////////////////
// Get Next DS Frame
bool DirectShowVideoProvider::NextFrame(DF &_df,int &_fn) {
	// Keep reading until it gets a good frame
	while (true) {
		if (WaitForSingleObject(m_hFrameReady, INFINITE) != WAIT_OBJECT_0) return false;

		// Set object to receive data
		DF df;

		// Read frame
		HRESULT hr = m_pR->ReadFrame(ReadFrame, &df);
		if (FAILED(hr)) return false;

		// End of file
		if (hr == S_FALSE) return false;

		// Valid timestamp
		if (df.timestamp >= 0) {
			// Frame number
			int frameno = (int)((double)df.timestamp / defd + 0.5);

			// Got a good one
			if (frameno >= 0 && frameno <= (signed) num_frames) {
				_fn = frameno;
				_df = df;
				if (zoom != 1.0 || dar != 1.0) {
					_df.frame.Rescale(height*zoom*dar,height*zoom,wxIMAGE_QUALITY_NORMAL);
				}
				return true;
			}
		}
	}
}


/////////////
// Get frame
wxBitmap DirectShowVideoProvider::GetFrame(int n) {
	// Normalize frame number
	if (n >= (signed) num_frames) n = num_frames-1;

	// Current
	if (n == last_fnum) return wxBitmap(rdf.frame);

	// Variables
	DF df;
	int fn;
	REFERENCE_TIME cur = defd * n + 10001;
	if (cur < 0) cur = 0;

	// Is next
	if (n == last_fnum + 1) {
		NextFrame(df,fn);
		last_fnum = n;
		rdf.frame = df.frame;
		return wxBitmap(rdf.frame);
	}

	// Not the next, reset and seek first
seek:
	ResetEvent(m_hFrameReady);

	// Seek
	if (FAILED(m_pGS->SetPositions(&cur, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning))) return wxBitmap(width,height);

	// Set time
	rdf.timestamp = -1;

	// Actually get data
	while (true) {
		// Get frame
		DF df;
		int fn;
		NextFrame(df,fn);

		// Preroll
		if (fn < n) continue;

		// Right frame
		if (fn == n) {
			// we want this frame, compare timestamps to account for decimation
			// we see this for the first time
			if (rdf.timestamp < 0) rdf.timestamp = df.timestamp;

			// early, ignore
			if (df.timestamp < rdf.timestamp) continue;

			// this is the frame we want
			rdf.frame = df.frame;
			break;
		}

		// Passed, seek back and try again
		else {
			cur -= defd;
			goto seek;
			//return wxBitmap(width,height);
		}
	}

	// Return frame
	last_fnum = n;
	return wxBitmap(rdf.frame);
}


////////////////
// Refresh subs
void DirectShowVideoProvider::RefreshSubtitles() {
}


///////////
// Set DAR
void DirectShowVideoProvider::SetDAR(double _dar) {
	dar = _dar;
	last_fnum = -2;
}


////////////
// Set Zoom
void DirectShowVideoProvider::SetZoom(double _zoom) {
	zoom = _zoom;
	last_fnum = -2;
}


///////////////////
// Get float frame
void DirectShowVideoProvider::GetFloatFrame(float* Buffer, int n) {
}


#endif

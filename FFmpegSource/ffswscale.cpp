//  Copyright (c) 2008 Fredrik Mellbin
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include "ffmpegsource.h"

SWScale::SWScale(PClip AChild, int AResizeToWidth, int AResizeToHeight, const char *AResizer, const char *AConvertToFormat, IScriptEnvironment *Env) : GenericVideoFilter(AChild) {
	Context = NULL;
	OrigWidth = vi.width;
	OrigHeight = vi.height;
	FlipOutput = vi.IsYUV();
	
	int ConvertFromFormat = PIX_FMT_NONE;
	if (vi.IsYV12())
		ConvertFromFormat = PIX_FMT_YUV420P;
	if (vi.IsYUY2())
		ConvertFromFormat = PIX_FMT_YUYV422;
	if (vi.IsRGB24())
		ConvertFromFormat = PIX_FMT_BGR24;
	if (vi.IsRGB32())
		ConvertFromFormat = PIX_FMT_RGB32;

	if (AResizeToHeight <= 0)
		AResizeToHeight = OrigHeight;
	else
		vi.height = AResizeToHeight;

	if (AResizeToWidth <= 0)
		AResizeToWidth = OrigWidth;
	else
		vi.width = AResizeToWidth;

	int ConvertToFormat = CSNameToPIXFMT(AConvertToFormat, ConvertFromFormat);
	if (ConvertToFormat == PIX_FMT_NONE)
		Env->ThrowError("SWScale: Invalid colorspace specified (%s)", AConvertToFormat);

	switch (ConvertToFormat) {
		case PIX_FMT_YUV420P: vi.pixel_type = VideoInfo::CS_I420; break;
		case PIX_FMT_YUYV422: vi.pixel_type = VideoInfo::CS_YUY2; break;
		case PIX_FMT_BGR24: vi.pixel_type = VideoInfo::CS_BGR24; break;
		case PIX_FMT_RGB32: vi.pixel_type = VideoInfo::CS_BGR32; break;
	}

	FlipOutput ^= vi.IsYUV();

	int Resizer = ResizerNameToSWSResizer(AResizer);
	if (Resizer == 0)
		Env->ThrowError("SWScale: Invalid resizer specified (%s)", AResizer);

	if (ConvertToFormat == PIX_FMT_YUV420P && vi.height & 1)
		Env->ThrowError("SWScale: mod 2 output height required", AResizer);

	if ((ConvertToFormat == PIX_FMT_YUV420P || ConvertToFormat == PIX_FMT_YUYV422) && vi.width & 1)
		Env->ThrowError("SWScale: mod 2 output width required", AResizer);

	// may one day need a SWS_CS_DEFAULT in flags
	Context = sws_getContext(OrigWidth, OrigHeight, ConvertFromFormat, vi.width, vi.height, ConvertToFormat, GetSWSCPUFlags(Env) | Resizer, NULL, NULL, NULL);
}

SWScale::~SWScale() {
	if (Context)
		sws_freeContext(Context);
}

PVideoFrame SWScale::GetFrame(int n, IScriptEnvironment *Env) {
	PVideoFrame Src = child->GetFrame(n, Env);
	PVideoFrame Dst = Env->NewVideoFrame(vi);

	uint8_t *SrcData[3] = {(uint8_t *)Src->GetReadPtr(PLANAR_Y), (uint8_t *)Src->GetReadPtr(PLANAR_U), (uint8_t *)Src->GetReadPtr(PLANAR_V)};
	int SrcStride[3] = {Src->GetPitch(PLANAR_Y), Src->GetPitch(PLANAR_U), Src->GetPitch(PLANAR_V)};

	if (FlipOutput) {
		uint8_t *DstData[3] = {Dst->GetWritePtr(PLANAR_Y) + Dst->GetPitch(PLANAR_Y) * (Dst->GetHeight(PLANAR_Y) - 1), Dst->GetWritePtr(PLANAR_U) + Dst->GetPitch(PLANAR_U) * (Dst->GetHeight(PLANAR_U) - 1), Dst->GetWritePtr(PLANAR_V) + Dst->GetPitch(PLANAR_V) * (Dst->GetHeight(PLANAR_V) - 1)};
		int DstStride[3] = {-Dst->GetPitch(PLANAR_Y), -Dst->GetPitch(PLANAR_U), -Dst->GetPitch(PLANAR_V)};
		sws_scale(Context, SrcData, SrcStride, 0, OrigHeight, DstData, DstStride);
	} else {
		uint8_t *DstData[3] = {Dst->GetWritePtr(PLANAR_Y), Dst->GetWritePtr(PLANAR_U), Dst->GetWritePtr(PLANAR_V)};
		int DstStride[3] = {Dst->GetPitch(PLANAR_Y), Dst->GetPitch(PLANAR_U), Dst->GetPitch(PLANAR_V)};
		sws_scale(Context, SrcData, SrcStride, 0, OrigHeight, DstData, DstStride);
	}

	return Dst;
}
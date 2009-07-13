//  Copyright (c) 2007-2009 Fredrik Mellbin
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

#include <string.h>
#include <errno.h>

#include "utils.h"
#include "indexing.h"

#ifdef FFMS_USE_UTF8_PATHS
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif



// Export the array but not its data type... fun...
typedef struct CodecTags{
    char str[20];
    enum CodecID id;
} CodecTags;

extern "C" {
extern const AVCodecTag ff_codec_bmp_tags[];
extern const CodecTags ff_mkv_codec_tags[];
extern const AVCodecTag ff_codec_movvideo_tags[];
extern const AVCodecTag ff_codec_wav_tags[];
}

extern int CPUFeatures;

int GetSWSCPUFlags() {
	int Flags = 0;

	if (CPUFeatures & FFMS_CPU_CAPS_MMX)
		Flags |= SWS_CPU_CAPS_MMX;
	if (CPUFeatures & FFMS_CPU_CAPS_MMX2)
		Flags |= SWS_CPU_CAPS_MMX2;
	if (CPUFeatures & FFMS_CPU_CAPS_3DNOW)
		Flags |= SWS_CPU_CAPS_3DNOW;
	if (CPUFeatures & FFMS_CPU_CAPS_ALTIVEC)
		Flags |= SWS_CPU_CAPS_ALTIVEC;
	if (CPUFeatures & FFMS_CPU_CAPS_BFIN)
		Flags |= SWS_CPU_CAPS_BFIN;

	return Flags;
}

int GetPPCPUFlags() {
	int Flags = 0;

	if (CPUFeatures & FFMS_CPU_CAPS_MMX)
		Flags |= PP_CPU_CAPS_MMX;
	if (CPUFeatures & FFMS_CPU_CAPS_MMX2)
		Flags |= PP_CPU_CAPS_MMX2;
	if (CPUFeatures & FFMS_CPU_CAPS_3DNOW)
		Flags |= PP_CPU_CAPS_3DNOW;
	if (CPUFeatures & FFMS_CPU_CAPS_ALTIVEC)
		Flags |= PP_CPU_CAPS_ALTIVEC;

	return Flags;
}

FFMS_TrackType HaaliTrackTypeToFFTrackType(int TT) {
	switch (TT) {
		case TT_VIDEO: return FFMS_TYPE_VIDEO; break;
		case TT_AUDIO: return FFMS_TYPE_AUDIO; break;
		case TT_SUB: return FFMS_TYPE_SUBTITLE; break;
		default: return FFMS_TYPE_UNKNOWN;
	}
}

int ReadFrame(uint64_t FilePos, unsigned int &FrameSize, CompressedStream *CS, MatroskaReaderContext &Context, char *ErrorMsg, unsigned MsgSize) {
	if (CS) {
		char CSBuffer[4096];

		unsigned int DecompressedFrameSize = 0;

		cs_NextFrame(CS, FilePos, FrameSize);

		for (;;) {
			int ReadBytes = cs_ReadData(CS, CSBuffer, sizeof(CSBuffer));
			if (ReadBytes < 0) {
				snprintf(ErrorMsg, MsgSize, "Error decompressing data: %s", cs_GetLastError(CS));
				return 1;
			}
			if (ReadBytes == 0) {
				FrameSize = DecompressedFrameSize;
				return 0;
			}

			if (Context.BufferSize < DecompressedFrameSize + ReadBytes) {
				Context.BufferSize = FrameSize;
				Context.Buffer = (uint8_t *)realloc(Context.Buffer, Context.BufferSize + 16);
				if (Context.Buffer == NULL)  {
					snprintf(ErrorMsg, MsgSize, "Out of memory");
					return 2;
				}
			}

			memcpy(Context.Buffer + DecompressedFrameSize, CSBuffer, ReadBytes);
			DecompressedFrameSize += ReadBytes;
		}
	} else {
		if (fseeko(Context.ST.fp, FilePos, SEEK_SET)) {
			snprintf(ErrorMsg, MsgSize, "fseek(): %s", strerror(errno));
			return 3;
		}

		if (Context.BufferSize < FrameSize) {
			Context.BufferSize = FrameSize;
			Context.Buffer = (uint8_t *)realloc(Context.Buffer, Context.BufferSize + 16);
			if (Context.Buffer == NULL) {
				snprintf(ErrorMsg, MsgSize, "Out of memory");
				return 4;
			}
		}

		size_t ReadBytes = fread(Context.Buffer, 1, FrameSize, Context.ST.fp);
		if (ReadBytes != FrameSize) {
			if (ReadBytes == 0) {
				if (feof(Context.ST.fp)) {
					snprintf(ErrorMsg, MsgSize, "Unexpected EOF while reading frame");
					return 5;
				} else {
					snprintf(ErrorMsg, MsgSize, "Error reading frame: %s", strerror(errno));
					return 6;
				}
			} else {
				snprintf(ErrorMsg, MsgSize, "Short read while reading frame");
				return 7;
			}
			snprintf(ErrorMsg, MsgSize, "Unknown read error");
			return 8;
		}

		return 0;
	}
}

void InitNullPacket(AVPacket *pkt) {
	av_init_packet(pkt);
	pkt->data = NULL;
	pkt->size = 0;
}

void FillAP(FFAudioProperties &AP, AVCodecContext *CTX, FFTrack &Frames) {
	AP.SampleFormat = static_cast<FFMS_SampleFormat>(CTX->sample_fmt);
	AP.BitsPerSample = av_get_bits_per_sample_format(CTX->sample_fmt);
	if (CTX->sample_fmt == SAMPLE_FMT_S32)
		AP.BitsPerSample = CTX->bits_per_raw_sample;

	AP.Channels = CTX->channels;;
	AP.ChannelLayout = CTX->channel_layout;
	AP.SampleRate = CTX->sample_rate;
	AP.NumSamples = (Frames.back()).SampleStart;
	AP.FirstTime = ((Frames.front().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;
	AP.LastTime = ((Frames.back().DTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;
}

#ifdef HAALISOURCE

unsigned vtSize(VARIANT &vt) {
	if (V_VT(&vt) != (VT_ARRAY | VT_UI1))
		return 0;
	long lb,ub;
	if (FAILED(SafeArrayGetLBound(V_ARRAY(&vt),1,&lb)) ||
		FAILED(SafeArrayGetUBound(V_ARRAY(&vt),1,&ub)))
		return 0;
	return ub - lb + 1;
}

void vtCopy(VARIANT& vt,void *dest) {
	unsigned sz = vtSize(vt);
	if (sz > 0) {
		void  *vp;
		if (SUCCEEDED(SafeArrayAccessData(V_ARRAY(&vt),&vp))) {
			memcpy(dest,vp,sz);
			SafeArrayUnaccessData(V_ARRAY(&vt));
		}
	}
}

#else

// used for matroska<->ffmpeg codec ID mapping to avoid Win32 dependency
typedef struct BITMAPINFOHEADER {
        uint32_t      biSize;
        int32_t       biWidth;
        int32_t       biHeight;
        uint16_t      biPlanes;
        uint16_t      biBitCount;
        uint32_t      biCompression;
        uint32_t      biSizeImage;
        int32_t       biXPelsPerMeter;
        int32_t       biYPelsPerMeter;
        uint32_t      biClrUsed;
        uint32_t      biClrImportant;
} BITMAPINFOHEADER;

#define MAKEFOURCC(ch0, ch1, ch2, ch3)\
	((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |\
	((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))

#endif

CodecID MatroskaToFFCodecID(char *Codec, void *CodecPrivate, unsigned int FourCC) {
/* Look up native codecs */
	for(int i = 0; ff_mkv_codec_tags[i].id != CODEC_ID_NONE; i++){
		if(!strncmp(ff_mkv_codec_tags[i].str, Codec,
			strlen(ff_mkv_codec_tags[i].str))){
				return ff_mkv_codec_tags[i].id;
			}
	}

/* Video codecs for "avi in mkv" mode */
	const AVCodecTag *const tags[] = { ff_codec_bmp_tags, 0 };

	if (!strcmp(Codec, "V_MS/VFW/FOURCC")) {
		BITMAPINFOHEADER *b = reinterpret_cast<BITMAPINFOHEADER *>(CodecPrivate);
		return av_codec_get_id(tags, b->biCompression);
	}

	if (!strcmp(Codec, "V_FOURCC")) {
		return av_codec_get_id(tags, FourCC);
	}

// FIXME
/* Audio codecs for "acm in mkv" mode */
		//#include "Mmreg.h"
		//((WAVEFORMATEX *)TI->CodecPrivate)->wFormatTag

/* Fixup for uncompressed video formats */

/* Fixup for uncompressed audio formats */

	return CODEC_ID_NONE;
}

void InitializeCodecContextFromMatroskaTrackInfo(TrackInfo *TI, AVCodecContext *CodecContext) {
	CodecContext->extradata = static_cast<uint8_t *>(TI->CodecPrivate);
	CodecContext->extradata_size = TI->CodecPrivateSize;

	if (TI->Type == TT_VIDEO) {
		CodecContext->coded_width = TI->AV.Video.PixelWidth;
		CodecContext->coded_height = TI->AV.Video.PixelHeight;
	} else if (TI->Type == TT_AUDIO) {
		CodecContext->sample_rate = mkv_TruncFloat(TI->AV.Audio.SamplingFreq);
		CodecContext->bits_per_coded_sample = TI->AV.Audio.BitDepth;
		CodecContext->channels = TI->AV.Audio.Channels;
	}
}

#ifdef HAALISOURCE

void InitializeCodecContextFromHaaliInfo(CComQIPtr<IPropertyBag> pBag, AVCodecContext *CodecContext) {
	if (pBag) {
		CComVariant pV;

		pV.Clear();
		if (SUCCEEDED(pBag->Read(L"Type", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4))) {

			unsigned int TT = pV.uintVal;

			if (TT == TT_VIDEO) {

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"Video.PixelWidth", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
					CodecContext->coded_width = pV.uintVal;

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"Video.PixelHeight", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
					CodecContext->coded_height = pV.uintVal;

			} else if (TT == TT_AUDIO) {

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"Audio.SamplingFreq", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
					CodecContext->sample_rate = pV.uintVal;

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"Audio.BitDepth", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
					CodecContext->bits_per_coded_sample = pV.uintVal;

				pV.Clear();
				if (SUCCEEDED(pBag->Read(L"Audio.Channels", &pV, NULL)) && SUCCEEDED(pV.ChangeType(VT_UI4)))
					CodecContext->channels = pV.uintVal;

			}
		}
  	}
}

#endif

FILE *ffms_fopen(const char *filename, const char *mode) {
#ifdef FFMS_USE_UTF8_PATHS
	// Hack: support utf8-in-char* filenames on windows
	wchar_t filename_wide[MAX_PATH*2];
	// 64 characters of mode string ought to be more than enough for everyone
	wchar_t mode_wide[64];
	if ((MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename, -1, filename_wide, MAX_PATH) > 0)
			&& (MultiByteToWideChar(CP_ACP, NULL, mode, -1, mode_wide, 60) > 0))
		return _wfopen(filename_wide, mode_wide);
	else
		return fopen(filename, mode);
#else
	return fopen(filename, mode);
#endif
}

size_t ffms_mbstowcs (wchar_t *wcstr, const char *mbstr, size_t max) {
#ifdef FFMS_USE_UTF8_PATHS
	// try utf8 first
	int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, mbstr, -1, NULL, 0);
	if (len > 0) {
		MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, mbstr, -1, wcstr, max);
		return static_cast<size_t>(len);
	}
	// failed, use local ANSI codepage
	else {
		len = MultiByteToWideChar(CP_ACP, NULL, mbstr, -1, wcstr, max);
		return static_cast<size_t>(len);
	}
#else
	return mbstowcs(wcstr, mbstr, max);
#endif
}


// ffms_fstream stuff
void ffms_fstream::open(const char *filename, std::ios_base::openmode mode) {
#ifdef FFMS_USE_UTF8_PATHS
	// Hack: support utf8-in-char* filenames on windows
	wchar_t filename_wide[MAX_PATH*2];
	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename, -1, filename_wide, MAX_PATH) > 0)
		std::fstream::open(filename_wide, mode);
	else
		std::fstream::open(filename, mode);
#else
	std::fstream::open(filename, mode);
#endif
}

ffms_fstream::ffms_fstream(const char *filename, std::ios_base::openmode mode) {
	open(filename, mode);
}

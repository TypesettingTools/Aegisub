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

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <io.h>
#	include <fcntl.h>
extern "C" {
#	include "libavutil/avstring.h"
}
#endif // _WIN32


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
extern bool GlobalUseUTF8Paths;



FFMS_Exception::FFMS_Exception(int ErrorType, int SubType, const char *Message) : _ErrorType(ErrorType), _SubType(SubType), _Message(Message) {
}

FFMS_Exception::FFMS_Exception(int ErrorType, int SubType, const std::string &Message) : _ErrorType(ErrorType), _SubType(SubType), _Message(Message) {
}

FFMS_Exception::~FFMS_Exception() throw () {
}

const std::string &FFMS_Exception::GetErrorMessage() const {
	return _Message;
}

int FFMS_Exception::CopyOut(FFMS_ErrorInfo *ErrorInfo) const {
	if (ErrorInfo) {
		ErrorInfo->ErrorType = _ErrorType;
		ErrorInfo->SubType = _SubType;
		
		if (ErrorInfo->BufferSize > 0) {
			memset(ErrorInfo->Buffer, 0, ErrorInfo->BufferSize);
			_Message.copy(ErrorInfo->Buffer, ErrorInfo->BufferSize - 1);
		}
	}

	return (_ErrorType << 16) | _SubType;
}

TrackCompressionContext::TrackCompressionContext(MatroskaFile *MF, TrackInfo *TI, unsigned int Track) {
	CS = NULL;
	CompressedPrivateData = NULL;
	CompressedPrivateDataSize = 0;
	CompressionMethod = TI->CompMethod;

	if (CompressionMethod == COMP_ZLIB) {
		char ErrorMessage[512];
		CS = cs_Create(MF, Track, ErrorMessage, sizeof(ErrorMessage));
		if (CS == NULL) {
			std::ostringstream buf;
			buf << "Can't create MKV track decompressor: " << ErrorMessage;
			throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
		}
	} else if (CompressionMethod == COMP_PREPEND) {
		CompressedPrivateData		= TI->CompMethodPrivate;
		CompressedPrivateDataSize	= TI->CompMethodPrivateSize;
	} else {
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, 
			"Can't create MKV track decompressor: unknown or unsupported compression method");
	}
}

TrackCompressionContext::~TrackCompressionContext() {
	if (CS)
		cs_Destroy(CS);
}

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

#ifdef WITH_LIBPOSTPROC
// not exactly a pretty solution but it'll never get called anyway
	if (CPUFeatures & FFMS_CPU_CAPS_MMX)
		Flags |= PP_CPU_CAPS_MMX;
	if (CPUFeatures & FFMS_CPU_CAPS_MMX2)
		Flags |= PP_CPU_CAPS_MMX2;
	if (CPUFeatures & FFMS_CPU_CAPS_3DNOW)
		Flags |= PP_CPU_CAPS_3DNOW;
	if (CPUFeatures & FFMS_CPU_CAPS_ALTIVEC)
		Flags |= PP_CPU_CAPS_ALTIVEC;
#endif // WITH_LIBPOSTPROC

	return Flags;
}

void ClearErrorInfo(FFMS_ErrorInfo *ErrorInfo) {
	if (ErrorInfo) {
		ErrorInfo->ErrorType = FFMS_ERROR_SUCCESS;
		ErrorInfo->SubType = FFMS_ERROR_SUCCESS;

		if (ErrorInfo->BufferSize > 0)
			ErrorInfo->Buffer[0] = 0;
	}
}

FFMS_TrackType HaaliTrackTypeToFFTrackType(int TT) {
	switch (TT) {
		case TT_VIDEO: return FFMS_TYPE_VIDEO; break;
		case TT_AUDIO: return FFMS_TYPE_AUDIO; break;
		case TT_SUB: return FFMS_TYPE_SUBTITLE; break;
		default: return FFMS_TYPE_UNKNOWN;
	}
}

const char *GetLAVCSampleFormatName(AVSampleFormat s) {
	switch (s) {
		case AV_SAMPLE_FMT_U8:	return "8-bit unsigned integer";
		case AV_SAMPLE_FMT_S16:	return "16-bit signed integer";
		case AV_SAMPLE_FMT_S32:	return "32-bit signed integer";
		case AV_SAMPLE_FMT_FLT:	return "Single-precision floating point";
		case AV_SAMPLE_FMT_DBL:	return "Double-precision floating point";
		default:				return "Unknown";
	}
}

void ReadFrame(uint64_t FilePos, unsigned int &FrameSize, TrackCompressionContext *TCC, MatroskaReaderContext &Context) {
	if (TCC && TCC->CS) {
		CompressedStream *CS = TCC->CS;
		unsigned int DecompressedFrameSize = 0;

		cs_NextFrame(CS, FilePos, FrameSize);

		for (;;) {
			int ReadBytes = cs_ReadData(CS, Context.CSBuffer, sizeof(Context.CSBuffer));
			if (ReadBytes < 0) {
				std::ostringstream buf;
				buf << "Error decompressing data: " << cs_GetLastError(CS);
				throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
			}

			if (ReadBytes == 0) {
				FrameSize = DecompressedFrameSize;
				memset(Context.Buffer + DecompressedFrameSize, 0,
					Context.BufferSize  + FF_INPUT_BUFFER_PADDING_SIZE - DecompressedFrameSize);
				return;
			}

			if (Context.BufferSize < DecompressedFrameSize + ReadBytes) {
				Context.BufferSize = DecompressedFrameSize + ReadBytes;
				Context.Buffer = (uint8_t *)realloc(Context.Buffer, Context.BufferSize + FF_INPUT_BUFFER_PADDING_SIZE);
				if (Context.Buffer == NULL)
					throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_ALLOCATION_FAILED,
					"Out of memory");
			}

			memcpy(Context.Buffer + DecompressedFrameSize, Context.CSBuffer, ReadBytes);
			DecompressedFrameSize += ReadBytes;
		}
	} else {
		if (fseeko(Context.ST.fp, FilePos, SEEK_SET)) {
			std::ostringstream buf;
			buf << "fseek(): " << strerror(errno);
			throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_SEEKING, buf.str());
		}

		if (TCC && TCC->CompressionMethod == COMP_PREPEND) {
			unsigned ReqBufsize = FrameSize + TCC->CompressedPrivateDataSize + 16;
			if (Context.BufferSize < ReqBufsize) {
				Context.BufferSize = FrameSize + TCC->CompressedPrivateDataSize;
				Context.Buffer = (uint8_t *)realloc(Context.Buffer, ReqBufsize);
				if (Context.Buffer == NULL)
					throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_ALLOCATION_FAILED, "Out of memory");
			}

			/* // maybe faster? maybe not?
			for (int i=0; i < TCC->CompressedPrivateDataSize; i++)
				*(Context.Buffer)++ = ((uint8_t *)TCC->CompressedPrivateData)[i];
			*/
			// screw it, memcpy and fuck the losers who use header compression
			memcpy(Context.Buffer, TCC->CompressedPrivateData, TCC->CompressedPrivateDataSize);
		}
		else if (Context.BufferSize < FrameSize) {
			Context.BufferSize = FrameSize;
			Context.Buffer = (uint8_t *)realloc(Context.Buffer, Context.BufferSize + 16);
			if (Context.Buffer == NULL)
				throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_ALLOCATION_FAILED,
					"Out of memory");
		}

		uint8_t *TargetPtr = Context.Buffer;
		if (TCC && TCC->CompressionMethod == COMP_PREPEND)
			TargetPtr += TCC->CompressedPrivateDataSize;

		size_t ReadBytes = fread(TargetPtr, 1, FrameSize, Context.ST.fp);
		if (ReadBytes != FrameSize) {
			return;
			if (ReadBytes == 0) {
				if (feof(Context.ST.fp)) {
					throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ,
						"Unexpected EOF while reading frame");
				} else {
					std::ostringstream buf;
					buf << "Error reading frame: " << strerror(errno);
					throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_SEEKING, buf.str());
				}
			} else {
				throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ,
					"Short read while reading frame");
			}
			throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ,
				"Unknown read error");
		}

		return;
	}
}

void InitNullPacket(AVPacket &pkt) {
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
}

void FillAP(FFMS_AudioProperties &AP, AVCodecContext *CTX, FFMS_Track &Frames) {
	AP.SampleFormat = static_cast<FFMS_SampleFormat>(CTX->sample_fmt);
	AP.BitsPerSample = av_get_bits_per_sample_fmt(CTX->sample_fmt);
	if (CTX->sample_fmt == AV_SAMPLE_FMT_S32 && CTX->bits_per_raw_sample)
		AP.BitsPerSample = CTX->bits_per_raw_sample;

	AP.Channels = CTX->channels;;
	AP.ChannelLayout = CTX->channel_layout;
	AP.SampleRate = CTX->sample_rate;
	AP.NumSamples = (Frames.back()).SampleStart + (Frames.back()).SampleCount;
	AP.FirstTime = ((Frames.front().PTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;
	AP.LastTime = ((Frames.back().PTS * Frames.TB.Num) / (double)Frames.TB.Den) / 1000;
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

#endif

CodecID MatroskaToFFCodecID(char *Codec, void *CodecPrivate, unsigned int FourCC, unsigned int BitsPerSample) {
/* Look up native codecs */
	for(int i = 0; ff_mkv_codec_tags[i].id != CODEC_ID_NONE; i++){
		if(!strncmp(ff_mkv_codec_tags[i].str, Codec,
			strlen(ff_mkv_codec_tags[i].str))) {

				// Uncompressed and exotic format fixup
				// This list is incomplete
				CodecID CID = ff_mkv_codec_tags[i].id;
				switch (CID) {
					case CODEC_ID_PCM_S16LE:
						switch (BitsPerSample) {
							case 8: CID = CODEC_ID_PCM_S8; break;
							case 16: CID = CODEC_ID_PCM_S16LE; break;
							case 24: CID = CODEC_ID_PCM_S24LE; break;
							case 32: CID = CODEC_ID_PCM_S32LE; break;
						}	
						break;
					case CODEC_ID_PCM_S16BE:
						switch (BitsPerSample) {
							case 8: CID = CODEC_ID_PCM_S8; break;
							case 16: CID = CODEC_ID_PCM_S16BE; break;
							case 24: CID = CODEC_ID_PCM_S24BE; break;
							case 32: CID = CODEC_ID_PCM_S32BE; break;
						}	
						break;
					default:
						break;
				}
				
				return CID;
			}
	}

/* Video codecs for "avi in mkv" mode */
	const AVCodecTag *const tags[] = { ff_codec_bmp_tags, 0 };

	if (!strcmp(Codec, "V_MS/VFW/FOURCC")) {
		FFMS_BITMAPINFOHEADER *b = reinterpret_cast<FFMS_BITMAPINFOHEADER *>(CodecPrivate);
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


// All this filename chikanery that follows is supposed to make sure both local
// codepage (used by avisynth etc) and UTF8 (potentially used by API users) strings
// work correctly on Win32.
// It's a really ugly hack, and I blame Microsoft for it.
#ifdef _WIN32
static wchar_t *dup_char_to_wchar(const char *s, unsigned int cp) {
	wchar_t *w;
	int l;
	if (!(l = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, s, -1, NULL, 0)))
		return NULL;
	if (!(w = (wchar_t *)malloc(l * sizeof(wchar_t))))
		return NULL;
	if (MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, s, -1 , w, l) <= 0) {
		free(w);
		w = NULL;
	}
	return w;
}
#endif

FILE *ffms_fopen(const char *filename, const char *mode) {
#ifdef _WIN32
	unsigned int codepage;
	if (GlobalUseUTF8Paths)
		codepage = CP_UTF8;
	else
		codepage = CP_ACP;
	
	FILE *ret;
	wchar_t *filename_wide	= dup_char_to_wchar(filename, codepage);
	wchar_t *mode_wide		= dup_char_to_wchar(mode, codepage);
	if (filename_wide && mode_wide)
		ret = _wfopen(filename_wide, mode_wide);
	else
		ret = fopen(filename, mode);

	free(filename_wide);
	free(mode_wide);

	return ret;
#else
	return fopen(filename, mode);
#endif /* _WIN32 */
}

size_t ffms_mbstowcs (wchar_t *wcstr, const char *mbstr, size_t max) {
#ifdef _WIN32
	// this is only called by HaaliOpenFile anyway, so I think this is safe
	return static_cast<size_t>(MultiByteToWideChar((GlobalUseUTF8Paths ? CP_UTF8 : CP_ACP), MB_ERR_INVALID_CHARS, mbstr, -1, wcstr, max));
#else
	return mbstowcs(wcstr, mbstr, max);
#endif
}


// ffms_fstream stuff
void ffms_fstream::open(const char *filename, std::ios_base::openmode mode) {
	// Unlike MSVC, mingw's iostream library doesn't have an fstream overload
	// that takes a wchar_t* filename, which means you can't open unicode
	// filenames with it on Windows. gg.
#if defined(_WIN32) && !defined(__MINGW32__)
	unsigned int codepage = GlobalUseUTF8Paths ? CP_UTF8 : CP_ACP;

	wchar_t *filename_wide = dup_char_to_wchar(filename, codepage);
	if (filename_wide)
		std::fstream::open(filename_wide, mode);
	else
		std::fstream::open(filename, mode);

	free(filename_wide);
#else // defined(_WIN32) && !defined(__MINGW32__)
	std::fstream::open(filename, mode);
#endif // defined(_WIN32) && !defined(__MINGW32__)
}

ffms_fstream::ffms_fstream(const char *filename, std::ios_base::openmode mode) {
	open(filename, mode);
}


#ifdef _WIN32
int ffms_wchar_open(const char *fname, int oflags, int pmode) {
    wchar_t *wfname = dup_char_to_wchar(fname, CP_UTF8);
    if (wfname) {
        int ret = _wopen(wfname, oflags, pmode);
        free(wfname);
        return ret;
    }
    return -1;
}

static int ffms_lavf_file_open(URLContext *h, const char *filename, int flags) {
    int access;
    int fd;

    av_strstart(filename, "file:", &filename);

    if (flags & URL_RDWR) {
        access = _O_CREAT | _O_TRUNC | _O_RDWR;
    } else if (flags & URL_WRONLY) {
        access = _O_CREAT | _O_TRUNC | _O_WRONLY;
    } else {
        access = _O_RDONLY;
    }
#ifdef _O_BINARY
    access |= _O_BINARY;
#endif
    fd = ffms_wchar_open(filename, access, 0666);
    if (fd == -1)
        return AVERROR(ENOENT);
    h->priv_data = (void *) (intptr_t) fd;
    return 0;
}

// Hijack lavf's file protocol handler's open function and use our own instead.
// Hack by nielsm.
void ffms_patch_lavf_file_open() {
	extern URLProtocol *first_protocol;
	URLProtocol *proto = first_protocol;
	while (proto != NULL) {
		if (strcmp("file", proto->name) == 0) {
			break;
		}
		proto = proto->next;
	}
	if (proto != NULL) {
		proto->url_open = &ffms_lavf_file_open;
	}
}
#endif // _WIN32

// End of filename hackery.


#ifdef HAALISOURCE

CComPtr<IMMContainer> HaaliOpenFile(const char *SourceFile, enum FFMS_Sources SourceMode) {
	CComPtr<IMMContainer> pMMC;

	CLSID clsid = HAALI_MPEG_PARSER;
	if (SourceMode == FFMS_SOURCE_HAALIOGG)
		clsid = HAALI_OGG_PARSER;

	if (FAILED(pMMC.CoCreateInstance(clsid)))
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_ALLOCATION_FAILED,
			"Can't create parser");

	CComPtr<IMemAlloc> pMA;
	if (FAILED(pMA.CoCreateInstance(CLSID_MemAlloc)))
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_ALLOCATION_FAILED,
			"Can't create memory allocator");

	CComPtr<IMMStream> pMS;
	if (FAILED(pMS.CoCreateInstance(CLSID_DiskFile)))
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_ALLOCATION_FAILED,
			"Can't create disk file reader");

	WCHAR WSourceFile[2048];
	ffms_mbstowcs(WSourceFile, SourceFile, 2000);
	CComQIPtr<IMMStreamOpen> pMSO(pMS);
	if (FAILED(pMSO->Open(WSourceFile)))
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ,
			"Can't open file");

	if (FAILED(pMMC->Open(pMS, 0, NULL, pMA))) {
		if (SourceMode == FFMS_SOURCE_HAALIMPEG)
			throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_INVALID_ARGUMENT,
				"Can't parse file, most likely a transport stream not cut at packet boundaries");
		else
			throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_INVALID_ARGUMENT,
				"Can't parse file");
	}

	return pMMC;
}

#endif

void LAVFOpenFile(const char *SourceFile, AVFormatContext *&FormatContext) {
	if (av_open_input_file(&FormatContext, SourceFile, NULL, 0, NULL) != 0) {
		std::ostringstream buf;
		buf << "Couldn't open '" << SourceFile << "'";
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ, buf.str());
	}
	
	if (av_find_stream_info(FormatContext) < 0) {
		av_close_input_file(FormatContext);
		FormatContext = NULL;
		throw FFMS_Exception(FFMS_ERROR_PARSER, FFMS_ERROR_FILE_READ,
			"Couldn't find stream information");
	}
}


// attempt to correct framerate to the proper NTSC fraction, if applicable
// code stolen from Perian
void CorrectNTSCRationalFramerate(int *Num, int *Den) {
	AVRational TempFPS;
	TempFPS.den = *Num; // not a typo
	TempFPS.num = *Den; // still not a typo

	av_reduce(&TempFPS.num, &TempFPS.den, TempFPS.num, TempFPS.den, INT_MAX);

	if (TempFPS.num == 1) {
		*Num = TempFPS.den;
		*Den = TempFPS.num;
	}
	else {
		double FTimebase = av_q2d(TempFPS);
		double NearestNTSC = floor(FTimebase * 1001.0 + 0.5) / 1001.0;
		const double SmallInterval = 1.0/120.0;

		if (fabs(FTimebase - NearestNTSC) < SmallInterval) {
			*Num = int((1001.0 / FTimebase) + 0.5);
			*Den = 1001;
		}
	}
}

// correct the timebase if it is invalid
void CorrectTimebase(FFMS_VideoProperties *VP, FFMS_TrackTimeBase *TTimebase) {
	double Timebase = (double)TTimebase->Num / TTimebase->Den;
	double FPS = (double)VP->FPSNumerator / VP->FPSDenominator;
	if ((1000/Timebase) / FPS < 1) {
		TTimebase->Den = VP->FPSNumerator;
		TTimebase->Num = (int64_t)VP->FPSDenominator * 1000;
	}
}

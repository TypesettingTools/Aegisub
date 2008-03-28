//  Copyright (c) 2007-2008 Fredrik Mellbin
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

CodecID MatroskaToFFCodecID(TrackInfo *TI) {
	char *Codec = TI->CodecID;
/* Video Codecs */
	if (!strcmp(Codec, "V_MS/VFW/FOURCC")) {
		// fourcc list from ffdshow
		switch (((BITMAPINFOHEADER *)TI->CodecPrivate)->biCompression) {
			case MAKEFOURCC('F', 'F', 'D', 'S'):
			case MAKEFOURCC('F', 'V', 'F', 'W'):
			case MAKEFOURCC('X', 'V', 'I', 'D'):
			case MAKEFOURCC('D', 'I', 'V', 'X'):
			case MAKEFOURCC('D', 'X', '5', '0'):
			case MAKEFOURCC('M', 'P', '4', 'V'):
			case MAKEFOURCC('3', 'I', 'V', 'X'):
			case MAKEFOURCC('W', 'V', '1', 'F'):
			case MAKEFOURCC('F', 'M', 'P', '4'):
			case MAKEFOURCC('S', 'M', 'P', '4'):
				return CODEC_ID_MPEG4;
			case MAKEFOURCC('D', 'I', 'V', '3'):
			case MAKEFOURCC('D', 'V', 'X', '3'):
			case MAKEFOURCC('M', 'P', '4', '3'):
				return CODEC_ID_MSMPEG4V3;
			case MAKEFOURCC('M', 'P', '4', '2'):
				return CODEC_ID_MSMPEG4V2;
			case MAKEFOURCC('M', 'P', '4', '1'):
				return CODEC_ID_MSMPEG4V1;
			case MAKEFOURCC('W', 'M', 'V', '1'):
				return CODEC_ID_WMV1;
			case MAKEFOURCC('W', 'M', 'V', '2'):
				return CODEC_ID_WMV2;
			case MAKEFOURCC('W', 'M', 'V', '3'):
				return CODEC_ID_WMV3;
/*
			case MAKEFOURCC('M', 'S', 'S', '1'):
			case MAKEFOURCC('M', 'S', 'S', '2'):
			case MAKEFOURCC('W', 'V', 'P', '2'):
			case MAKEFOURCC('W', 'M', 'V', 'P'):
				return CODEC_ID_WMV9_LIB;
*/
			case MAKEFOURCC('W', 'V', 'C', '1'):
				return CODEC_ID_VC1;
			case MAKEFOURCC('V', 'P', '5', '0'):
				return CODEC_ID_VP5;
			case MAKEFOURCC('V', 'P', '6', '0'):
			case MAKEFOURCC('V', 'P', '6', '1'):
			case MAKEFOURCC('V', 'P', '6', '2'):
				return CODEC_ID_VP6;
			case MAKEFOURCC('V', 'P', '6', 'F'):
			case MAKEFOURCC('F', 'L', 'V', '4'):
				return CODEC_ID_VP6F;	
			case MAKEFOURCC('C', 'A', 'V', 'S'):
				return CODEC_ID_CAVS;	
			case MAKEFOURCC('M', 'P', 'G', '1'):
			case MAKEFOURCC('M', 'P', 'E', 'G'):
				return CODEC_ID_MPEG2VIDEO;	// not a typo
			case MAKEFOURCC('M', 'P', 'G', '2'):
			case MAKEFOURCC('E', 'M', '2', 'V'):
			case MAKEFOURCC('M', 'M', 'E', 'S'):
				return CODEC_ID_MPEG2VIDEO;
			case MAKEFOURCC('H', '2', '6', '3'):
			case MAKEFOURCC('S', '2', '6', '3'):
			case MAKEFOURCC('L', '2', '6', '3'):
			case MAKEFOURCC('M', '2', '6', '3'):
			case MAKEFOURCC('U', '2', '6', '3'):
			case MAKEFOURCC('X', '2', '6', '3'):
				return CODEC_ID_H263;
			case MAKEFOURCC('H', '2', '6', '4'):
			case MAKEFOURCC('X', '2', '6', '4'):
			case MAKEFOURCC('V', 'S', 'S', 'H'):
			case MAKEFOURCC('D', 'A', 'V', 'C'):
			case MAKEFOURCC('P', 'A', 'V', 'C'):
			case MAKEFOURCC('A', 'V', 'C', '1'):
				return CODEC_ID_H264;
			case MAKEFOURCC('M', 'J', 'P', 'G'):
			case MAKEFOURCC('L', 'J', 'P', 'G'):
			case MAKEFOURCC('M', 'J', 'L', 'S'):
			case MAKEFOURCC('J', 'P', 'E', 'G'): // questionable fourcc?
			case MAKEFOURCC('A', 'V', 'R', 'N'):
			case MAKEFOURCC('M', 'J', 'P', 'A'):
				return CODEC_ID_MJPEG;
			case MAKEFOURCC('D', 'V', 'S', 'D'):
			case MAKEFOURCC('D', 'V', '2', '5'):
			case MAKEFOURCC('D', 'V', '5', '0'):
			case MAKEFOURCC('C', 'D', 'V', 'C'):
			case MAKEFOURCC('C', 'D', 'V', '5'):
			case MAKEFOURCC('D', 'V', 'I', 'S'):
			case MAKEFOURCC('P', 'D', 'V', 'C'):
				return CODEC_ID_DVVIDEO;
			case MAKEFOURCC('H', 'F', 'Y', 'U'):
			case MAKEFOURCC('F', 'F', 'V', 'H'):
				return CODEC_ID_HUFFYUV;
			case MAKEFOURCC('C', 'Y', 'U', 'V'):
				return CODEC_ID_CYUV;
			case MAKEFOURCC('A', 'S', 'V', '1'):
				return CODEC_ID_ASV1;
			case MAKEFOURCC('A', 'S', 'V', '2'):
				return CODEC_ID_ASV2;
			case MAKEFOURCC('V', 'C', 'R', '1'):
				return CODEC_ID_VCR1;
			case MAKEFOURCC('T', 'H', 'E', 'O'):
				return CODEC_ID_THEORA;
			case MAKEFOURCC('S', 'V', 'Q', '1'):
				return CODEC_ID_SVQ1;
			case MAKEFOURCC('S', 'V', 'Q', '3'):
				return CODEC_ID_SVQ3;
			case MAKEFOURCC('R', 'P', 'Z', 'A'):
				return CODEC_ID_RPZA;
			case MAKEFOURCC('F', 'F', 'V', '1'):
				return CODEC_ID_FFV1;
			case MAKEFOURCC('V', 'P', '3', '1'):
				return CODEC_ID_VP3;
			case MAKEFOURCC('R', 'L', 'E', '8'):
				return CODEC_ID_MSRLE;
			case MAKEFOURCC('M', 'S', 'Z', 'H'):
				return CODEC_ID_MSZH;
			case MAKEFOURCC('Z', 'L', 'I', 'B'):
				return CODEC_ID_ZLIB;
			case MAKEFOURCC('F', 'L', 'V', '1'):
				return CODEC_ID_FLV1;
/*
			case MAKEFOURCC('P', 'N', 'G', '1'):
				return CODEC_ID_COREPNG;
*/
			case MAKEFOURCC('M', 'P', 'N', 'G'):
				return CODEC_ID_PNG;
/*
			case MAKEFOURCC('A', 'V', 'I', 'S'):
				return CODEC_ID_AVISYNTH;
*/
			case MAKEFOURCC('C', 'R', 'A', 'M'):
				return CODEC_ID_MSVIDEO1;
			case MAKEFOURCC('R', 'T', '2', '1'):
				return CODEC_ID_INDEO2;
			case MAKEFOURCC('I', 'V', '3', '2'):
			case MAKEFOURCC('I', 'V', '3', '1'):
				return CODEC_ID_INDEO3;
			case MAKEFOURCC('C', 'V', 'I', 'D'):
				return CODEC_ID_CINEPAK;
			case MAKEFOURCC('R', 'V', '1', '0'):
				return CODEC_ID_RV10;
			case MAKEFOURCC('R', 'V', '2', '0'):
				return CODEC_ID_RV20;
			case MAKEFOURCC('8', 'B', 'P', 'S'):
				return CODEC_ID_8BPS;
			case MAKEFOURCC('Q', 'R', 'L', 'E'):
				return CODEC_ID_QTRLE;
			case MAKEFOURCC('D', 'U', 'C', 'K'):
				return CODEC_ID_TRUEMOTION1;
			case MAKEFOURCC('T', 'M', '2', '0'):
				return CODEC_ID_TRUEMOTION2;
			case MAKEFOURCC('T', 'S', 'C', 'C'):
				return CODEC_ID_TSCC;
			case MAKEFOURCC('S', 'N', 'O', 'W'):
				return CODEC_ID_SNOW;
			case MAKEFOURCC('Q', 'P', 'E', 'G'):
			case MAKEFOURCC('Q', '1', '_', '0'):
			case MAKEFOURCC('Q', '1', '_', '1'):
				return CODEC_ID_QPEG;
			case MAKEFOURCC('H', '2', '6', '1'):
			case MAKEFOURCC('M', '2', '6', '1'):
				return CODEC_ID_H261;
			case MAKEFOURCC('L', 'O', 'C', 'O'):
				return CODEC_ID_LOCO;
			case MAKEFOURCC('W', 'N', 'V', '1'):
				return CODEC_ID_WNV1;
			case MAKEFOURCC('C', 'S', 'C', 'D'):
				return CODEC_ID_CSCD;
			case MAKEFOURCC('Z', 'M', 'B', 'V'):
				return CODEC_ID_ZMBV;
			case MAKEFOURCC('U', 'L', 'T', 'I'):
				return CODEC_ID_ULTI;
			case MAKEFOURCC('V', 'I', 'X', 'L'):
				return CODEC_ID_VIXL;
			case MAKEFOURCC('A', 'A', 'S', 'C'):
				return CODEC_ID_AASC;
			case MAKEFOURCC('F', 'P', 'S', '1'):
				return CODEC_ID_FRAPS;
			default:
				return CODEC_ID_NONE;
		}

	} else if (!strcmp(Codec, "V_MPEG4/ISO/AVC"))
		return CODEC_ID_H264;
	else if (!strcmp(Codec, "V_MPEG4/ISO/AP"))
		return CODEC_ID_MPEG4;
	else if (!strcmp(Codec, "V_MPEG4/ISO/ASP"))
		return CODEC_ID_MPEG4;
	else if (!strcmp(Codec, "V_MPEG4/ISO/SP"))
		return CODEC_ID_MPEG4;
	else if (!strcmp(Codec, "V_MPEG4/MS/V3"))
		return CODEC_ID_MSMPEG4V3;
	else if (!strcmp(Codec, "V_MPEG2"))
		return CODEC_ID_MPEG2VIDEO;
	else if (!strcmp(Codec, "V_MPEG1"))
		return CODEC_ID_MPEG2VIDEO; // still not a typo
	else if (!strcmp(Codec, "V_VC1"))
		return CODEC_ID_VC1;
	else if (!strcmp(Codec, "V_SNOW"))
		return CODEC_ID_SNOW;
	else if (!strcmp(Codec, "V_THEORA"))
		return CODEC_ID_THEORA;
	else if (!strcmp(Codec, "V_UNCOMPRESSED"))
		return CODEC_ID_NONE; // bleh
	else if (!strcmp(Codec, "V_QUICKTIME"))
		return CODEC_ID_SVQ3;
	else if (!strcmp(Codec, "V_CIPC"))
		return CODEC_ID_NONE; // don't know, don't care
	else if (!strncmp(Codec, "V_REAL/RV", 9)) {
		switch (Codec[9]) {
			case '1': 
				return CODEC_ID_RV10;
			case '2': 
				return CODEC_ID_RV20;
			case '3': 
				return CODEC_ID_RV30;
			case '4': 
				return CODEC_ID_RV40;
			default:
				return CODEC_ID_NONE;
		}
/* Audio Codecs */
	} else if (!strcmp(Codec, "A_AC3"))
		return CODEC_ID_AC3;
	else if (!strcmp(Codec, "A_EAC3"))
		return CODEC_ID_AC3;
	else if (!strcmp(Codec, "A_MPEG/L3"))
		return CODEC_ID_MP3;
	else if (!strcmp(Codec, "A_MPEG/L2"))
		return CODEC_ID_MP2;
	else if (!strcmp(Codec, "A_MPEG/L1"))
		return CODEC_ID_MP2; // correct?
	else if (!strcmp(Codec, "A_DTS"))
		return CODEC_ID_DTS;
	else if (!strcmp(Codec, "A_PCM/INT/LIT")) {
		switch (TI->AV.Audio.BitDepth) {
			case 8: return CODEC_ID_PCM_S8;
			case 16: return CODEC_ID_PCM_S16LE;
			case 24: return CODEC_ID_PCM_S24LE;
			case 32: return CODEC_ID_PCM_S32LE;
			default: return CODEC_ID_NONE;
		}
	} else if (!strcmp(Codec, "A_PCM/INT/BIG")) {
		switch (TI->AV.Audio.BitDepth) {
			case 8: return CODEC_ID_PCM_S8;
			case 16: return CODEC_ID_PCM_S16BE;
			case 24: return CODEC_ID_PCM_S24BE;
			case 32: return CODEC_ID_PCM_S32BE;
			default: return CODEC_ID_NONE;
		}
	} else if (!strcmp(Codec, "A_PCM/FLOAT/IEEE"))
		return CODEC_ID_NONE; // no float codec id?
	else if (!strcmp(Codec, "A_FLAC"))
		return CODEC_ID_FLAC;
	else if (!strcmp(Codec, "A_MPC"))
		return CODEC_ID_MUSEPACK8;
	else if (!strcmp(Codec, "A_TTA1"))
		return CODEC_ID_TTA;
	else if (!strcmp(Codec, "A_WAVPACK4"))
		return CODEC_ID_WAVPACK;
	else if (!strcmp(Codec, "A_VORBIS"))
		return CODEC_ID_VORBIS;
	else if (!strcmp(Codec, "A_REAL/14_4"))
		return CODEC_ID_RA_144;
	else if (!strcmp(Codec, "A_REAL/28_8"))
		return CODEC_ID_RA_288;
	else if (!strcmp(Codec, "A_REAL/COOK"))
		return CODEC_ID_COOK;
	else if (!strcmp(Codec, "A_REAL/SIPR"))
		return CODEC_ID_NONE; // no sipr codec id?
	else if (!strcmp(Codec, "A_REAL/ATRC"))
		return CODEC_ID_ATRAC3;
	else if (!strncmp(Codec, "A_AAC", 5))
		return CODEC_ID_AAC;
	else if (!strcmp(Codec, "A_SPEEX"))
		return CODEC_ID_SPEEX;
	else if (!strcmp(Codec, "A_QUICKTIME"))
		return CODEC_ID_NONE; // no
	else if (!strcmp(Codec, "A_MS/ACM")) {
		// nothing useful here anyway?
		//#include "Mmreg.h"
		//((WAVEFORMATEX *)TI->CodecPrivate)->wFormatTag
		return CODEC_ID_NONE;
	} else
		return CODEC_ID_NONE;
}

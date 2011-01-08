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

#ifndef FFMSCOMPAT_H
#define	FFMSCOMPAT_H

#ifdef _WIN32
#	define snprintf _snprintf
#	ifdef __MINGW32__
#		define fseeko fseeko64
#		define ftello ftello64
#	else
#		define fseeko _fseeki64
#		define ftello _ftelli64
#	endif
#endif

// Compatibility with older/newer ffmpegs
#ifdef LIBAVFORMAT_VERSION_INT
#	if (LIBAVFORMAT_VERSION_INT) < (AV_VERSION_INT(52,34,1)) 
#		define ff_codec_bmp_tags codec_bmp_tags
#		define ff_codec_movvideo_tags codec_movvideo_tags
#		define ff_codec_wav_tags codec_wav_tags
#	endif
#endif

#ifdef LIBAVCODEC_VERSION_INT
#	if (LIBAVCODEC_VERSION_INT) >= (AV_VERSION_INT(52,29,0))
#		define FFMS_HAVE_FFMPEG_COLORSPACE_INFO
#	else
#		define AVCOL_RANGE_JPEG 2
#		ifdef _MSC_VER
#			pragma message("WARNING: Your FFmpeg is too old to support reporting colorspace and luma range information. The corresponding fields of FFMS_VideoProperties will be set to 0. Please update FFmpeg to get rid of this warning.")
#		else
#			warning "Your FFmpeg is too old to support reporting colorspace and luma range information. The corresponding fields of FFMS_VideoProperties will be set to 0. Please update FFmpeg to get rid of this warning."
#		endif
#	endif
#	if (LIBAVCODEC_VERSION_INT) < (AV_VERSION_INT(52,30,2))
#		define AV_PKT_FLAG_KEY PKT_FLAG_KEY
#	endif
#	if (LIBAVCODEC_VERSION_INT) >= (AV_VERSION_INT(52,94,3)) // there are ~3 revisions where this will break but fixing that is :effort:
#		undef SampleFormat
#	else
#		define AVSampleFormat SampleFormat
#		define av_get_bits_per_sample_fmt av_get_bits_per_sample_format
#		define AV_SAMPLE_FMT_U8		SAMPLE_FMT_U8
#		define AV_SAMPLE_FMT_S16	SAMPLE_FMT_S16
#		define AV_SAMPLE_FMT_S32	SAMPLE_FMT_S32
#		define AV_SAMPLE_FMT_FLT	SAMPLE_FMT_FLT
#		define AV_SAMPLE_FMT_DBL	SAMPLE_FMT_DBL
#	endif
#endif

#ifdef LIBAVUTIL_VERSION_INT
#	if (LIBAVUTIL_VERSION_INT) < (AV_VERSION_INT(50, 8, 0))
#		define av_get_pix_fmt avcodec_get_pix_fmt
#	endif
#endif

#ifdef LIBSWSCALE_VERSION_INT
#	if (LIBSWSCALE_VERSION_INT) < (AV_VERSION_INT(0,8,0))
#		define FFMS_SWS_CONST_PARAM
#	else
#		define FFMS_SWS_CONST_PARAM const
#	endif
#endif

#endif // FFMSCOMPAT_H

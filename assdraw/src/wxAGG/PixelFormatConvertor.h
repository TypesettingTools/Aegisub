/*
* Copyright (c) 2007, ai-chan
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the ASSDraw3 Team nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY AI-CHAN ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL AI-CHAN BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WX_AGG_PIXEL_FORMAT_CONVERTOR_H
#define WX_AGG_PIXEL_FORMAT_CONVERTOR_H

#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgba.h"

namespace {

    /// Given a particular combination of channel type, bits per pixel and 
    /// channel indices, return the AGG format that matches.
    /// The actual template specializations that follow give the actual types, 
    /// and using a combination of parameters that are not listed will give 
    /// a compile-time error.
    template <typename Channel, int bitsPerPixel, int r, int g, int b, int a> 
    struct wxWidgetsToAGGHelper {
        //empty
    };

    /// 24-bit RGB 
    template <> struct wxWidgetsToAGGHelper<unsigned char, 24, 0, 1, 2, -1> {
        typedef agg::pixfmt_rgb24 format;
    };

    /// 24-bit BGR
    template <> struct wxWidgetsToAGGHelper<unsigned char, 24, 2, 1, 0, -1> {
        typedef agg::pixfmt_bgr24 format;
    };

    /// 32-bit RGB, alpha unused but stored as ARGB. 
    template <> struct wxWidgetsToAGGHelper<unsigned char, 32, 1, 2, 3, -1> {
        typedef agg::pixfmt_argb32 format;
    };

    /// 32-bit RGB, alpha unused but stored as RGBA.
    template <> struct wxWidgetsToAGGHelper<unsigned char, 32, 0, 1, 2, -1> {
        typedef agg::pixfmt_rgba32 format;
    };

    /// 32-bit BGR, alpha unused but stored as ABGR. 
    template <> struct wxWidgetsToAGGHelper<unsigned char, 32, 3, 2, 1, -1> {
        typedef agg::pixfmt_abgr32 format;
    };

    /// 32-bit BGR, alpha unused but stored as BGRA.
    template <> struct wxWidgetsToAGGHelper<unsigned char, 32, 2, 1, 0, -1> {
        typedef agg::pixfmt_bgra32 format;
    };

    /// 32-bit RGBA
    template <> struct wxWidgetsToAGGHelper<unsigned char, 32, 0, 1, 2, 3> {
        typedef agg::pixfmt_rgba32 format;
    };

    /// 32-bit BGRA
    template <> struct wxWidgetsToAGGHelper<unsigned char, 32, 2, 1, 0, 3> {
        typedef agg::pixfmt_bgra32 format;
    };

    /// 32-bit ARGB
    template <> struct wxWidgetsToAGGHelper<unsigned char, 32, 1, 2, 3, 0> {
        typedef agg::pixfmt_argb32 format;
    };

    /// 32-bit ABGR
    template <> struct wxWidgetsToAGGHelper<unsigned char, 32, 3, 2, 1, 0> {
        typedef agg::pixfmt_abgr32 format;
    };
}

namespace GUI {
    /// Convert between a wxWidgets pixel format class and an AGG pixel format class.
    /// Usage examples: 
    /// PixelFormatConvertor<wxNativePixelFormat>::AGGType or 
    /// PixelFormatConvertor<wxAlphaPixelFormat>::AGGType.
    template <typename wxWidgetsPixelFormat>
    class PixelFormatConvertor {
    public:
        typedef wxWidgetsPixelFormat wxWidgetsType;

        // Break out the wxWidgets parameters and feed to the helper class.
        typedef typename wxWidgetsToAGGHelper<typename wxWidgetsPixelFormat::ChannelType, 
                                              wxWidgetsPixelFormat::BitsPerPixel,
                                              wxWidgetsPixelFormat::RED,
                                              wxWidgetsPixelFormat::GREEN,
                                              wxWidgetsPixelFormat::BLUE,
                                              wxWidgetsPixelFormat::ALPHA>::format AGGType;
    };
}

#endif

// Copyright (c) 2006, Rodrigo Braz Monteiro
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
#include <png.h>
#include "png_wrap.h"
#include "prs_video_frame.h"


///////////////
// Constructor
PNGWrapper::PNGWrapper() {
	initialized = false;
	pos = 0;
}


//////////////
// Destructor
PNGWrapper::~PNGWrapper() {
	if (initialized) End();
}


//////////////
// Read image
void PNGWrapper::Read(PRSVideoFrame *frame) {
	// Begin
	Begin();

	// Initialize libpng structures
	png_structp png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) throw 1;
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		throw 1;
	}
	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		throw 1;
	}

	// Set jump for error handling (man, I hate this lib)
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		throw 1;
	}

	// Set data reading
	png_set_read_fn(png_ptr,this,(png_rw_ptr) memory_read_data);

	// Read data
	//png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_16, NULL);

	// Get image size
	//int w = png_get_image_width(png_ptr,info_ptr);
	//int h = png_get_image_height(png_ptr,info_ptr);

	// Read info
	png_uint_32 w, h;
	int bit_depth, color_type, interlace_type, compression_type, filter_method;
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_method);

	// Transformations
	color_type = png_get_color_type(png_ptr, info_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
    if (bit_depth > 8) png_set_strip_16(png_ptr);
	if (bit_depth < 8) png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_RGB) png_set_filler(png_ptr, 0xFFFF, PNG_FILLER_AFTER);
	png_read_update_info(png_ptr, info_ptr);
	int rowLen = png_get_rowbytes(png_ptr,info_ptr);

	// Allocate frame data
	int bpp = 4;
	frame->ownData = true;
	frame->data[0] = new char[h*rowLen];
	frame->w = rowLen/4;
	frame->h = h;
	frame->pitch = rowLen;
	frame->colorSpace = ColorSpace_RGB32;

	// Get rows
	png_bytepp row_pointers = (png_bytepp) png_malloc (png_ptr,sizeof(png_bytep) * h);
	char *dst = frame->data[0];
	for (size_t i=0;i<h;i++) row_pointers[i] = (png_bytep) (dst+rowLen*i);
	png_read_image(png_ptr, row_pointers);
	png_read_end(png_ptr, end_info);

	// Clean up
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	End();
}


//////////////
// Initialize
void PNGWrapper::Begin() {
	// Check initialization
	if (initialized) End();
	initialized = true;
}


////////////
// Clean up
void PNGWrapper::End() {
	// Check initialization
	if (!initialized) return;
	initialized = false;
}


/////////////
// Read data
void PNGWrapper::memory_read_data(png_structp png_ptr, png_bytep dstData, png_size_t length) {
	PNGWrapper *wrapper = (PNGWrapper*) png_get_io_ptr(png_ptr);
	wrapper->ReadData(dstData,length);
}

void PNGWrapper::ReadData(png_bytep dstData, png_size_t length) {
	memcpy(dstData,((char*)data)+pos,length);
	pos += (int) length;
}

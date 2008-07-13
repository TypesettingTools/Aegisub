// Copyright (c) 2007, 2points
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
#include "config.h"
#ifdef WITH_FFMPEG

#ifdef WIN32
#define __STDC_CONSTANT_MACROS 1
#include <stdint.h>
#endif /* WIN32 */

#include "dialog_progress.h"
#include "lavc_keyframes.h"

///////////////
// Constructor
LAVCKeyFrames::LAVCKeyFrames(const Aegisub::String filename) 
 : file(0), stream(0), streamN(-1) {
	// Open LAVCFile
	file = LAVCFile::Create(filename);

	// Find video stream
	for (unsigned int i = 0; i < file->fctx->nb_streams; ++i) {
		if (file->fctx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) {
			stream = file->fctx->streams[i];
			streamN = i;
			break;
		}
	}
	if (streamN == -1) throw _T("Could not find a video stream");
}

//////////////
// Destructor
LAVCKeyFrames::~LAVCKeyFrames() {
	if (file) file->Release();
}

////////////////////////////
// Parse file for keyframes
wxArrayInt LAVCKeyFrames::GetKeyFrames() {
	wxArrayInt keyframes;
	
	AVPacket packet;
	// sanity check stream duration
	if (stream->duration == AV_NOPTS_VALUE) 
		throw _T("ffmpeg keyframes reader: demuxer returned invalid stream length");
	int total_frames = stream->duration;	// FIXME: this will most likely NOT WORK for VFR files!
	register unsigned int frameN = 0;		// Number of parsed frames

	volatile bool canceled = false;
        DialogProgress *progress = new DialogProgress(NULL,_("Load keyframes"),&canceled,_("Reading keyframes from video"),0,total_frames);
        progress->Show();
	progress->SetProgress(0,1);

	while (av_read_frame(file->fctx, &packet) >= 0 && !canceled) {
		// Check if packet is part of video stream
		if (packet.stream_index == streamN) {
			// Increment number of passed frames
			++frameN;
			
			/* Might need some adjustments here, to make it
			appear as fluid as wanted. Just copied 2points thingy,
			and reduced it a bit */
			if ((frameN & (1024 - 1)) == 0)
				progress->SetProgress(frameN,total_frames);

			// Aegisub starts counting at frame 0, so the result must be
			// parsed frames - 1
			if (packet.flags == PKT_FLAG_KEY)
				keyframes.Add(frameN - 1);
		}
		av_free_packet(&packet);
	}
 	
	// Clean up progress
        if (!canceled) progress->Destroy();
        else throw wxString(_T("Keyframe loading cancelled by user"));
	
	return keyframes;
}

#endif // WITH_FFMPEG

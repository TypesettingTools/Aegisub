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


//
// This is a configuration file for the Aegisub project
//
// In order to use it, copy it as setup.h and edit anything you might want there
// DO NOT commit your personal setup.h to the repository
//



////////////////////////////////////
// Enable DirectShow Video Provider
// Requires: Win32, DirectX SDK
#define USE_DIRECTSHOW 0


///////////////////////////////////
// Enable DirectSound Audio Player
// Requires: Win32, DirectX SDK
#define USE_DIRECTSOUND 1


/////////////////////////////////
// Enable PortAudio Audio Player
// Requires: PortAudio library
#define USE_PORTAUDIO 0


////////////////////////////////
// Enable Hunspell spellchecker
#define USE_HUNSPELL 0


//////////////////////////////
// Enable LAVC video provider
// Requires: FFMPEG library
#define USE_LAVC 0


////////////////////////
// Enable PRS Exporting
// Requires: wxPNG library
#define USE_PRS 1


/////////////////////
// Enable FexTracker
// Requires: Win32, FexTracker library
#define USE_FEXTRACKER 1


// The following two are Linux-specific, so it would involve changing the makefiles
// Therefore, I haven't changed the code to make them work, yet


/////////////////
// Enable LibASS
// Requires: libass library, GNU?
#define USE_LIBASS 0


//////////////
// Enable ASA
// Requires: asa library
#define USE_ASA 0

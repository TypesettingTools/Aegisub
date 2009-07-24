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
// Precompiled Header File
//
// In order to use it, set the project to use this header as precompiled and
// insert it in every source file (under C/C++ -> Advanced -> Force Includes),
// then set stdwx.cpp to generate the precompiled header
//
// Note: make sure that you disable use of precompiled headers on md5.c and
// MatroskaParser.c, as well as any possible future .c files.
//


////////////
// C++ only
#ifdef __cplusplus

/////////
// Setup
#define WIN32_LEAN_AND_MEAN


#include "config.h"


/////////////////////
// wxWidgets headers
#include <wx/wxprec.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/tokenzr.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/sashwin.h>
#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/grid.h>
#include <wx/fontdlg.h>
#include <wx/clipbrd.h>
#include <wx/msgdlg.h>
#include <wx/stackwalk.h>
#include <wx/spinctrl.h>
#include <wx/wfstream.h>
#include <wx/tipdlg.h>
#include <wx/event.h>
#include <wx/stc/stc.h>
#include <wx/string.h>
#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <wx/glcanvas.h>



///////////////
// STD headers
#include <vector>
#include <list>
#include <map>

#endif // C++

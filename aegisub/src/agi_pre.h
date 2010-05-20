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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file agi_pre.h
/// @brief Precompiled headers include file, including all headers that should be precompiled
/// @ingroup main
///
/// In order to use it, set the project to use this header as precompiled and
/// insert it in every source file (under C/C++ -> Advanced -> Force Includes),
/// then set stdwx.cpp to generate the precompiled header
///
/// @note Make sure that you disable use of precompiled headers on md5.c and
///       MatroskaParser.c, as well as any possible future .c files.

#ifndef AGI_PRE_H

/// @brief Inclusion guard.
/// @todo Why is this even nessicary? GCC seems to include agi_pre.h twice for no reason.
#define AGI_PRE_H

// C++ only
#ifdef __cplusplus

#include "config.h"

/////////
// Setup
#define AGI_PRE

// Block msvc from complaining about not using msvc-specific versions for
// insecure C functions.
#ifdef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS_DEFINED
#else
#define _CRT_SECURE_NO_WARNINGS
#endif

///////////////
// STD headers
#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "boost/shared_ptr.hpp"

// General headers
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <iconv.h>
#include <locale.h>
#include <math.h>
#ifdef _OPENMP
// Not all compilers have <omp.h> (example: MSVC Express)
#include <omp.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#ifdef _WIN32
// "Lean and mean" causes windows.h to include less stuff, mostly rarely-used things.
// We can't build without being "lean and mean", some of the things included by it has
// macros that clash with variable names around Aegisub causing strange build errors.
#define WIN32_LEAN_AND_MEAN
// Windows.h must always be the first one, it defines a load of important things
#include <windows.h>
#include <objbase.h>
#include <mmsystem.h>
//#include <process.h> // Currently only used in audio_player_dsound2.cpp

#else

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <unistd.h>
#endif

/////////////////////
// wxWidgets headers
#include <wx/wxprec.h> // Leave this first.

// Windows
#ifdef __WXMSW__
#include <wx/msw/registry.h>
#endif

// All platforms.
#include <wx/accel.h>
#include <wx/app.h>
#include <wx/arrstr.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/choicdlg.h>
#include <wx/choice.h>
#include <wx/choicebk.h>
#include <wx/clipbrd.h>
#include <wx/colordlg.h>
#include <wx/colour.h>
#include <wx/combobox.h>
#include <wx/config.h>
#include <wx/control.h>
#include <wx/dataobj.h>
#include <wx/datetime.h>
#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/dcscreen.h>
#include <wx/dialog.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include <wx/display.h>
#include <wx/dnd.h>
#include <wx/docview.h>
#include <wx/dynarray.h>
#include <wx/event.h>
#include <wx/file.h>
#include <wx/filedlg.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/filesys.h>
#include <wx/font.h>
#include <wx/fontdlg.h>
#include <wx/fontenum.h>
#include <wx/frame.h>
#include <wx/fs_inet.h>
#include <wx/gauge.h>
#include <wx/gbsizer.h>
#include <wx/gdicmn.h>
#include <wx/glcanvas.h>
#include <wx/grid.h>
#include <wx/hashmap.h>
#include <wx/hyperlink.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/laywin.h>
#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/log.h>
#include <wx/menu.h>
#include <wx/menuitem.h>
#include <wx/mimetype.h>
#include <wx/msgdlg.h>
#include <wx/mstream.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/protocol/http.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/rawbmp.h>
#include <wx/recguard.h>
#include <wx/regex.h>
#include <wx/sashwin.h>
#include <wx/scrolbar.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/stackwalk.h>
#include <wx/statbmp.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/stc/stc.h>
#include <wx/stdpaths.h>
#include <wx/stopwatch.h>
#include <wx/strconv.h>
#include <wx/string.h>
#include <wx/sysopt.h>
#include <wx/textctrl.h>
#include <wx/textfile.h>
#include <wx/tglbtn.h>
#include <wx/thread.h>
#include <wx/timer.h>
#include <wx/tipdlg.h>
#include <wx/tokenzr.h>
#include <wx/toolbar.h>
#include <wx/treebook.h>
#include <wx/txtstrm.h>
#include <wx/utils.h>
#include <wx/validate.h>
#include <wx/valtext.h>
#include <wx/wfstream.h>
#include <wx/window.h>
#include <wx/xml/xml.h>
#include <wx/zipstrm.h>

#ifdef HAVE_APPLE_OPENGL_FRAMEWORK
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#ifdef _CRT_SECURE_NO_WARNINGS_DEFINED
#undef _CRT_SECURE_NO_WARNINGS
#endif

///////////////////
// Aegisub headers
#include "include/aegisub/exception.h"

#endif // C++

#endif // AGI_PRE_H

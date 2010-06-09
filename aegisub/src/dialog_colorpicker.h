// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file dialog_colorpicker.h
/// @see dialog_colorpicker.cpp
/// @ingroup tools_ui
///

#ifndef AGI_PRE
#include <wx/colour.h>
#endif

/// Callback function for GetColorFromUser
typedef void (*ColorCallback)(void* userdata, wxColor color);

/// Wrapper used by templated version of GetColorFromUser
template<class T, void (T::*method)(wxColor)>
void ColorCallbackWrapper(void* obj, wxColor color) {
	(static_cast<T*>(obj)->*method)(color);
}

/// @brief Get a color from the user via a color picker dialog
/// @param parent Parent window
/// @param original Initial color to select
/// @param callback Function called whenever the selected color changes if not NULL
/// @param userdata Passed to callback function
/// @return Last selected color when dialog is closed, or original if the dialog was cancelled
wxColor GetColorFromUser(wxWindow* parent, wxColor original, ColorCallback callback = NULL, void* userdata = NULL);

/// @brief Get a color from the user via a color picker dialog
/// @param T Class which the callback method belongs to
/// @param method Callback method
/// @param parent Parent window
/// @param original Initial color to select
/// @param callbackObj Object to call callback method on. Must be of type T.
/// @return Last selected color when dialog is closed, or original if the dialog was cancelled
template<class T, void (T::*method)(wxColor)>
wxColor GetColorFromUser(wxWindow* parent, wxColor original, T* callbackObj) {
	return GetColorFromUser(parent, original, &ColorCallbackWrapper<T, method>, callbackObj);
}

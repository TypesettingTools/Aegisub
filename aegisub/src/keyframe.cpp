// Copyright (c) 2007, Alysson Souza e Silva
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

/// @file keyframe.cpp
/// @brief Read and store video keyframe data
/// @ingroup video_input
///

///////////
// Headers
#include "config.h"

#include <wx/msgdlg.h>
#include "video_context.h"
#include "keyframe.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "options.h"
#include "vfr.h"



/// @brief Load Keyframes 
/// @param filename 
///
void KeyFrameFile::Load(wxString filename) {
	// Load
	try {
		// Open file
		wxArrayInt keyFrames;
		keyFrames.Empty();
		TextFileReader file(filename,_T("ASCII"));

		wxString cur = file.ReadLineFromFile();
		// Detect type (Only Xvid, DivX, x264 and Aegisub's keyframe files are currently supported)
		if (cur == _T("# keyframe format v1")) { OpenAegiKeyFrames(file, keyFrames); }
		else if (cur.StartsWith(_T("# XviD 2pass stat file"))) { OpenXviDKeyFrames(file, keyFrames); }
		else if (cur.StartsWith(_T("##map version"))) { OpenDivXKeyFrames(file, keyFrames); }
		else if (cur.StartsWith(_T("#options:"))) { Openx264KeyFrames(file, keyFrames); }
		else { throw(_T("Invalid or unsupported keyframes file.")); }

		// Set keyframes
		VideoContext::Get()->SetOverKeyFrames(keyFrames);
		VideoContext::Get()->SetKeyFramesName(filename);

		// Add to recent
		Options.AddToRecentList(filename,_T("Recent keyframes"));
	}
	// Fail
	catch (const wchar_t *error) {
		wxString err(error);
		wxMessageBox(err, _T("Error opening keyframes file"), wxOK | wxICON_ERROR, NULL);
	}
	catch (...) {
		wxMessageBox(_T("Unknown error"), _T("Error opening keyframes file"), wxOK | wxICON_ERROR, NULL);
	}
}



/// @brief Save Keyframes 
/// @param filename 
///
void KeyFrameFile::Save(wxString filename) {
	// Get keyframes
	wxArrayInt keyFrames = VideoContext::Get()->GetKeyFrames();

	// Write header
	TextFileWriter file(filename,_T("ASCII"));
	file.WriteLineToFile(_T("# keyframe format v1"));
	file.WriteLineToFile(wxString::Format(_T("fps %f"),VideoContext::Get()->GetFPS()));

	// Write keyframes
	for (unsigned int i=0;i<keyFrames.Count();i++) {
		file.WriteLineToFile(wxString::Format(_T("%i"),keyFrames[i]));
	}

	// Add to recent
	Options.AddToRecentList(filename,_T("Recent keyframes"));
}



/// @brief Aegisub keyframes file 
/// @param file      
/// @param keyFrames 
///
void KeyFrameFile::OpenAegiKeyFrames(TextFileReader& file, wxArrayInt& keyFrames)
{
	double fps;
	wxString cur = file.ReadLineFromFile();

	// Read header
	if (cur.Left(4) != _T("fps ")) throw _T("Invalid keyframes file, missing FPS.");
	cur = cur.Mid(4);
	cur.ToDouble(&fps);
	if (fps == 0.0) throw _T("Invalid FPS.");

	// Set FPS
	if (!VideoContext::Get()->IsLoaded()) {
		VideoContext::Get()->SetFPS(fps);
		VFR_Input.SetCFR(fps);
		if (!VFR_Output.IsLoaded()) VFR_Output.SetCFR(fps);
	}

	// Read lines
	while (file.HasMoreLines()) {
		cur = file.ReadLineFromFile();
		if (!cur.IsEmpty() && !cur.StartsWith(_T("#")) && cur.IsNumber()) {
			long temp;
			cur.ToLong(&temp);
			keyFrames.Add(temp);
		}
	}	
}



/// @brief XviD stats file 
/// @param file      
/// @param keyFrames 
///
void KeyFrameFile::OpenXviDKeyFrames(TextFileReader& file, wxArrayInt& keyFrames)
{
	wxString cur = file.ReadLineFromFile();
	unsigned int count = 0;

	// Read lines
	while (file.HasMoreLines()) {
		if (cur.StartsWith(_T("i"))) {			
			keyFrames.Add(count);
			count++;
		}
		else if (cur.StartsWith(_T("p")) || cur.StartsWith(_T("b"))) {
			count++;
		}
		cur = file.ReadLineFromFile();
	}
}


/// @brief DivX stats file 
/// @param file      
/// @param keyFrames 
///
void KeyFrameFile::OpenDivXKeyFrames(TextFileReader& file, wxArrayInt& keyFrames)
{
	wxString cur = file.ReadLineFromFile();
	unsigned int count = 0;

	// Read lines
	while (file.HasMoreLines())
	{
		if (cur.Contains(_T("I"))) {
			keyFrames.Add(count);
			count++;
		}
		else if (cur.Contains(_T("P")) || cur.Contains(_T("B"))) {
			count++;
		}
		cur = file.ReadLineFromFile();
	}
}


/// @brief x264 stats file 
/// @param file      
/// @param keyFrames 
///
void KeyFrameFile::Openx264KeyFrames(TextFileReader& file, wxArrayInt& keyFrames)
{
	wxString cur = file.ReadLineFromFile();
	unsigned int count = 0;
	size_t pos;

	// Read lines
	while (file.HasMoreLines())
	{
		pos = cur.Find(_T("type:"));
		if (cur.Mid(pos,6).Right(1).Lower() == (_T("i"))) {
			keyFrames.Add(count);
			count++;
		}
		else if (cur.Mid(pos,6).Right(1).Lower() == (_T("p")) || cur.Mid(pos,6).Right(1).Lower() == (_T("b"))) {
			count++;
		}
		cur = file.ReadLineFromFile();
	}
}



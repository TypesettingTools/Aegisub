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

/// @file subtitle_format.h
/// @see subtitle_format.cpp
/// @ingroup subtitle_io
///

#pragma once

#ifndef AGI_PRE
#include <list>

#include <wx/arrstr.h>
#include <wx/string.h>
#endif

#include <libaegisub/exception.h>

class AssEntry;
class AssFile;
class FractionalTime;

/// DOCME
/// @class SubtitleFormat
/// @brief DOCME
///
/// DOCME
class SubtitleFormat {
	wxString name;

	/// Get this format's wildcards for a load dialog
	virtual wxArrayString GetReadWildcards() const { return wxArrayString(); }
	/// Get this format's wildcards for a save dialog
	virtual wxArrayString GetWriteWildcards() const { return wxArrayString(); }

	/// List of loaded subtitle formats
	static std::list<SubtitleFormat*> formats;

protected:
	typedef std::list<AssEntry*> LineList;

	/// Strip override tags
	void StripTags(LineList &lines) const;
	/// Convert newlines to the specified character(s)
	/// @param lineEnd newline character(s)
	/// @param mergeLineBreaks Should multiple consecutive line breaks be merged into one?
	void ConvertNewlines(LineList &lines, wxString const& newline, bool mergeLineBreaks = true) const;
	/// Remove All commented and empty lines
	void StripComments(LineList &lines) const;
	/// Remove everything but the dialogue lines
	void StripNonDialogue(LineList &lines) const;
	/// @brief Split and merge lines so there are no overlapping lines
	///
	/// Algorithm described at http://devel.aegisub.org/wiki/Technical/SplitMerge
	void RecombineOverlaps(LineList &lines) const;
	/// Merge sequential identical lines
	void MergeIdentical(LineList &lines) const;

	/// Prompt the user for a framerate to use
	/// @param showSMPTE Include SMPTE as an option?
	FractionalTime AskForFPS(bool showSMPTE=false) const;

public:
	/// Constructor
	/// @param Subtitle format name
	/// @note Automatically registers the format
	SubtitleFormat(wxString const& name);
	/// Destructor
	/// @note Automatically unregisters the format
	virtual ~SubtitleFormat();

	/// Get this format's name
	wxString GetName() const { return name; }

	/// @brief Check if the given file can be read by this format
	///
	/// Default implementation simply checks if the file's extension is in the
	/// format's wildcard list
	virtual bool CanReadFile(wxString const& filename) const;

	/// @brief Check if the given file can be written by this format
	///
	/// Default implementation simply checks if the file's extension is in the
	/// format's wildcard list
	virtual bool CanWriteFile(wxString const& filename) const;

	/// @brief Check if the given subtitles can be losslessly written by this format
	///
	/// Default implementation rejects files with attachments, non-default
	/// styles, and any overrides
	virtual bool CanSave(const AssFile *file) const;

	/// Load a subtitle file
	/// @param[out] target Destination to read lines into
	/// @param filename File to load
	/// @param forceEncoding Encoding to use or empty string for default
	virtual void ReadFile(AssFile *target, wxString const& filename, wxString const& forceEncoding="") const { }

	/// Save a subtitle file
	/// @param src Data to write
	/// @param filename File to write to
	/// @param forceEncoding Encoding to use or empty string for default
	virtual void WriteFile(const AssFile *src, wxString const& filename, wxString const& encoding="") const { }

	/// Get the wildcards for a save or load dialog
	/// @param mode 0: load 1: save
	static wxString GetWildcards(int mode);

	/// Get a subtitle format that can read the given file or NULL if none can
	static const SubtitleFormat *GetReader(wxString const& filename);
	/// Get a subtitle format that can write the given file or NULL if none can
	static const SubtitleFormat *GetWriter(wxString const& filename);
	/// Initialize subtitle formats
	static void LoadFormats();
	/// Deinitialize subtitle formats
	static void DestroyFormats();
};

DEFINE_SIMPLE_EXCEPTION(SubtitleFormatParseError, agi::InvalidInputException, "subtitle_io/parse/generic")

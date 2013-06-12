// Copyright (c) 2005, Rodrigo Braz Monteiro
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

/// @file ass_entry.h
/// @see ass_entry.cpp
/// @ingroup subs_storage
///

#pragma once

#include <boost/intrusive/list_hook.hpp>
#include <string>

enum class AssEntryGroup {
	INFO = 0,
	STYLE,
	FONT,
	GRAPHIC,
	DIALOGUE,
	GROUP_MAX
};

class AssEntry : public boost::intrusive::make_list_base_hook<boost::intrusive::link_mode<boost::intrusive::auto_unlink>>::type {
public:
	virtual ~AssEntry() { }

	/// Create a copy of this entry
	virtual AssEntry *Clone() const=0;

	/// Section of the file this entry belongs to
	virtual AssEntryGroup Group() const=0;

	/// ASS or SSA Section header for this entry's group
	std::string const& GroupHeader(bool ssa=false) const;

	/// @brief Get this line's raw entry data in ASS format
	virtual const std::string GetEntryData() const=0;

	/// Get this line in SSA format
	virtual std::string GetSSAText() const { return GetEntryData(); }
};

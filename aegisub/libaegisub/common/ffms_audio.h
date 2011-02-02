// Copyright (c) 2008-2009, Karl Blomster <thefluff@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @file ffms_audio.h
/// @brief FFmpegSource Audio support.
/// @ingroup fmms audio

#include "../../libffms/include/ffms.h"
#include <libaegisub/exception.h>

#ifndef AGI_PRE
#ifdef WIN32
#include <objbase.h>
#endif

#include <map>
#endif

namespace ffms {

/// @class Audio
/// Audio file support.
class Audio {
    FFMS_AudioSource *AudioSource;  ///< audio source object
    bool COMInited;                 ///< COM initialization state

    mutable char FFMSErrMsg[1024];          ///< FFMS error message
    mutable FFMS_ErrorInfo ErrInfo;         ///< FFMS error codes/messages

    void Close();
    void LoadAudio(std::string filename);

    Audio(std::string filename);
    virtual ~Audio();

    /// @brief Checks sample endianness
    /// @return Returns true.
    /// FFMS always delivers native endian samples.
    bool AreSamplesNativeEndian() const { return true; }
    bool NeedsCache() const { return true; }

    virtual void GetAudio(void *buf, int64_t start, int64_t count) const;

};

} // namespace ffms

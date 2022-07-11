//  Copyright (c) 2008-2009 Niels Martin Hansen
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


/// @file util_osx.h
/// @brief OSX Utilities
/// @ingroup libaegisub osx
///
/// Utility functions for running regular *NIX libraries inside application
/// bundles on Apple Macintosh OS X.
///
/// The GetBundle*Directory functions return the paths of directories inside
/// the appliaction bundle where the application can store static data and
/// shared libraries for its own use.
/// (The bundle and its contents should be considered read-only.)
///
/// When linking with this library, be sure to add '-framework CoreFoundation'
/// to the GCC commandline.

#include <string>

namespace agi {
	namespace osx {
		class AppNapDisabler {
			void *handle;
		public:
			AppNapDisabler(std::string reason);
			~AppNapDisabler();
		};
	}
    namespace util {
/// @brief Get the esources directory.
/// @return Resources directory.
///
/// Mainly for user interface elements such as graphics and strings
std::string GetBundleResourcesDirectory();

/// @brief Get the shared support directory
/// @return Shared support directory
///
/// This is a suitable location for static configuration files. (Remember,
/// bundle is considered read-only.)
std::string GetBundleSharedSupportDirectory();

std::string GetApplicationSupportDirectory();
    } // namespace util
} // namespace agi

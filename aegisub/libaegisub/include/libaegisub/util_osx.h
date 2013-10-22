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
			AppNapDisabler(std::string const& reason);
			~AppNapDisabler();
		};
	}
    namespace util {
/// @brief Get the full name of bundle.
///  @return Full name of bundle.
///  Get the full name of the bundle itself.
///
///  @warning May return "" if the current executable is not inside a bundle.
std::string GetBundlePath();

/// @brief Get the esources directory.
/// @return Resources directory.
///
/// Mainly for user interface elements such as graphics and strings
std::string GetBundleResourcesDirectory();

/// @brief Get the built-in plugins directory.
/// @return Built-in plugins directory.
///
/// This is generaly only used by native Carbon and Cocoa applications. It is
/// not for general shared libraries.
std::string GetBundleBuiltInPlugInsDirectory();

/// @brief Get the private Frameworks directory.
/// @return Private Framework directory.
///
/// These are suitable locations for shared libraries.
std::string GetBundlePrivateFrameworksDirectory();

/// @brief Get the shared Frameworks directory.
/// @return Shared Framework directory.
///
/// @see GetBundlePrivateFrameworksDirectory()
/// @note Does anyone know the difference between private and shared frameworks
///       inside a bundle?
std::string GetBundleSharedFrameworksDirectory();

/// @brief Get the shared support directory
/// @return Shared support directory
///
/// This is a suitable location for static configuration files. (Remember,
/// bundle is considered read-only.)
std::string GetBundleSharedSupportDirectory();

/// @brief Get the main executable path.
/// @return Main executable path.
///
/// The binary run when the user launches the bundle from Finder.
std::string GetBundleExecutablePath();

/// @brief Get the auxillary executable path.
/// @return Auxillary executable path.
///
/// Pass the basename of the executable to get the path.
std::string GetBundleAuxillaryExecutablePath(std::string const& executableName);

std::string GetApplicationSupportDirectory();

/// @brief Open a URI using the Launcher.
/// @param location URI of file
/// @note If this is a FILE or DIRECTORY the path must be ABSOLUTE no 'file://'
void OpenLocation(std::string const& location);
    } // namespace util
} // namespace agi

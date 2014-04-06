// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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
// Aegisub Project http://www.aegisub.org/

#include <libaegisub/exception.h>
#include <libaegisub/fs_fwd.h>

#include <boost/filesystem/path.hpp>
#include <cstdint>
#include <ctime>
#include <iterator>
#include <memory>
#include <string>

#undef CreateDirectory

namespace agi {
	namespace fs {
		/// Define a filesystem error which takes a path or a string
#define DEFINE_FS_EXCEPTION(type, base, message) \
		struct type : public base { \
			type(path const& p) : base(message + p.string()) { } \
			type(std::string const& s) : base(s) { } \
			const char *GetName() const { return ""; } \
			Exception *Copy() const { return new type(*this); } \
		}

		/// @class agi::FileSystemError
		/// @extends agi::Exception
		/// @brief Base class for errors related to the file system
		///
		/// This base class can not be instantiated.
		/// File system errors do not support inner exceptions, as they
		/// are always originating causes for errors.
		DEFINE_BASE_EXCEPTION_NOINNER(FileSystemError, Exception)

		/// A file can't be accessed for some reason
		DEFINE_FS_EXCEPTION(FileNotAccessible, FileSystemError, "File is not accessible: ");

		/// A file can't be accessed because there's no file by the given name
		DEFINE_FS_EXCEPTION(FileNotFound, FileNotAccessible, "File not found: ");

		/// An error of some unknown type has occured
		DEFINE_SIMPLE_EXCEPTION_NOINNER(FileSystemUnknownError, FileSystemError, "filesystem/unknown");

		/// The path exists, but isn't a file
		DEFINE_FS_EXCEPTION(NotAFile, FileNotAccessible, "Path is not a file (and should be): ");

		/// The path exists, but isn't a directory
		DEFINE_FS_EXCEPTION(NotADirectory, FileNotAccessible, "Path is not a directory (and should be): ");

		/// The given path is too long for the filesystem
		DEFINE_FS_EXCEPTION(PathTooLog, FileSystemError, "Path is too long: ");

		/// Insufficient free space to complete operation
		DEFINE_FS_EXCEPTION(DriveFull, FileSystemError, "Insufficient free space to write file: ");

		/// Base class for access denied errors
		DEFINE_FS_EXCEPTION(AccessDenied, FileNotAccessible, "Access denied to path: ");

		/// Trying to read the file gave an access denied error
		DEFINE_FS_EXCEPTION(ReadDenied, AccessDenied, "Access denied when trying to read: ");

		/// Trying to write the file gave an access denied error
		DEFINE_FS_EXCEPTION(WriteDenied, AccessDenied, "Access denied when trying to write: ");

		/// File exists and cannot be overwritten due to being read-only
		DEFINE_FS_EXCEPTION(ReadOnlyFile, WriteDenied, "File is read-only: ");

		bool Exists(path const& p);
		bool FileExists(path const& file);
		bool DirectoryExists(path const& dir);

		/// Get the local-charset encoded shortname for a file
		///
		/// This is purely for compatibility with external libraries which do
		/// not support unicode filenames on Windows. On all other platforms,
		/// it is a no-op.
		std::string ShortName(path const& file_path);

		/// Check for amount of free space on a path
		uintmax_t FreeSpace(path const& dir_path);

		/// Get the size in bytes of the file at path
		///
		/// @throws agi::FileNotFound if path does not exist
		/// @throws agi::acs::NotAFile if path is a directory
		/// @throws agi::acs::Read if path exists but could not be read
		uintmax_t Size(path const& file_path);

		/// Get the modification time of the file at path
		///
		/// @throws agi::FileNotFound if path does not exist
		/// @throws agi::acs::NotAFile if path is a directory
		/// @throws agi::acs::Read if path exists but could not be read
		time_t ModifiedTime(path const& file_path);

		/// Create a directory and all required intermediate directories
		/// @throws agi::acs::Write if the directory could not be created.
		///
		/// Trying to create a directory which already exists is not an error.
		bool CreateDirectory(path const& dir_path);

		/// Touch the given path
		///
		/// Creates the file if it does not exist, or updates the modified
		/// time if it does
		void Touch(path const& file_path);

		/// Rename a file or directory
		/// @param from Source path
		/// @param to   Destination path
		void Rename(path const& from, path const& to);

		/// Copy a file
		/// @param from Source path
		/// @param to   Destination path
		///
		/// The destination path will be created if it does not exist.
		void Copy(path const& from, path const& to);

		/// Delete a file
		/// @param path Path to file to delete
		/// @throws agi::FileNotAccessibleError if file exists but could not be deleted
		bool Remove(path const& file);

		/// Check if the file has the given extension
		/// @param p Path to check
		/// @param ext Case-insensitive extension, without leading dot
		bool HasExtension(path const& p, std::string const& ext);

		agi::fs::path Canonicalize(agi::fs::path const& path);

		class DirectoryIterator {
			struct PrivData;
			std::shared_ptr<PrivData> privdata;
			std::string value;
		public:
			typedef path value_type;
			typedef path* pointer;
			typedef path& reference;
			typedef size_t difference_type;
			typedef std::forward_iterator_tag iterator_category;

			bool operator==(DirectoryIterator const&) const;
			bool operator!=(DirectoryIterator const& rhs) const { return !(*this == rhs); }
			DirectoryIterator& operator++();
			std::string const& operator*() const { return value; }

			DirectoryIterator(path const& p, std::string const& filter);
			DirectoryIterator();
			~DirectoryIterator();

			template<typename T> void GetAll(T& cont);
		};

		inline DirectoryIterator& begin(DirectoryIterator &it) { return it; }
		inline DirectoryIterator end(DirectoryIterator &) { return DirectoryIterator(); }

		template<typename T>
		inline void DirectoryIterator::GetAll(T& cont) {
			copy(*this, end(*this), std::back_inserter(cont));
		}
	}
}

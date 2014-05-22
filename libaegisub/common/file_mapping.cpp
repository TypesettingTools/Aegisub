// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "libaegisub/file_mapping.h"

#include "libaegisub/fs.h"
#include "libaegisub/make_unique.h"
#include "libaegisub/util.h"

#include <boost/filesystem/path.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <limits>

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace boost::interprocess;

namespace {
char *map(int64_t s_offset, uint64_t length, boost::interprocess::mode_t mode,
	uint64_t file_size, agi::file_mapping const& file,
	std::unique_ptr<mapped_region>& region, uint64_t& mapping_start)
{
	static char dummy = 0;
	if (length == 0) return &dummy;

	auto offset = static_cast<uint64_t>(s_offset);
	if (offset + length > file_size)
		throw agi::InternalError("Attempted to map beyond end of file", nullptr);

	// Check if we can just use the current mapping
	if (region && offset >= mapping_start && offset + length <= mapping_start + region->get_size())
		return static_cast<char *>(region->get_address()) + offset - mapping_start;

	if (sizeof(size_t) == 4) {
		mapping_start = offset & ~0xFFFFFULL; // Align to 1 MB bondary
		length += static_cast<size_t>(offset - mapping_start);
		// Map 16 MB or length rounded up to the next MB
		length = std::min<uint64_t>(std::max<uint64_t>(0x1000000U, (length + 0xFFFFF) & ~0xFFFFF), file_size - mapping_start);
	}
	else {
		// Just map the whole file
		mapping_start = 0;
		length = file_size;
	}

	if (length > std::numeric_limits<size_t>::max())
		throw std::bad_alloc();

	try {
		region = agi::make_unique<mapped_region>(file, mode, mapping_start, static_cast<size_t>(length));
	}
	catch (interprocess_exception const&) {
		throw agi::fs::FileSystemUnknownError("Failed mapping a view of the file");
	}

	return static_cast<char *>(region->get_address()) + offset - mapping_start;
}

}

namespace agi {
file_mapping::file_mapping(fs::path const& filename, bool temporary)
#ifdef _WIN32
: handle(CreateFileW(filename.wstring().c_str(),
	temporary ? read_write : read_only,
	temporary ? FILE_SHARE_READ | FILE_SHARE_WRITE : FILE_SHARE_READ,
	nullptr,
	temporary ? OPEN_ALWAYS : OPEN_EXISTING,
	0, 0))
{
	if (handle == ipcdetail::invalid_file()) {
		switch (GetLastError()) {
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
			throw fs::FileNotFound(filename);
		case ERROR_ACCESS_DENIED:
			throw fs::ReadDenied(filename);
		default:
			throw fs::FileSystemUnknownError(util::ErrorString(GetLastError()));
		}
	}
#else
: handle(temporary
	? ipcdetail::create_or_open_file(filename.string().c_str(), read_write)
	: ipcdetail::open_existing_file(filename.string().c_str(), read_only))
{
	if (handle == ipcdetail::invalid_file()) {
		switch (errno) {
		case ENOENT:
			throw fs::FileNotFound(filename);
		case EACCES:
			throw fs::ReadDenied(filename);
		case EIO:
			throw fs::FileSystemUnknownError("Fatal I/O opening path: " + filename.string());
		}
	}
#endif
}

file_mapping::~file_mapping() {
	if (handle != ipcdetail::invalid_file()) {
		ipcdetail::close_file(handle);
	}
}

read_file_mapping::read_file_mapping(fs::path const& filename)
: file(filename, false)
{
	offset_t size = 0;
	ipcdetail::get_file_size(file.get_mapping_handle().handle, size);
	file_size = static_cast<uint64_t>(size);
}

read_file_mapping::~read_file_mapping() { }

const char *read_file_mapping::read() {
	return read(0, size());
}

const char *read_file_mapping::read(int64_t offset, uint64_t length) {
	return map(offset, length, read_only, file_size, file, region, mapping_start);
}

temp_file_mapping::temp_file_mapping(fs::path const& filename, uint64_t size)
: file(filename, true)
, file_size(size)
{
	auto handle = file.get_mapping_handle().handle;
#ifdef _WIN32
	LARGE_INTEGER li;
	li.QuadPart = size;
	SetFilePointerEx(handle, li, nullptr, FILE_BEGIN);
	SetEndOfFile(handle);
#else
	unlink(filename.string().c_str());
	if (ftruncate(handle, size) == -1) {
		switch (errno) {
		case EBADF:  throw InternalError("Error opening file " + filename.string() + " not handled", nullptr);
		case EFBIG:  throw fs::DriveFull(filename);
		case EINVAL: throw InternalError("File opened incorrectly: " + filename.string(), nullptr);
		case EROFS:  throw fs::WriteDenied(filename);
		default: throw fs::FileSystemUnknownError("Unknown error opening file: " + filename.string());
		}
	}
#endif
}

temp_file_mapping::~temp_file_mapping() { }

const char *temp_file_mapping::read(int64_t offset, uint64_t length) {
	return map(offset, length, read_only, file_size, file, read_region, read_mapping_start);
}

char *temp_file_mapping::write(int64_t offset, uint64_t length) {
	return map(offset, length, read_write, file_size, file, write_region, write_mapping_start);
}
}

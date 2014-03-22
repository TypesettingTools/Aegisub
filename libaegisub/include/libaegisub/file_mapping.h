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

#include <libaegisub/fs_fwd.h>

#include <boost/interprocess/detail/os_file_functions.hpp>
#include <cstdint>

namespace agi {
	// boost::interprocess::file_mapping is awesome and uses CreateFileA on Windows
	class file_mapping {
		boost::interprocess::file_handle_t handle;

	public:
		file_mapping(fs::path const& filename, bool temporary);
		~file_mapping();
		boost::interprocess::mapping_handle_t get_mapping_handle() const {
			return boost::interprocess::ipcdetail::mapping_handle_from_file_handle(handle);
		}
	};

	class read_file_mapping {
		file_mapping file;
		std::unique_ptr<boost::interprocess::mapped_region> region;
		uint64_t mapping_start = 0;
		uint64_t file_size = 0;

	public:
		read_file_mapping(fs::path const& filename);
		~read_file_mapping();

		uint64_t size() const { return file_size; }
		const char *read(int64_t offset, uint64_t length);
		const char *read(); // Map the entire file
	};

	class temp_file_mapping {
		file_mapping file;
		std::unique_ptr<boost::interprocess::mapped_region> region;
		uint64_t mapping_start = 0;
		uint64_t file_size = 0;

	public:
		temp_file_mapping(fs::path const& filename, uint64_t size);
		~temp_file_mapping();

		const char *read(int64_t offset, uint64_t length);
		char *write(int64_t offset, uint64_t length);
	};
}

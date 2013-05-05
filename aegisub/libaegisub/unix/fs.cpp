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

#include "config.h"

#include "libaegisub/access.h"
#include "libaegisub/fs.h"
#include "libaegisub/io.h"

#include <boost/filesystem.hpp>
#include <fcntl.h>
#include <fnmatch.h>
#include <sys/time.h>

namespace bfs = boost::filesystem;

namespace agi { namespace fs {
std::string ShortName(path const& p) {
	return p.string();
}

void Touch(path const& file) {
	CreateDirectory(file.parent_path());

	int fd = open(file.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0644);
	if (fd >= 0) {
		futimes(fd, nullptr);
		close(fd);
	}
}

void Copy(fs::path const& from, fs::path const& to) {
	acs::CheckFileRead(from);
	CreateDirectory(to.parent_path());
	acs::CheckDirWrite(to.parent_path());

	std::unique_ptr<std::istream> in(io::Open(from, true));
	io::Save(to).Get() << in->rdbuf();
}

struct DirectoryIterator::PrivData {
	boost::system::error_code ec;
	bfs::directory_iterator it;
	std::string filter;
	PrivData(path const& p, std::string const& filter) : it(p, ec), filter(filter) { }

	bool bad() const {
		return
			it == bfs::directory_iterator() ||
			(!filter.empty() && fnmatch(filter.c_str(), it->path().filename().c_str(), 0));
	}
};

DirectoryIterator::DirectoryIterator() { }
DirectoryIterator::DirectoryIterator(path const& p, std::string const& filter)
: privdata(new PrivData(p, filter))
{
	if (privdata->it == bfs::directory_iterator())
		privdata.reset();
	else if (privdata->bad())
		++*this;
	else
		value = privdata->it->path().filename().string();
}

bool DirectoryIterator::operator==(DirectoryIterator const& rhs) const {
	return privdata.get() == rhs.privdata.get();
}

DirectoryIterator& DirectoryIterator::operator++() {
	if (!privdata) return *this;

	++privdata->it;

	while (privdata->bad()) {
		if (privdata->it == bfs::directory_iterator()) {
			privdata.reset();
			return *this;
		}
		++privdata->it;
	}

	value = privdata->it->path().filename().string();

	return *this;
}

DirectoryIterator::~DirectoryIterator() { }

} }

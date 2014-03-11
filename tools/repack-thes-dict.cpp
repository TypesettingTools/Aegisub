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

#include <libaegisub/charset_conv.h>
#include <libaegisub/dispatch.h>
#include <libaegisub/io.h>
#include <libaegisub/line_iterator.h>
#include <libaegisub/log.h>
#include <libaegisub/util.h>

#include <boost/algorithm/string.hpp>
#include <boost/locale/generator.hpp>
#include <boost/phoenix/core/argument.hpp>
#include <boost/phoenix/operator/comparison.hpp>
#include <fstream>

namespace {
using boost::phoenix::placeholders::_1;

void convert(std::string const& path) {
	std::unique_ptr<std::ifstream> idx(agi::io::Open(path + ".idx"));
	std::unique_ptr<std::ifstream> dat(agi::io::Open(path + ".dat"));

	std::ostringstream idx_out_buffer;
	agi::io::Save idx_out(path + ".out.idx");
	agi::io::Save dat_out(path + ".out.dat");

	idx_out.Get() << "UTF-8\n";
	dat_out.Get() << "UTF-8\n";

	std::string encoding_name;
	getline(*idx, encoding_name);

	agi::charset::IconvWrapper conv(encoding_name.c_str(), "utf-8");

	std::string unused_entry_count;
	getline(*idx, unused_entry_count);

	int entry_count = 0;

	for (auto const& line : agi::line_iterator<std::string>(*idx, encoding_name)) {
		std::vector<std::string> chunks;
		boost::split(chunks, line, _1 == '|');
		if (chunks.size() != 2)
			continue;
		if (chunks[0].find(' ') != std::string::npos)
			continue;

		++entry_count;

		idx_out_buffer << chunks[0] << '|' << dat_out.Get().tellp() << '\n';
		dat->seekg(atoi(chunks[1].c_str()));

		agi::line_iterator<std::string> iter{*dat, encoding_name};
		dat_out.Get() << *iter << '\n';

		std::vector<std::string> header;
		boost::split(header, *iter, _1 == '|');
		int meanings = atoi(header[1].c_str());
		for (int i = 0; i < meanings; ++i)
			dat_out.Get() << *++iter << '\n';
	}

	idx_out.Get() << entry_count << '\n' << idx_out_buffer.str();
}

}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("usage: respack-thes-dict <path-to-dict-without-extension>\n");
		return 1;
	}
	agi::dispatch::Init([](agi::dispatch::Thunk f) { });
	std::locale::global(boost::locale::generator().generate(""));
	agi::log::log = new agi::log::LogSink;

	convert(argv[1]);
}


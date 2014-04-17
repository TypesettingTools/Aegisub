// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file line_iterator.h
/// @brief An iterator over lines in a stream
/// @ingroup libaegisub

#pragma once

#include <iterator>
#include <memory>

#include <cstdint>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <libaegisub/charset_conv.h>

namespace agi {

/// @class line_iterator
/// @brief An iterator over lines in a stream
template<class OutputType = std::string>
class line_iterator final : public std::iterator<std::input_iterator_tag, OutputType> {
	std::istream *stream = nullptr; ///< Stream to iterator over
	OutputType value; ///< Value to return when this is dereference
	std::shared_ptr<agi::charset::IconvWrapper> conv;
	int cr; ///< CR character in the source encoding
	int lf; ///< LF character in the source encoding
	size_t width;  ///< width of LF character in the source encoding

	/// @brief Convert a string to the output type
	/// @param str Line read from the file
	///
	/// line_iterator users can either ensure that operator>> is defined for
	/// their desired output type or simply provide a specialization of this
	/// method which does the conversion.
	inline bool convert(std::string &str);
	/// @brief Get the next line from the stream
	/// @param[out] str String to fill with the next line
	void getline(std::string &str);

	/// @brief Get the next value from the stream
	void next();
public:
	/// @brief Constructor
	/// @param stream The stream to read from. The calling code is responsible
	///               for ensuring that the stream remains valid for the
	///               lifetime of the iterator and that it get cleaned up.
	/// @param encoding Encoding of the text read from the stream
	line_iterator(std::istream &stream, std::string encoding = "utf-8")
	: stream(&stream)
	, cr('\r')
	, lf('\n')
	, width(1)
	{
		if (boost::to_lower_copy(encoding) != "utf-8") {
			agi::charset::IconvWrapper c("utf-8", encoding.c_str());
			c.Convert("\r", 1, reinterpret_cast<char *>(&cr), sizeof(int));
			c.Convert("\n", 1, reinterpret_cast<char *>(&lf), sizeof(int));
			width = c.RequiredBufferSize("\n");
			conv = std::make_shared<agi::charset::IconvWrapper>(encoding.c_str(), "utf-8");
		}

		++(*this);
	}

	/// @brief Invalid iterator constructor; use for end iterator
	line_iterator() = default;

	/// @brief Copy constructor
	/// @param that line_iterator to copy from
	line_iterator(line_iterator<OutputType> const&) = default;

	OutputType const& operator*() const { return value; }
	OutputType const* operator->() const { return &value; }

	line_iterator<OutputType>& operator++() {
		next();
		return *this;
	}
	line_iterator<OutputType> operator++(int) {
		line_iterator<OutputType> tmp(*this);
		++*this;
		return tmp;
	}

	bool operator==(line_iterator<OutputType> const& rgt) const { return stream == rgt.stream; }
	bool operator!=(line_iterator<OutputType> const& rgt) const { return !operator==(rgt); }

	// typedefs needed by some stl algorithms
	typedef OutputType* pointer;
	typedef OutputType& reference;
	typedef const OutputType* const_pointer;
	typedef const OutputType& const_reference;

	line_iterator<OutputType>& operator=(line_iterator<OutputType> that) {
		using std::swap;
		swap(*this, that);
		return *this;
	}

	void swap(line_iterator<OutputType> &that) throw() {
		using std::swap;
		swap(stream, that.stream);
		swap(value, that.value);
		swap(conv, that.conv);
		swap(lf, that.lf);
		swap(cr, that.cr);
		swap(width, that.width);
	}
};

// Enable range-based for
template<typename T>
line_iterator<T>& begin(line_iterator<T>& it) { return it; }

template<typename T>
line_iterator<T> end(line_iterator<T>&) { return agi::line_iterator<T>(); }

template<class OutputType>
void line_iterator<OutputType>::getline(std::string &str) {
	union {
		int32_t chr;
		char buf[4];
	} u;

	for (;;) {
		u.chr = 0;
		std::streamsize read = stream->rdbuf()->sgetn(u.buf, width);
		if (read < (std::streamsize)width) {
			for (int i = 0; i < read; i++) {
				str += u.buf[i];
			}
			stream->setstate(std::ios::eofbit);
			return;
		}
		if (u.chr == cr) continue;
		if (u.chr == lf) return;
		for (int i = 0; i < read; i++) {
			str += u.buf[i];
		}
	}
}

template<class OutputType>
void line_iterator<OutputType>::next() {
	if (!stream) return;
	if (!stream->good()) {
		stream = nullptr;
		return;
	}
	std::string str, cstr, *target;
	if (width == 1) {
		std::getline(*stream, str);
		if (str.size() && str.back() == '\r')
			str.pop_back();
	}
	else {
		getline(str);
	}
	if (conv.get()) {
		conv->Convert(str, cstr);
		target = &cstr;
	}
	else {
		target = &str;
	}
	if (!convert(*target))
		next();
}

template<>
inline void line_iterator<std::string>::next() {
	if (!stream) return;
	if (!stream->good()) {
		stream = nullptr;
		return;
	}
	std::string cstr;
	std::string *target = conv ? &cstr : &value;
	if (width == 1) {
		std::getline(*stream, *target);
		if (target->size() && target->back() == '\r')
			target->pop_back();
	}
	else
		getline(*target);
	if (conv.get()) {
		value.clear();
		conv->Convert(*target, value);
	}
}

template<class OutputType>
inline bool line_iterator<OutputType>::convert(std::string &str) {
	boost::interprocess::ibufferstream ss(str.data(), str.size());
	ss >> value;
	return !ss.fail();
}

template<class T>
void swap(agi::line_iterator<T> &lft, agi::line_iterator<T> &rgt) {
	lft.swap(rgt);
}

}

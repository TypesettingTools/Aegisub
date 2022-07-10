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

#include <boost/interprocess/streams/bufferstream.hpp>
#include <cstdint>

namespace agi {

namespace charset {
class IconvWrapper;
}

class line_iterator_base {
	std::istream* stream = nullptr; ///< Stream to iterate over
	std::shared_ptr<agi::charset::IconvWrapper> conv;
	int cr = '\r';    ///< CR character in the source encoding
	int lf = '\n';    ///< LF character in the source encoding
	size_t width = 1; ///< width of LF character in the source encoding

  protected:
	bool getline(std::string& str);

  public:
	line_iterator_base(std::istream& stream, std::string encoding = "utf-8");

	line_iterator_base() = default;
	line_iterator_base(line_iterator_base const&) = default;
	line_iterator_base(line_iterator_base&&) = default;

	line_iterator_base& operator=(line_iterator_base const&) = default;
	line_iterator_base& operator=(line_iterator_base&&) = default;

	bool operator==(line_iterator_base const& rgt) const { return stream == rgt.stream; }
	bool operator!=(line_iterator_base const& rgt) const { return !operator==(rgt); }
};

/// @class line_iterator
/// @brief An iterator over lines in a stream
template <class OutputType = std::string>
class line_iterator final : public line_iterator_base,
                            public std::iterator<std::input_iterator_tag, OutputType> {
	OutputType value; ///< Value to return when this is dereference

	/// @brief Convert a string to the output type
	/// @param str Line read from the file
	///
	/// line_iterator users can either ensure that operator>> is defined for
	/// their desired output type or simply provide a specialization of this
	/// method which does the conversion.
	inline bool convert(std::string& str);

	/// @brief Get the next value from the stream
	void next();

  public:
	/// @brief Constructor
	/// @param stream The stream to read from. The calling code is responsible
	///               for ensuring that the stream remains valid for the
	///               lifetime of the iterator and that it get cleaned up.
	/// @param encoding Encoding of the text read from the stream
	line_iterator(std::istream& stream, std::string encoding = "utf-8")
	    : line_iterator_base(stream, std::move(encoding)) {
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

	// typedefs needed by some stl algorithms
	typedef OutputType* pointer;
	typedef OutputType& reference;
	typedef const OutputType* const_pointer;
	typedef const OutputType& const_reference;
};

// Enable range-based for
template <typename T> line_iterator<T>& begin(line_iterator<T>& it) {
	return it;
}

template <typename T> line_iterator<T> end(line_iterator<T>&) {
	return agi::line_iterator<T>();
}

template <class OutputType> void line_iterator<OutputType>::next() {
	std::string str;
	if(!getline(str)) return;
	if(!convert(str)) next();
}

template <> inline void line_iterator<std::string>::next() {
	value.clear();
	getline(value);
}

template <class OutputType> inline bool line_iterator<OutputType>::convert(std::string& str) {
	boost::interprocess::ibufferstream ss(str.data(), str.size());
	ss >> value;
	return !ss.fail();
}

} // namespace agi

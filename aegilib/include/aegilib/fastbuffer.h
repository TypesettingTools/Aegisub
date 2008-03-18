// Copyright (c) 2005, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


#pragma once
#include <vector>
#include "utils.h"


namespace Athenasub {
	// Fast buffer class
	template <typename T>
	class FastBuffer {
	private:
		std::vector<T> buffer;
		size_t _size;

	public:
		// Constructor
		FastBuffer() { _size = 0; }

		// Gets the stored size
		size_t GetSize() const { return _size; }

		// Shifts all the buffer left, destroying steps entries
		void ShiftLeft(size_t steps) {
			steps = Min(_size,steps);
			memcpy(&buffer[0],&buffer[steps],(_size-steps)*sizeof(T));
			_size -= steps;
		}

		// Get a read pointer
		const T* GetReadPtr() const { return &buffer[0]; }

		// Get a non-const read pointer
		T* GetMutableReadPtr() { return &buffer[0]; }

		// Get a write pointer to a new area of the specified size
		T* GetWritePtr(size_t size) {
			size_t oldSize = _size;
			_size += size;
			if (buffer.size() < _size+4) buffer.resize(_size+4);
			return &buffer[oldSize];
		}

		// Assume that has a certain size, discarding anything beyond it
		void AssumeSize(size_t size) {
			_size = Min(size,_size);
		}

		// Pre-Allocates memory
		void Alloc(size_t size) {
			buffer.resize(size);
		}

		// Finds a line break
		void FindLineBreak(size_t start,size_t end,int &pos,T &character) {
			pos = -1;
			character = 0;
			size_t c1 = '\n';
			size_t c2 = '\r';
			size_t chr;
			for (size_t i=start;i<end;i++) {
				chr = buffer[i];
				if (chr == c1 || chr == c2) {
					pos = (int)i;
					character = (T)chr;
					return;
				}
			}
		}
	};
}

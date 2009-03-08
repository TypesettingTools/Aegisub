// Copyright (c) 2008, Niels Martin Hansen
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
// Website: http://www.aegisub.net
// Contact: mailto:jiifurusu@gmail.com
//

#pragma once

#ifndef _AEGISUB_ENDIAN_H
#define _AEGISUB_ENDIAN_H


// Sanity check

#ifndef HAVE_LITTLE_ENDIAN
# ifndef HAVE_BIG_ENDIAN
// We neither have big nor little endian from configuration
#  ifdef HAVE_UNIVERSAL_ENDIAN
// But this is an OS X system building a universal binary
// Apple's GCC defines _BIG_ENDIAN when building for PPC
#   ifdef _BIG_ENDIAN
#    define HAVE_BIG_ENDIAN
#   else
#    define HAVE_LITTLE_ENDIAN
#   endif
#   undef HAVE_DYNAMIC_ENDIAN
#  else // !HAVE_UNIVERSAL_ENDIAN
// We aren't building an OS X universal binary
// Use the dynamic endian code
#   ifndef HAVE_DYNAMIC_ENDIAN
#    define HAVE_DYNAMIC_ENDIAN
#   endif
#  endif //HAVE_UNIVERSAL_ENDIAN
# endif // HAVE_BIG_ENDIAN
#endif // HAVE_LITTLE_ENDIAN

#ifdef HAVE_LITTLE_ENDIAN
# ifdef HAVE_BIG_ENDIAN
#  error You cannot have both HAVE_LITTLE_ENDIAN and HAVE_BIG_ENDIAN defined at the same time
# endif
#endif


#include <stdint.h>


namespace Endian {

	// Unconditionally reverse endianness

	// These are only defined for unsigned ints,
	// Use reinterpret_cast on the values if you need signed values.

	inline uint16_t Reverse(uint16_t val)
	{
		return
			((val & 0x00FF) << 8) |
			((val & 0xFF00) >> 8);
	}

	inline uint32_t Reverse(uint32_t val)
	{
		return
			((val & 0x000000FF) << 24) |
			((val & 0x0000FF00) <<  8) |
			((val & 0x00FF0000) >>  8) |
			((val & 0xFF000000) >> 24);
	}

	inline uint64_t Reverse(uint64_t val)
	{
		return
			((val & 0x00000000000000FF) << 56) |
			((val & 0x000000000000FF00) << 40) |
			((val & 0x0000000000FF0000) << 24) |
			((val & 0x00000000FF000000) <<  8) |
			((val & 0x000000FF00000000) >>  8) |
			((val & 0x0000FF0000000000) >> 24) |
			((val & 0x00FF000000000000) >> 40) |
			((val & 0xFF00000000000000) >> 56);
	}


#ifndef HAVE_DYNAMIC_ENDIAN


	// Regular, fast, templatized conditional reversing

	template <class T>
	inline T LittleToMachine(T val)
	{
#ifdef HAVE_BIG_ENDIAN
		// We're on big endian, reverse little to big
		return Reverse(val);
#else
		// We're on little endian and input is little
		return val;
#endif
	}

	template <class T>
	inline T BigToMachine(T val)
	{
#ifdef HAVE_LITTLE_ENDIAN
		// We're on little endian, reverse big to little
		return Reverse(val);
#else
		// We're on big endian and input is big
		return val;
#endif
	}

	template <class T>
	inline T MachineToLittle(T val)
	{
#ifdef HAVE_BIG_ENDIAN
		// We're on big endian, reverse to little
		return Reverse(val);
#else
		// Already on little, nothing to be done
		return val;
#endif
	}

	template <class T>
	inline T MachineToBig(T val)
	{
#ifdef HAVE_LITTLE_ENDIAN
		// We're on little endian, reverse to big
		return Reverse(val);
#else
		// Already on big, nothing to be done
		return val;
#endif
	}


#else // HAVE_DYNAMIC_ENDIAN


	// Dynamic endianness handling

	// Exploit that bit-shifting operations always can put bytes into
	// machine word order, while unions can be used to access bytes
	// only from an explicitly given byte order.
	// This is probably slower than when we explicitly know
	// the endianness of the machine we are on, but it's the same
	// code for any platform!


	// Unions to pack together ints and get their physical bytes

	union bytes16 {
		uint8_t byte[2];
		uint16_t word;
	};
	union bytes32 {
		uint8_t byte[4];
		uint32_t word;
	};
	union bytes64 {
		uint8_t byte[8];
		uint64_t word;
	};


	// 16 bit words

	inline uint16_t MachineToBig(uint16_t val)
	{
		bytes16 pack;
		// Store the bytes into the correct positions in the word
		pack.byte[0] = (val & 0xFF00) >> 8;
		pack.byte[1] = val & 0x00FF;
		// And return a value now encoded as big endian
		return pack.word;
	}

	inline uint16_t MachineToLittle(uint16_t val)
	{
		bytes16 pack;
		// Store the bytes into the correct positions in the word
		pack.byte[0] = val & 0x00FF;
		pack.byte[1] = (val & 0xFF00) >> 8;
		// And return a value now encoded as little endian
		return pack.word;
	}

	inline uint16_t BigToMachine(uint16_t val)
	{
		bytes16 pack;
		// Put our word into the pack
		pack.word = val;
		// And produce a machine endian value of it
		return uint16_t(pack.byte[1]) | (uint16_t(pack.byte[0]) << 8);
	}

	inline uint16_t LittleToMachine(uint16_t val)
	{
		bytes16 pack;
		// Put our word into the pack
		pack.word = val;
		// And produce a machine endian value of it
		return uint16_t(pack.byte[0]) | (uint16_t(pack.byte[1]) << 8);
	}


	// 32 bit words

	inline uint32_t MachineToBig(uint32_t val)
	{
		bytes32 pack;
		pack.byte[0] = (val & 0xFF000000) >> 24;
		pack.byte[1] = (val & 0x00FF0000) >> 16;
		pack.byte[2] = (val & 0x0000FF00) >>  8;
		pack.byte[3] =  val & 0x000000FF       ;
		return pack.word;
	}

	inline uint32_t MachineToLittle(uint32_t val)
	{
		bytes32 pack;
		pack.byte[0] =  val & 0x000000FF       ;
		pack.byte[1] = (val & 0x0000FF00) >>  8;
		pack.byte[2] = (val & 0x00FF0000) >> 16;
		pack.byte[3] = (val & 0xFF000000) >> 24;
		return pack.word;
	}

	inline uint32_t BigToMachine(uint32_t val)
	{
		bytes32 pack;
		pack.word = val;
		return
			(uint32_t(pack.byte[0]) << 24) |
			(uint32_t(pack.byte[1]) << 16) |
			(uint32_t(pack.byte[2]) <<  8) |
			 uint32_t(pack.byte[3]);
	}

	inline uint32_t LittleToMachine(uint32_t val)
	{
		bytes32 pack;
		pack.word = val;
		return
			(uint32_t(pack.byte[3]) << 24) |
			(uint32_t(pack.byte[2]) << 16) |
			(uint32_t(pack.byte[1]) <<  8) |
			 uint32_t(pack.byte[0]);
	}


	// 64 bit words

	inline uint64_t MachineToBig(uint64_t val)
	{
		bytes64 pack;
		pack.byte[0] = (val & 0xFF00000000000000) >> 56;
		pack.byte[1] = (val & 0x00FF000000000000) >> 48;
		pack.byte[2] = (val & 0x0000FF0000000000) >> 40;
		pack.byte[3] = (val & 0x000000FF00000000) >> 32;
		pack.byte[4] = (val & 0x00000000FF000000) >> 24;
		pack.byte[5] = (val & 0x0000000000FF0000) >> 16;
		pack.byte[6] = (val & 0x000000000000FF00) >>  8;
		pack.byte[7] =  val & 0x00000000000000FF       ;
		return pack.word;
	}

	inline uint64_t MachineToLittle(uint64_t val)
	{
		bytes64 pack;
		pack.byte[0] =  val & 0x00000000000000FF       ;
		pack.byte[1] = (val & 0x000000000000FF00) >>  8;
		pack.byte[2] = (val & 0x0000000000FF0000) >> 16;
		pack.byte[3] = (val & 0x00000000FF000000) >> 24;
		pack.byte[4] = (val & 0x000000FF00000000) >> 32;
		pack.byte[5] = (val & 0x0000FF0000000000) >> 40;
		pack.byte[6] = (val & 0x00FF000000000000) >> 48;
		pack.byte[7] = (val & 0xFF00000000000000) >> 56;
		return pack.word;
	}

	inline uint64_t BigToMachine(uint64_t val)
	{
		bytes64 pack;
		pack.word = val;
		return
			(uint64_t(pack.byte[0]) << 56) |
			(uint64_t(pack.byte[1]) << 48) |
			(uint64_t(pack.byte[2]) << 40) |
			(uint64_t(pack.byte[3]) << 32) |
			(uint64_t(pack.byte[4]) << 24) |
			(uint64_t(pack.byte[5]) << 16) |
			(uint64_t(pack.byte[6]) <<  8) |
			 uint64_t(pack.byte[7]);
	}

	inline uint64_t LittleToMachine(uint64_t val)
	{
		bytes64 pack;
		pack.word = val;
		return
			(uint64_t(pack.byte[7]) << 56) |
			(uint64_t(pack.byte[6]) << 48) |
			(uint64_t(pack.byte[5]) << 40) |
			(uint64_t(pack.byte[4]) << 32) |
			(uint64_t(pack.byte[3]) << 24) |
			(uint64_t(pack.byte[2]) << 16) |
			(uint64_t(pack.byte[1]) <<  8) |
			 uint64_t(pack.byte[0]);
	}


#endif

};

#endif

// Copyright (c) 2009, Niels Martin Hansen
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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file block_cache.h
/// @ingroup utility
/// @brief Template class for creating caches for blocks of data


// Include guard
#ifndef BLOCK_CACHE_H
#define BLOCK_CACHE_H


#include <vector>
#include <algorithm>



/// @class BasicDataBlockFactory
/// @brief Simple factory for allocating blocks for DataBlockCache
/// @tparam BlockT Type of blocks to produce
///
/// This is the default block factory class used by DataBlockCache if another isn't specified.
/// It allocates blocks on the heap using operator new and the default constructor, and does
/// nothing special to initialise the blocks.
///
/// Custom block factories could use a large internally pre-allocated buffer to create the
/// requested blocks in to avoid the default allocator.
template <typename BlockT>
struct BasicDataBlockFactory {
	/// @brief Allocates a block and returns it
	/// @param i Index of the block to allocate
	/// @return A pointer to the allocated block
	///
	/// This default implementation does not use the i parameter. Custom implementations
	/// of block factories should use i to determine what data to fill into the block.
	BlockT *ProduceBlock(size_t i)
	{
		(void)i;
		return new BlockT;
	}

	/// @brief De-allocate a block
	/// @param block Pointer to the block to de-allocate
	///
	/// It is guaranteed that block was returned by ProduceBlock.
	void DisposeBlock(BlockT *block)
	{
		delete block;
	}

	/// @brief Retrieve the amount of memory consumed by a single block
	/// @return Number of bytes consumed by a block
	///
	/// All blocks must consume the same amount of memory. The size of a block
	/// is used to manage and limit the size of the cache.
	size_t GetBlockSize() const
	{
		return sizeof(BlockT);
	}
};



/// @class DataBlockCache
/// @brief Cache for blocks of data in a stream or similar
/// @tparam BlockT             Type of blocks to store
/// @tparam MacroblockExponent Controls the number of blocks per macroblock, for tuning memory usage
/// @tparam BlockFactoryT      Type of block factory, see BasicDataBlockFactory class for detail on these
template <
	typename BlockT,
	int MacroblockExponent = 6,
	typename BlockFactoryT = BasicDataBlockFactory<BlockT>
>
class DataBlockCache {

	/// Type of an array of blocks
	typedef std::vector<BlockT*> BlockArray;

	/// DOCME
	struct MacroBlock {
		/// How many times data in the macroblock has been accessed
		int access_count;
		/// The blocks contained in the macroblock
		BlockArray blocks;
	};

	/// Type of an array of macro blocks
	typedef std::vector<MacroBlock> MacroBlockArray;

	/// The data in the cache
	MacroBlockArray data;

	/// Number of blocks per macroblock
	size_t macroblock_size;

	/// Bitmask to extract the inside-macroblock index for a block by bitwise and
	size_t macroblock_index_mask;

	/// Factory object for blocks
	BlockFactoryT factory;

	/// Used in sorting the macroblocks by access count for aging
	struct AccessData {
		MacroBlock *mb;
		AccessData(MacroBlock *_mb) : mb(_mb) { }
		// Sort in decreasing order: most accesses first
		bool operator < (const AccessData &other) { return mb->access_count > other.mb->access_count; }
	};

	/// @brief Dispose of all blocks in a macroblock and mark it empty
	/// @param mb_index Index of macroblock to clear
	void KillMacroBlock(MacroBlock &mb)
	{
		mb.access_count = 0;

		for (size_t bi = 0; bi < mb.blocks.size(); ++bi)
		{
			BlockT *b = mb.blocks[bi];
			if (!b)
				factory.DisposeBlock(b);
		}

		mb.blocks.clear();
	}

public:
	/// @brief Constructor
	/// @param block_count Total number of blocks the cache will manage
	/// @param factory     Factory object to use for producing blocks
	///
	/// Note that the block_count is the maximum block index the cache will ever see,
	/// it is an error to request a block number greater than block_count.
	///
	/// The factory object passed must respond well to copying.
	DataBlockCache(size_t block_count, BlockFactoryT factory = BlockFactoryT())
		: factory(factory)
	{
		SetBlockCount(block_count);
	}


	/// @brief Destructor
	///
	/// Disposes of all cached blocks
	~DataBlockCache()
	{
		// Clear all blocks by aging to zero bytes
		Age(0);
	}


	/// @brief Change the number of blocks in cache
	/// @param block_count New number of blocks to hold
	///
	/// This will completely de-allocate the cache and re-allocate it with the new block count.
	void SetBlockCount(size_t block_count)
	{
		if (data.size() > 0)
			Age(0);

		macroblock_size = 1 << MacroblockExponent;

		macroblock_index_mask = ~(((~0) >> MacroblockExponent) << MacroblockExponent);

		data.resize( (block_count + macroblock_size - 1) >> MacroblockExponent );
	}


	/// @brief Clean up the cache
	/// @param max_size Target maximum size of the cache in bytes
	///
	/// Passing a max_size of 0 (zero) causes the cache to be completely flushed
	/// in a fast manner.
	///
	/// The max_size is not a hard limit, the cache size might somewhat exceed the max
	/// after the aging operation, though it shouldn't be by much.
	void Age(size_t max_size)
	{
		// Quick way out: get rid of everything
		if (max_size == 0)
		{
			for (size_t mbi = 0; mbi < data.size(); ++mbi)
			{
				KillMacroBlock(data[mbi]);
			}
			return;
		}

		// Get a list of macro blocks sorted by access count
		std::vector<AccessData> access_data;
		access_data.reserve(data.size());
		// For whatever reason, G++ pukes if I try using iterators here...
		for (size_t mbi = 0; mbi != data.size(); ++mbi)
			access_data.push_back(AccessData(&data[mbi]));
		std::sort(access_data.begin(), access_data.end());

		// Sum up data size until we hit the max
		size_t cur_size = 0;
		size_t block_size = factory.GetBlockSize();
		size_t mbi = 0;
		for (; mbi < access_data.size(); ++mbi)
		{
			BlockArray &ba = access_data[mbi].mb->blocks;
			for (size_t i = 0; i < ba.size(); ++i)
			{
				if (ba[i] != 0)
					cur_size += block_size;
			}
			// Cut access count in half for live blocks, so parts that don't get accessed
			// a lot will eventually be killed off.
			access_data[mbi].mb->access_count /= 2;

			if (cur_size >= max_size)
			{
				++mbi;
				break;
			}
		}
		// Hit max, clear all remaining blocks
		for (; mbi < access_data.size(); ++mbi)
		{
			KillMacroBlock(*access_data[mbi].mb);
		}
	}


	/// @brief Obtain a data block from the cache
	/// @param      i       Index of the block to retrieve
	/// @param[out] created On return, tells whether the returned block was created during the operation
	/// @return A pointer to the block in cache
	///
	/// It is legal to pass 0 (null) for created, in this case nothing is returned in it.
	BlockT *Get(size_t i, bool *created = 0)
	{
		size_t mbi = i >> MacroblockExponent;
		assert(mbi < data.size());

		MacroBlock &mb = data[mbi];
		mb.access_count += 1;

		if (mb.blocks.size() == 0)
		{
			mb.blocks.resize(macroblock_size);
		}

		size_t block_index = i & macroblock_index_mask;
		assert(block_index < mb.blocks.size());

		BlockT *b = mb.blocks[block_index];

		if (!b)
		{
			b = factory.ProduceBlock(i);
			assert(b != 0);
			mb.blocks[block_index] = b;

			if (created) *created = true;
		}
		else
			if (created) *created = false;

		return b;
	}


	/// @brief Speculatively add blocks not present to the cache
	/// @param forward  Assume forwards linear access is plausible
	/// @param backward Assume backwards linear access is plausible
	/// @return Number of blocks added to the cache
	///
	/// Assuming forwards and/or backwards linear access causes the macroblock access data to be
	/// used for speculating in macroblocks that may be accessed soon, and also allocate block
	/// in macroblocks that may otherwise not have been accessed recently.
	///
	/// @todo Implement this.
	size_t Speculate(bool forward, bool backward)
	{
		(void)forward;
		(void)backward;
		return 0;
	}
};


// Include guard
#endif


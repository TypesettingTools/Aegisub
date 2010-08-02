// Copyright (c) 2005, 2006, Rodrigo Braz Monteiro
// Copyright (c) 2006, 2007, Niels Martin Hansen
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

/// @file audio_renderer_spectrum.cpp
/// @brief Caching frequency-power spectrum renderer for audio display
/// @ingroup audio_ui
///

#include "config.h"

#ifndef AGI_PRE
#include <assert.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include <algorithm>
#include <list>
#include <utility>
#include <vector>

#include <wx/log.h>
#endif

#include <libaegisub/log.h>

#include "include/aegisub/audio_provider.h"
#include "audio_renderer_spectrum.h"
#include "colorspace.h"
#include "fft.h"
#include "main.h"
#include "utils.h"


/// @class AudioSpectrumCache
/// @brief Base class for the frequency-power cache tree.
///
/// The cached frequency-power are kept in a shallow tree-structure composed of
/// intermediate branches and final leaves, both accessed through a common
/// interface, which is this class.
///
/// The term "cache line" here means the frequency-power data derived from
/// some range of audio samples, calculated by a single FFT.
class AudioSpectrumCache {
public:

	/// The type of frequency-power data at one point in time.
	typedef std::vector<float> CacheLine;


	/// The type of timestamps for last access.
	typedef unsigned int CacheAccessTime;

	/// Holds last-access data for a range of range of cache lines.
	/// Ranges can overlap, in case overlapping FFT's are used to increase precision.
	struct CacheAgeData {
		/// Last time this range of cache lines were accessed.
		CacheAccessTime access_time;
		/// First line in the range.
		unsigned long first_line;
		/// Number of lines in the range.
		unsigned long num_lines;

		/// @brief Comparison operator for sorting age data by last access time.
		/// @param second The age data structure to compare against.
		/// @return Returns true if last access time of this range is less than that of the other.
		bool operator< (const CacheAgeData& second) const { return access_time < second.access_time; }

		/// @brief Constructor.
		/// @param t     Initial access time to set.
		/// @param first First line in the range.
		/// @param num   Number of lines in the range.
		///
		CacheAgeData(CacheAccessTime t, unsigned long first, unsigned long num) : access_time(t), first_line(first), num_lines(num) { }
	};

	/// Type of a list of cache age data.
	typedef std::vector<CacheAgeData> CacheAgeList;

	/// @brief Retrieve frequency-power data.
	/// @param i       Index of the block to get the line from.
	/// @param overlap Index of the overlap in the block to get the line for.
	/// @param created [out] Set to true if the data had to be calculated, false if the data
	///                was found in cache.
	/// @param access_time Timestamp to mark the cache data as accessed at.
	/// @return Returns a reference to the frequency-power data requested.
	///
	/// The data are fetched from the cache if they are cached, otherwise the required
	/// audio data are retrieved, the FFT derived, and power data calculated.
	virtual CacheLine& GetLine(unsigned long i, unsigned int overlap, bool &created, CacheAccessTime access_time) = 0;

	/// @brief Get the size of the cache subtree.
	/// @return Number of lines stored in all nodes below this one in the tree.
	virtual size_t GetManagedLineCount() = 0;

	/// @brief Retrieve cache access times.
	/// @param ages [in,out] List to append age data of managed lines to.
	///
	/// Existing contents of the list is kept, new entries are added to the end.
	virtual void GetLineAccessTimes(CacheAgeList &ages) = 0;

	/// @brief Remove data from the cache.
	/// @param line_id Index of the block the cache node to remove starts at.
	/// @return Returns true if the object the method was called on no longer manages
	//          any cache lines and can safely be deleted.
	virtual bool KillLine(unsigned long line_id) = 0;


	/// @brief Set the FFT size used globally.
	/// @param new_length Number of audio samples to use in calculation.
	static void SetLineLength(unsigned long new_length)
	{
		line_length = new_length;
		null_line.resize(new_length, 0);
	}


	/// @brief Destructor, does nothing in base class.
	virtual ~AudioSpectrumCache() {};

protected:

	/// Global template for cache lines.
	static CacheLine null_line;
	/// Number of audio samples used for power calculation, determining the
	/// frequency resolution of the frequency-power data.
	static unsigned long line_length;
};

// Actual variables allocating memory for the static class members
AudioSpectrumCache::CacheLine AudioSpectrumCache::null_line;
unsigned long AudioSpectrumCache::line_length;



/// @class FinalSpectrumCache
/// @brief Leaf node in frequency-power cache tree, holds actual data.
///
/// This class stores frequency-power data and is responsible for calculating it as well.
class FinalSpectrumCache : public AudioSpectrumCache {
private:

	/// The stored data.
	std::vector<CacheLine> data;

	unsigned long start,   ///< Start of block range
	              length;  ///< Number of blocks
	unsigned int overlaps; ///< How many lines per block

	/// Last access time for cache management.
	CacheAccessTime last_access;

public:

	/// @brief Returns stored frequency-power data.
	/// @param i       Index of the block to get the line from.
	/// @param overlap Index of the overlap in the block to get the line for.
	/// @param created [out] Set to true if the data had to be calculated, false if the data
	///                was found in cache.
	/// @param access_time Timestamp to mark the cache data as accessed at.
	/// @return Returns a reference to the frequency-power data requested.
	CacheLine& GetLine(unsigned long i, unsigned int overlap, bool &created, CacheAccessTime access_time)
	{
		last_access = access_time;

		// This check ought to be redundant
		if (i >= start && i-start < length)
			return data[i - start + overlap*length];
		else
			return null_line;
	}


	/// @brief Get number of lines in cache.
	/// @return Number of lines stored at this leaf.
	size_t GetManagedLineCount()
	{
		return data.size();
	}


	/// @brief Add own cache age data to list of age data.
	/// @param ages [in,out] List to add cache age data to.
	///
	/// Produces a single cache age data object, representing the entire node,
	/// and adds it to the list.
	void GetLineAccessTimes(CacheAgeList &ages)
	{
		ages.push_back(CacheAgeData(last_access, start, data.size()));
	}


	/// @brief Return true if this is the line to remove.
	/// @param line_id Index of the block the cache node to remove starts at.
	/// @return Returns true if this is the cache block to remove.
	///
	/// This function won't actually delete anything, instead it is the responsibility
	/// of the caller to delete the cache node if this function returns true.
	bool KillLine(unsigned long line_id)
	{
		return start == line_id;
	}


	/// @brief Constructor, derives FFT and calculates frequency-power data.
	/// @param provider  Audio provider to get audio from.
	/// @param _start    Index of first block to calculate data for.
	/// @param _length   Number of blocks to calculate data for.
	/// @param _overlaps Number of lines to calculate per block.
	FinalSpectrumCache(AudioProvider *provider, unsigned long _start, unsigned long _length, unsigned int _overlaps)
	{
		start = _start;
		length = _length;
		overlaps = _overlaps;

		if (overlaps <  1) overlaps = 1;
		// Add an upper limit to number of overlaps or trust user to do sane things?
		// Any limit should probably be a function of length

		assert(length > 2);

		// First fill the data vector with blanks
		// Both start and end are included in the range stored, so we have end-start+1 elements
		data.resize(length*overlaps, null_line);

		unsigned int overlap_offset = line_length / overlaps * 2; // FIXME: the result seems weird/wrong without this factor 2, but why?

		FFT fft; // Use FFTW instead? A wavelet?

		for (unsigned int overlap = 0; overlap < overlaps; ++overlap) {
			// Start sample number of the next line calculated
			// line_length is half of the number of samples used to calculate a line, since half of the output from
			// a Fourier transform of real data is redundant, and not interesting for the purpose of creating
			// a frequenmcy/power spectrum.
			int64_t sample = start * line_length*2 + overlap*overlap_offset;

			long len = length;
#ifdef _OPENMP
#pragma omp parallel shared(overlap,len)
#endif
			{
				short *raw_sample_data = new short[line_length*2];
				float *sample_data = new float[line_length*2];
				float *out_r = new float[line_length*2];
				float *out_i = new float[line_length*2];

#ifdef _OPENMP
#pragma omp for
#endif
				for (long i = 0; i < len; ++i) {
					// Initialize
					sample = start * line_length*2 + overlap*overlap_offset + i*line_length*2;

					provider->GetAudio(raw_sample_data, sample, line_length*2);
					for (size_t j = 0; j < line_length; ++j) {
						sample_data[j*2] = (float)raw_sample_data[j*2];
						sample_data[j*2+1] = (float)raw_sample_data[j*2+1];
					}

					fft.Transform(line_length*2, sample_data, out_r, out_i);

					CacheLine &line = data[i + length*overlap];
					for (size_t j = 0; j < line_length; ++j) {
						line[j] = sqrt(out_r[j]*out_r[j] + out_i[j]*out_i[j]);
					}

					//sample += line_length*2;
				}

				delete[] raw_sample_data;
				delete[] sample_data;
				delete[] out_r;
				delete[] out_i;
			}
		}
	}


	/// @brief Destructor, does nothing.
	///
	/// All data is managed by C++ types and gets deleted when those types'
	/// destructors are implicitly run.
	virtual ~FinalSpectrumCache()
	{
	}

};


/// @class IntermediateSpectrumCache
/// @brief Intermediate node in the spectrum cache tree.
///
/// References further nodes in the spectrum cache tree and delegates operations to them.
class IntermediateSpectrumCache : public AudioSpectrumCache {
private:

	/// The child-nodes in the cache tree.
	std::vector<AudioSpectrumCache*> sub_caches;

	unsigned long start,           ///< DOCME
	              length,          ///< DOCME
	              subcache_length; ///< DOCME
	unsigned int overlaps;         ///< Number of overlaps used.
	bool subcaches_are_final;      ///< Are the children leaf nodes?
	int depth;                     ///< How deep is this in the tree.

	/// Audio provider to pass on to child nodes.
	AudioProvider *provider;

public:

	/// @brief Delegate line retrieval to a child node.
	/// @param i       Index of the block to get the line from.
	/// @param overlap Index of the overlap in the block to get the line for.
	/// @param created [out] Set to true if the data had to be calculated, false if the data
	///                was found in cache.
	/// @param access_time Timestamp to mark the cache data as accessed at.
	/// @return Returns a reference to the frequency-power data requested.
	///
	/// Will create the required child node if it doesn't exist yet.
	CacheLine &GetLine(unsigned long i, unsigned int overlap, bool &created, CacheAccessTime access_time)
	{
		if (i >= start && i-start <= length) {
			// Determine which sub-cache this line resides in
			size_t subcache = (i-start) / subcache_length;
			assert(subcache < sub_caches.size());

			if (!sub_caches[subcache]) {
				created = true;
				if (subcaches_are_final) {
					sub_caches[subcache] = new FinalSpectrumCache(provider, start+subcache*subcache_length, subcache_length, overlaps);
				} else {
					sub_caches[subcache] = new IntermediateSpectrumCache(provider, start+subcache*subcache_length, subcache_length, overlaps, depth+1);
				}
			}

			return sub_caches[subcache]->GetLine(i, overlap, created, access_time);
		} else {
			return null_line;
		}
	}


	/// @brief Iterate all direct children and return the sum of their managed line count.
	/// @return Returns the sum of the managed line count of all children.
	size_t GetManagedLineCount()
	{
		size_t res = 0;
		for (size_t i = 0; i < sub_caches.size(); ++i) {
			if (sub_caches[i])
				res += sub_caches[i]->GetManagedLineCount();
		}
		return res;
	}


	/// @brief Get access time data for all child nodes in cache tree.
	/// @param ages [in,out] List for child nodes to add their data to.
	void GetLineAccessTimes(CacheAgeList &ages)
	{
		for (size_t i = 0; i < sub_caches.size(); ++i) {
			if (sub_caches[i])
				sub_caches[i]->GetLineAccessTimes(ages);
		}
	}


	/// @brief Remove block with given index from cache.
	/// @param line_id Index of the block the cache node to remove starts at.
	/// @return Returns true if this node has no more live childs, false if
	///         there is a least one line child.
	///
	/// Iterates the child nodes, calls the method recursively on all live
	/// nodes, deletes any node returning true, and counts number of nodes
	/// still alive, returning true if any are alive.
	bool KillLine(unsigned long line_id)
	{
		int sub_caches_left = 0;
		for (size_t i = 0; i < sub_caches.size(); ++i) {
			if (sub_caches[i]) {
				if (sub_caches[i]->KillLine(line_id)) {
					delete sub_caches[i];
					sub_caches[i] = 0;
				} else {
					sub_caches_left++;
				}
			}
		}
		return sub_caches_left == 0;
	}


	/// @brief Constructor.
	/// @param _provider Audio provider to pass to child nodes.
	/// @param _start    Index of first block to manage.
	/// @param _length   Number of blocks to manage.
	/// @param _overlaps Number of lines per block.
	/// @param _depth    Number of levels in the tree above this node.
	///
	/// Determine how many sub-caches are required, how big they
	/// should be and allocates memory to store their pointers.
	IntermediateSpectrumCache(AudioProvider *_provider, unsigned long _start, unsigned long _length, unsigned int _overlaps, int _depth)
	{
		provider = _provider;
		start = _start;
		length = _length;
		overlaps = _overlaps;
		depth = _depth;

		// FIXME: this calculation probably needs tweaking
		int num_subcaches = 1;
		unsigned long tmp = length;
		while (tmp > 0) {
			tmp /= 16;
			num_subcaches *= 2;
		}
		subcache_length = length / (num_subcaches-1);

		subcaches_are_final = num_subcaches <= 4;

		sub_caches.resize(num_subcaches, 0);
	}


	/// @brief Destructor, deletes all still-live sub caches.
	virtual ~IntermediateSpectrumCache()
	{
		for (size_t i = 0; i < sub_caches.size(); ++i)
			if (sub_caches[i])
				delete sub_caches[i];
	}

};




/// @class AudioSpectrumCacheManager
/// @brief Manages a frequency-power cache tree.
///
/// The primary task of this class is to manage the amount of memory consumed by
/// the cache and delete items when it grows too large.
class AudioSpectrumCacheManager {
private:

	/// Root node of the cache tree.
	IntermediateSpectrumCache *cache_root;

	unsigned long cache_hits,   ///< Number of times the cache was used to retrieve data
	              cache_misses; ///< Number of times data had to be calculated

	/// Current time, used for cache aging purposes.
	AudioSpectrumCache::CacheAccessTime cur_time;

	/// Maximum number of lines to keep in cache.
	unsigned long max_lines_cached;

public:

	/// @brief Wrapper around cache tree, to get frequency-power data
	/// @param i       Block to get data from.
	/// @param overlap Line in block to get data from.
	/// @return Returns a reference to the requested line.
	///
	AudioSpectrumCache::CacheLine &GetLine(unsigned long i, unsigned int overlap)
	{
		bool created = false;
		AudioSpectrumCache::CacheLine &res = cache_root->GetLine(i, overlap, created, cur_time++);
		if (created)
			cache_misses++;
		else
			cache_hits++;
		return res;
	}


	/// @brief Remove old data from the cache.
	///
	/// Ages the cache by finding the least recently accessed data and removing cache data
	/// until the total number of lines stored in the tree is less than the maximum.
	void Age()
	{
		LOG_D("audio/renderer/spectrum/cache") << "stats: hits=" << cache_hits << " misses=" << cache_misses << " misses%=" << cache_misses/float(cache_hits+cache_misses)*100 << " managed lines=" << cache_root->GetManagedLineCount() << "(max=" << max_lines_cached << ")";

		// 0 means no limit
		if (max_lines_cached == 0)
			return;
		// No reason to proceed with complicated stuff if the count is too small
		// (FIXME: does this really pay off?)
		if (cache_root->GetManagedLineCount() < max_lines_cached)
			return;

		// Get and sort ages
		AudioSpectrumCache::CacheAgeList ages;
		cache_root->GetLineAccessTimes(ages);
		std::sort(ages.begin(), ages.end());

		// Number of lines we have found used so far
		// When this exceeds max_lines_caches go into kill-mode
		unsigned long cumulative_lines = 0;
		// Run backwards through the line age list (the most recently accessed items are at end)
		AudioSpectrumCache::CacheAgeList::reverse_iterator it = ages.rbegin();

		// Find the point where we have too many lines cached
		while (cumulative_lines < max_lines_cached) {
			if (it == ages.rend()) {
				LOG_D("audio/renderer/spectrum/") << "done aging did not exeed max_lines_cached";
				return;
			}
			cumulative_lines += it->num_lines;
			++it;
		}

		// By here, we have exceeded max_lines_cached so backtrack one
		--it;

		// And now start cleaning up
		for (; it != ages.rend(); ++it) {
			cache_root->KillLine(it->first_line);
		}

		LOG_D("audio/renderer/spectrum/") << "done aging, managed lines now=" << cache_root->GetManagedLineCount() << " (max=" << max_lines_cached << ")";
		assert(cache_root->GetManagedLineCount() < max_lines_cached);
	}


	/// @brief Constructor
	/// @param provider     Audio provider to pass to cache tree nodes.
	/// @param line_length  Number of audio samples to use per block.
	/// @param num_lines    Number of blocks to produce in total from the audio.
	/// @param num_overlaps Number of overlaps per block.
	///
	/// Initialises the cache tree root and calculates the maximum number of cache lines
	/// to keep based on the Audio Spectrum Memory Max configuration setting.
	AudioSpectrumCacheManager(AudioProvider *provider, unsigned long line_length, unsigned long num_lines, unsigned int num_overlaps)
	{
		cache_hits = cache_misses = 0;
		cur_time = 0;
		cache_root = new IntermediateSpectrumCache(provider, 0, num_lines, num_overlaps, 0);

		// option is stored in megabytes, but we want number of bytes
		unsigned long max_cache_size = OPT_GET("Audio/Renderer/Spectrum/Memory Max")->GetInt();
		// It can't go too low
		if (max_cache_size < 5) max_cache_size = 128;
		max_cache_size *= 1024 * 1024;
		unsigned long line_size = sizeof(AudioSpectrumCache::CacheLine::value_type) * line_length;
		max_lines_cached = max_cache_size / line_size;
	}


	/// @brief Destructor, deletes the cache tree root node.
	~AudioSpectrumCacheManager()
	{
		delete cache_root;
	}
};


// AudioSpectrum, documented in .h file


AudioSpectrum::AudioSpectrum(AudioProvider *_provider)
{
	provider = _provider;

	// Determine the quality of the spectrum rendering based on an index
	int quality_index = OPT_GET("Audio/Renderer/Spectrum/Quality")->GetInt();
	if (quality_index < 0) quality_index = 0;
	if (quality_index > 5) quality_index = 5; // no need to go freaking insane

	// Line length determines the balance between resolution in the time and frequency domains.
	// Larger line length gives better resolution in frequency domain,
	// smaller gives better resolution in time domain.
	// Any values uses the same amount of memory, but larger values takes (slightly) more CPU.
	// Line lengths must be powers of 2 due to the FFT algorithm.
	// 2^8 is a good compromise between time and frequency domain resolution, any smaller
	// gives an unreasonably low resolution in the frequency domain.

	// Increasing the number of overlaps gives better resolution in the time domain.
	// Doubling the number of overlaps doubles memory and CPU use, and also
	// doubles resolution in the time domain.

	switch (quality_index) {
		case 0:
			// No overlaps, good comprimise between time/frequency resolution.
			// 4 bytes used per sample.
			line_length = 1<<8;
			fft_overlaps = 1;
			break;
		case 1:
			// Double frequency resolution, the resulting half time resolution
			// is countered with an overlap.
			// 8 bytes per sample.
			line_length = 1<<9;
			fft_overlaps = 2;
			break;
		case 2:
			// Resulting double resolution in both domains.
			// 16 bytes per sample.
			line_length = 1<<9;
			fft_overlaps = 4;
			break;
		case 3:
			// Double frequency and quadrouble time resolution.
			// 32 bytes per sample.
			line_length = 1<<9;
			fft_overlaps = 8;
			break;
		case 4:
			// Quadrouble resolution in both domains.
			// 64 bytes per sample.
			line_length = 1<<10;
			fft_overlaps = 16;
			break;
		case 5:
			// Eight-double resolution in both domains.
			// 256 bytes per sample.
			line_length = 1<<11;
			fft_overlaps = 64;
			break;
		default:
			throw _T("Internal error in AudioSpectrum class - impossible quality index");
	}

	int64_t _num_lines = provider->GetNumSamples() / line_length / 2;
	num_lines = (unsigned long)_num_lines;

	AudioSpectrumCache::SetLineLength(line_length);
	cache = new AudioSpectrumCacheManager(provider, line_length, num_lines, fft_overlaps);

	power_scale = 1;
	minband = OPT_GET("Audio/Renderer/Spectrum/Cutoff")->GetInt();
	maxband = line_length - minband * 2/3; // TODO: make this customisable?

	// Generate colour maps
	unsigned char *palptr = colours_normal;
	for (int i = 0; i < 256; i++) {
		//hsl_to_rgb(170 + i * 2/3, 128 + i/2, i, palptr+0, palptr+1, palptr+2);	// Previous
		hsl_to_rgb((255+128-i)/2, 128 + i/2, MIN(255,2*i), palptr+0, palptr+1, palptr+2);	// Icy blue
		palptr += 3;
	}
	palptr = colours_selected;
	for (int i = 0; i < 256; i++) {
		//hsl_to_rgb(170 + i * 2/3, 128 + i/2, i*3/4+64, palptr+0, palptr+1, palptr+2);
		hsl_to_rgb((255+128-i)/2, 128 + i/2, MIN(255,3*i/2+64), palptr+0, palptr+1, palptr+2);	// Icy blue
		palptr += 3;
	}
}


AudioSpectrum::~AudioSpectrum()
{
	delete cache;
}


void AudioSpectrum::RenderRange(int64_t range_start, int64_t range_end, bool selected, unsigned char *img, int imgleft, int imgwidth, int imgpitch, int imgheight)
{
	unsigned long first_line = (unsigned long)(fft_overlaps * range_start / line_length / 2);
	unsigned long last_line = (unsigned long)(fft_overlaps * range_end / line_length / 2);

	float *power = new float[line_length];

	int last_imgcol_rendered = -1;

	unsigned char *palette;
	if (selected)
		palette = colours_selected;
	else
		palette = colours_normal;

	// Some scaling constants
	const int maxpower = (1 << (16 - 1))*256;

	const double upscale = power_scale * 16384 / line_length;
	const double onethirdmaxpower = maxpower / 3, twothirdmaxpower = maxpower * 2/3;
	const double logoverscale = log(maxpower*upscale - twothirdmaxpower);

	// Note that here "lines" are actually bands of power data
	unsigned long baseline = first_line / fft_overlaps;
	unsigned int overlap = first_line % fft_overlaps;
	for (unsigned long i = first_line; i <= last_line; ++i) {
		// Handle horizontal compression and don't unneededly re-render columns
		int imgcol = imgleft + imgwidth * (i - first_line) / (last_line - first_line + 1);
		if (imgcol <= last_imgcol_rendered)
			continue;

		AudioSpectrumCache::CacheLine &line = cache->GetLine(baseline, overlap);
		++overlap;
		if (overlap >= fft_overlaps) {
			overlap = 0;
			++baseline;
		}

		// Apply a "compressed" scaling to the signal power
		for (unsigned int j = 0; j < line_length; j++) {
			// First do a simple linear scale power calculation -- 8 gives a reasonable default scaling
			power[j] = line[j] * upscale;
			if (power[j] > maxpower * 2/3) {
				double p = power[j] - twothirdmaxpower;
				p = log(p) * onethirdmaxpower / logoverscale;
				power[j] = p + twothirdmaxpower;
			}
		}

/// @internal Macro that stores pixel data, depends on local variables in AudioSpectrum::RenderRange
#define WRITE_PIXEL \
	if (intensity < 0) intensity = 0; \
	if (intensity > 255) intensity = 255; \
	img[((imgheight-y-1)*imgpitch+x)*3 + 0] = palette[intensity*3+0]; \
	img[((imgheight-y-1)*imgpitch+x)*3 + 1] = palette[intensity*3+1]; \
	img[((imgheight-y-1)*imgpitch+x)*3 + 2] = palette[intensity*3+2];

		// Handle horizontal expansion
		int next_line_imgcol = imgleft + imgwidth * (i - first_line + 1) / (last_line - first_line + 1);
		if (next_line_imgcol >= imgpitch)
			next_line_imgcol = imgpitch-1;

		for (int x = imgcol; x <= next_line_imgcol; ++x) {

			// Decide which rendering algo to use
			if (maxband - minband > imgheight) {
				// more than one frequency sample per pixel (vertically compress data)
				// pick the largest value per pixel for display

				// Iterate over pixels, picking a range of samples for each
				for (int y = 0; y < imgheight; ++y) {
					int sample1 = MAX(0,maxband * y/imgheight + minband);
					int sample2 = MIN(signed(line_length-1),maxband * (y+1)/imgheight + minband);
					float maxval = 0;
					for (int samp = sample1; samp <= sample2; samp++) {
						if (power[samp] > maxval) maxval = power[samp];
					}
					int intensity = int(256 * maxval / maxpower);
					WRITE_PIXEL
				}
			}
			else {
				// less than one frequency sample per pixel (vertically expand data)
				// interpolate between pixels
				// can also happen with exactly one sample per pixel, but how often is that?

				// Iterate over pixels, picking the nearest power values
				for (int y = 0; y < imgheight; ++y) {
					float ideal = (float)(y+1.)/imgheight * maxband;
					float sample1 = power[(int)floor(ideal)+minband];
					float sample2 = power[(int)ceil(ideal)+minband];
					float frac = ideal - floor(ideal);
					int intensity = int(((1-frac)*sample1 + frac*sample2) / maxpower * 256);
					WRITE_PIXEL
				}
			}
		}

/// @internal The WRITE_PIXEL macro is only defined inside AudioSpectrum::RenderRange
#undef WRITE_PIXEL

	}

	delete[] power;

	cache->Age();
}


void AudioSpectrum::SetScaling(float _power_scale)
{
	power_scale = _power_scale;
}





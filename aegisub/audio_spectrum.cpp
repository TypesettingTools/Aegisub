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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//

#include <assert.h>
#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include "audio_spectrum.h"
#include "fft.h"
#include "colorspace.h"
#include "options.h"
#include "utils.h"
#include <wx/log.h>


// Audio spectrum FFT data cache

// Spectrum cache basically caches the raw result of FFT
class AudioSpectrumCache {
public:
	// Type of a single FFT result line
	typedef std::vector<float> CacheLine;

	// Types for cache aging
	typedef unsigned int CacheAccessTime;
	struct CacheAgeData {
		CacheAccessTime access_time;
		unsigned long first_line;
		unsigned long num_lines; // includes overlap-lines
		bool operator< (const CacheAgeData& second) const { return access_time < second.access_time; }
		CacheAgeData(CacheAccessTime t, unsigned long first, unsigned long num) : access_time(t), first_line(first), num_lines(num) { }
	};
	typedef std::vector<CacheAgeData> CacheAgeList;

	// Get the overlap'th overlapping FFT in FFT group i, generating it if needed
	virtual CacheLine& GetLine(unsigned long i, unsigned int overlap, bool &created, CacheAccessTime access_time) = 0;

	// Get the total number of cache lines currently stored in this cache node's sub tree
	virtual size_t GetManagedLineCount() = 0;

	// Append to a list of last access times to the cache
	virtual void GetLineAccessTimes(CacheAgeList &ages) = 0;

	// Delete the cache storage starting with the given line id
	// Return true if the object called on is empty and can safely be deleted too
	virtual bool KillLine(unsigned long line_id) = 0;

	// Set the FFT size used
	static void SetLineLength(unsigned long new_length)
	{
		line_length = new_length;
		null_line.resize(new_length, 0);
	}

	virtual ~AudioSpectrumCache() {};

protected:
	// A cache line containing only zero-values
	static CacheLine null_line;
	// The FFT size
	static unsigned long line_length;
};

AudioSpectrumCache::CacheLine AudioSpectrumCache::null_line;
unsigned long AudioSpectrumCache::line_length;


// Bottom level FFT cache, holds actual power data itself

class FinalSpectrumCache : public AudioSpectrumCache {
private:
	std::vector<CacheLine> data;
	unsigned long start, length; // start and end of range
	unsigned int overlaps;

	CacheAccessTime last_access;

public:
	CacheLine& GetLine(unsigned long i, unsigned int overlap, bool &created, CacheAccessTime access_time)
	{
		last_access = access_time;

		// This check ought to be redundant
		if (i >= start && i-start < length)
			return data[i - start + overlap*length];
		else
			return null_line;
	}

	size_t GetManagedLineCount()
	{
		return data.size();
	}

	void GetLineAccessTimes(CacheAgeList &ages)
	{
		ages.push_back(CacheAgeData(last_access, start, data.size()));
	}

	bool KillLine(unsigned long line_id)
	{
		return start == line_id;
	}

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

		// Raw sample data
		short *raw_sample_data = new short[line_length*2];
		float *sample_data = new float[line_length*2];
		// Real and imaginary components of the output
		float *out_r = new float[line_length*2];
		float *out_i = new float[line_length*2];

		FFT fft; // Use FFTW instead? A wavelet?

		for (unsigned int overlap = 0; overlap < overlaps; ++overlap) {
			// Start sample number of the next line calculated
			// line_length is half of the number of samples used to calculate a line, since half of the output from
			// a Fourier transform of real data is redundant, and not interesting for the purpose of creating
			// a frequenmcy/power spectrum.
			int64_t sample = start * line_length*2 + overlap*overlap_offset;

			for (unsigned long i = 0; i < length; ++i) {
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

				sample += line_length*2;
			}
		}

		delete[] raw_sample_data;
		delete[] sample_data;
		delete[] out_r;
		delete[] out_i;
	}

	virtual ~FinalSpectrumCache()
	{
	}

};


// Non-bottom-level cache, refers to other caches to do the work

class IntermediateSpectrumCache : public AudioSpectrumCache {
private:
	std::vector<AudioSpectrumCache*> sub_caches;
	unsigned long start, length, subcache_length;
	unsigned int overlaps;
	bool subcaches_are_final;
	int depth;
	AudioProvider *provider;

public:
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

	size_t GetManagedLineCount()
	{
		size_t res = 0;
		for (size_t i = 0; i < sub_caches.size(); ++i) {
			if (sub_caches[i])
				res += sub_caches[i]->GetManagedLineCount();
		}
		return res;
	}

	void GetLineAccessTimes(CacheAgeList &ages)
	{
		for (size_t i = 0; i < sub_caches.size(); ++i) {
			if (sub_caches[i])
				sub_caches[i]->GetLineAccessTimes(ages);
		}
	}

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

	virtual ~IntermediateSpectrumCache()
	{
		for (size_t i = 0; i < sub_caches.size(); ++i)
			if (sub_caches[i])
				delete sub_caches[i];
	}

};



class AudioSpectrumCacheManager {
private:
	IntermediateSpectrumCache *cache_root;
	unsigned long cache_hits, cache_misses;
	AudioSpectrumCache::CacheAccessTime cur_time;

	unsigned long max_lines_cached;

public:
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

	void Age()
	{
		wxLogDebug(_T("AudioSpectrumCacheManager stats: hits=%u, misses=%u, misses%%=%f, managed lines=%u (max=%u)"), cache_hits, cache_misses, cache_misses/float(cache_hits+cache_misses)*100, cache_root->GetManagedLineCount(), max_lines_cached);

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
				wxLogDebug(_T("AudioSpectrumCacheManager done aging did not exceed max_lines_cached"));
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

		wxLogDebug(_T("AudioSpectrumCacheManager done aging, managed lines now=%u (max=%u)"), cache_root->GetManagedLineCount(), max_lines_cached);
		assert(cache_root->GetManagedLineCount() < max_lines_cached);
	}

	AudioSpectrumCacheManager(AudioProvider *provider, unsigned long line_length, unsigned long num_lines, unsigned int num_overlaps)
	{
		cache_hits = cache_misses = 0;
		cur_time = 0;
		cache_root = new IntermediateSpectrumCache(provider, 0, num_lines, num_overlaps, 0);

		// option is stored in megabytes, but we want number of bytes
		unsigned long max_cache_size = Options.AsInt(_T("Audio Spectrum Memory Max")) * 1024 * 1024;
		unsigned long line_size = sizeof(AudioSpectrumCache::CacheLine::value_type) * line_length;
		max_lines_cached = max_cache_size / line_size;
	}

	~AudioSpectrumCacheManager()
	{
		delete cache_root;
	}
};


// AudioSpectrum

AudioSpectrum::AudioSpectrum(AudioProvider *_provider)
{
	provider = _provider;

	int quality_index = Options.AsInt(_T("Audio Spectrum Quality"));
	if (quality_index < 0) quality_index = 0;
	if (quality_index > 5) quality_index = 5; // no need to go freaking insane
	if (quality_index > 1)
		line_length = 1 << (8 + quality_index - 1);
	else
		line_length = 1 << 8;
	if (quality_index == 0)
		fft_overlaps = 1;
	else if (quality_index == 1)
		fft_overlaps = 4;
	else
		fft_overlaps = 1 << quality_index;

	int64_t _num_lines = provider->GetNumSamples() / line_length / 2;
	//assert (_num_lines < (1<<31)); // hope it fits into 32 bits...
	num_lines = (unsigned long)_num_lines;

	AudioSpectrumCache::SetLineLength(line_length);
	cache = new AudioSpectrumCacheManager(provider, line_length, num_lines, fft_overlaps);

	power_scale = 1;
	minband = Options.AsInt(_T("Audio Spectrum Cutoff"));
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
					int sample1 = maxband * y/imgheight + minband;
					int sample2 = maxband * (y+1)/imgheight + minband;
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

#undef WRITE_PIXEL

	}

	delete[] power;

	cache->Age();
}


void AudioSpectrum::SetScaling(float _power_scale)
{
	power_scale = _power_scale;
}



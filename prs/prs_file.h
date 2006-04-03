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


//////////////
// Prototypes
class PRSEntry;
class PRSImage;
class PRSDisplay;


///////////
// Headers
#include <list>
#include <vector>
#include "prs_video_frame.h"


///////////////////////////////
// Pre-Rendered Subtitles file
class PRSFile {
private:
	std::list<PRSEntry*> entryList;
	std::list<PRSCachedFrame> frameCache;
	int cacheMemSize;
	int maxCache;
	void Reset();
	void EnforceCacheLimit();

public:
	PRSFile();
	~PRSFile();

	void AddEntry(PRSEntry *entry);

	void Save(std::string path);
	void Load(std::string path,bool reset=true);

	void SaveText(std::string path);
	void LoadText(std::string path,bool reset=true);

	void GetDisplayBlocksAtFrame(int n,std::vector<PRSDisplay*> &blocks);
	bool HasDataAtFrame(int n);
	void DrawFrame(int n,PRSVideoFrame *frame);
	PRSImage *GetImageByID(int id);

	PRSVideoFrame *CachedGetFrameByID(int id);
	void ClearCache();
	void SetCacheLimit(int bytes);

	PRSImage *FindDuplicateImage(PRSImage *img);
};

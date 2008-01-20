// Copyright (c) 2008, Simone Cociancich
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
// Contact: mailto:jiifurusu@gmail.com
//


//#include <assert.h>


// Discards values of selected that are past the end of AssFile::Line
#define CHOP_SELECTED(ass, sel) \
	for(; sel.back() >= (int)ass->Line.size(); sel.pop_back())


// Conversions between std::vector<v_t> and AVs
#define VECTOR_AV(v, av, v_t, s_t) \
  for(std::vector<v_t>::const_iterator it = v.begin(); it != v.end(); it++) \
    av_push(av, newSV ## s_t(*it))

#define AV_VECTOR(av, v, s_t) \
  for(int i = 0; i > -1 && i <= av_len(av); i++) { \
    SV **_val_ptr = av_fetch(av, i, 0); \
    if(_val_ptr) v.push_back(Sv ## s_t(*_val_ptr)); \
  }


// Utilities to manipolate hash elements
#define dHV \
	SV **HV_r;\
	HV *HV_tb; const char *HV_KEY; I32 HV_klen

#define HV_TOUCH(hv, k, kl) \
	HV_tb = hv;\
	HV_KEY = k;\
	HV_klen = kl;\
	HV_r = hv_fetch(HV_tb, HV_KEY, HV_klen, 1);\
	if(HV_r)

#define HV_FETCH(hv, k, kl) \
	HV_tb = hv;\
	HV_KEY = k;\
	HV_klen = kl;\
	HV_r = hv_fetch(HV_tb, HV_KEY, HV_klen, 0);\
	if(HV_r)

#define HV_VAL (*HV_r)

#define HV_STORE(si) \
	hv_store(HV_tb, HV_KEY, HV_klen, si, 0)

#define HV_FAS(hv, k, kl, vt, v) \
	HV_TOUCH(hv, k, kl) HV_STORE(newSV ## vt (v))
#define HV_FAA(hv, k, kl, vt, a) \
	HV_FETCH(hv, k, kl) a = Sv ## vt (HV_VAL)


// Utilities to manipulate list elements
#define dAV \
	SV **AV_r;\
	AV *AV_ar; I32 AV_KEY

#define AV_TOUCH(av, k) \
	AV_ar = av;\
	AV_KEY = k;\
	AV_r = av_fetch(AV_ar, AV_KEY, 1);\
	if(AV_r)

#define AV_FETCH(av, k) \
	AV_ar = av;\
	AV_KEY = k;\
	AV_r = av_fetch(AV_ar, AV_KEY, 0);\
	if(AV_r)

#define AV_VAL (*AV_r)

#define AV_STORE(si) \
	av_store(AV_ar, AV_KEY, si)

#define AV_FAS(av, k, vt, v) \
	AV_TOUCH(av, k, kl) AV_STORE(newSV ## vt (v))
#define AV_FAA(av, k, vt, a) \
	AV_FETCH(av, k, kl) a = Sv ## vt (AV_VAL)

#define AV_COPY(av_src, av_dst) \
	av_clear(av_dst);\
	for(I32 i = 0; i <= av_len(av_src); i++) {\
		AV_FETCH(av_src, i) {\
		  SV *src = AV_VAL;\
		  AV_TOUCH(av_dst, i)\
			AV_STORE(newSVsv(src));\
		}\
	}

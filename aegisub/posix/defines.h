#pragma once

#ifndef _DEFINES_H
#define _DEFINES_H

#define __AVISYNTH_H__
#define WX_PRECOMP

#include "acconf.h"

#ifdef HAVE_STDINT_H
#define __STDC_CONSTANT_MACROS 1
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include <stdwx.h>

// These shouldn't be needed any longer, if there are
// any occurrences of __int64 replace them with long long.
//typedef int64_t __int64;
//typedef uint64_t __uint64;
#define abs64 llabs

#include "res.h"

#endif /* _DEFINES_H */


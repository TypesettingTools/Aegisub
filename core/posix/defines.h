#pragma once

#ifndef _DEFINES_H
#define _DEFINES_H

#define __AVISYNTH_H__
#define WX_PRECOMP

#include "acconf.h"

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

typedef int64_t __int64;
typedef uint64_t __uint64;
#define abs64 llabs

#define NO_SPELLCHECKER
#define NO_FEX

#if defined(HAVE_LIBAVCODEC) && defined(HAVE_LIBAVFORMAT)
#define USE_LAVC
#endif
#if defined(HAVE_ASA) && defined(HAVE_ASA_H)
#define USE_ASA
#endif
#if defined(HAVE_LIBSSA) && defined(HAVE_LIBSSA_LIBSSA_H)
#define USE_LIBSSA
#endif

#include "res.h"

#endif /* _DEFINES_H */


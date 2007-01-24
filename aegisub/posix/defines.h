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

#include <stdwx.h>

typedef int64_t __int64;
typedef uint64_t __uint64;
#define abs64 llabs

#if defined(HAVE_LIBAVCODEC) && defined(HAVE_LIBAVFORMAT)
#define USE_LAVC 1
#endif
#if defined(HAVE_ASA) && defined(HAVE_ASA_H)
#define USE_ASA 1
#endif
#if defined(HAVE_LIBSSA) && defined(HAVE_LIBSSA_LIBSSA_H)
#define USE_LIBSSA 1
#endif
#ifdef HAVE_LIBPORTAUDIO
#define USE_PORTAUDIO 1
#else
#error no audio system available
#endif

#define USE_DIRECTSHOW 0
#define USE_DIRECTSOUND 0
#define USE_HUNSPELL 0
#ifndef USE_LAVC
#define USE_LAVC 0
#endif
#define USE_PRS 0
#define USE_FEXTRACKER 0
#ifndef USE_LIBSSA
#define USE_LIBSSA 0
#endif
#ifndef USE_ASA
#define USE_ASA 0
#endif

#include "res.h"

#endif /* _DEFINES_H */


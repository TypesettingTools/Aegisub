#define LAGI_PRE

#include "config.h"

// Common C
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#else
#  include <time.h>
#endif

// Windows C
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#endif

// Common C++
#include <deque>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <map>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <functional>
#include <memory>
#else
#include <tr1/functional>
#include <tr1/memory>
#endif

#ifdef __DEPRECATED // Dodge GCC warnings
# undef __DEPRECATED
# include <strstream>
# define __DEPRECATED
#else
# include <strstream>
#endif

// Cajun
#include "libaegisub/cajun/elements.h"
#include "libaegisub/cajun/reader.h"
#include "libaegisub/cajun/visitor.h"
#include "libaegisub/cajun/writer.h"

// Universalchardet
#ifndef _WIN32
#define _X86_ 1
#endif
#include "../universalchardet/nscore.h"
#include "../universalchardet/nsUniversalDetector.h"
#include "../universalchardet/nsMBCSGroupProber.h"
#include "../universalchardet/nsCharSetProber.h"


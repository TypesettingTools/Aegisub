#define LAGI_PRE

// Common C
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>

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
#include <map>
#include <memory>
#include <sstream>
#include <string>

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
#include "../universalchardet/nscore.h"
#include "../universalchardet/nsUniversalDetector.h"
#include "../universalchardet/nsMBCSGroupProber.h"
#include "../universalchardet/nsCharSetProber.h"


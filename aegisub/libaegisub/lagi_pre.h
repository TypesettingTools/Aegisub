#include "config.h"

#define LAGI_PRE

// Common C
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <stdint.h>
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

// Unix C
#ifndef _WIN32
#include <sys/statvfs.h>
#include <sys/param.h>
#endif

// Common C++
#include <algorithm>
#include <deque>
#include <climits>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <locale>
#include <numeric>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

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

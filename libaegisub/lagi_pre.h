#ifndef _WIN32
#include "../acconf.h"
#endif

#define WIN32_LEAN_AND_MEAN

// Common C
#include <cassert>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <sys/stat.h>
#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#else
#  include <ctime>
#endif

// Unix C
#ifndef _WIN32
#include <sys/statvfs.h>
#include <sys/param.h>
#endif

// Common C++
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#include <algorithm>
#include <deque>
#include <functional>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Boost
#include <boost/container/list.hpp>
#include <boost/container/map.hpp>
#define BOOST_NO_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_SCOPED_ENUMS
#include <boost/interprocess/streams/bufferstream.hpp>

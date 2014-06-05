#ifdef __cplusplus
#ifndef _WIN32
#include "../acconf.h"
#endif

#define WIN32_LEAN_AND_MEAN

// Common C
#include <cassert>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdint>
#include <ctime>

// Common C++
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Boost
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>
#include <boost/regex.hpp>
#define BOOST_NO_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_SCOPED_ENUMS
#include <boost/interprocess/streams/bufferstream.hpp>
#endif

# Copyright 2018 Mike Dev
# Copyright 2019 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt
# We support CMake 3.5, but prefer 3.16 policies and behavior

# NOTE: this doesn't work atm
#  it was partailyl converted by the tools/cmake2meson.py utility and partially by hand, but it isn't needed so I didn't finish it!


boost_regex_inc = include_directories('boost/regex')
boost_regex_dep = library('boost_regex' +'.'+ prj_version, 
include_directories: [boost_regex_inc])
if not get_option('BOOST_REGEX_STANDALONE')
  # target_link_libraries(['boost_regex', 'INTERFACE', 'Boost::config', 'Boost::throw_exception', 'Boost::predef', 'Boost::assert'])
else
  # target_compile_definitions(['boost_regex', 'INTERFACE', 'BOOST_REGEX_STANDALONE'])
endif
ICU_dep = dependency('ICU')
#option(BOOST_REGEX_ENABLE_ICU "Boost.Regex: enable ICU support" ${ICU_FOUND})
if ICU_dep.found()
  boost_regex_icu_dep = library('boost_regex_icu')
  # target_include_directories(['boost_regex_icu', 'INTERFACE', 'include'])
  if not get_option('BOOST_REGEX_STANDALONE')
    # target_link_libraries(['boost_regex_icu', 'INTERFACE', 'Boost::config', 'Boost::throw_exception', 'Boost::predef', 'Boost::assert'])
  else
    # target_compile_definitions(['boost_regex_icu', 'INTERFACE', 'BOOST_REGEX_STANDALONE'])
  endif
  ICU_dep = dependency('ICU')
  # target_link_libraries(['boost_regex_icu', 'INTERFACE', 'ICU::data', 'ICU::i18n', 'ICU::uc'])
endif

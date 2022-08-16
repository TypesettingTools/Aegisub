locale_sources = files([
    'src/boost/locale/encoding/codepage.cpp',
    'src/boost/locale/shared/date_time.cpp',
    'src/boost/locale/shared/format.cpp',
    'src/boost/locale/shared/formatting.cpp',
    'src/boost/locale/shared/generator.cpp',
    'src/boost/locale/shared/ids.cpp',
    'src/boost/locale/shared/localization_backend.cpp',
    'src/boost/locale/shared/message.cpp',
    'src/boost/locale/shared/mo_lambda.cpp',
    'src/boost/locale/util/codecvt_converter.cpp',
    'src/boost/locale/util/default_locale.cpp',
    'src/boost/locale/util/gregorian.cpp',
    'src/boost/locale/util/info.cpp',
    'src/boost/locale/util/locale_data.cpp',

    # icu
    'src/boost/locale/icu/boundary.cpp',
    'src/boost/locale/icu/codecvt.cpp',
    'src/boost/locale/icu/collator.cpp',
    'src/boost/locale/icu/conversion.cpp',
    'src/boost/locale/icu/date_time.cpp',
    'src/boost/locale/icu/formatter.cpp',
    'src/boost/locale/icu/icu_backend.cpp',
    'src/boost/locale/icu/numeric.cpp',
    'src/boost/locale/icu/time_zone.cpp'
])

locale_args = [
    '-DBOOST_THREAD_NO_LIB=1',
    '-DBOOST_LOCALE_NO_POSIX_BACKEND=1',
    '-DBOOST_LOCALE_NO_WINAPI_BACKEND=1',
    '-DBOOST_LOCALE_NO_STD_BACKEND=1',
    '-DBOOST_LOCALE_WITH_ICONV=1',
    '-DBOOST_LOCALE_WITH_ICU=1',
]

if not is_static
    locale_args += '-DBOOST_LOCALE_DYN_LINK=1'
endif

cxx = meson.get_compiler('cpp')
iconv_dep = cxx.find_library('iconv', required: false)
if not (iconv_dep.found() or cxx.has_function('iconv_open'))
    iconv_dep = dependency('iconv')
endif
locale_deps = [iconv_dep, icu_deps]

inc = 'src'
boost_locale = library('boost_locale', locale_sources,
                       include_directories: inc,
                       cpp_args: locale_args,
                       dependencies: [thread_dep, locale_deps, boost_thread_dep])

boost_locale_dep = declare_dependency(link_with: boost_locale, include_directories: inc,
                                      compile_args: '-DBOOST_ALL_NO_LIB=1')

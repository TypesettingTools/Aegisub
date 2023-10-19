locale_sources = files(
    'src/boost/locale/encoding/codepage.cpp',
    'src/boost/locale/icu/boundary.cpp',
    'src/boost/locale/icu/codecvt.cpp',
    'src/boost/locale/icu/collator.cpp',
    'src/boost/locale/icu/conversion.cpp',
    'src/boost/locale/icu/codecvt.cpp',
    'src/boost/locale/icu/date_time.cpp',
    'src/boost/locale/icu/formatter.cpp',
    'src/boost/locale/icu/icu_backend.cpp',
    'src/boost/locale/icu/numeric.cpp',
    'src/boost/locale/icu/time_zone.cpp',
    'src/boost/locale/shared/date_time.cpp',
    'src/boost/locale/shared/format.cpp',
    'src/boost/locale/shared/formatting.cpp',
    'src/boost/locale/shared/generator.cpp',
    'src/boost/locale/shared/ids.cpp',
    'src/boost/locale/shared/iconv_codecvt.cpp',
    'src/boost/locale/shared/localization_backend.cpp',
    'src/boost/locale/shared/message.cpp',
    'src/boost/locale/shared/mo_lambda.cpp',
    'src/boost/locale/std/codecvt.cpp',
    'src/boost/locale/std/collate.cpp',
    'src/boost/locale/std/converter.cpp',
    'src/boost/locale/std/numeric.cpp',
    'src/boost/locale/std/std_backend.cpp',
    'src/boost/locale/util/codecvt_converter.cpp',
    'src/boost/locale/util/default_locale.cpp',
    'src/boost/locale/util/encoding.cpp',
    'src/boost/locale/icu/formatters_cache.cpp',
    'src/boost/locale/util/gregorian.cpp',
    'src/boost/locale/util/info.cpp',
    'src/boost/locale/util/locale_data.cpp'
)

locale_args = ['-DBOOST_THREAD_NO_LIB=1']
locale_deps = []

if not is_static
    locale_args += '-DBOOST_LOCALE_DYN_LINK=1'
endif

if host_machine.system() == 'windows'
    locale_sources += files(
        'src/boost/locale/win32/collate.cpp',
        'src/boost/locale/win32/converter.cpp',
        'src/boost/locale/win32/lcid.cpp',
        'src/boost/locale/win32/numeric.cpp',
        'src/boost/locale/win32/win_backend.cpp',
        # included on windows/cygwin if std *or* win32 included
    )
    locale_args += '-DBOOST_LOCALE_NO_POSIX_BACKEND=1'
else
    locale_sources += files(
        'src/boost/locale/posix/codecvt.cpp',
        'src/boost/locale/posix/collate.cpp',
        'src/boost/locale/posix/converter.cpp',
        'src/boost/locale/posix/numeric.cpp',
        'src/boost/locale/posix/posix_backend.cpp',
    )
    locale_args += '-DBOOST_LOCALE_NO_WINAPI_BACKEND=1'
endif

cxx = meson.get_compiler('cpp')
iconv_dep = cxx.find_library('iconv', required: false)
if not (iconv_dep.found() or cxx.has_function('iconv_open'))
    iconv_dep = dependency('iconv')
endif
locale_deps += iconv_dep
locale_args += '-DBOOST_LOCALE_WITH_ICONV=1'

locale_deps += icu_deps
locale_args += '-DBOOST_LOCALE_WITH_ICU=1'


boost_locale = library(
    'boost_locale' + '.' + prj_version,
    locale_sources,
    include_directories: [inc,include_directories('src/')],
    cpp_args: locale_args,
    dependencies: [thread_dep, locale_deps, boost_thread_dep],
)

boost_locale_dep = declare_dependency(
    link_with: boost_locale,
    include_directories: inc,
    compile_args: '-DBOOST_ALL_NO_LIB=1',
)

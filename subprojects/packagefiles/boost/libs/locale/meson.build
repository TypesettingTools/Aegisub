locale_sources = files([
    'src/encoding/codepage.cpp',
    'src/shared/date_time.cpp',
    'src/shared/format.cpp',
    'src/shared/formatting.cpp',
    'src/shared/generator.cpp',
    'src/shared/ids.cpp',
    'src/shared/localization_backend.cpp',
    'src/shared/message.cpp',
    'src/shared/mo_lambda.cpp',
    'src/util/codecvt_converter.cpp',
    'src/util/default_locale.cpp',
    'src/util/info.cpp',
    'src/util/locale_data.cpp',
    # icu
    'src/icu/boundary.cpp',
    'src/icu/codecvt.cpp',
    'src/icu/collator.cpp',
    'src/icu/conversion.cpp',
    'src/icu/date_time.cpp',
    'src/icu/formatter.cpp',
    'src/icu/icu_backend.cpp',
    'src/icu/numeric.cpp',
    'src/icu/time_zone.cpp',
    # std - docs say disabled by default on windows and solaris,
    # but jamfile seemingly only disables on solaris
    'src/std/codecvt.cpp',
    'src/std/collate.cpp',
    'src/std/converter.cpp',
    'src/std/numeric.cpp',
    'src/std/std_backend.cpp',
    # included if using posix, win32 or std backend (ie always)
    'src/util/gregorian.cpp',
])

locale_args = ['-DBOOST_THREAD_NO_LIB=1']
locale_deps = []

if not is_static
    locale_args += '-DBOOST_LOCALE_DYN_LINK=1'
endif

if host_machine.system() == 'windows'
    locale_sources += files([
        'src/win32/collate.cpp',
        'src/win32/converter.cpp',
        'src/win32/numeric.cpp',
        'src/win32/win_backend.cpp',
        # included on windows/cygwin if std *or* win32 included
        'src/win32/lcid.cpp',
    ])
    locale_args += '-DBOOST_LOCALE_NO_POSIX_BACKEND=1'
else
    locale_sources += files([
        'src/posix/collate.cpp',
        'src/posix/converter.cpp',
        'src/posix/numeric.cpp',
        'src/posix/codecvt.cpp',
        'src/posix/posix_backend.cpp',
    ])
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

boost_locale = library('boost_locale', locale_sources,
                       include_directories: inc,
                       cpp_args: locale_args,
                       dependencies: [thread_dep, locale_deps, boost_thread_dep])

boost_locale_dep = declare_dependency(link_with: boost_locale, include_directories: inc,
                                      compile_args: '-DBOOST_ALL_NO_LIB=1')

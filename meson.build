project('Aegisub', ['c', 'cpp'],
        license: 'BSD-3-Clause',
        meson_version: '>=0.60.0',
        default_options: [
            'b_ndebug=if-release',
            'cpp_std=c++20',
            'buildtype=debugoptimized',
            'harfbuzz:icu=disabled',
            'ffmpeg:libdav1d=enabled',
            'luajit:amalgam=true',
            'luajit:lua52compat=true',
            'luajit:luajit=false',
        ],
        version: '3.4.1')

cmake = import('cmake')

if host_machine.system() == 'windows'
    add_project_arguments('-DNOMINMAX', '-D_WIN32_WINNT=0x0601', language: 'cpp')

    if not get_option('csri').disabled()
        add_global_arguments('-DCSRI_NO_EXPORT', language: 'c')
    endif

    sys_nasm = find_program('nasm', required: false)
    if not sys_nasm.found()
        nasm = subproject('nasm').get_variable('nasm')
        meson.override_find_program('nasm', nasm)
    endif
endif

if host_machine.system() == 'windows'
    powershell_exe = find_program('powershell')
    version_sh = [powershell_exe,
                  '-ExecutionPolicy', 'Bypass',
                  '-File', meson.current_source_dir() / 'tools' / 'version.ps1']
else
    version_sh = find_program('tools/version.sh')
endif
version_inc = include_directories('.')
version_h = custom_target('git_version.h',
                          command: [version_sh, meson.current_build_dir(), meson.current_source_dir()],
                          build_by_default: true,
                          build_always_stale: true, # has internal check whether target file will be refreshed
                          output: ['git_version.h'])

meson.add_dist_script(version_sh, meson.current_build_dir(), meson.current_source_dir(), 'dist')

if host_machine.system() == 'darwin' and get_option('build_osx_bundle')
    prefix = meson.current_build_dir() / 'Aegisub.app' / 'Contents'
    bindir = prefix / 'MacOS'
    datadir = prefix / 'SharedSupport'
    localedir = prefix / 'Resources'
    dataroot = datadir
else
    prefix = get_option('prefix')
    bindir = prefix / get_option('bindir')
    datadir = prefix / get_option('datadir')
    localedir = prefix / get_option('localedir')
    dataroot = datadir / 'aegisub'
endif
docdir = prefix / 'doc'

# MSVC sets this automatically with -MDd, but it has a different meaning on other platforms
if get_option('debug') and host_machine.system() != 'windows'
    add_project_arguments('-D_DEBUG', language: 'cpp')
endif

conf = configuration_data()
conf.set_quoted('P_DATA', dataroot)
conf.set_quoted('P_LOCALE', localedir)
if get_option('credit') != ''
    conf.set_quoted('BUILD_CREDIT', get_option('credit'))
endif
if get_option('official_release')
    conf.set('AEGI_OFFICIAL_RELEASE', 1)
endif
conf.set('WITH_UPDATE_CHECKER', get_option('enable_update_checker'))
if get_option('enable_update_checker')
    conf.set_quoted('UPDATE_CHECKER_SERVER', get_option('update_server'))
    conf.set_quoted('UPDATE_CHECKER_BASE_URL', get_option('update_url'))
endif

deps = []
deps_inc = []

if host_machine.system() == 'darwin'
    add_languages('objc', 'objcpp')
    add_project_arguments('-DGL_SILENCE_DEPRECATION', language: 'cpp')
    # meson neither supports objcpp_std nor inherits cpp_std https://github.com/mesonbuild/meson/issues/5495
    add_project_arguments('-std=c++20', language: 'objcpp')
elif host_machine.system() != 'windows'
    conf.set('WITH_FONTCONFIG', 1)
    deps += dependency('fontconfig')
endif

cxx = meson.get_compiler('cpp')
cc = meson.get_compiler('c')
deps += cc.find_library('m', required: false)
deps += cc.find_library('dl', required: false)

deps += dependency('iconv', fallback: ['iconv', 'libiconv_dep'])

deps += dependency('libass', version: '>=0.9.7',
                   fallback: ['libass', 'libass_dep'])

boost_modules = ['chrono', 'thread', 'locale', 'regex']
if not get_option('local_boost')
    boost_dep = dependency('boost', version: '>=1.70.0',
                            modules: boost_modules + ['system'],
                            required: false,
                            static: get_option('default_library') == 'static')
endif

if get_option('local_boost') or not boost_dep.found()
    boost_dep = []
    boost = subproject('boost')
    foreach module: boost_modules
        boost_dep += boost.get_variable('boost_' + module + '_dep')
    endforeach
endif

deps += boost_dep
if host_machine.system() == 'windows'
    conf.set('BOOST_USE_WINDOWS_H', 1)
endif

deps += dependency('zlib')

wx_minver = '>=' + get_option('wx_version')
if host_machine.system() == 'darwin'
    wx_minver = '>=3.1.0'
endif
wx_dep = dependency('wxWidgets', version: wx_minver,
                    required: false,
                    modules: ['std', 'stc', 'gl'])

if wx_dep.found()
    deps += wx_dep
else
    build_shared = true
    if get_option('default_library') == 'static'
        build_shared = false
    endif
    build_type = 'Release'
    if get_option('buildtype') == 'debug'
        build_type = 'Debug'
    endif

    opt_var = cmake.subproject_options()
    opt_var.set_override_option('cpp_std', 'c++14')
    opt_var.add_cmake_defines({
        'wxBUILD_INSTALL': false,
        'wxBUILD_PRECOMP': false,       # otherwise breaks project generation w/ meson
        'wxBUILD_SHARED': build_shared,

        'wxUSE_WEBVIEW': false,         # breaks build on linux
        'CMAKE_BUILD_TYPE': build_type,
        'wxUSE_IMAGE': true,
        'wxBUILD_MONOLITHIC': true      # otherwise breaks project generation w/ meson
    })

    wx = cmake.subproject('wxWidgets', options: opt_var)

    deps += [
        wx.dependency('wxmono'),
        wx.dependency('wxregex'),
        wx.dependency('wxscintilla')
    ]

    if host_machine.system() == 'windows' or host_machine.system() == 'darwin'
        deps += [
            wx.dependency('wxpng'),
        ]
    endif

    if host_machine.system() == 'windows'
        deps += [
            wx.dependency('wxzlib'),
            wx.dependency('wxexpat'),
        ]

        if cc.has_header('rpc.h')
            deps += cc.find_library('rpcrt4', required: true)
        else
            error('Missing Windows SDK RPC Library (rpc.h / rpcrt4.lib)')
        endif
        if cc.has_header('commctrl.h')
            deps += cc.find_library('comctl32', required: true)
        else
            error('Missing Windows SDK Common Controls Library (commctrl.h / comctl32.lib)')
        endif
    endif
endif

deps += dependency('icu-uc', version: '>=4.8.1.1')
deps += dependency('icu-i18n', version: '>=4.8.1.1')

dep_avail = []
foreach dep: [
    # audio, in order of precedence
    ['libpulse', [], 'PulseAudio', [], []],
    ['alsa', [], 'ALSA', [], []],
    ['portaudio-2.0', [], 'PortAudio', [], []],
    ['openal', '>=0.0.8', 'OpenAL', [], ['OpenAL']],
    # video
    ['ffms2', '>=2.22', 'FFMS2', ['ffms2', 'ffms2_dep'], []],
    # other
    ['fftw3', [], 'FFTW3', [], []],
    ['hunspell', [], 'Hunspell', ['hunspell', 'hunspell_dep'], []],
    ['uchardet', [], 'uchardet', ['uchardet', 'uchardet_dep'], []],
]
    optname = dep[0].split('-')[0]
    if not get_option(optname).disabled()
        # [provide] section is ignored if required is false;
        # must provided define fallback explicitly
        # (with meson 0.56 you can do allow_fallback: true):
        #d = dependency(dep[0], version: dep[1],
        #               required: false, allow_fallback: true)
        if dep[3].length() > 0
            d = dependency(dep[0], version: dep[1], fallback: dep[3])
        else
            d = dependency(dep[0], version: dep[1], required: false)
        endif

        if not d.found() and dep[4].length() > 0 and host_machine.system() == 'darwin'
            d = dependency('appleframeworks', modules: dep[4], required: false)
        endif

        if d.found()
            deps += d
            conf.set('WITH_@0@'.format(dep[0].split('-')[0].to_upper()), 1)
            dep_avail += dep[2]
        elif get_option(optname).enabled()
            error('@0@ enabled but not found'.format(dep[2]))
        endif
    endif
endforeach

if host_machine.system() == 'windows' and get_option('avisynth').enabled()
    conf.set('WITH_AVISYNTH', 1) # bundled separately with installer
endif

if host_machine.system() == 'windows' and not get_option('directsound').disabled()
    dsound_dep = cc.find_library('dsound', required: get_option('directsound'))
    winmm_dep = cc.find_library('winmm', required: get_option('directsound'))
    ole32_dep = cc.find_library('ole32', required: get_option('directsound'))
    have_dsound_h = cc.has_header('dsound.h')
    if not have_dsound_h and get_option('directsound').enabled()
        error('DirectSound enabled but dsound.h not found')
    endif

    dxguid_dep = cc.find_library('dxguid', required: true)
    if dsound_dep.found() and winmm_dep.found() and ole32_dep.found() and dxguid_dep.found() and have_dsound_h
        deps += [dsound_dep, winmm_dep, ole32_dep, dxguid_dep]
        conf.set('WITH_DIRECTSOUND', 1)
        dep_avail = ['DirectSound'] + dep_avail
    endif
endif

if host_machine.system() == 'darwin'
    frameworks_dep = dependency('appleframeworks', modules : ['CoreText', 'CoreFoundation', 'AppKit', 'Carbon', 'IOKit'])
    deps += frameworks_dep
endif

# TODO: OSS

def_audio = get_option('default_audio_output')
if def_audio != 'auto'
    if not dep_avail.contains(def_audio)
        error('Default audio output "@0@" selected but not available'.format(def_audio))
    endif
elif dep_avail.length() != 0
    def_audio = dep_avail[0]
else
    def_audio = ''
endif

conf_meson = configuration_data()
conf_meson.set('DEFAULT_PLAYER_AUDIO', def_audio)

if get_option('enable_update_checker')
    libcurl_dep = dependency('libcurl', required: false)

    if (not libcurl_dep.found())
        libcurl_opt = cmake.subproject_options()
        options_dict = {
            'HTTP_ONLY': true,
            'BUILD_CURL_EXE': false,
            'CMAKE_BUILD_TYPE': get_option('buildtype') == 'release' ? 'Release' : 'Debug',
        }
        if host_machine.system() == 'windows'
            options_dict += {'CURL_USE_SCHANNEL': true}
            deps += cc.find_library('crypt32')
        elif host_machine.system() == 'darwin'
            options_dict += {'CURL_USE_SECTRANSP': true}
        else
            options_dict += {'CURL_USE_OPENSSL': true}
        endif
        if get_option('default_library') == 'static' and (host_machine.system() == 'windows' or host_machine.system() == 'darwin')
            options_dict += {'BUILD_SHARED_LIBS': false}
            add_project_arguments('-DCURL_STATICLIB', language: ['cpp'])
        endif
        libcurl_opt.add_cmake_defines(options_dict)
        libcurl = cmake.subproject('curl', options: libcurl_opt)

        libcurl_dep = libcurl.dependency('libcurl')

        # Horrible meson hack to work around the cmake subproject creating broken include directories
        libcurl_dep = libcurl_dep.partial_dependency(compile_args: true, link_args: true, links: true, sources: true)
        libcurl_fixed_inc = declare_dependency(include_directories: subproject('curl_meson').get_variable('libcurl_fixed_inc'))
        deps += [libcurl_fixed_inc]
    endif
    deps += [libcurl_dep]
endif

luajit = dependency('luajit', version: '>=2.0.0', required: get_option('system_luajit'),
                    allow_fallback: false, method: 'pkg-config')
if luajit.found() and luajit.type_name() != 'internal'
    luajit_test = cc.run(files('tests/tests/luajit_52.c'), dependencies: luajit)

    if luajit_test.returncode() != 0
        if get_option('system_luajit')
            error('System luajit found but not compiled in 5.2 mode')
        else
            message('System luajit found but not compiled in 5.2 mode')
            luajit = dependency('', required: false)
        endif
    else
        deps += luajit
    endif
endif

if not luajit.found()
    message('Using built-in luajit')
    subproject('luajit') # Initialize subproject to ignore system luajit
    luajit = dependency('luajit')
    assert(luajit.type_name() == 'internal', 'System luajit used instead of built-in luajit')
endif

if luajit.type_name() == 'internal'
    deps += luajit
endif

deps += subproject('luabins').get_variable('luabins_dep')

dep_gl = dependency('gl', required: false)
if not dep_gl.found()
    if host_machine.system() == 'windows'
        dep_gl = cc.find_library('opengl32', required: false)
    else
        dep_gl = cc.find_library('GL', required: false)
    endif

    if not cc.has_header('GL/gl.h')
        dep_gl = dependency('', required: false)
    endif
endif
if host_machine.system() == 'darwin'
    conf.set('HAVE_OPENGL_GL_H', 1)
endif

if not dep_gl.found()
    error('OpenGL implementation not found')
endif

deps += dep_gl

if not get_option('csri').disabled() and host_machine.system() == 'windows'
    conf.set('WITH_CSRI', 1)

    csri_sp = subproject('csri')
    deps += csri_sp.get_variable('csri_dep')
endif

if cc.has_function('alloca', prefix: '#include <stdlib.h>')
    # The files using alloca() already include <stdlib.h> unconditionally.
elif cc.has_function('alloca', prefix: '#include <alloca.h>')
    conf.set('HAVE_ALLOCA_H', true)
elif cc.has_header_symbol('malloc.h', '_alloca', prefix: '#include <stdlib.h>')
    # cc.has_function() can't find _alloca() intrinsic on cl.exe, 
    # so we use cc.has_header_symbol().
    conf.set('HAVE_MALLOC_H', true)
    conf.set('HAVE_UNDERLINE_ALLOCA', true)
elif cc.has_header_symbol('malloc.h', 'alloca', prefix: '#include <stdlib.h>')
    # clang-cl.exe has the wrong name for cl.exe's _alloca()
    conf.set('HAVE_MALLOC_H', true)
endif

acconf = configure_file(output: 'acconf.h', configuration: conf)

subdir('automation')
subdir('libaegisub')
subdir('packages')
subdir('po')
subdir('src')
subdir('tests')

aegisub_cpp_pch = ['src/include/agi_pre.h']
aegisub_c_pch = ['src/include/agi_pre_c.h']

aegisub = executable('aegisub', aegisub_src, version_h, acconf, resrc,
                     link_with: [libresrc, libaegisub],
                     include_directories: [libaegisub_inc, libresrc_inc, version_inc, deps_inc, include_directories('src')],
                     cpp_pch: aegisub_cpp_pch,
                     c_pch: aegisub_c_pch,
                     install: true,
                     install_dir: bindir,
                     dependencies: deps,
                     win_subsystem: 'windows')

if host_machine.system() == 'windows'
    mt_exe = find_program('mt.exe')
    apply_manifest = find_program(meson.project_source_root() / 'tools/apply-manifest.py')
    custom_target('apply-manifest',
                  input: aegisub,
                  output: 'applied_manifest',
                  command: [apply_manifest, mt_exe, '@INPUT@'],
                  build_by_default: true)
endif

project('Aegisub', ['c', 'cpp'], license: 'BSD-3-Clause',
        meson_version: '>=0.49.0',
        default_options: ['cpp_std=c++14', 'b_lto=true'])

if host_machine.system() == 'windows'
    version_sh = find_program('tools/version.ps1')
else
    version_sh = find_program('tools/version.sh')
endif
version_inc = include_directories('.')
version_h = custom_target('git_version.h',
                          command: [version_sh, meson.source_root()],
                          build_by_default: true,
                          build_always_stale: true, # has internal check whether target file will be refreshed
                          output: ['git_version.h', 'git_version.xml'])

dataroot = get_option('prefix') / get_option('datadir') / 'aegisub'
add_project_arguments('-DP_DATA="@0@"'.format(dataroot), language: 'cpp')

if host_machine.system() == 'windows'
    add_project_arguments('-DNOMINMAX', '-D_WIN32_WINNT=0x0602', language: 'cpp')
endif

conf = configuration_data()
conf.set_quoted('P_DATA', dataroot)
if get_option('build_credit') != ''
    conf.set_quoted('BUILD_CREDIT', get_option('build_credit'))
endif
conf.set('WITH_UPDATE_CHECKER', get_option('enable_update_checker'))

deps = []
deps_inc = []

if host_machine.system() == 'darwin'
    add_languages('objc')
elif host_machine.system() != 'windows'
    deps += dependency('fontconfig')
endif

cxx = meson.get_compiler('cpp')
cc = meson.get_compiler('c')
deps += cc.find_library('m', required: false)
deps += cc.find_library('dl', required: false)
iconv_dep = cc.find_library('iconv', required: false)
if not iconv_dep.found() and host_machine.system() == 'windows'
    iconv_sp = subproject('iconv') # this really needs to be replaced with a proper port
    deps_inc += iconv_sp.get_variable('iconv_incs')
    iconv_dep = iconv_sp.get_variable('libiconv_dep')
endif
deps += iconv_dep

deps += dependency('libass', version: '>=0.9.7')
deps += dependency('boost', version: '>=1.50.0',
                   modules: ['chrono', 'filesystem', 'locale', 'regex',
                             'system', 'thread'])
deps += dependency('zlib')

icu_uc_dep = dependency('icu-uc', version: '>=4.8.1.1')
deps += icu_uc_dep
icu_i18n_dep = dependency('icu-i18n', version: '>=4.8.1.1')
deps += icu_i18n_dep

dep_avail = []
foreach dep: [
    # audio, in order of precedence
    ['libpulse', '', 'PulseAudio'],
    ['alsa', '', 'ALSA'],
    ['portaudio-2.0', '', 'PortAudio'],
    ['openal', '>=0.0.8', 'OpenAL'],
    # video
    ['ffms2', '', 'FFMS2'],
    # other
    ['fftw3', '', 'FFTW3'],
    ['hunspell', '', 'Hunspell'],
    ['uchardet', '', 'uchardet'],
]
    d = dependency(dep[0], version: dep[1] != '' ? dep[1]: '>=0',
                   required: false)

    optname = dep[0].split('-')[0]
    if d.found() and not get_option(optname).disabled()
        deps += d
        conf.set('WITH_@0@'.format(dep[0].split('-')[0].to_upper()), '1')
        dep_avail += dep[2]
    elif get_option(optname).enabled()
        error('@0@ enabled but not found'.format(dep[2]))
    endif
endforeach

if host_machine.system() == 'windows' and not get_option('directsound').disabled()
    dsound_dep = cc.find_library('dsound', required: get_option('directsound'))
    winmm_dep = cc.find_library('winmm', required: get_option('directsound'))
    ole32_dep = cc.find_library('ole32', required: get_option('directsound'))
    have_dsound_h = cc.has_header('dsound.h')
    if not have_dsound_h and get_option('directsound').enabled()
        error('DirectSound enabled but dsound.h not found')
    endif
    if dsound_dep.found() and winmm_dep.found() and ole32_dep.found() and have_dsound_h
        deps += [dsound_dep, winmm_dep, ole32_dep]
        conf.set('WITH_DIRECTSOUND', '1')
        dep_avail += 'DirectSound'
    endif
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

conf_platform = configuration_data()
conf_platform.set('DEFAULT_PLAYER_AUDIO', def_audio)

luajit = dependency('luajit', version: '>=2.0.0', required: get_option('system_luajit'))
if luajit.found()
    luajit_test = cc.run('''#include <lauxlib.h>
int main(void)
{
    lua_State *L = luaL_newstate();
    if (!L) return 1;
    // This is valid in lua 5.2, but a syntax error in 5.1
    const char testprogram[] = "function foo() while true do break return end end";
    return luaL_loadstring(L, testprogram) == LUA_ERRSYNTAX;
}''', dependencies: luajit)

    if luajit_test.returncode() == 1
        if get_option('system_luajit')
            error('System luajit found but not compiled in 5.2 mode')
        else
            message('System luajit found but not compiled in 5.2 mode; using built-in luajit')
        endif
    else
        deps += luajit
    endif
else
    message('System luajit not found; using built-in luajit')
endif

if not deps.contains(luajit)
    luajit_sp = subproject('luajit')
    luajit_inc = luajit_sp.get_variable('incdir')
    deps += luajit_sp.get_variable('luajit_dep')
else
    luajit_inc = include_directories(luajit.get_pkgconfig_variable('includedir'))
endif
subdir('vendor/luabins/src')

deps += dependency('wxWidgets', version: '>=3.0.0',
                   modules: ['std', 'stc', 'gl'])

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

if not dep_gl.found()
    error('OpenGL implementation not found')
endif

deps += dep_gl

acconf = configure_file(output: 'acconf.h', configuration: conf)

subdir('automation')
subdir('libaegisub')
subdir('packages')
subdir('po')
subdir('src')

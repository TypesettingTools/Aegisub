project('Aegisub', ['c', 'cpp'],
        license: 'BSD-3-Clause',
        meson_version: '>=0.56.1',
        default_options: ['cpp_std=c++14', 'buildtype=debugoptimized'],
        version: '3.2.2')

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
    version_sh = find_program('tools/version.ps1')
else
    version_sh = find_program('tools/version.sh')
endif
version_inc = include_directories('.')
version_h = custom_target('git_version.h',
                          command: [version_sh, meson.current_build_dir(), meson.current_source_dir()],
                          build_by_default: true,
                          build_always_stale: true, # has internal check whether target file will be refreshed
                          output: ['git_version.h'])

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
conf.set('WITH_UPDATE_CHECKER', get_option('enable_update_checker'))

deps = []
deps_inc = []

if host_machine.system() == 'darwin'
    add_languages('objc', 'objcpp')
    add_project_arguments('-DGL_SILENCE_DEPRECATION', language: 'cpp')
    # meson neither supports objcpp_std nor inherits cpp_std https://github.com/mesonbuild/meson/issues/5495
    add_project_arguments('-std=c++11', language: 'objcpp')
elif host_machine.system() != 'windows'
    conf.set('WITH_FONTCONFIG', 1)
    deps += dependency('fontconfig')
endif

cxx = meson.get_compiler('cpp')
cc = meson.get_compiler('c')
deps += cc.find_library('m', required: false)
deps += cc.find_library('dl', required: false)

if meson.version().version_compare('>=0.60.0')
  iconv_dep = dependency('iconv', fallback: ['iconv', 'libiconv_dep'])
else
  iconv_dep = cc.find_library('iconv', required: false)
  if not (iconv_dep.found() or cc.has_function('iconv_open'))
      iconv_sp = subproject('iconv') # this really needs to be replaced with a proper port
      iconv_dep = iconv_sp.get_variable('libiconv_dep')
  endif
endif
deps += iconv_dep

deps += dependency('libass', version: '>=0.9.7',
                   fallback: ['libass', 'libass_dep'])

boost_modules = ['chrono', 'filesystem', 'thread', 'locale', 'regex']
if not get_option('local_boost')
    boost_dep = dependency('boost', version: '>=1.50.0',
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
    opt_var.add_cmake_defines({
        'CMAKE_BUILD_TYPE': build_type,

        'wxBUILD_INSTALL': false,
        'wxBUILD_PRECOMP': false,       # otherwise breaks project generation w/ meson
        'wxBUILD_MONOLITHIC': true,     # otherwise breaks project generation w/ meson
        'wxBUILD_SHARED': build_shared,

        'WXWIN_COMPATIBILITY_2_4': false,
        'WXWIN_COMPATIBILITY_2_6': false,
        'WXWIN_COMPATIBILITY_2_8': false,
        'wxDIALOG_UNIT_COMPATIBILITY': false,
        'wxUSE_ON_FATAL_EXCEPTION': 1,
        'wxUSE_STACKWALKER': 0,
        'wxUSE_DEBUGREPORT': 0,
        'wxUSE_DEBUG_CONTEXT': 0,
        'wxUSE_MEMORY_TRACING': 0,
        'wxUSE_GLOBAL_MEMORY_OPERATORS': 0,
        'wxUSE_DEBUG_NEW_ALWAYS': 0,
        'wxUSE_UNICODE': 1,
        'wxUSE_WCHAR_T': 1,
        'wxUSE_EXCEPTIONS': 1,
        'wxUSE_EXTENDED_RTTI': 0,
        'wxUSE_STL': 0,
        'wxUSE_LOG': 1,
        'wxUSE_LOGWINDOW': 1,
        'wxUSE_LOGGUI': 1,
        'wxUSE_LOG_DIALOG': 1,
        'wxUSE_CMDLINE_PARSER': 0,
        'wxUSE_THREADS': 1,
        'wxUSE_STREAMS': 1,
        'wxUSE_STD_IOSTREAM': 0,
        'wxUSE_STD_STRING': 1,
        'wxUSE_PRINTF_POS_PARAMS': 1,
        'wxUSE_LONGLONG': 1,
        'wxUSE_BASE64': 0,
        'wxUSE_CONSOLE_EVENTLOOP': 1,
        'wxUSE_FILE': 1,
        'wxUSE_FFILE': 1,
        'wxUSE_FSVOLUME': 0,
        'wxUSE_STDPATHS': 1,
        'wxUSE_TEXTBUFFER': 0,
        'wxUSE_TEXTFILE': 0,
        'wxUSE_INTL': 1,
        'wxUSE_XLOCALE': 1,
        'wxUSE_DATETIME': 1,
        'wxUSE_TIMER': 1,
        'wxUSE_STOPWATCH': 1,
        'wxUSE_FSWATCHER': 0,
        'wxUSE_CONFIG': 0,
        'wxUSE_CONFIG_NATIVE': 0,
        'wxUSE_DIALUP_MANAGER': 0,
        'wxUSE_DYNLIB_CLASS': 1,
        'wxUSE_DYNAMIC_LOADER': 1,
        'wxUSE_SOCKETS': 0,
        'wxUSE_IPV6': 0,
        'wxUSE_FILESYSTEM': 0,
        'wxUSE_FS_ZIP': 0,
        'wxUSE_FS_ARCHIVE': 0,
        'wxUSE_FS_INET': 0,
        'wxUSE_ARCHIVE_STREAMS': 1,
        'wxUSE_ZIPSTREAM': 1,
        'wxUSE_TARSTREAM': 0,
        'wxUSE_APPLE_IEEE': 0,
        'wxUSE_JOYSTICK': 0,
        'wxUSE_FONTENUM': 1,
        'wxUSE_FONTMAP': 1,
        'wxUSE_MIMETYPE': 0,
        'wxUSE_PROTOCOL': 0,
        'wxUSE_PROTOCOL_FILE': 0,
        'wxUSE_PROTOCOL_FTP': 0,
        'wxUSE_PROTOCOL_HTTP': 0,
        'wxUSE_URL': 0,
        'wxUSE_URL_NATIVE': 0,
        'wxUSE_VARIANT': 1,
        'wxUSE_ANY': 0,
        'wxUSE_REGEX': 'OFF',
        'wxUSE_SYSTEM_OPTIONS': 1,
        'wxUSE_SOUND': 0,
        'wxUSE_MEDIACTRL': 0,
        'wxUSE_GSTREAMER': 0,
        'wxUSE_XRC': 0,
        'wxUSE_XML': 0,
        'wxUSE_AUI': 0,
        'wxUSE_RIBBON': 0,
        'wxUSE_PROPGRID': 0,
        'wxUSE_STC': 1,
        'wxUSE_GRAPHICS_CONTEXT': 1,
        'wxUSE_GRAPHICS_GDIPLUS': 1,
        'wxUSE_CONTROLS': 1,
        'wxUSE_POPUPWIN': 1,
        'wxUSE_TIPWINDOW': 0,
        'wxUSE_ANIMATIONCTRL': 0,
        'wxUSE_BUTTON': 1,
        'wxUSE_BMPBUTTON': 1,
        'wxUSE_CALENDARCTRL': 1,
        'wxUSE_CHECKBOX': 1,
        'wxUSE_CHECKLISTBOX': 1,
        'wxUSE_CHOICE': 1,
        'wxUSE_COLLPANE': 0,
        'wxUSE_COLOURPICKERCTRL': 1,
        'wxUSE_COMBOBOX': 1,
        'wxUSE_DATAVIEWCTRL': 1,
        'wxUSE_DATEPICKCTRL': 1,
        'wxUSE_DIRPICKERCTRL': 1,
        'wxUSE_EDITABLELISTBOX': 1,
        'wxUSE_FILECTRL': 1,
        'wxUSE_FILEPICKERCTRL': 1,
        'wxUSE_FONTPICKERCTRL': 1,
        'wxUSE_GAUGE': 1,
        'wxUSE_HEADERCTRL': 1,
        'wxUSE_HYPERLINKCTRL': 1,
        'wxUSE_LISTBOX': 1,
        'wxUSE_LISTCTRL': 1,
        'wxUSE_RADIOBOX': 1,
        'wxUSE_RADIOBTN': 1,
        'wxUSE_SCROLLBAR': 1,
        'wxUSE_SEARCHCTRL': 1,
        'wxUSE_SLIDER': 1,
        'wxUSE_SPINBTN': 1,
        'wxUSE_SPINCTRL': 1,
        'wxUSE_STATBOX': 1,
        'wxUSE_STATLINE': 1,
        'wxUSE_STATTEXT': 1,
        'wxUSE_STATBMP': 1,
        'wxUSE_TEXTCTRL': 1,
        'wxUSE_TOGGLEBTN': 1,
        'wxUSE_TREECTRL': 1,
        'wxUSE_STATUSBAR': 1,
        'wxUSE_NATIVE_STATUSBAR': 1,
        'wxUSE_TOOLBAR': 1,
        'wxUSE_TOOLBAR_NATIVE': 1,
        'wxUSE_NOTEBOOK': 1,
        'wxUSE_LISTBOOK': 1,
        'wxUSE_CHOICEBOOK': 1,
        'wxUSE_TREEBOOK': 1,
        'wxUSE_TOOLBOOK': 1,
        'wxUSE_TASKBARICON': 0,
        'wxUSE_TAB_DIALOG': 0,
        'wxUSE_GRID': 0,
        'wxUSE_MINIFRAME': 0,
        'wxUSE_COMBOCTRL': 1,
        'wxUSE_ODCOMBOBOX': 1,
        'wxUSE_BITMAPCOMBOBOX': 0,
        'wxUSE_REARRANGECTRL': 1,
        'wxUSE_ACCEL': 1,
        'wxUSE_HOTKEY': 1,
        'wxUSE_CARET': 1,
        'wxUSE_DISPLAY': 1,
        'wxUSE_GEOMETRY': 1,
        'wxUSE_IMAGLIST': 1,
        'wxUSE_INFOBAR': 1,
        'wxUSE_MENUS': 1,
        'wxUSE_NOTIFICATION_MESSAGE': 0,
        'wxUSE_SASH': 1,
        'wxUSE_SPLITTER': 1,
        'wxUSE_TOOLTIPS': 1,
        'wxUSE_VALIDATORS': 1,
        'wxUSE_COMMON_DIALOGS': 1,
        'wxUSE_BUSYINFO': 0,
        'wxUSE_CHOICEDLG': 1,
        'wxUSE_COLOURDLG': 1,
        'wxUSE_DIRDLG': 1,
        'wxUSE_FILEDLG': 1,
        'wxUSE_FINDREPLDLG': 0,
        'wxUSE_FONTDLG': 1,
        'wxUSE_MSGDLG': 1,
        'wxUSE_PROGRESSDLG': 0,
        'wxUSE_STARTUP_TIPS': 0,
        'wxUSE_TEXTDLG': 1,
        'wxUSE_NUMBERDLG': 1,
        'wxUSE_SPLASH': 0,
        'wxUSE_WIZARDDLG': 0,
        'wxUSE_ABOUTDLG': 1,
        'wxUSE_METAFILE': 0,
        'wxUSE_ENH_METAFILE': 0,
        'wxUSE_WIN_METAFILES_ALWAYS': 0,
        'wxUSE_MDI': 0,
        'wxUSE_DOC_VIEW_ARCHITECTURE': 0,
        'wxUSE_MDI_ARCHITECTURE': 0,
        'wxUSE_PRINTING_ARCHITECTURE': 0,
        'wxUSE_HTML': 0,
        'wxUSE_GLCANVAS': 1,
        'wxUSE_RICHTEXT': 0,
        'wxUSE_CLIPBOARD': 1,
        'wxUSE_DATAOBJ': 1,
        'wxUSE_DRAG_AND_DROP': 1,
        'wxUSE_ACCESSIBILITY': 0,
        'wxUSE_SNGLINST_CHECKER': 1,
        'wxUSE_DRAGIMAGE': 1,
        'wxUSE_IPC': 0,
        'wxUSE_HELP': 0,
        'wxUSE_MS_HTML_HELP': 0,
        'wxUSE_WXHTML_HELP': 0,
        'wxUSE_RESOURCES': 0,
        'wxUSE_CONSTRAINTS': 0,
        'wxUSE_SPLINES': 0,
        'wxUSE_MOUSEWHEEL': 1,
        'wxUSE_POSTSCRIPT': 0,
        'wxUSE_AFM_FOR_POSTSCRIPT': 0,
        'wxUSE_SVG': 0,
        'wxODBC_FWD_ONLY_CURSORS': false,
        'wxODBC_BACKWARD_COMPATABILITY': false,
        'wxUSE_IOSTREAMH': 0,
        'wxUSE_IMAGE': 1,
        'wxUSE_LIBJPEG': 'OFF',
        'wxUSE_LIBTIFF': 'OFF',
        'wxUSE_TGA': 'OFF',
        'wxUSE_GIF': 'OFF',
        'wxUSE_PNM': 'OFF',
        'wxUSE_PCX': 'OFF',
        'wxUSE_IFF': 'OFF',
        'wxUSE_XPM': 'OFF',
        'wxUSE_ICO_CUR': 1,
        'wxUSE_PALETTE': 1,
        'wxUSE_ALL_THEMES': 1,
        'wxUSE_UNICODE_MSLU': 0,
        'wxUSE_MFC': 0,
        'wxUSE_OLE': 1,
        'wxUSE_OLE_AUTOMATION': 0,
        'wxUSE_ACTIVEX': 0,
        'wxUSE_DC_CACHEING': 1,
        'wxUSE_DIB_FOR_BITMAP': 0,
        'wxUSE_WXDIB': 1,
        'wxUSE_POSTSCRIPT_ARCHITECTURE_IN_MSW': 0,
        'wxUSE_REGKEY': 1,
        'wxUSE_RICHEDIT': 0,
        'wxUSE_RICHEDIT2': 0,
        'wxUSE_OWNER_DRAWN': 1,
        'wxUSE_TASKBARICON_BALLOONS': 0,
        'wxUSE_UXTHEME': 1,
        'wxUSE_UXTHEME_AUTO': 1,
        'wxUSE_INKEDIT': 0,
        'wxUSE_INICONF': 0,
        'wxUSE_DATEPICKCTRL_GENERIC': 0,
        'wxUSE_CRASHREPORT': 1,
        'wxUSE_AUTOID_MANAGEMENT': 1,
        'wxUSE_FILE_HISTORY': 0,
        'wxUSE_UIACTIONSIMULATOR': 0,
        'wxUSE_CAIRO': 0,
        'wxUSE_COMMANDLINKBUTTON': 0,
        'wxUSE_RICHMSGDLG': 0,
        'wxUSE_STD_CONTAINERS': 1,
        'wxUSE_STD_STRING_CONV_IN_WXSTRING': 0,
        'wxUSE_ARTPROVIDER_STD': 0,
        'wxUSE_ARTPROVIDER_TANGO': 0,
        'wxUSE_DC_TRANSFORM_MATRIX': 0,
        'wxUSE_MARKUP': 0,
        'wxUSE_TREELISTCTRL': 1,
        'wxUSE_TIMEPICKCTRL': 1,
        'wxUSE_WEBVIEW': 0,
        'wxUSE_RICHTOOLTIP': 0,
        'wxUSE_COMPILER_TLS': 2,
        'wxUSE_PREFERENCES_EDITOR': 0,
        'wxUSE_STD_CONTAINERS_COMPATIBLY': 0,
        'wxUSE_TASKBARBUTTON': 0,
        'wxUSE_ADDREMOVECTRL': 0,
        'wxUSE_ACTIVITYINDICATOR': 0,
        'wxUSE_WINRT': 0,
    })

    wx = cmake.subproject('wxWidgets', options: opt_var)

    deps += [
        wx.dependency('wxmono'),
        wx.dependency('wxscintilla'),
        # wx.dependency('wxpng'),
        # wx.dependency('wxzlib'),
        # wx.dependency('wxexpat'),
    ]

    if host_machine.system() == 'windows'
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
        dep_avail += 'DirectSound'
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

conf_platform = configuration_data()
conf_platform.set('DEFAULT_PLAYER_AUDIO', def_audio)

luajit = dependency('luajit', version: '>=2.0.0', required: get_option('system_luajit'))
if luajit.found() and luajit.type_name() != 'internal'
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
    deps += subproject('luajit').get_variable('luajit_dep')
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

acconf = configure_file(output: 'acconf.h', configuration: conf)

subdir('automation')
subdir('libaegisub')
subdir('packages')
subdir('po')
subdir('src')
subdir('tests')

aegisub_cpp_pch = ['src/include/agi_pre.h']
aegisub_c_pch = ['src/include/agi_pre_c.h']

aegisub = executable('aegisub', aegisub_src, version_h, acconf,
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

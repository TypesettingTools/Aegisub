
cpp = meson.get_compiler('cpp')
test_ut8_conversion = cpp.compiles(
    '''#include <cuchar>
#include "UTF8StringHelper.hpp"
int main(void)
{
    const std::u8string str = u8"abc\u300adef";
    std::string const normal_str = ToECSString(str);
}''',
    include_directories: include_directories('ass'),
)

if not test_ut8_conversion
    error('No proper unicode support is present in the compiler!')
endif


libaegisub_src = [
    'ass/dialogue_parser.cpp',
    'ass/time.cpp',
    'ass/uuencode.cpp',
    'ass/UTF8StringHelper.hpp',

    'audio/provider_convert.cpp',
    'audio/provider.cpp',
    'audio/provider_dummy.cpp',
    'audio/provider_hd.cpp',
    'audio/provider_lock.cpp',
    'audio/provider_pcm.cpp',
    'audio/provider_ram.cpp',

    'common/calltip_provider.cpp',
    'common/character_count.cpp',
    'common/charset_6937.cpp',
    'common/charset_conv.cpp',
    'common/charset.cpp',
    'common/color.cpp',
    'common/file_mapping.cpp',
    'common/format.cpp',
    'common/fs.cpp',
    'common/hotkey.cpp',
    'common/io.cpp',
    'common/json.cpp',
    'common/kana_table.cpp',
    'common/karaoke_matcher.cpp',
    'common/keyframe.cpp',
    'common/line_iterator.cpp',
    'common/log.cpp',
    'common/mru.cpp',
    'common/option.cpp',
    'common/option_value.cpp',
    'common/parser.cpp',
    'common/path.cpp',
    'common/thesaurus.cpp',
    'common/util.cpp',
    'common/vfr.cpp',
    'common/ycbcr_conv.cpp',
    'common/cajun/elements.cpp',
    'common/cajun/reader.cpp',
    'common/cajun/writer.cpp',

    'lua/modules.cpp',
    'lua/script_reader.cpp',
    'lua/utils.cpp',
    'lua/modules/lfs.cpp',
    'lua/modules/re.cpp',
    'lua/modules/unicode.cpp',
    'lua/modules/lpeg.c',
]

if host_machine.system() == 'darwin'
    libaegisub_src += [
        'osx/dispatch.mm',
        'osx/spellchecker.mm',
        'osx/util.mm',
    ]
else
    libaegisub_src += 'common/dispatch.cpp'
endif

if host_machine.system() == 'windows'
    libaegisub_src += [
        'windows/access.cpp',
        'windows/charset_conv_win.cpp',
        'windows/fs.cpp',
        'windows/log_win.cpp',
        'windows/path_win.cpp',
        'windows/util_win.cpp',
    ]
else
    libaegisub_src += [
        'unix/access.cpp',
        'unix/fs.cpp',
        'unix/log.cpp',
        'unix/path.cpp',
        'unix/util.cpp',
    ]
endif

libaegisub_cpp_pch = ['include/lagi_pre.h']
libaegisub_c_pch = ['include/lagi_pre_c.h']

libaegisub_inc = include_directories('include')

libaegisub = static_library(
    'aegisub',
    libaegisub_src,
    acconf,
    include_directories: [libaegisub_inc, deps_inc],
    cpp_pch: libaegisub_cpp_pch,
    c_pch: libaegisub_c_pch,
    dependencies: deps,
)

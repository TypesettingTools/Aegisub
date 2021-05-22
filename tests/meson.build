all_test_deps = []
all_test_dep_libs = []
all_test_sources = []


gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_dep')
gmock_dep = gtest_proj.get_variable('gmock_dep')


all_test_deps += [
    gtest_dep, 
    gmock_dep,
    iconv_dep,
    boost_dep
]
all_test_dep_libs += [
    libaegisub
]
all_test_sources += [
    'support/main.cpp',
    'support/util.cpp',

    'tests/access.cpp',
    'tests/audio.cpp',
    'tests/cajun.cpp',
    'tests/calltip_provider.cpp',
    'tests/character_count.cpp',
    'tests/color.cpp',
    'tests/dialogue_lexer.cpp',
    'tests/format.cpp',
    'tests/fs.cpp',
    'tests/hotkey.cpp',
    'tests/iconv.cpp',
    'tests/ifind.cpp',
    'tests/karaoke_matcher.cpp',
    'tests/keyframe.cpp',
    'tests/line_iterator.cpp',
    'tests/line_wrap.cpp',
    'tests/mru.cpp',
    'tests/option.cpp',
    'tests/path.cpp',
    'tests/signals.cpp',
    'tests/split.cpp',
    'tests/syntax_highlight.cpp',
    'tests/thesaurus.cpp',
    'tests/time.cpp',
    'tests/type_name.cpp',
    'tests/util.cpp',
    'tests/uuencode.cpp',
    'tests/vfr.cpp',
    'tests/word_split.cpp'
]

test_inc = include_directories('support')

# force UTF8 for MSVC
if host_machine.system() == 'windows'
    extra_args = ['/utf-8']
else
    extra_args = []
endif

e = executable(
    'gtest-main',
    all_test_sources,
    include_directories : [test_inc, libaegisub_inc, deps_inc],
    dependencies : all_test_deps,
    cpp_args : extra_args,
    link_with : all_test_dep_libs,
)    
test('gtest main', e)


# setup test env
if host_machine.system() == 'windows'
    setup_sh = find_program('setup.bat')
else
    setup_sh = find_program('setup.sh')
endif

test_data_des = meson.project_build_root()
custom_target('setup-test-data',
    input: e,
    output: 'applied_test_setup',
    command: [setup_sh, test_data_des],
    build_by_default: true
)

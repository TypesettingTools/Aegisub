aegisub_lua_args = []
# force UTF8 for MSVC
if host_machine.system() == 'windows'
    aegisub_lua_args += ['/utf-8']
endif

aegisub_lua = executable(
    'aegisub-lua',
    'aegisub.cpp',
    include_directories: libaegisub_inc,
    dependencies: deps,
    cpp_args: aegisub_lua_args + conf_defines,
    link_with: libaegisub,
)

# Requires busted installed through luarocks for Lua 5.1; the fixtures are
# POSIX-only, so the suite is never registered on Windows
have_busted = false
if host_machine.system() != 'windows'
    luarocks = find_program('luarocks', required: false)
    if luarocks.found()
        have_busted = run_command(luarocks, '--lua-version=5.1', 'show', 'busted', check: false).returncode() == 0
    endif
endif

if have_busted
    lr_path = run_command(luarocks, '--lua-version=5.1', 'path', '--lr-path', check: false).stdout().strip()
    lr_cpath = run_command(luarocks, '--lua-version=5.1', 'path', '--lr-cpath', check: false).stdout().strip()
    test('automation',
        aegisub_lua,
        args: ['tests/busted.lua', '-p', 'moon', 'tests/modules'],
        workdir: meson.current_source_dir() / '..',
        env: {'LUA_PATH': lr_path + ';;', 'LUA_CPATH': lr_cpath + ';;'},
        suite: 'automation')
else
    message('luarocks with busted for Lua 5.1 not found; not adding the automation test suite')
endif

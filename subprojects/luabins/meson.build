project('luabins', ['c'])
luabins_src = files(
    'src/fwrite.c',
    'src/load.c',
    'src/luabins.c',
    'src/luainternals.c',
    'src/savebuffer.c',
    'src/save.c',
    'src/write.c',
)

deps = []

luajit = dependency('luajit', version: '>=2.0.0', required: false,
                    allow_fallback: false, method: 'pkg-config')
if luajit.found() and luajit.type_name() != 'internal'
    luajit_test = meson.get_compiler('c').run('''#include <lauxlib.h>
int main(void)
{
    lua_State *L = luaL_newstate();
    if (!L) return 1;
    // This is valid in lua 5.2, but a syntax error in 5.1
    const char testprogram[] = "function foo() while true do break return end end";
    return luaL_loadstring(L, testprogram) == LUA_ERRSYNTAX;
}''', dependencies: luajit)

    if luajit_test.returncode() == 1
        message('System luajit found but not compiled in 5.2 mode')
        luajit = dependency('', required: false)
    else
        deps += luajit
    endif
else
    message('System luajit not found')
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

luabins = static_library('luabins', luabins_src, dependencies: deps)
luabins_dep = declare_dependency(link_with: luabins)

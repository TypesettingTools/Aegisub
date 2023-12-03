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

luajit = dependency('luajit', version: '>=2.0.0')
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
        message('System luajit found but not compiled in 5.2 mode; using built-in luajit')
    else
        deps += luajit
    endif
else
    message('System luajit not found; using built-in luajit')
endif

if not deps.contains(luajit)
    deps += subproject('luajit').get_variable('luajit_dep')
endif

luabins = static_library('luabins', luabins_src, dependencies: deps)
luabins_dep = declare_dependency(link_with: luabins)

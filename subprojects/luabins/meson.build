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

# Define variables used by the copypasted Aegisub code.
cc = meson.get_compiler('c')
lua_provider = 'auto'
# Lua dependency code from ../../meson.build (Aegisub's meson.build).
lua_auto = lua_provider == 'auto'
if ['auto', 'luajit-system'].contains(lua_provider)
    lua = dependency('luajit', 
                     version: '>=2.0.0', 
                     required: lua_provider == 'luajit-system',
                     allow_fallback: lua_auto)

    luajit_test = cc.run('''#include <lauxlib.h>
int main(void)
{
    lua_State *L = luaL_newstate();
    if (!L) return 1;
    // This is valid in lua 5.2, but a syntax error in 5.1
    const char testprogram[] = "function foo() while true do break return end end";
    return luaL_loadstring(L, testprogram) == LUA_ERRSYNTAX;
}''', dependencies: lua)

    if luajit_test.returncode() == 1 and lua_provider == 'luajit-system'
        error('luajit found but not compiled in 5.2 mode')
    else
        lua = disabler()
        warning('luajit found but not compiled in 5.2 mode; trying fallback')
    endif
endif
if (lua_auto and not lua.found()) or lua_provider == 'lua-system'
    lua = dependency('lua52', 'lua-5.2', 
                     required: lua_provider == 'lua_system')
endif
if (lua_auto and not lua.found()) or lua_provider == 'luajit-local'
    lua = subproject('luajit').get_variable('luajit_dep')
endif
deps += lua

luabins = static_library('luabins', luabins_src, dependencies: deps)
luabins_dep = declare_dependency(link_with: luabins)

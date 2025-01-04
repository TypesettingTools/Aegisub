#include <stdio.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

int main(void) {
    int ret = 0;

    lua_State *L = luaL_newstate();
    if (!L)
        return 1;

    luaL_openlibs(L);

    const char *test = "function foo() while true do table.pack(1,2) break return end end foo()";
    if (luaL_loadstring(L, test) == LUA_ERRSYNTAX) {
        printf("%s\n", lua_tostring(L, -1));
        ret = 2;
        goto exit;
    }

    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        printf("%s\n", lua_tostring(L, -1));
        ret = 3;
        goto exit;
    }

exit:
    lua_close(L);
    return ret;
}

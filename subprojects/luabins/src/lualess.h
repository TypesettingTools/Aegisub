/*
* lualess.h
* Lua-related definitions for lua-less builds (based on Lua manual)
* See copyright notice in luabins.h
*/

#ifndef LUABINS_LUALESS_H_INCLUDED_
#define LUABINS_LUALESS_H_INCLUDED_

#include <stddef.h> /* ptrdiff_t */

/* WARNING: Change these if your luaconf.h has different settings */
typedef double lua_Number;
typedef ptrdiff_t lua_Integer;

typedef void * (*lua_Alloc) (void *ud,
                             void *ptr,
                             size_t osize,
                             size_t nsize)
                             ;

/*
* lua_Alloc-compatible allocator to use in Lua-less applications
* with lbs_SaveBuffer. Based on sample code from Lua 5.1 manual.
*/
void * lbs_simplealloc(
    void * ud,
    void * ptr,
    size_t osize,
    size_t nsize
  );

#endif /* LUABINS_LUALESS_H_INCLUDED_ */

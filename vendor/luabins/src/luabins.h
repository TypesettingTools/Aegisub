/*
* luabins.h
* luabins -- Lua Binary Serialization Module
* See copyright notice at the end of this file.
*/

#ifndef LUABINS_H_INCLUDED_
#define LUABINS_H_INCLUDED_

#define LUABINS_VERSION     "Luabins 0.3"
#define LUABINS_COPYRIGHT   "Copyright (C) 2009-2010, Luabins authors"
#define LUABINS_DESCRIPTION "Trivial Lua Binary Serialization Library"

/* Define LUABINS_LUABUILTASCPP if your Lua is built as C++ */

/* Can't be more than 255 */
#define LUABINS_MAXTUPLE (250)

#define LUABINS_MAXTABLENESTING (250)

/*
* Save Lua values from given state at given stack index range.
* Lua value is left untouched. Note that empty range is not an error.
* You may save from 0 to LUABINS_MAXTUPLE values.
* Returns 0 on success, pushes saved data as a string on the top of stack.
* Returns non-zero on failure, pushes error message on the top
* of the stack.
*/
int luabins_save(lua_State * L, int index_from, int index_to);

/*
* Load Lua values from given byte chunk.
* Returns 0 on success, pushes loaded values on stack.
* Sets count to the number of values pushed.
* Note that to have zero loaded items is a valid scenario.
* Returns non-zero on failure, pushes error message on the top
* of the stack.
*/
int luabins_load(
    lua_State * L,
    const unsigned char * data,
    size_t len,
    int * count
  );

/******************************************************************************
* Copyright (C) 2009-2010 Luabins authors. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#endif /* LUABINS_H_INCLUDED_ */

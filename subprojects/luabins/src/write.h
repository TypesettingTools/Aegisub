/*
* write.h
* Luabins Lua-less write API
* See copyright notice in luabins.h
*/

#ifndef LUABINS_WRITE_H_INCLUDED_
#define LUABINS_WRITE_H_INCLUDED_

#include "saveload.h"
#include "savebuffer.h"

#define LUABINS_APPEND ((size_t)-1)

#define lbs_writeTupleSize(sb, tuple_size) \
  lbsSB_writechar((sb), (tuple_size))

int lbs_writeTableHeaderAt(
    luabins_SaveBuffer * sb,
    size_t offset, /* Pass LUABINS_APPEND to append to the end of buffer */
    int array_size,
    int hash_size
  );

#define lbs_writeTableHeader(sb, array_size, hash_size) \
  lbs_writeTableHeaderAt((sb), LUABINS_APPEND, (array_size), (hash_size))

#define lbs_writeNil(sb) \
  lbsSB_writechar((sb), LUABINS_CNIL)

#define lbs_writeBoolean(sb, value) \
  lbsSB_writechar((sb), ((value) == 0) ? LUABINS_CFALSE : LUABINS_CTRUE)

int lbs_writeNumber(luabins_SaveBuffer * sb, lua_Number value);

#define lbs_writeInteger lbs_writeNumber

int lbs_writeString(
    luabins_SaveBuffer * sb,
    const char * value,
    size_t length
  );

#endif /* LUABINS_WRITE_H_INCLUDED_ */

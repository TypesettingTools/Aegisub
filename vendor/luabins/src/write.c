/*
* write.c
* Luabins Lua-less write API
* See copyright notice in luabins.h
*/

#include "luaheaders.h"

#include "write.h"

int lbs_writeTableHeaderAt(
    luabins_SaveBuffer * sb,
    size_t offset, /* Pass LUABINS_APPEND to append to the end of buffer */
    int array_size,
    int hash_size
  )
{
  int result = lbsSB_grow(sb, 1 + LUABINS_LINT + LUABINS_LINT);
  if (result == LUABINS_ESUCCESS)
  {
    /*
    * We have to reset offset here in case it was beyond the buffer.
    * Otherwise sequental overwrites may break.
    */

    size_t length = lbsSB_length(sb);
    if (offset > length)
    {
      offset = length;
    }

    lbsSB_overwritechar(sb, offset, LUABINS_CTABLE);
    lbsSB_overwrite(
        sb,
        offset + 1,
        (const unsigned char *)&array_size,
        LUABINS_LINT
      );
    lbsSB_overwrite(
        sb,
        offset + 1 + LUABINS_LINT,
        (const unsigned char *)&hash_size,
        LUABINS_LINT
      );
  }

  return result;
}

int lbs_writeNumber(luabins_SaveBuffer * sb, lua_Number value)
{
  int result = lbsSB_grow(sb, 1 + LUABINS_LNUMBER);
  if (result == LUABINS_ESUCCESS)
  {
    lbsSB_writechar(sb, LUABINS_CNUMBER);
    lbsSB_write(sb, (const unsigned char *)&value, LUABINS_LNUMBER);
  }
  return result;
}

int lbs_writeString(
    luabins_SaveBuffer * sb,
    const char * value,
    size_t length
  )
{
  int result = lbsSB_grow(sb, 1 + LUABINS_LSIZET + length);
  if (result == LUABINS_ESUCCESS)
  {
    lbsSB_writechar(sb, LUABINS_CSTRING);
    lbsSB_write(sb, (const unsigned char *)&length, LUABINS_LSIZET);
    lbsSB_write(sb, (const unsigned char *)value, length);
  }
  return result;
}

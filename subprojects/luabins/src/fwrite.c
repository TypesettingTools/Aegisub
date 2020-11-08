/*
* fwrite.c
* Luabins Lua-less write API using FILE * as buffer
* See copyright notice in luabins.h
*/

#include "luaheaders.h"

#include "fwrite.h"

/* TODO: Note that stream errors are ignored. Handle them better? */

void lbs_fwriteTableHeader(
    FILE * f,
    int array_size,
    int hash_size
  )
{
  fputc(LUABINS_CTABLE, f);
  fwrite(
      (const unsigned char *)&array_size,
      LUABINS_LINT,
      1,
      f
    );
  fwrite(
      (const unsigned char *)&hash_size,
      LUABINS_LINT,
      1,
      f
    );
}

void lbs_fwriteNumber(FILE * f, lua_Number value)
{
  fputc(LUABINS_CNUMBER, f);
  fwrite((const unsigned char *)&value, LUABINS_LNUMBER, 1, f);
}

void lbs_fwriteString(
    FILE * f,
    const char * value,
    size_t length
  )
{
  fputc(LUABINS_CSTRING, f);
  fwrite((const unsigned char *)&length, LUABINS_LSIZET, 1, f);
  fwrite((const unsigned char *)value, length, 1, f);
}

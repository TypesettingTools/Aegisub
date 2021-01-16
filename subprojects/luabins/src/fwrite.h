/*
* fwrite.h
* Luabins Lua-less write API using FILE * as buffer
* See copyright notice in luabins.h
*/

#ifndef LUABINS_FWRITE_H_INCLUDED_
#define LUABINS_FWRITE_H_INCLUDED_

#include "saveload.h"

#define lbs_fwriteTupleSize(f, tuple_size) \
  fputc((int)(tuple_size), (f))

void lbs_fwriteTableHeader(
    FILE * f,
    int array_size,
    int hash_size
  );

#define lbs_fwriteNil(f) \
  fputc(LUABINS_CNIL, (f))

#define lbs_fwriteBoolean(f, value) \
  fputc(((value) == 0) ? LUABINS_CFALSE : LUABINS_CTRUE, (f))

void lbs_fwriteNumber(FILE * f, lua_Number value);

#define lbs_fwriteInteger lbs_fwriteNumber

void lbs_fwriteString(
    FILE * f,
    const char * value,
    size_t length
  );

#endif /* LUABINS_FWRITE_H_INCLUDED_ */

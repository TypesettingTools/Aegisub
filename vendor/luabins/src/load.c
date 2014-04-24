/*
* load.c
* Luabins load code
* See copyright notice in luabins.h
*/

#include <string.h>

#include "luaheaders.h"

#include "luabins.h"
#include "saveload.h"
#include "luainternals.h"

#if 0
  #define XSPAM(a) printf a
#else
  #define XSPAM(a) (void)0
#endif

#if 0
  #define SPAM(a) printf a
#else
  #define SPAM(a) (void)0
#endif

typedef struct lbs_LoadState
{
  const unsigned char * pos;
  size_t unread;
} lbs_LoadState;

static void lbsLS_init(
    lbs_LoadState * ls,
    const unsigned char * data,
    size_t len
  )
{
  ls->pos = (len > 0) ? data : NULL;
  ls->unread = len;
}

#define lbsLS_good(ls) \
  ((ls)->pos != NULL)

#define lbsLS_unread(ls) \
  ((ls)->unread)

static unsigned char lbsLS_readbyte(lbs_LoadState * ls)
{
  if (lbsLS_good(ls))
  {
    if (lbsLS_unread(ls) > 0)
    {
      const unsigned char b = *ls->pos;
      ++ls->pos;
      --ls->unread;
      return b;
    }
    else
    {
      ls->unread = 0;
      ls->pos = NULL;
    }
  }
  return 0;
}

static const unsigned char * lbsLS_eat(lbs_LoadState * ls, size_t len)
{
  const unsigned char * result = NULL;
  if (lbsLS_good(ls))
  {
    if (lbsLS_unread(ls) >= len)
    {
      XSPAM(("* eat: len %u\n", (int)len));
      result = ls->pos;
      ls->pos += len;
      ls->unread -= len;
      XSPAM(("* eat: done len %u\n", (int)len));
    }
    else
    {
      ls->unread = 0;
      ls->pos = NULL;
    }
  }
  return result;
}

static int lbsLS_readbytes(
    lbs_LoadState * ls,
    unsigned char * buf,
    size_t len
  )
{
  const unsigned char * pos = lbsLS_eat(ls, len);
  if (pos != NULL)
  {
    memcpy(buf, pos, len);
    return LUABINS_ESUCCESS;
  }
  SPAM(("load: Failed to read %lu bytes\n", (unsigned long)len));
  return LUABINS_EBADDATA;
}

static int load_value(lua_State * L, lbs_LoadState * ls);

static int load_table(lua_State * L, lbs_LoadState * ls)
{
  int array_size = 0;
  int hash_size = 0;
  unsigned int total_size = 0;

  int result = lbsLS_readbytes(ls, (unsigned char *)&array_size, LUABINS_LINT);
  if (result == LUABINS_ESUCCESS)
  {
    result = lbsLS_readbytes(ls, (unsigned char *)&hash_size, LUABINS_LINT);
  }

  if (result == LUABINS_ESUCCESS)
  {
    total_size = array_size + hash_size;
/*
    SPAM((
        "LT SIZE CHECK\n"
        "* array_size %d limit 0 .. %d\n"
        "* hash_size %d limit >0\n"
        "* hash_size bytes %d, limit %d\n"
        "* unread %u limit >min_size %u (total_size %u)\n",
        array_size, MAXASIZE,
        hash_size,
        ceillog2((unsigned int)hash_size), MAXBITS,
        (unsigned int)lbsLS_unread(ls),
        (unsigned int)luabins_min_table_data_size(total_size),
        (unsigned int)total_size
      ));
*/
    if (
        array_size < 0 || array_size > MAXASIZE ||
        hash_size < 0  ||
        (hash_size > 0 && ceillog2((unsigned int)hash_size) > MAXBITS) ||
        lbsLS_unread(ls) < luabins_min_table_data_size(total_size)
      )
    {
      result = LUABINS_EBADSIZE;
    }
  }

  if (result == LUABINS_ESUCCESS)
  {
    unsigned int i = 0;

    XSPAM((
        "* load: creating table a:%d + h:%d = %d\n",
        array_size, hash_size, total_size
      ));

    lua_createtable(L, array_size, hash_size);

    for (i = 0; i < total_size; ++i)
    {
      int key_type = LUA_TNONE;

      result = load_value(L, ls); /* Load key. */
      if (result != LUABINS_ESUCCESS)
      {
        break;
      }

      /* Table key can't be nil or NaN */
      key_type = lua_type(L, -1);
      if (key_type == LUA_TNIL)
      {
        /* Corrupt data? */
        SPAM(("load: nil as key detected\n"));
        result = LUABINS_EBADDATA;
        break;
      }

      if (key_type == LUA_TNUMBER)
      {
        lua_Number key = lua_tonumber(L, -1);
        if (luai_numisnan(key))
        {
          /* Corrupt data? */
          SPAM(("load: NaN as key detected\n"));
          result = LUABINS_EBADDATA;
          break;
        }
      }

      result = load_value(L, ls); /* Load value. */
      if (result != LUABINS_ESUCCESS)
      {
        break;
      }

      lua_rawset(L, -3);
    }
  }

  return result;
}

static int load_value(lua_State * L, lbs_LoadState * ls)
{
  int result = LUABINS_ESUCCESS;
  unsigned char type = lbsLS_readbyte(ls);
  if (!lbsLS_good(ls))
  {
    SPAM(("load: Failed to read value type byte\n"));
    return LUABINS_EBADDATA;
  }

  XSPAM(("* load: begin load_value\n"));

  luaL_checkstack(L, 1, "load_value");

  switch (type)
  {
  case LUABINS_CNIL:
    XSPAM(("* load: nil\n"));
    lua_pushnil(L);
    break;

  case LUABINS_CFALSE:
    XSPAM(("* load: false\n"));
    lua_pushboolean(L, 0);
    break;

  case LUABINS_CTRUE:
    XSPAM(("* load: true\n"));
    lua_pushboolean(L, 1);
    break;

  case LUABINS_CNUMBER:
    {
      lua_Number value;

      XSPAM(("* load: number\n"));

      result = lbsLS_readbytes(ls, (unsigned char *)&value, LUABINS_LNUMBER);
      if (result == LUABINS_ESUCCESS)
      {
        lua_pushnumber(L, value);
      }
    }
    break;

  case LUABINS_CSTRING:
    {
      size_t len = 0;

      XSPAM(("* load: string\n"));

      result = lbsLS_readbytes(ls, (unsigned char *)&len, LUABINS_LSIZET);
      if (result == LUABINS_ESUCCESS)
      {
        const unsigned char * pos = lbsLS_eat(ls, len);

        XSPAM(("* load: string size %u\n", (int)len));

        if (pos != NULL)
        {
          lua_pushlstring(L, (const char *)pos, len);
        }
        else
        {
          result = LUABINS_EBADSIZE;
        }
      }
    }
    break;

  case LUABINS_CTABLE:
    XSPAM(("* load: table\n"));
    result = load_table(L, ls);
    break;

  default:
    SPAM(("load: Unknown type char 0x%02X found\n", type));
    result = LUABINS_EBADDATA;
    break;
  }

  XSPAM(("* load: end load_value\n"));

  return result;
}

int luabins_load(
    lua_State * L,
    const unsigned char * data,
    size_t len,
    int * count
  )
{
  lbs_LoadState ls;
  int result = LUABINS_ESUCCESS;
  unsigned char num_items = 0;
  int base = 0;
  int i = 0;

  base = lua_gettop(L);

  lbsLS_init(&ls, data, len);
  num_items = lbsLS_readbyte(&ls);
  if (!lbsLS_good(&ls))
  {
    SPAM(("load: failed to read num_items byte\n"));
    result = LUABINS_EBADDATA;
  }
  else if (num_items > LUABINS_MAXTUPLE)
  {
    SPAM(("load: tuple too large: %d\n", (int)num_items));
    result = LUABINS_EBADSIZE;
  }
  else
  {
    XSPAM(("* load: tuple size %d\n", (int)num_items));
    for (
        i = 0;
        i < num_items && result == LUABINS_ESUCCESS;
        ++i
      )
    {
      XSPAM(("* load: loading tuple item %d\n", i));
      result = load_value(L, &ls);
    }
  }

  if (result == LUABINS_ESUCCESS && lbsLS_unread(&ls) > 0)
  {
    SPAM(("load: %lu chars left at tail\n", lbsLS_unread(&ls)));
    result = LUABINS_ETAILEFT;
  }

  if (result == LUABINS_ESUCCESS)
  {
    *count = num_items;
  }
  else
  {
    lua_settop(L, base); /* Discard intermediate results */
    switch (result)
    {
    case LUABINS_EBADDATA:
      lua_pushliteral(L, "can't load: corrupt data");
      break;

    case LUABINS_EBADSIZE:
      lua_pushliteral(L, "can't load: corrupt data, bad size");
      break;

    case LUABINS_ETAILEFT:
      lua_pushliteral(L, "can't load: extra data at end");
      break;

    default: /* Should not happen */
      lua_pushliteral(L, "load failed");
      break;
    }
  }

  return result;
}

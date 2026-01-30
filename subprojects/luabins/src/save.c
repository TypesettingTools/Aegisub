/*
* save.c
* Luabins save code
* See copyright notice in luabins.h
*/

#include "luaheaders.h"

#include "luabins.h"
#include "saveload.h"
#include "savebuffer.h"
#include "write.h"

/* TODO: Test this with custom allocator! */

#if 0
  #define SPAM(a) printf a
#else
  #define SPAM(a) (void)0
#endif

static int save_value(
    lua_State * L,
    luabins_SaveBuffer * sb,
    int index,
    int nesting
  );

/* Returns 0 on success, non-zero on failure */
static int save_table(
    lua_State * L,
    luabins_SaveBuffer * sb,
    int index,
    int nesting
  )
{
  int result = LUABINS_ESUCCESS;
  int header_pos = 0;
  int total_size = 0;

  if (nesting > LUABINS_MAXTABLENESTING)
  {
    return LUABINS_ETOODEEP;
  }

  /* TODO: Hauling stack for key and value removal
     may get too heavy for larger tables. Think out a better way.
  */

  header_pos = lbsSB_length(sb);
  result = lbs_writeTableHeader(sb, 0, 0);

  result = lbsSB_grow(sb, LUABINS_LINT + LUABINS_LINT);
  if (result == LUABINS_ESUCCESS)
  {
    lua_checkstack(L, 2); /* Key and value */
    lua_pushnil(L); /* key for lua_next() */
  }

  while (result == LUABINS_ESUCCESS && lua_next(L, index) != 0)
  {
    int value_pos = lua_gettop(L); /* We need absolute values */
    int key_pos = value_pos - 1;

    /* Save key. */
    result = save_value(L, sb, key_pos, nesting);

    /* Save value. */
    if (result == LUABINS_ESUCCESS)
    {
      result = save_value(L, sb, value_pos, nesting);
    }

    if (result == LUABINS_ESUCCESS)
    {
      /* Remove value from stack, leave key for the next iteration. */
      lua_pop(L, 1);
      ++total_size;
    }
  }

  if (result == LUABINS_ESUCCESS)
  {
    /*
      Note that if array has holes, lua_objlen() may report
      larger than actual array size. So we need to adjust.

      TODO: Note inelegant downsize from size_t to int.
            Handle integer overflow here.
    */
    int array_size = luabins_min(total_size, (int)lua_objlen(L, index));
    int hash_size = luabins_max(0, total_size - array_size);

    result = lbs_writeTableHeaderAt(sb, header_pos, array_size, hash_size);
  }

  return result;
}

/* Returns 0 on success, non-zero on failure */
static int save_value(
    lua_State * L,
    luabins_SaveBuffer * sb,
    int index,
    int nesting
  )
{
  int result = LUABINS_ESUCCESS;

  switch (lua_type(L, index))
  {
  case LUA_TNIL:
    result = lbs_writeNil(sb);
    break;

  case LUA_TBOOLEAN:
    result = lbs_writeBoolean(sb, lua_toboolean(L, index));
    break;

  case LUA_TNUMBER:
    result = lbs_writeNumber(sb, lua_tonumber(L, index));
    break;

  case LUA_TSTRING:
    {
      size_t len = 0;
      const char * buf = lua_tolstring(L, index, &len);

      result = lbs_writeString(sb, buf, len);
    }
    break;

  case LUA_TTABLE:
    result = save_table(L, sb, index, nesting + 1);
    break;

  case LUA_TNONE:
  case LUA_TFUNCTION:
  case LUA_TTHREAD:
  case LUA_TUSERDATA:
  default:
    result = LUABINS_EBADTYPE;
  }

  return result;
}

int luabins_save(lua_State * L, int index_from, int index_to)
{
  unsigned char num_to_save = 0;
  int index = index_from;
  int base = lua_gettop(L);
  luabins_SaveBuffer sb;

  /*
  * TODO: If lua_error() would happen below, would leak the buffer.
  */

  if (index_to - index_from > LUABINS_MAXTUPLE)
  {
    lua_pushliteral(L, "can't save that many items");
    return LUABINS_EFAILURE;
  }

  /* Allowing to call luabins_save(L, 1, lua_gettop(L))
     from C function, called from Lua with no arguments
     (when lua_gettop() would return 0)
  */
  if (index_to < index_from)
  {
    index_from = 0;
    index_to = 0;
    num_to_save = 0;
  }
  else
  {
    if (
        index_from < 0 || index_from > base ||
        index_to < 0 || index_to > base
      )
    {
      lua_pushliteral(L, "can't save: inexistant indices");
      return LUABINS_EFAILURE;
    }

    num_to_save = index_to - index_from + 1;
  }

  {
    void * alloc_ud = NULL;
    lua_Alloc alloc_fn = lua_getallocf(L, &alloc_ud);
    lbsSB_init(&sb, alloc_fn, alloc_ud);
  }

  lbs_writeTupleSize(&sb, num_to_save);
  for ( ; index <= index_to; ++index)
  {
    int result = 0;

    result = save_value(L, &sb, index, 0);
    if (result != LUABINS_ESUCCESS)
    {
      switch (result)
      {
      case LUABINS_EBADTYPE:
        lua_pushliteral(L, "can't save: unsupported type detected");
        break;

      case LUABINS_ETOODEEP:
        lua_pushliteral(L, "can't save: nesting is too deep");
        break;

      case LUABINS_ETOOLONG:
        lua_pushliteral(L, "can't save: not enough memory");
        break;

      default: /* Should not happen */
        lua_pushliteral(L, "save failed");
        break;
      }

      lbsSB_destroy(&sb);

      return result;
    }
  }

  {
    size_t len = 0UL;
    const unsigned char * buf = lbsSB_buffer(&sb, &len);
    lua_pushlstring(L, (const char *)buf, len);
    lbsSB_destroy(&sb);
  }

  return LUABINS_ESUCCESS;
}

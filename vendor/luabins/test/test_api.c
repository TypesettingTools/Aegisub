/*
* test_api.c
* Luabins API tests
* See copyright notice in luabins.h
*/

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifdef __cplusplus
}
#endif /* __cplusplus */

#include "luabins.h"

#define STACKGUARD "-- stack ends here --"

/* Note this one does not dump values to protect from embedded zeroes. */
static int dump_lua_stack(lua_State * L, int base)
{
  int top = lua_gettop(L);

  if (top == 0)
  {
    lua_pushliteral(L, "-- stack is empty --");
  }
  else
  {
    int pos = 0;
    luaL_Buffer b;

    luaL_buffinit(L, &b);

    for (pos = top; pos > 0; --pos)
    {
      luaL_addstring(&b, (pos != base) ? "[" : "{");
      lua_pushinteger(L, pos);
      luaL_addvalue(&b);
      luaL_addstring(&b, (pos != base) ? "] - " : "} -");
      luaL_addstring(&b, luaL_typename(L, pos));
      luaL_addstring(&b, "\n");
    }

    luaL_pushresult(&b);
  }

  if (lua_gettop(L) != top + 1)
  {
    return luaL_error(L, "dumpstack not balanced %d %d", top, lua_gettop(L));
  }

  return 1;
}

static void fatal(lua_State * L, const char * msg)
{
  dump_lua_stack(L, 0);
  fprintf(stderr, "%s\nSTACK\n%s", msg, lua_tostring(L, -1));
  lua_pop(L, 1);
  fflush(stderr);
  exit(1);
}

static void check(lua_State * L, int base, int extra)
{
  int top = lua_gettop(L);
  if (top != base + extra)
  {
    fatal(L, "stack unbalanced");
  }

  lua_pushliteral(L, STACKGUARD);
  if (lua_rawequal(L, -1, base) == 0)
  {
    fatal(L, "stack guard corrupted");
  }
  lua_pop(L, 1);
}

static void checkerr(lua_State * L, int base, const char * err)
{
  int top = lua_gettop(L);
  if (top != base + 1)
  {
    fatal(L, "stack unbalanced on error");
  }

  lua_pushliteral(L, STACKGUARD);
  if (lua_rawequal(L, -1, base) == 0)
  {
    fatal(L, "stack guard corrupted");
  }
  lua_pop(L, 1);

  lua_pushstring(L, err);
  if (lua_rawequal(L, -1, -2) == 0)
  {
    fprintf(stderr, "actual:   '%s'\n", lua_tostring(L, -2));
    fprintf(stderr, "expected: '%s'\n", err);
    fatal(L, "error message mismatch");
  }
  lua_pop(L, 2); /* Pops error message as well */
}

static int push_testdataset(lua_State * L)
{
  int base = lua_gettop(L);

  lua_pushnil(L);
  lua_pushboolean(L, 0);
  lua_pushboolean(L, 1);
  lua_pushinteger(L, 42);
  lua_pushliteral(L, "luabins");

  lua_newtable(L);

  if (lua_gettop(L) - base != 6)
  {
    fatal(L, "push_dataset broken");
  }

  return 6;
}

static void check_testdataset_on_top(lua_State * L)
{
  int base = lua_gettop(L);

  /* TODO: Check table contents */
  if (!lua_istable(L, -1))
  {
    fatal(L, "dataset (-1) is not table");
  }

  lua_pushliteral(L, "luabins");
  if (!lua_rawequal(L, -1, -2 - 1))
  {
    fatal(L, "dataset (-2) value mismatch");
  }
  lua_pop(L, 1);

  lua_pushinteger(L, 42);
  if (!lua_rawequal(L, -1, -3 - 1))
  {
    fatal(L, "dataset (-3) value mismatch");
  }
  lua_pop(L, 1);

  lua_pushboolean(L, 1);
  if (!lua_rawequal(L, -1, -4 - 1))
  {
    fatal(L, "dataset (-4) value mismatch");
  }
  lua_pop(L, 1);

  lua_pushboolean(L, 0);
  if (!lua_rawequal(L, -1, -5 - 1))
  {
    fatal(L, "dataset (-5) value mismatch");
  }
  lua_pop(L, 1);

  lua_pushnil(L);
  if (!lua_rawequal(L, -1, -6 - 1))
  {
    fatal(L, "dataset (-6) value mismatch");
  }
  lua_pop(L, 1);

  if (lua_gettop(L) != base)
  {
    fatal(L, "check_dataset_on_top broken");
  }
}

void test_api()
{
  int base = 0;
  int count = 0;
  const unsigned char * str;
  size_t length = 0;

  lua_State * L = lua_open();
  luaL_openlibs(L);

  printf("---> BEGIN test_api\n");

  /* Push stack check value */
  lua_pushliteral(L, STACKGUARD);
  base = lua_gettop(L);

  /* Sanity check */
  check(L, base, 0);

  /* Save error: inexistant index */
  if (luabins_save(L, lua_gettop(L) + 1, lua_gettop(L) + 1) == 0)
  {
    fatal(L, "save should fail");
  }

  checkerr(L, base, "can't save: inexistant indices");

  if (luabins_save(L, -1, -1) == 0)
  {
    fatal(L, "save should fail");
  }

  checkerr(L, base, "can't save: inexistant indices");

  /* Assuming other save errors to be tested in test.lua */

  /* Trigger load error */

  if (luabins_load(L, (const unsigned char *)"", 0, &count) == 0)
  {
    fatal(L, "load should fail");
  }

  checkerr(L, base, "can't load: corrupt data");

  /* Assuming other load errors to be tested in test.lua */

  /* Do empty save */
  if (luabins_save(L, base, base - 1) != 0)
  {
    fatal(L, "empty save failed");
  }
  check(L, base, 1);

  str = (const unsigned char *)lua_tolstring(L, -1, &length);
  if (str == NULL || length == 0)
  {
    fatal(L, "bad empty save string");
  }

  /* Load empty save */

  if (luabins_load(L, str, length, &count) != 0)
  {
    fatal(L, "empty load failed");
  }

  if (count != 0)
  {
    fatal(L, "bad empty load count");
  }

  /* Pop saved data string */
  check(L, base, 1);
  lua_pop(L, 1);
  check(L, base, 0);

  {
    /* Save test dataset */

    int num_items = push_testdataset(L);
    check(L, base, num_items);

    if (luabins_save(L, base + 1, base + num_items) != 0)
    {
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
      fatal(L, "test dataset save failed");
    }

    check(L, base, num_items + 1);

    /* Load test dataset */

    str = (const unsigned char *)lua_tolstring(L, -1, &length);
    if (str == NULL || length == 0)
    {
      fatal(L, "bad empty save string");
    }

    if (luabins_load(L, str, length, &count) != 0)
    {
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
      fatal(L, "test dataset load failed");
    }

    if (count != num_items)
    {
      fatal(L, "wrong test dataset load count");
    }

    check(L, base, num_items + 1 + num_items);

    check_testdataset_on_top(L); /* Check loaded data */

    lua_pop(L, 1 + num_items);

    check_testdataset_on_top(L); /* Check original data intact */

    lua_pop(L, num_items);

    check(L, base, 0);

    /* Assuming further tests are done in test.lua */
  }

  lua_close(L);

  printf("---> OK\n");
}

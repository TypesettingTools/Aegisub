/*
* test_savebuffer.c
* Luabins SaveBuffer tests
* See copyright notice in luabins.h
*/

/*
* TODO: Tests are tuned for old fixed-increment memory allocation strategy.
*       Test for exponential growth corner-cases specifically.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lualess.h"
#include "savebuffer.h"

#include "test.h"

/******************************************************************************/

static size_t NOT_CHANGED = (size_t)-1;
static void * NOT_CHANGED_PTR = NULL;

static size_t DUMMY = (size_t)-42;
static void * DUMMY_PTR = NULL;

static void * g_last_ud = NULL;
static size_t g_last_osize = 0;

static void reset_alloc_globals()
{
  g_last_ud = NOT_CHANGED_PTR;
  g_last_osize = NOT_CHANGED;
}

static void init_globals()
{
  NOT_CHANGED_PTR = (void *)&NOT_CHANGED;
  DUMMY_PTR = (void *)&DUMMY;

  reset_alloc_globals();
}

static void * dummy_alloc(
    void * ud,
    void * ptr,
    size_t osize,
    size_t nsize
  )
{
  g_last_ud = ud;
  g_last_osize = osize;

  if (nsize == 0)
  {
    free(ptr);
    return NULL;
  }
  else
  {
    return realloc(ptr, nsize);
  }
}

/******************************************************************************/

static void check_alloc(void * expected_ud, size_t expected_osize)
{
  if (g_last_ud != expected_ud)
  {
    fprintf(
        stderr,
        "userdata mismatch in allocator: got %p, expected %p\n",
        g_last_ud, expected_ud
      );
    exit(1);
  }

  if (g_last_osize != expected_osize)
  {
    fprintf(
        stderr,
        "old size mismatch in allocator: got %lu, expected %lu\n",
        g_last_osize, expected_osize
      );
    exit(1);
  }

  reset_alloc_globals();
}

static void check_buffer(
    luabins_SaveBuffer * sb,
    const char * expected_buf_c,
    size_t expected_length,
    void * expected_ud,
    size_t expected_osize
  )
{
  const unsigned char * expected_buf = (const unsigned char *)expected_buf_c;

  {
    size_t actual_length = lbsSB_length(sb);
    if (actual_length != expected_length)
    {
      fprintf(
          stderr,
          "lbsSB_length mismatch in allocator: got %lu, expected %lu\n",
          actual_length, expected_length
        );
      exit(1);
    }
  }

  {
    size_t actual_length = (size_t)-1;
    const unsigned char * actual_buf = lbsSB_buffer(sb, &actual_length);
    if (actual_length != expected_length)
    {
      fprintf(
          stderr,
          "lsbSB_buffer length mismatch in allocator: got %lu, expected %lu\n",
          actual_length, expected_length
        );
      exit(1);
    }

    if (memcmp(actual_buf, expected_buf, expected_length) != 0)
    {
      fprintf(
          stderr,
          "lsbSB_buffer buf mismatch in allocator\n"
        );
      exit(1);
    }
  }

  check_alloc(expected_ud, expected_osize);
}

/******************************************************************************/

TEST (test_init_destroy,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  check_buffer(&sb, "", 0, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_destroy(&sb);
  check_alloc(NOT_CHANGED_PTR, NOT_CHANGED);
})

TEST (test_grow_zero,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_grow(&sb, 0);
  check_buffer(&sb, "", 0, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_destroy(&sb);
  check_alloc(NOT_CHANGED_PTR, NOT_CHANGED);
})

TEST (test_grow_bufsiz,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_grow(&sb, 1024);
  check_buffer(&sb, "", 0, DUMMY_PTR, 0);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 1344);
})

TEST (test_grow_one,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_grow(&sb, 1);
  check_buffer(&sb, "", 0, DUMMY_PTR, 0);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_grow_one_grow_one_noop,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_grow(&sb, 1);
  check_buffer(&sb, "", 0, DUMMY_PTR, 0);

  lbsSB_grow(&sb, 1);
  check_buffer(&sb, "", 0, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_grow_one_grow_bufsiz_noop,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_grow(&sb, 1);
  check_buffer(&sb, "", 0, DUMMY_PTR, 0);

  lbsSB_grow(&sb, 255);
  check_buffer(&sb, "", 0, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_grow_one_grow_bufsiz_one,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_grow(&sb, 1);
  check_buffer(&sb, "", 0, DUMMY_PTR, 0);

  lbsSB_grow(&sb, 257);
  check_buffer(&sb, "", 0, DUMMY_PTR, 256);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 512);
})

/******************************************************************************/

TEST (test_write_empty,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_write(&sb, (unsigned char*)"", 0);
  check_buffer(&sb, "", 0, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_destroy(&sb);
  check_alloc(NOT_CHANGED_PTR, NOT_CHANGED);
})

TEST (test_write,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_write(&sb, (unsigned char*)"42", 3);
  check_buffer(&sb, "42", 3, DUMMY_PTR, 0);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_write_embedded_zero,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_write(&sb, (unsigned char*)"4\02", 4);
  check_buffer(&sb, "4\02", 4, DUMMY_PTR, 0);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_write_write_smallsiz,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_write(&sb, (unsigned char*)"01234567", 8);
  check_buffer(&sb, "01234567", 8, DUMMY_PTR, 0);

  lbsSB_write(&sb, (unsigned char*)"01234567", 8);
  check_buffer(&sb, "0123456701234567", 8 + 8, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_write(&sb, (unsigned char*)"0123", 4);
  check_buffer(
      &sb,
      "01234567012345670123",
      8 + 8 + 4,
      NOT_CHANGED_PTR,
      NOT_CHANGED
    );

  lbsSB_write(&sb, (unsigned char*)"0123456789ABCDEF", 16);
  check_buffer(
      &sb,
      "012345670123456701230123456789ABCDEF",
      8 + 8 + 4 + 16,
      NOT_CHANGED_PTR,
      NOT_CHANGED
    );

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

/******************************************************************************/

TEST (test_writechar,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_writechar(&sb, 'A');
  check_buffer(&sb, "A", 1, DUMMY_PTR, 0);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_writechar_zero,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_writechar(&sb, '\0');
  check_buffer(&sb, "\0", 1, DUMMY_PTR, 0);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_write_writechar_smallsiz,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_write(&sb, (unsigned char*)"01234567", 8);
  check_buffer(&sb, "01234567", 8, DUMMY_PTR, 0);

  lbsSB_writechar(&sb, 'A');
  check_buffer(&sb, "01234567A", 8 + 1, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

/******************************************************************************/

TEST (test_overwrite_empty,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_overwrite(&sb, 0, (unsigned char*)"42", 3);
  check_buffer(&sb, "42", 3, DUMMY_PTR, 0);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_overwrite_inplace,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_write(&sb, (unsigned char*)"ABCD", 4);
  check_buffer(&sb, "ABCD", 4, DUMMY_PTR, 0);

  lbsSB_overwrite(&sb, 1, (unsigned char*)"42", 2);
  check_buffer(&sb, "A42D", 4, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_overwrite_overflow,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_write(&sb, (unsigned char*)"ABCD", 4);
  check_buffer(&sb, "ABCD", 4, DUMMY_PTR, 0);

  lbsSB_overwrite(&sb, 3, (unsigned char*)"42", 2);
  check_buffer(&sb, "ABC42", 5, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_overwrite_overflow_grows,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_write(&sb, (unsigned char*)"012345", 6);
  check_buffer(&sb, "012345", 6, DUMMY_PTR, 0);

  lbsSB_overwrite(&sb, 4, (unsigned char*)"ABCDEF", 6);
  check_buffer(&sb, "0123ABCDEF", 10, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_overwrite_large_offset_appends,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_write(&sb, (unsigned char*)"012345", 6);
  check_buffer(&sb, "012345", 6, DUMMY_PTR, 0);

  lbsSB_overwrite(&sb, 100, (unsigned char*)"ABCDEF", 6);
  check_buffer(&sb, "012345ABCDEF", 12, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

/******************************************************************************/

TEST (test_overwritechar_empty_buffer,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_overwritechar(&sb, 0, 'A');
  check_buffer(&sb, "A", 1, DUMMY_PTR, 0);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_overwritechar_inplace,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_write(&sb, (unsigned char*)"ABCD", 4);
  check_buffer(&sb, "ABCD", 4, DUMMY_PTR, 0);

  lbsSB_overwritechar(&sb, 1, '!');
  check_buffer(&sb, "A!CD", 4, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

TEST (test_overwritechar_large_offset_appends,
{
  luabins_SaveBuffer sb;
  lbsSB_init(&sb, dummy_alloc, DUMMY_PTR);

  lbsSB_write(&sb, (unsigned char*)"01234567", 8);
  check_buffer(&sb, "01234567", 8, DUMMY_PTR, 0);

  lbsSB_overwritechar(&sb, 100, '!');
  check_buffer(&sb, "01234567!", 9, NOT_CHANGED_PTR, NOT_CHANGED);

  lbsSB_destroy(&sb);
  check_alloc(DUMMY_PTR, 256);
})

/******************************************************************************/

void test_savebuffer()
{
  init_globals();

  test_init_destroy();
  test_grow_zero();
  test_grow_bufsiz();
  test_grow_one();
  test_grow_one_grow_one_noop();
  test_grow_one_grow_bufsiz_noop();
  test_grow_one_grow_bufsiz_one();

  test_write_empty();
  test_write();
  test_write_embedded_zero();
  test_write_write_smallsiz();

  test_writechar();
  test_writechar_zero();
  test_write_writechar_smallsiz();

  test_overwrite_empty();
  test_overwrite_inplace();
  test_overwrite_overflow();
  test_overwrite_overflow_grows();
  test_overwrite_large_offset_appends();

  test_overwritechar_empty_buffer();
  test_overwritechar_inplace();
  test_overwritechar_large_offset_appends();
}

/*
* test_fwrite_api.c
* Luabins Lua-less fwrite API tests
* See copyright notice in luabins.h
*/

/*
* WARNING: This suite is format-specific. Change it when format changes.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lualess.h"
#include "fwrite.h"

#include "test.h"
#include "util.h"

/******************************************************************************/

/*
* Note it is different from test_savebuffer variant.
* We're interested in higher level stuff here.
*/
static void check_buffer(
    FILE * f,
    const char * expected_buf_c,
    size_t expected_length
  )
{
  const unsigned char * expected_buf = (unsigned char *)expected_buf_c;
  unsigned char * actual_buf = NULL;
  size_t actual_length = ftell(f);
  size_t actually_read = 0;

  fseek(f, 0, SEEK_SET);

  actual_buf = (unsigned char *)malloc(actual_length);
  actually_read = fread(actual_buf, actual_length, 1, f);
  if (actually_read != 1ul)
  {
    fprintf(
        stderr,
        "fread count error: got %lu, expected %lu\n",
        actually_read, 1ul
      );

    free(actual_buf);
    fclose(f);
    exit(1);
  }

  fseek(f, actual_length, SEEK_SET);

  if (actual_length != expected_length)
  {
    fprintf(
        stderr,
        "length mismatch: got %lu, expected %lu\n",
        actual_length, expected_length
      );
    fprintf(stderr, "actual:\n");
    fprintbuf(stderr, actual_buf, actual_length);
    fprintf(stderr, "expected:\n");
    fprintbuf(stderr, expected_buf, expected_length);

    free(actual_buf);
    fclose(f);
    exit(1);
  }

  if (memcmp(actual_buf, expected_buf, expected_length) != 0)
  {
    fprintf(stderr, "buffer mismatch\n");
    fprintf(stderr, "actual:\n");
    fprintbuf(stderr, actual_buf, actual_length);
    fprintf(stderr, "expected:\n");
    fprintbuf(stderr, expected_buf, expected_length);

    free(actual_buf);
    fclose(f);
    exit(1);
  }

  free(actual_buf);
}

/******************************************************************************/

#define CAT(a, b) a ## b

#define TEST_NAME(x) CAT(test_fwrite, x)
#define CALL_NAME(x) CAT(lbs_fwrite, x)
#define BUFFER_NAME (f)
#define INIT_BUFFER \
  FILE * f = tmpfile();

#define DESTROY_BUFFER \
  fclose(f);

#define CHECK_BUFFER check_buffer

#include "write_tests.inc"

/******************************************************************************/

void test_fwrite_api()
{
  RUN_GENERATED_TESTS;
}

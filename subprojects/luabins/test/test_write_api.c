/*
* test_write_api.c
* Luabins Lua-less write API tests
* See copyright notice in luabins.h
*/

/*
* WARNING: This suite is format-specific. Change it when format changes.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Should be included first */
#include "lualess.h"
#include "write.h"

#include "test.h"
#include "util.h"

/******************************************************************************/

/*
* Note it is different from test_savebuffer variant.
* We're interested in higher level stuff here.
*/
static void check_buffer(
    luabins_SaveBuffer * sb,
    const char * expected_buf_c,
    size_t expected_length
  )
{
  const unsigned char * expected_buf = (const unsigned char *)expected_buf_c;

  size_t actual_length = (size_t)-1;
  const unsigned char * actual_buf = lbsSB_buffer(sb, &actual_length);
  if (actual_length != expected_length)
  {
    fprintf(
        stderr,
        "lsbSB_buffer length mismatch: got %lu, expected %lu\n",
        actual_length, expected_length
      );
    fprintf(stderr, "actual:\n");
    fprintbuf(stderr, actual_buf, actual_length);
    fprintf(stderr, "expected:\n");
    fprintbuf(stderr, expected_buf, expected_length);
    exit(1);
  }

  if (memcmp(actual_buf, expected_buf, expected_length) != 0)
  {
    fprintf(stderr, "lsbSB_buffer buffer mismatch\n");
    fprintf(stderr, "actual:\n");
    fprintbuf(stderr, actual_buf, actual_length);
    fprintf(stderr, "expected:\n");
    fprintbuf(stderr, expected_buf, expected_length);

    exit(1);
  }
}

/******************************************************************************/

#define CAT(a, b) a ## b

#define TEST_NAME(x) CAT(test_write, x)
#define CALL_NAME(x) CAT(lbs_write, x)
#define BUFFER_NAME (&sb)
#define INIT_BUFFER \
  luabins_SaveBuffer sb; \
  lbsSB_init(BUFFER_NAME, lbs_simplealloc, NULL);

#define DESTROY_BUFFER \
  lbsSB_destroy(BUFFER_NAME);

#define CHECK_BUFFER check_buffer

#include "write_tests.inc"

/******************************************************************************/

TEST (test_writeTableHeaderAt,
{
  INIT_BUFFER;

  {
    unsigned char tuple_size = 0x01;
    int array_size = 0x00;
    int hash_size = 0x00;
    int table_header_pos = 0;

    lbs_writeTupleSize(BUFFER_NAME, tuple_size);
    table_header_pos = lbsSB_length(BUFFER_NAME);
    lbs_writeTableHeader(BUFFER_NAME, array_size, hash_size);

    CHECK_BUFFER(
        &sb,
        "\x01" "T" "\x00\x00\x00\x00" "\x00\x00\x00\x00",
        1 + 1 + 4 + 4
      );

    array_size = 0xAB;
    hash_size = 0xCD;

    lbs_writeTableHeaderAt(
        BUFFER_NAME,
        table_header_pos,
        array_size,
        hash_size
      );
    CHECK_BUFFER(
        BUFFER_NAME,
        "\x01" "T" "\xAB\x00\x00\x00" "\xCD\x00\x00\x00",
        1 + 1 + 4 + 4
      );
  }

  DESTROY_BUFFER;
})

/******************************************************************************/

void test_write_api()
{
  RUN_GENERATED_TESTS;

  test_writeTableHeaderAt();
}

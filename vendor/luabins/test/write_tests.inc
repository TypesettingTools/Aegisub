/*
* write_tests.inc
* Luabins Lua-less write API
* See copyright notice in luabins.h
*/

/* Note this file intentionally does not have include guards */

/*
* Depends on following symbols:
*   -- TEST_NAME
*   -- CALL_NAME
*   -- BUFFER_NAME
*   -- INIT_BUFFER
*   -- DESTROY_BUFFER
*   -- CHECK_BUFFER
*
* Defines symbols
*   -- RUN_GENERATED_TESTS
*/

TEST (TEST_NAME(TupleSize),
{
  INIT_BUFFER;

  {
    unsigned char tuple_size = 0xAB;

    CALL_NAME(TupleSize)(BUFFER_NAME, tuple_size);
    CHECK_BUFFER(BUFFER_NAME, "\xAB", 1);
  }

  DESTROY_BUFFER;
})

/******************************************************************************/

TEST (TEST_NAME(TableHeader),
{
  INIT_BUFFER;

  {
    int array_size = 0xAB;
    int hash_size = 0xCD;

    CALL_NAME(TableHeader)(BUFFER_NAME, array_size, hash_size);
    CHECK_BUFFER(
        BUFFER_NAME,
        "T" "\xAB\x00\x00\x00" "\xCD\x00\x00\x00",
        1 + 4 + 4
      );
  }

  DESTROY_BUFFER;
})

/******************************************************************************/

TEST (TEST_NAME(Nil),
{
  INIT_BUFFER;

  {
    CALL_NAME(Nil)(BUFFER_NAME);
    CHECK_BUFFER(BUFFER_NAME, "-", 1);
  }

  DESTROY_BUFFER;
})

/******************************************************************************/

TEST (TEST_NAME(Boolean),
{
  INIT_BUFFER;

  {
    CALL_NAME(Boolean)(BUFFER_NAME, 1);
    CHECK_BUFFER(BUFFER_NAME, "1", 1);

    CALL_NAME(Boolean)(BUFFER_NAME, 0);
    CHECK_BUFFER(BUFFER_NAME, "10", 1 + 1);
  }

  DESTROY_BUFFER;
})

/******************************************************************************/

TEST (TEST_NAME(Number),
{
  INIT_BUFFER;

  {
    /* Note number is a double */
    CALL_NAME(Number)(BUFFER_NAME, 1.0);
    CHECK_BUFFER(BUFFER_NAME, "N" "\x00\x00\x00\x00\x00\x00\xF0\x3F", 1 + 8);
  }

  DESTROY_BUFFER;
})

/******************************************************************************/

TEST (TEST_NAME(Integer),
{
  INIT_BUFFER;

  {
    /* Note integer is alsow written as a double */
    CALL_NAME(Integer)(BUFFER_NAME, 1);
    CHECK_BUFFER(BUFFER_NAME, "N" "\x00\x00\x00\x00\x00\x00\xF0\x3F", 1 + 8);
  }

  DESTROY_BUFFER;
})

/******************************************************************************/

TEST (TEST_NAME(StringEmpty),
{
  INIT_BUFFER;

  {
    CALL_NAME(String)(BUFFER_NAME, "", 0);
    CHECK_BUFFER(BUFFER_NAME, "S" "\x00\x00\x00\x00", 1 + 4);
  }

  DESTROY_BUFFER;
})

TEST (TEST_NAME(StringSimple),
{
  INIT_BUFFER;

  {
    CALL_NAME(String)(BUFFER_NAME, "Luabins", 7);
    CHECK_BUFFER(
        BUFFER_NAME,
        "S" "\x07\x00\x00\x00" "Luabins",
        1 + 4 + 7
      );
  }

  DESTROY_BUFFER;
})

TEST (TEST_NAME(StringEmbeddedZero),
{
  INIT_BUFFER;

  {
    CALL_NAME(String)(BUFFER_NAME, "Embedded\0Zero", 13);
    CHECK_BUFFER(
        BUFFER_NAME,
        "S" "\x0D\x00\x00\x00" "Embedded\0Zero",
        1 + 4 + 13
      );
  }

  DESTROY_BUFFER;
})

/******************************************************************************/

#define RUN_GENERATED_TESTS \
  TEST_NAME(TupleSize)(); \
  TEST_NAME(TableHeader)(); \
  TEST_NAME(Nil)(); \
  TEST_NAME(Boolean)(); \
  TEST_NAME(Number)(); \
  TEST_NAME(Integer)(); \
  TEST_NAME(StringEmpty)(); \
  TEST_NAME(StringSimple)(); \
  TEST_NAME(StringEmbeddedZero)();

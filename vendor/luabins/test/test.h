/*
* test.h
* Luabins test basics
* See copyright notice in luabins.h
*/

#ifndef LUABINS_TEST_H_INCLUDED_
#define LUABINS_TEST_H_INCLUDED_

#define STRINGIZE(s) #s

#define TEST(name, body) \
  static void name() \
  { \
    printf("---> BEGIN %s\n", STRINGIZE(name)); \
    body \
    printf("---> OK\n"); \
  }

void test_savebuffer();
void test_write_api();
void test_fwrite_api();
void test_api();

#endif /* LUABINS_TEST_H_INCLUDED_ */

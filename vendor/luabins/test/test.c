/*
* test.c
* Luabins test suite
* See copyright notice in luabins.h
*/

#include <stdio.h>

#include "test.h"

int main()
{
#ifdef __cplusplus
  printf("luabins C API test compiled as C++\n");
#else
  printf("luabins C API test compiled as plain C\n");
#endif /* __cplusplus */

  test_savebuffer();
  test_write_api();
  test_fwrite_api();
  test_api();

  return 0;
}

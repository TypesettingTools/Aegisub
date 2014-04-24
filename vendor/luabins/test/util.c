/*
* util.c
* Luabins test utilities
* See copyright notice in luabins.h
*/

#include "util.h"

void fprintbuf(FILE * out, const unsigned char * b, size_t len)
{
  size_t i = 0;
  for (i = 0; i < len; ++i)
  {
    fprintf(out, "%02X ", b[i]);
  }
  fprintf(out, "\n");
}

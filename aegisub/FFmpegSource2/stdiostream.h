//  Copyright (c) 2007-2009 Fredrik Mellbin
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#ifndef STDIOSTREAM_H
#define STDIOSTREAM_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "MatroskaParser.h"

#define	CACHESIZE 65536

/************\
* Structures *
\************/

/* first we need to create an I/O object that the parser will use to read the
 * source file
 */
struct StdIoStream {
  struct InputStream base;
  FILE *fp;
  int error;
};

typedef struct StdIoStream StdIoStream;

/***********\
* Functions *
\***********/

/* read count bytes into buffer starting at file position pos
 * return the number of bytes read, -1 on error or 0 on EOF
 */
int StdIoRead(StdIoStream *st, ulonglong pos, void *buffer, int count);

/* scan for a signature sig(big-endian) starting at file position pos
 * return position of the first byte of signature or -1 if error/not found
 */
longlong StdIoScan(StdIoStream *st, ulonglong start, unsigned signature);

/* return cache size, this is used to limit readahead */
unsigned StdIoGetCacheSize(StdIoStream *st);

/* return last error message */
const char *StdIoGetLastError(StdIoStream *st);

/* memory allocation, this is done via stdlib */
void *StdIoMalloc(StdIoStream *st, size_t size);

void *StdIoRealloc(StdIoStream *st, void *mem, size_t size);

void StdIoFree(StdIoStream *st, void *mem);

/* progress report handler for lengthy operations
 * returns 0 to abort operation, nonzero to continue
 */
int StdIoProgress(StdIoStream *st, ulonglong cur, ulonglong max);

longlong StdIoGetFileSize(StdIoStream *st);

void InitStdIoStream(StdIoStream *st);

#endif /* #ifndef STDIOSTREAM_H */

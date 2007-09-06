#ifndef STDIOSTREAM_H
#define STDIOSTREAM_H

#include "MatroskaParser.h"


/************\
* Structures *
\************/


/* first we need to create an I/O object that the parser will use to read the
 * source file
 */
struct StdIoStream {
  struct InputStream  base;
  FILE		      *fp;
  int		      error;
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
void  *StdIoMalloc(StdIoStream *st, size_t size);

void  *StdIoRealloc(StdIoStream *st, void *mem, size_t size);

void  StdIoFree(StdIoStream *st, void *mem);

/* progress report handler for lengthy operations
 * returns 0 to abort operation, nonzero to continue
 */
int   StdIoProgress(StdIoStream *st, ulonglong cur, ulonglong max);


#endif /* #ifndef STDIOSTREAM_H */

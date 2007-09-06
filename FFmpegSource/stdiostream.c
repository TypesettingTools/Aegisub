#include "stdiostream.h"

#define	CACHESIZE     65536

/* StdIoStream methods */

/* read count bytes into buffer starting at file position pos
 * return the number of bytes read, -1 on error or 0 on EOF
 */
int   StdIoRead(StdIoStream *st, ulonglong pos, void *buffer, int count) {
  size_t  rd;
  if (_fseeki64(st->fp, pos, SEEK_SET)) {
    st->error = errno;
    return -1;
  }
  rd = fread(buffer, 1, count, st->fp);
  if (rd == 0) {
    if (feof(st->fp))
      return 0;
    st->error = errno;
    return -1;
  }
  return rd;
}

/* scan for a signature sig(big-endian) starting at file position pos
 * return position of the first byte of signature or -1 if error/not found
 */
longlong StdIoScan(StdIoStream *st, ulonglong start, unsigned signature) {
  int	      c;
  unsigned    cmp = 0;
  FILE	      *fp = st->fp;

  if (_fseeki64(fp, start, SEEK_SET))
    return -1;

  while ((c = getc(fp)) != EOF) {
    cmp = ((cmp << 8) | c) & 0xffffffff;
    if (cmp == signature)
      return _ftelli64(fp) - 4;
  }

  return -1;
}

/* return cache size, this is used to limit readahead */
unsigned StdIoGetCacheSize(StdIoStream *st) {
  return CACHESIZE;
}

/* return last error message */
const char *StdIoGetLastError(StdIoStream *st) {
  return strerror(st->error);
}

/* memory allocation, this is done via stdlib */
void  *StdIoMalloc(StdIoStream *st, size_t size) {
  return malloc(size);
}

void  *StdIoRealloc(StdIoStream *st, void *mem, size_t size) {
  return realloc(mem,size);
}

void  StdIoFree(StdIoStream *st, void *mem) {
  free(mem);
}

/* progress report handler for lengthy operations
 * returns 0 to abort operation, nonzero to continue
 */
int   StdIoProgress(StdIoStream *st, ulonglong cur, ulonglong max) {
  return 1;
}

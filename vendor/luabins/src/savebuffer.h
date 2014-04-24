/*
* savebuffer.h
* Luabins save buffer
* See copyright notice in luabins.h
*/

#ifndef LUABINS_SAVEBUFFER_H_INCLUDED_
#define LUABINS_SAVEBUFFER_H_INCLUDED_

typedef struct luabins_SaveBuffer
{
  lua_Alloc alloc_fn;
  void * alloc_ud;

  unsigned char * buffer;
  size_t buf_size;
  size_t end;

} luabins_SaveBuffer;

void lbsSB_init(
    luabins_SaveBuffer * sb,
    lua_Alloc alloc_fn,
    void * alloc_ud
  );

/*
* Ensures that there is at least delta size available in buffer.
* New size is aligned by blockSize increments.
* Returns non-zero if resize failed.
* If you pre-sized the buffer, subsequent writes up to the new size
* are guaranteed to not fail.
*/
int lbsSB_grow(luabins_SaveBuffer * sb, size_t delta);

/*
* Returns non-zero if write failed.
* Allocates buffer as needed.
*/
int lbsSB_write(
    luabins_SaveBuffer * sb,
    const unsigned char * bytes,
    size_t length
  );

/*
* Returns non-zero if write failed.
* Allocates buffer as needed.
* Convenience function.
*/
int lbsSB_writechar(
    luabins_SaveBuffer * sb,
    unsigned char byte
  );

#define lbsSB_length(sb) ( (sb)->end )

/*
* If offset is greater than total length, data is appended to the end.
* Returns non-zero if write failed.
* Allocates buffer as needed.
*/
int lbsSB_overwrite(
    luabins_SaveBuffer * sb,
    size_t offset,
    const unsigned char * bytes,
    size_t length
  );

/*
* If offset is greater than total length, data is appended to the end.
* Returns non-zero if write failed.
* Allocates buffer as needed.
* Convenience function.
*/
int lbsSB_overwritechar(
    luabins_SaveBuffer * sb,
    size_t offset,
    unsigned char byte
  );

/*
* Returns a pointer to the internal buffer with data.
* Note that buffer is NOT zero-terminated.
* Buffer is valid until next operation with the given sb.
*/
const unsigned char * lbsSB_buffer(luabins_SaveBuffer * sb, size_t * length);

void lbsSB_destroy(luabins_SaveBuffer * sb);

#endif /* LUABINS_SAVEBUFFER_H_INCLUDED_ */

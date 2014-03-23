/**
 * charbuf.c
 * A dynamic character buffer.
 *
 * Supports efficient append and concat.
 */

#include <string.h>

#include <libc.h>

#include <torgo/charbuf.h>
#include <torgo/string.h>

/*
 * CharBuf = CStr Int Int
 *
 * A CharBuf maintains an oversized buffer in which appends can
 * occur. If there is no room in the buffer it is doubled to 
 * allow for more room.
 */
struct CharBuf {
  char *data;
  size_t size;
  size_t used;
};

/*
 * mkCharBuf :: Int -> CharBuf
 *
 * Allocate memory for a new CharBuf.
 * Initial size of the buffer is 'size'
 */
struct CharBuf* mkCharBuf(size_t size) {
  struct CharBuf *buf = malloc(sizeof *buf);
  if (! buf) goto exit;

  buf->data = malloc(size);
  if (!buf->data) goto fail;

  buf->used = 0;
  buf->size = size;

exit:
  return buf;

fail:
  free(buf);
  goto exit;
}

/*
 * freeCharBuf :: CharBuf -> ()
 *
 * Release memory associated with the CharBuf
 */
void freeCharBuf(struct CharBuf *buf) {
  free(buf->data);
  free(buf);
}

/*
 * clearCharBuf :: CharBuf -> ()
 *
 * Clears the CharBuf buffer, but does no
 * release the memory.
 */
void clearCharBuf(struct CharBuf *buf) {
  buf->data[0] = '\0';
  buf->used = 0;
}

/*
 * isEmptyCharBuf :: CharBuf -> Bool
 *
 * Check if CharBuf is empty.
 */
int isEmptyCharBuf(const struct CharBuf *buf) {
  return buf->used == 0;
}

/*
 * concatCharBuf :: CharBuf -> CStr -> CharBuf
 *
 * Concatenate the contents of the CharBuf and
 * the String into a new buffer.
 * Releases the old buffer.
 */
const char* concatCharBuf(struct CharBuf *buf, const char *str) {
  size_t strSize = strlen(str);
  if (buf->size - buf->used < strSize + 1) {
    size_t size = strSize + (buf->size * 2);
    char *data = malloc(size);
    if (!data) return NULL;

    memcpy(data, buf->data, buf->used);
    free(buf->data);
    buf->data = data;
    buf->size = size;
  }

  memcpy(buf->data + buf->used, str, strSize);
  buf->used += strSize;
  *(buf->data + buf->used) = '\0';
  return buf->data;
}

/*
 * appendCharBuf :: CharBuf -> Char
 *
 * Places 'c' at the end of the buffer.
 * Maintains the '\0' terminator
 */
char appendCharBuf(struct CharBuf *buf, char c) {
  char cstr[2] = { c, '\0' };
  concatCharBuf(buf, cstr);
  return c;
}

/*
 * fromCharBuf :: CharBuf -> CStr
 *
 * Returns pointer to the buffer for use in stdlib
 * functions.
 */
const char* fromCharBuf(const struct CharBuf *buf) {
  return buf->data;
}

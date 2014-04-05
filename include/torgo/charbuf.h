#ifndef SHELL_CHARBUF_H
#define SHELL_CHARBUF_H

/**
 * charbuf.h
 */

#include <stddef.h>

typedef struct CharBuf CharBuf;

CharBuf* mkCharBuf(size_t size);
void freeCharBuf(CharBuf *buf);

void clearCharBuf(CharBuf *buf);


int isEmptyCharBuf(const CharBuf *buf);
const char* concatCharBuf(CharBuf *buf, const char *str);
char appendCharBuf(CharBuf *buf, char c);
char dropLastCharBuf(CharBuf *buf);

const char* fromCharBuf(const CharBuf *buf);

#endif /* ! SHELL_CHARBUF_H */

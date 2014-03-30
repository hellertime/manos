#ifndef SHELL_CHARBUF_H
#define SHELL_CHARBUF_H

/**
 * charbuf.h
 */

#include <stddef.h>

struct CharBuf;

struct CharBuf* mkCharBuf(size_t size);
void freeCharBuf(struct CharBuf *buf);

void clearCharBuf(struct CharBuf *buf);


int isEmptyCharBuf(const struct CharBuf *buf);
const char* concatCharBuf(struct CharBuf *buf, const char *str);
char appendCharBuf(struct CharBuf *buf, char c);

const char* fromCharBuf(const struct CharBuf *buf);

#endif /* ! SHELL_CHARBUF_H */

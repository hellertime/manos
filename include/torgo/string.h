#ifndef SHELL_STRING_H
#define SHELL_STRING_H

/**
 * string.h
 * Shell string type.
 */

#include <stddef.h>

/*
 * String = (CStr, Int)
 *
 * Wraps a C string, giving it O(1) size lookup.
 */
struct String {
  const char *str;
  size_t size;
};

struct String* mkString(const char *cstr);
void freeString(struct String *str);

void assignString(struct String *str, const char *cstr);
const char* fromString(const struct String *str);

struct String* copyString(const struct String *str);
struct String* concatString(const struct String *a, const struct String *b);

int matchString(const struct String *a, const struct String *b);

#endif /* ! SHELL_STRING_H */

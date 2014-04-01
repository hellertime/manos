#ifndef TORGO_STRING_H
#define TORGO_STRING_H

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
typedef struct String {
  const char *str;
  size_t size;
} String;

String* mkString(const char *cstr);
void freeString(String *str);

void assignString(String *str, const char *cstr);
const char* fromString(const String *str);

String* copyString(const String *str);
String* concatString(const String *a, const String *b);

int matchString(const String *a, const String *b);

#endif /* ! SHELL_STRING_H */

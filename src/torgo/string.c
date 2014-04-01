/**
 * string.c
 * Shell string type. Wraps the char* and captures its size.
 * Immutable. Concatenation requires a memory copy.
 */

#include <string.h>

#include <manos.h>

#include <torgo/dstring.h>

/*
 * assignString :: &String -> CStr
 *
 * Constructs a valid String without copying.
 * Can be used to pass raw C strings into 
 * String routies.
 */
void assignString(String *str, const char *cstr) {
  str->size = strlen(cstr);
  str->str = cstr;
}

/*
 * fromString :: String -> CStr
 *
 * Grab the underlying pointer for passing to stdio
 */
const char * fromString(const String *str) {
  return str->str;
}

/*
 * mkString :: CStr -> String
 *
 * Copy C string into a String
 */
String* mkString(const char *cstr) {
  String* str = kmalloc(sizeof *str);
  if (! str) goto exit;

  str->size = strlen(cstr);
  char *cstr0 = kmalloc(str->size + 1);
  if (! cstr0) goto fail;

  memcpy(cstr0, cstr, str->size);
  *(cstr0 + str->size) = '\0';
  str->str = cstr0;

exit:
  return str;

fail:
  kfree(cstr0);
  goto exit;
}

/*
 * freeString :: String -> ()
 *
 * Release the memory allocated for String.
 */
void freeString(String *str) {
  str->size = 0;
  kfree((void*)str->str);
  kfree(str);
}

/*
 * copyString :: String -> String
 *
 * Like strdup.
 */
String* copyString(const String *str) {
  return mkString(str->str);
}

/*
 * concatString :: String -> String -> String
 *
 * Concatenates 'a' and 'b' by copying.
 */
String* concatString(const String *a, const String *b) {
  String *str = kmalloc(sizeof *str);
  if (! str) goto exit;

  char *cstr = kmalloc(a->size + b->size + 1);
  if (! cstr) goto fail;

  memcpy(cstr, a->str, a->size);
  memcpy(cstr + a->size, b->str, b->size);
  str->size = a->size + b->size;
  *(cstr + str->size) = '\0';
  str->str = cstr;

exit:
  return str;

fail:
  kfree(str);
  goto exit;
}

/*
 * matchString :: String -> String -> Bool
 *
 * Compare the strings for equality
 */
int matchString(const String *a, const String *b) {
  if (a->size != b->size) return 0;

  const char *aptr = a->str;
  const char *bptr = b->str;

  while (*aptr && (*aptr++ == *bptr++))
    ;

  return ((size_t)(aptr - a->str) == a->size);
}

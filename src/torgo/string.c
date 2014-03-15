/**
 * string.c
 * Shell string type. Wraps the char* and captures its size.
 * Immutable. Concatenation requires a memory copy.
 */

#include <shell/mem.h>
#include <shell/string.h>

/*
 * shellStrlen :: Cstr -> Int
 *
 * Unoptimized strlen
 */
size_t shellStrlen(const char *cstr) {
  const char *c = cstr;
  while (*c) c++;
  return c - cstr;
}

/*
 * shellStrdup :: Cstr -> Cstr
 *
 * Unoptimized strdup implementation
 */
char* shellStrdup(const char *s) {
  size_t size = strlen(s);
  char *c = malloc(size + 1);
  memcpy(c, s, size);
  c[size] = '\0';
  return c;
}

/*
 * streq :: Cstr -> Cstr -> Bool
 *
 * Less powerful version of strcmp
 * can only test for equality not 
 * ordering. Does a bytewise comparison.
 */
int streq(const char *a, const char *b) {
  while (*a && *a == *b) {
    a++;
    b++;
  }

  return *a == *b;
}

/*
 * assignString :: &String -> CStr
 *
 * Constructs a valid String without copying.
 * Can be used to pass raw C strings into 
 * String routies.
 */
void assignString(struct String *str, const char *cstr) {
  str->size = strlen(cstr);
  str->str = cstr;
}

/*
 * fromString :: String -> CStr
 *
 * Grab the underlying pointer for passing to stdio
 */
const char * fromString(const struct String *str) {
  return str->str;
}

/*
 * mkString :: CStr -> String
 *
 * Copy C string into a String
 */
struct String* mkString(const char *cstr) {
  struct String* str = malloc(sizeof *str);
  if (! str) goto exit;

  str->size = strlen(cstr);
  char *cstr0 = malloc(str->size + 1);
  if (! cstr0) goto fail;

  memcpy(cstr0, cstr, str->size);
  *(cstr0 + str->size) = '\0';
  str->str = cstr0;

exit:
  return str;

fail:
  free(cstr0);
  goto exit;
}

/*
 * freeString :: String -> ()
 *
 * Release the memory allocated for String.
 */
void freeString(struct String *str) {
  str->size = 0;
  free((void*)str->str);
  free(str);
}

/*
 * copyString :: String -> String
 *
 * Like strdup.
 */
struct String* copyString(const struct String *str) {
  return mkString(str->str);
}

/*
 * concatString :: String -> String -> String
 *
 * Concatenates 'a' and 'b' by copying.
 */
struct String* concatString(const struct String *a, const struct String *b) {
  struct String *str = malloc(sizeof *str);
  if (! str) goto exit;

  char *cstr = malloc(a->size + b->size + 1);
  if (! cstr) goto fail;

  memcpy(cstr, a->str, a->size);
  memcpy(cstr + a->size, b->str, b->size);
  str->size = a->size + b->size;
  *(cstr + str->size) = '\0';
  str->str = cstr;

exit:
  return str;

fail:
  free(str);
  goto exit;
}

/*
 * matchString :: String -> String -> Bool
 *
 * Compare the strings for equality
 */
int matchString(const struct String *a, const struct String *b) {
  if (a->size != b->size) return 0;

  const char *aptr = a->str;
  const char *bptr = b->str;

  while (*aptr && (*aptr++ == *bptr++))
    ;

  return (aptr - a->str == a->size);
}

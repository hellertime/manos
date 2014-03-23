#include <string.h>
#include <stdlib.h>

#include <libc.h>

void assert(int pred) {
	if (!pred) {
		__asm("bkpt");
		abort();
	}
}

char* strdup(const char *c) {
  int n = strlen(c);
  char *dup = malloc(n + 1);
  if (dup) {
    memcpy(dup, c, n);
    *(dup + n) = 0;
  }
  return dup;
}

int streq(const char *a, const char *b) {
  if (!a || !b) return 0; /* NULL != NULL */

  while (*a && *b && *a == *b) {
    a++;
    b++;
  }

  return *a == *b;
}


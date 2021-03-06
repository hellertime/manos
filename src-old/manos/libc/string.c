#include <stdlib.h>
#include <string.h>

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
  if (strlen(a) != strlen(b)) return 0;
  return nstreq(a, b, strlen(a));
}

int nstreq(const char *a, const char *b, size_t n) {
  if (!a || !b) return 0; /* NULL != NULL */

  while (--n && *a && *b && *a == *b) {
    a++;
    b++;
  }

  return *a == *b;
}

#if defined PLATFORM_K70CW
void nanosleep(unsigned long int nanos);
__asm(
		"    .global nanosleep\n"
		"nanosleep:\n"
		"    adds r0,r0,#-1\n"
		"    bne  nanosleep\n"
		"    bx   lr\n"
	);
#else
#error "nanosleep() undefined"
#endif

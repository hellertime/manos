CPPFLAGS = -Iinclude
CFLAGS = -Wall -pedantic -std=c99 -Werror -g -ggdb -gdwarf-2 -g3 -DPLATFORM_NICE

all: torgo

MANOS_LIBC_SOURCES = \
  src/manos/libc/malloc.c \
  src/manos/libc/util.c

MANOS_LIBC_OBJECTS = $(MANOS_LIBC_SOURCES:.c=.o)

$(MANOS_LIBC_OBJECTS): %.o : %.c
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $@ -c $<
	@echo "CC $(@F)"

libc.a: $(MANOS_LIBC_OBJECTS)
	@$(AR) -rs $@ $^ 2>/dev/null
	@echo "LINK $@"

MANOS_KERNEL_SOURCES = \
  src/manos/kernel/dev.c \
  src/manos/kernel/devled.c \
  src/manos/kernel/err.c

MANOS_KERNEL_OBJECTS = $(MANOS_KERNEL_SOURCES:.c=.o)

$(MANOS_KERNEL_OBJECTS): %.o : %.c
	@$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $< -lc -L.
	@echo "CC $(@F)"

libkernel.a: $(MANOS_KERNEL_OBJECTS)
	@$(AR) -rs $@ $^ 2>/dev/null
	@echo "LINK $@"

TORGO_SOURCES = \
  src/torgo/commands.c \
  src/torgo/charbuf.c \
  src/torgo/env.c \
  src/torgo/mem.c \
  src/torgo/parser.c \
  src/torgo/string.c


torgo: $(TORGO_SOURCES) libkernel.a libc.a
	@$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $< -lkernel -lc -L.
	@echo "CC $(@F)"

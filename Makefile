CPPFLAGS = -Iinclude -DPLATFORM_NICE
CFLAGS = -Wall -pedantic -std=c99 -Werror -g -ggdb -gdwarf-2 -g3 -DPLATFORM_NICE

all: torgo

MANOS_LIBC_SOURCES = \
  src/manos/libc/malloc.c \
  src/manos/libc/string.c \
  src/manos/libc/util.c

MANOS_LIBC_OBJECTS = $(MANOS_LIBC_SOURCES:.c=.o)

$(MANOS_LIBC_OBJECTS): %.o : %.c
	@$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<
	@echo "CC $(@F)"

libc2.a: $(MANOS_LIBC_OBJECTS)
	@$(AR) -rs $@ $^ 2>/dev/null
	@echo "LINK $@"

MANOS_KERNEL_SOURCES = \
  src/manos/kernel/dev.c \
  src/manos/kernel/devled.c \
  src/manos/kernel/err.c \
  src/manos/kernel/portal.c

MANOS_KERNEL_OBJECTS = $(MANOS_KERNEL_SOURCES:.c=.o)

$(MANOS_KERNEL_OBJECTS): %.o : %.c
	@$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<
	@echo "CC $(@F)"

libkernel.a: $(MANOS_KERNEL_OBJECTS)
	@$(AR) -rs $@ $^ 2>/dev/null
	@echo "LINK $@"

TORGO_SOURCES = \
  src/torgo/commands.c \
  src/torgo/charbuf.c \
  src/torgo/env.c \
  src/torgo/parser.c \
  src/torgo/string.c

TORGO_OBJECTS = $(TORGO_SOURCES:.c=.o)

$(TORGO_OBJECTS): %.o : %.c
	@$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<
	@echo "CC $(@F)"

libtorgo.a: $(TORGO_OBJECTS)
	@$(AR) -rs $@ $^ 2>/dev/null
	@echo "LINK $@"

torgo: libtorgo.a libkernel.a libc2.a
torgo: % : src/torgo/%.c
	@$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $< -ltorgo -lkernel -lc2 -L.
	@echo "CC $(@F)"

clean:
	rm -vf *.a
	rm -f $(TORGO_OBJECTS)
	rm -f $(MANOS_LIBC_OBJECTS)
	rm -f $(MANOS_KERNEL_OBJECTS)

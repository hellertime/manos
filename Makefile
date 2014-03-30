SRCS = $(sort $(wildcard src/*/*.c))
OBJS = $(SRCS:.c=.o)

LDFLAGS =
CPPFLAGS =
CFLAGS = -pipe
CFLAGS_C99 = -std=c99
CFLAGS_ERR = -Wall -pedantic -Werror -Wextra
CFLAGS_DBG = -g -ggdb -gdwarf-2 -g3

CFLAGS_ALL = $(CFLAGS_C99)
CFLAGS_ALL += $(CFLAGS_ERR)
CFLAGS_ALL += $(CFLAGS_DBG)
CFLAGS_ALL += -I./include
CFLAGS_ALL += $(CPP_FLAGS) $(CFLAGS)

AR = ar
RANLIB = ranlib

STATIC_LIBS = lib/libmanos.a
ALL_LIBS = $(STATIC_LIBS)
TESTS = t/sns-walk
ALL_PROGS = $(TESTS)

all: $(ALL_LIBS) $(ALL_PROGS)

%.o: %.c
	@$(CC) $(CFLAGS_ALL) -c -o $@ $<
	@echo "CC $(@F)"

lib/libmanos.a: $(OBJS)
	rm -f $@
	@$(AR) rc $@ $(OBJS) 2>/dev/null
	@$(RANLIB) $@
	@echo "LINK $(@F)"

t/sns-walk: t/sns-walk.c $(ALL_LIBS)
	@$(CC) $(CFLAGS_ALL) -o $@ $< -L./lib -lmanos
	@echo "CC $@"

clean:
	rm -f $(OBJS)
	rm -f $(ALL_LIBS)
	rm -f $(ALL_PROGS)

.PHONY: all clean

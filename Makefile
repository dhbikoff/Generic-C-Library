#
# Makefile for Assignment 3
# (Thanks to Hank Sola and Mark Zavislak for identifying some bugs in previous Makefiles)
#

CC = gcc
CFLAGS = -g -Wall -std=gnu99 -Wpointer-arith
LDFLAGS =
PURIFY = purify
PFLAGS=  -demangle-program=/usr/pubsw/bin/c++filt -linker=/usr/bin/ld -best-effort  

VECTOR_SRCS = vector.c
VECTOR_HDRS = $(VECTOR_SRCS:.c=.h)

HASHSET_SRCS = hashset.c
HASHSET_HDRS = $(HASHSET_SRCS:.c=.h)

VECTOR_TEST_SRCS = vectortest.c $(VECTOR_SRCS)
VECTOR_TEST_OBJS = $(VECTOR_TEST_SRCS:.c=.o)

HASHSET_TEST_SRCS = hashsettest.c $(VECTOR_SRCS) $(HASHSET_SRCS)
HASHSET_TEST_OBJS = $(HASHSET_TEST_SRCS:.c=.o)

SRCS = $(VECTOR_SRCS) $(HASHSET_SRCS) vectortest.c hashsettest.c
HDRS = $(VECTOR_HDRS) $(HASHSET_HDRS)

EXECUTABLES = vector-test hashset-test
PURIFY_EXECUTABLES = vector-test-pure hashset-test-pure

default: $(EXECUTABLES)

pure: $(PURIFY_EXECUTABLES)

vector-test : Makefile.dependencies $(VECTOR_TEST_OBJS)
	$(CC) -o $@ $(VECTOR_TEST_OBJS) $(LDFLAGS)

hashset-test : Makefile.dependencies $(HASHSET_TEST_OBJS)
	$(CC) -o $@ $(HASHSET_TEST_OBJS) $(LDFLAGS)

vector-test-pure : Makefile.dependencies $(VECTOR_TEST_OBJS)
	$(PURIFY) $(PFLAGS) $(CC) -o $@ $(VECTOR_TEST_OBJS) $(LDFLAGS)

hashset-test-pure : Makefile.dependencies $(HASHSET_TEST_OBJS)
	$(PURIFY) $(PFLAGS) $(CC) -o $@ $(HASHSET_TEST_OBJS) $(LDFLAGS)

# The dependencies below make use of make's default rules,
# under which a .o automatically depends on its .c and
# the action taken uses the $(CC) and $(CFLAGS) variables.
# These lines describe a few extra dependencies involved.

Makefile.dependencies:: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -MM $(SRCS) > Makefile.dependencies

-include Makefile.dependencies

clean:
	\rm -fr a.out $(EXECUTABLES) $(PURIFY_EXECUTABLES) *.o core Makefile.dependencies

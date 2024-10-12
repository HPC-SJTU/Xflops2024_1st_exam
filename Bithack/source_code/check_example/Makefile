
# The sources we're building
SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)

# What we're building
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))
PRODUCT = everybit

# What we're building with
CC = clang
CFLAGS = -std=c99 -Wall -m64 -g
LDFLAGS = -flto -fuse-ld=gold 

# We need to link against the timing library for whatever OS we're on.
PLATFORM = $(shell uname)
ifeq ($(PLATFORM),Linux)
	LDFLAGS += -lrt
else ifeq ($(PLATFORM),Darwin)
	LDFLAGS += -arch x86_64 -framework CoreServices
endif


OLD_MODE = $(shell cat .buildmode 2> /dev/null)
ifeq ($(DEBUG),1)
# We want debug mode.
CFLAGS += -O0
ifneq ($(OLD_MODE),debug)
$(shell echo debug >.buildmode)
endif
else
# We want release mode.
CFLAGS += -O3 -DNDEBUG
ifneq ($(OLD_MODE),release)
$(shell echo release >.buildmode)
endif
endif


# By default, make the product.
all:		gen check

# How to compile a C file
%.o:		%.c $(HEADERS) .buildmode
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<

# How to link the product
$(PRODUCT):	$(OBJECTS) .buildmode
	$(CC) $(OBJECTS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@

# How to clean up
clean:
	$(RM)  gen_test check_test *.o .buildmode *.gcov *.gcno *.gcda *.txt

gen: $(OBJECTS) .buildmode
	$(CC) bit_vector.o submit_func_.c gen_test.o $(LDFLAGS) $(EXTRA_LDFLAGS) -o gen_test
	./gen_test

check: $(OBJECTS) .buildmode
	$(CC) bit_vector.o submit_func.c check_test.o $(LDFLAGS) $(EXTRA_LDFLAGS) -o check_test
	./check_test
	diff -q out_bits.txt check_out_bits.txt

.PHONY:		all clean

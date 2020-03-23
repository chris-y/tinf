# Makefile

CC = zcc +zxn
CFLAGS = -v -startup=30 -subtype=dotn -clib=sdcc_iy -O3 -SO3 --opt-code-size --max-allocs-per-node200000 -pragma-define=CLIB_MALLOC_HEAP_SIZE=-1
RM = rm -f
COMMON_SRCS = adler32.c crc32.c tinfgzip.c tinflate.c tinfzlib.c
COMMON_OBJS = $(COMMON_SRCS:.c=.o)
PROGRAMS = tgunzip
LDFLAGS = -create-app -lzxn

all: $(PROGRAMS)

%.o: %.c $(XZ_HEADERS)
	$(CC) $(ALL_CFLAGS) -c -o $@ $<

tgunzip: $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(COMMON_OBJS) ../examples/tgunzip/tgunzip.c

.PHONY: clean
clean:
	-$(RM) $(COMMON_OBJS) zcc_opt.def

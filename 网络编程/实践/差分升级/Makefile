CC = gcc
CFLAGS = -g -Wall
BZ2_SRC = bzip2/blocksort.c bzip2/huffman.c bzip2/crctable.c bzip2/randtable.c bzip2/compress.c bzip2/decompress.c bzip2/bzlib.c
# LDFLAGS = -lbz2
TARGET = bsdiff bspatch

all:$(TARGET)
bsdiff:bsdiff.c $(BZ2_SRC)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

bspatch:bspatch.c $(BZ2_SRC)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)


clean:
	rm -f $(TARGET) *.o

.PHONY:all clean

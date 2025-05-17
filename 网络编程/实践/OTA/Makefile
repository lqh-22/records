CC = gcc
CFLAGS = -g -Wall
TARGET = server client.exe

BSDIFFSRC = bsdiff/bsdiff.c
BSPATCHSRC = bspatch/bspatch.c
BZ2_SRC = bzip2/blocksort.c bzip2/huffman.c bzip2/crctable.c bzip2/randtable.c bzip2/compress.c bzip2/decompress.c bzip2/bzlib.c

BZ2_OBJ = bzip2/blocksort.o bzip2/huffman.o bzip2/crctable.o bzip2/randtable.o bzip2/compress.o bzip2/decompress.o bzip2/bzlib.o

all: $(TARGET)
server: server.o bsdiff.o $(BZ2_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
client.exe: client.o bspatch.o $(BZ2_OBJ)
	gcc -o $@ $^ -lwsock32

client.o: client.c
	$(CC) -c $< $(CFLAGS)

server.o: server.c
	$(CC) -c $< $(CFLAGS)

bspatch.o: $(BSPATCHSRC)
	$(CC) -c $< $(CFLAGS)

bsdiff.o: $(BSDIFFSRC)
	$(CC) -c $< $(CFLAGS)


bzip2/%.o: bzip2/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o bsdiff/*.o bzip2/*.o $(TARGET)

.PHONY: all clean
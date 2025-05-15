CC = gcc
CFLAGS = -g -Wall
# LDFLAGS = -lbz2
TARGET = bsdiff bspatch

all:$(TARGET)
bsdiff:bsdiff.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

bspatch:bspatch.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

.PHONY:all clean

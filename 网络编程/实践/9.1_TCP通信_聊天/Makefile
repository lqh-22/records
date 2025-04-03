CC = gcc
OBJS = sertalker.o
CFLAGS = -g -Wall
LDFLAGS = -lpthread
TARGET = sertalker

all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.o:%.c
	$(CC) -c $< $(CFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY:all clean

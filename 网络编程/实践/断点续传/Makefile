CC = gcc
OBJS = server.o
CFLAGS = -g -Wall
TARGET = server

all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.o:%.c
	$(CC) -c $< $(CFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY:all clean

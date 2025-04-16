CC = gcc
OBJS = sendFile.o
CFLAGS = -g -Wall
TARGET = sendFile

all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.o:%.c
	$(CC) -c $< $(CFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY:all clean

CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = sync_dirs
OBJS = main.o list.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c list.h
	$(CC) $(CFLAGS) -c main.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

clean:
	rm -f $(TARGET) $(OBJS)

test-setup:
	./reset

test: test-setup $(TARGET)
	./$(TARGET) t1 t2

.PHONY: all clean test-setup test

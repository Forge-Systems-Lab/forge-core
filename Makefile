CC = gcc
CFLAGS = -O3 -Wall -Wextra -pthread -mavx2 -march=native -Isrc
TARGET = forge-core
SRCS = src/main.c src/worker.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET) intelligence.json

.PHONY: all clean

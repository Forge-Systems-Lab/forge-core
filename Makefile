CC = gcc
CFLAGS = -Wall -Wextra -O3 -mavx2 -march=native -pthread -Iinclude
SRC = src/main.c src/worker.c
OBJ = $(SRC:.c=.o)
TARGET = forge-core

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/main.o src/worker.o $(TARGET)

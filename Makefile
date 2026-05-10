CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -mavx2 -march=native -O3 -pthread
SRC = src/main.c src/worker.c src/cli.c
OBJ = $(SRC:.c=.o)
TARGET = forge-core

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

release: clean $(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

# --- COMPILER SETTINGS ---
CC = gcc
CFLAGS = -Wall -Wextra -pthread -Iinclude

# --- DIRECTORIES ---
SRC_DIR = src
OBJ_DIR = build
BIN_NAME = forge-core

# --- SOURCE DISCOVERY ---
# This automatically finds all .c files in your src folder
SRC = $(wildcard $(SRC_DIR)/*.c)
# This maps src/filename.c to build/filename.o
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# --- TARGETS ---

# 1. Release Mode: Max performance (-O3)
release: CFLAGS += -O3
release: $(BIN_NAME)

# 2. Debug Mode: Sanitizers + Debug Symbols for Valgrind
debug: CFLAGS += -O1 -g -fsanitize=address,undefined
debug: $(BIN_NAME)

# --- LINKING PHASE ---
$(BIN_NAME): $(OBJ)
	@echo "🔗 Linking: $@"
	$(CC) $(CFLAGS) $^ -o $@

# --- COMPILATION PHASE ---
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "🔨 Compiling: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# --- UTILS ---
clean:
	@echo "🧹 Cleaning the forge..."
	rm -rf $(OBJ_DIR) $(BIN_NAME) audit_data.json

.PHONY: release debug clean

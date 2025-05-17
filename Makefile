CC = gcc
CFLAGS = -Wall -Wextra -g -I./include
LDFLAGS = -lcurl

# Directories
SRC_DIR = src
SHELL_DIR = $(SRC_DIR)/shell
OBJ_DIR = obj
BIN_DIR = bin
INCLUDE_DIR = include

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
SHELL_FILES = $(wildcard $(SHELL_DIR)/*.c)
SRC_FILES += $(SHELL_FILES)

# Object files
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Target executable
TARGET = $(BIN_DIR)/cshell

# Default target
all: $(TARGET)

# Create directories
$(OBJ_DIR)/shell:
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

# Link the executable
$(TARGET): $(OBJ_FILES) | $(BIN_DIR)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)/shell
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Run the shell
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run 
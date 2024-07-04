# Variables
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LDFLAGS = -lSoapySDR -lm
INCLUDE_DIR = includes
SRC_DIR = src
OBJ_DIR = obj
TARGET = SpinoMod

# Sources and Objects
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Default target
all: $(TARGET)

# Link the target
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compile sources
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Phony targets
.PHONY: all clean

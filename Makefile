# ---------------------------------------------------------------
# Makefile – Hybrid Inventory Manager
# Compiles C files with gcc and C++ files with g++, then links
# everything into a single executable with g++.
# ---------------------------------------------------------------

TARGET  := inventory_manager

CC      := gcc
CXX     := g++
CFLAGS  := -std=c11   -Wall -Wextra -Iinclude
CXXFLAGS:= -std=c++17 -Wall -Wextra -Iinclude

SRC_DIR := src
OBJ_DIR := obj

# Source files
C_SRCS   := $(SRC_DIR)/inventory.c
CPP_SRCS := $(SRC_DIR)/InventoryManager.cpp $(SRC_DIR)/main.cpp

# Object files (placed in obj/)
C_OBJS   := $(patsubst $(SRC_DIR)/%.c,   $(OBJ_DIR)/%.o, $(C_SRCS))
CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CPP_SRCS))

ALL_OBJS := $(C_OBJS) $(CPP_OBJS)

# ---------------------------------------------------------------
.PHONY: all clean

all: $(OBJ_DIR) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Link
$(TARGET): $(ALL_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo ""
	@echo "  Build successful → ./$(TARGET)"
	@echo ""

# Compile C
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	@echo "  Cleaned."

# Compiler and flags
CC = g++
CFLAGS = -std=c++20 -pthread -I/usr/include/imgui
LIBS = -lGLEW -lglfw -lGL -lz -limgui -lstb
LDFLAGS =

# Directories
SRC_DIRS = . gl math ui tools
BIN_DIR = bin
OBJ_DIR = $(BIN_DIR)/obj
TARGET = $(BIN_DIR)/app
CONVERTER = $(BIN_DIR)/converter

# Source and object files
SRC = $(wildcard $(addsuffix /*.cpp, $(SRC_DIRS)))
OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC))

# Default build type
BUILD = debug

# Debug Build Configuration
debug: CFLAGS += -g
debug: compile

# Release Build Configuration (optimized)
release: CFLAGS += -O2
release: compile

# Profile Build Configuration (for profiling with gprof)
profile: CFLAGS += -pg -g
profile: compile

# Compilation and linking
compile: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(BIN_DIR)/shaders $(BIN_DIR)/models
	cp -rf textures models shaders $(BIN_DIR)/
	$(CC) $^ $(CFLAGS) $(LIBS) -o $@

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Run the program
run:
	cd $(BIN_DIR); ./app

# Debug the program with Valgrind
debug_run:
	cd $(BIN_DIR); valgrind ./app

# Tool for converting wavefront files
tool: $(OBJ_DIR)/tools/wavefrontConverter.o
	$(CC) $(CFLAGS) $(LIBS) -o $(CONVERTER) $^

# Install dependencies (assuming Ubuntu-based system)
install:
	@echo "To install dependencies, run:"
	@echo "  sudo apt-get install libimgui-dev libglew-dev libstb-dev cloc"

# Count lines of code
report:
	cloc . --exclude-dir=$(BIN_DIR)

# Clean the build (remove binaries and object files)
clean:
	rm -rf $(BIN_DIR)

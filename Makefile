# Compiler and flags
CC = g++
CFLAGS = -std=c++20 -pthread -Wall -I/usr/include/imgui
LIBS = -lGLEW -lglfw -lGL -lz -limgui -lstb -lgdal
LDFLAGS = 


# Directories
SRC_DIRS = . gl math sdf space event tools handler command controller ui libs/ImGuiFileDialog
BIN_DIR = bin
DATA_DIR = data
OBJ_DIR = obj
TARGET = $(BIN_DIR)/app
CONVERTER = $(BIN_DIR)/converter

# Source and object files
SRC = $(wildcard $(addsuffix /*.cpp, $(SRC_DIRS)))
OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC))

# Default build type
BUILD = debug



# Release Build Configuration (optimized)
release: CFLAGS += -O3 -march=native 
release: compile

# Profile Build Configuration (for profiling with gprof)
profile: CFLAGS += -pg -g
profile: compile

# Clean the build (remove binaries and object files)
clean:
	rm -rf $(BIN_DIR)

reset:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)

# Compilation and linking
compile: clean $(TARGET)

recompile: reset $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p $(BIN_DIR)/textures $(BIN_DIR)/models $(BIN_DIR)/shaders $(DATA_DIR) $(BIN_DIR)/$(DATA_DIR)
	cp -rf textures models shaders $(DATA_DIR) $(BIN_DIR)/
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Run the program
run:
	cd $(BIN_DIR); ./app

# Debug Build Configuration
debug: CFLAGS += -g
debug: compile
debug: $(TARGET)

debug_run:
	cd $(BIN_DIR); valgrind ./app

# Tool for converting wavefront files
tool: $(OBJ_DIR)/tools/wavefrontConverter.o
	$(CC) $(CFLAGS) $(LIBS) -o $(CONVERTER) $^

# Install dependencies (assuming Ubuntu-based system)
install:
	@echo "To install dependencies, run:"
	@echo "  sudo apt-get install libimgui-dev libglew-dev libstb-dev libgdal-dev gdal-bin"
	@echo "  sudo apt-get install cloc kcachegrind massif-visualizer"
	@echo "  make libs"

# Count lines of code
report:
	cloc . --exclude-dir=$(BIN_DIR) --exclude-dir=libs

cachegrind:
	cd bin; valgrind --tool=cachegrind --cachegrind-out-file=file.out ./app;  kcachegrind file.out

callgrind:
	cd bin; valgrind --tool=callgrind --callgrind-out-file=file.out ./app;  kcachegrind file.out

massif:
	cd bin; valgrind --tool=massif --massif-out-file=file.out ./app;  massif-visualizer file.out

libs:
	mkdir -p libs; 
	cd libs; 
	git clone https://github.com/aiekick/ImGuiFileDialog.git

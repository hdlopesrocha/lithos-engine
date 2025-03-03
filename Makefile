# Compiler and flags
CC=g++
CFLAGS = -std=c++20 -lGLEW -lglfw -lGL -lz -limgui -lstb -I/usr/include/imgui -pthread
LDFLAGS = 

# Source files
SRC=*.cpp gl/*.cpp math/*.cpp ui/*.cpp tools/*.cpp

# Target executable name
TARGET=bin/app
CONVERTER=bin/converter

# Default build type
BUILD = debug

# Debug Build Configuration
debug: CFLAGS += -g
debug: LDFLAGS +=
debug: compile

# Release Build Configuration (optimized)
release: CFLAGS += -O2
release: LDFLAGS +=
release: compile

# Profile Build Configuration (for profiling with gprof)
profile: CFLAGS += -pg -g
profile: LDFLAGS +=
profile: compile

# Compilation and linking
compile:
	# Prepare the directories
	mkdir -p bin
	mkdir -p bin/shaders
	mkdir -p bin/models
	cp -rf textures bin
	cp -rf models bin
	cp -rf shaders bin

	# Compile the source files with the chosen flags
	$(CC) $(SRC) $(CFLAGS) -o $(TARGET) $(LDFLAGS)

# Run the program
run:
	cd bin; ./app

# Debug the program with Valgrind
debug_run:
	cd bin; valgrind ./app

# Tool for converting wavefront files
tool:
	$(CC) $(CFLAGS) -o $(CONVERTER) tools/wavefrontConverter.cpp

# Install dependencies (assuming Ubuntu-based system)
install:
	sudo apt-get install libimgui-dev libglew-dev libstb-dev cloc

report:
	cloc . --exclude-dir=bin

# Clean the build (remove binaries and object files)
clean:
	rm -rf bin

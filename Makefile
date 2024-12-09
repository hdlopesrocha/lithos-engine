CC=g++
SRC=*.cpp gl/*.cpp math/*.cpp

STB_INCLUDE_PATH = dependencies/stb
RAPID_JSON_PATH = dependencies/rapidjson
TINYOBJ_INCLUDE_PATH = dependencies/tinyobjloader

CFLAGS = -pg -g -std=c++11 -lGLEW -lglfw -lGL -I$(STB_INCLUDE_PATH) -I$(RAPID_JSON_PATH)/include -I$(TINYOBJ_INCLUDE_PATH)


compile:
	mkdir -p bin
	mkdir -p bin/shaders
	mkdir -p bin/models
	cp -rf textures bin
	cp -rf models bin
	cp -rf shaders bin

	$(CC) $(SRC) $(CFLAGS) -o bin/app  

run:
	cd bin;	./app

debug:
	cd bin;	valgrind ./app


tool:
	$(CC) $(CFLAGS) -o bin/converter tools/wavefrontConverter.cpp

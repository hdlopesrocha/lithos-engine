CC=g++

STB_INCLUDE_PATH = dependencies/stb
RAPID_JSON_PATH = dependencies/rapidjson
TINYOBJ_INCLUDE_PATH = dependencies/tinyobjloader
IMGUI_INCLUDE_PATH = dependencies/imgui

SRC=*.cpp gl/*.cpp math/*.cpp ui/*.cpp tools/*.cpp $(IMGUI_INCLUDE_PATH)/*.cpp

CFLAGS = -pg -g -std=c++20 -lGLEW -lglfw -lGL -lz -I$(STB_INCLUDE_PATH) -I$(RAPID_JSON_PATH)/include -I$(TINYOBJ_INCLUDE_PATH) 



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

install:
	sudo apt-get install libimgui-dev 
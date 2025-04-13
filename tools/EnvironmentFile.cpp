#include "tools.hpp"


EnvironmentFile::EnvironmentFile(std::string solidFilename, std::string liquidFilename, std::string brushesFilename, Camera * camera) {
    this->solidFilename = solidFilename;
    this->liquidFilename = liquidFilename;
    this->brushesFilename = brushesFilename;
    this->camera = camera;
}

void EnvironmentFile::save(std::string filename){
    std::ofstream file = std::ofstream(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    std::ostringstream decompressed;
    size_t sizeOfData;

    sizeOfData = solidFilename.size();
	decompressed.write(reinterpret_cast<const char*>(&sizeOfData), sizeof(size_t) );
	decompressed.write(reinterpret_cast<const char*>(solidFilename.c_str()), sizeOfData );

    sizeOfData = liquidFilename.size();
	decompressed.write(reinterpret_cast<const char*>(&sizeOfData), sizeof(size_t) );
	decompressed.write(reinterpret_cast<const char*>(liquidFilename.c_str()), sizeOfData );	

    sizeOfData = brushesFilename.size();
	decompressed.write(reinterpret_cast<const char*>(&sizeOfData), sizeof(size_t) );
	decompressed.write(reinterpret_cast<const char*>(brushesFilename.c_str()), sizeOfData );	


	decompressed.write(reinterpret_cast<const char*>(&camera->position), 3* sizeof(float) );
	decompressed.write(reinterpret_cast<const char*>(&camera->quaternion), 4* sizeof(float) );


	std::istringstream inputStream(decompressed.str());
 	gzipCompressToOfstream(inputStream, file);
	file.close();

    std::cout << "EnvironmentFile::save('" << filename <<"') Ok!" << std::endl;

};


EnvironmentFile::EnvironmentFile(std::string filename, Camera * camera) {
	std::ifstream file = std::ifstream(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return;
    }

    std::stringstream decompressed = gzipDecompressFromIfstream(file);

    char str[256];
    size_t sizeOfData;
    memset(str, 0, 256);
	decompressed.read(reinterpret_cast<char*>(&sizeOfData), sizeof(size_t) );
	decompressed.read(reinterpret_cast<char*>(str), sizeOfData);
    this->solidFilename = std::string(str, sizeOfData);
    
    memset(str, 0, 256);
	decompressed.read(reinterpret_cast<char*>(&sizeOfData), sizeof(size_t) );
	decompressed.read(reinterpret_cast<char*>(str), sizeOfData);
    this->liquidFilename = std::string(str, sizeOfData);
    
    memset(str, 0, 256);
	decompressed.read(reinterpret_cast<char*>(&sizeOfData), sizeof(size_t) );
	decompressed.read(reinterpret_cast<char*>(str), sizeOfData);
    this->brushesFilename = std::string(str, sizeOfData);

    decompressed.read(reinterpret_cast<char*>(&camera->position), 3*sizeof(float) );
    decompressed.read(reinterpret_cast<char*>(&camera->quaternion), 4*sizeof(float) );

    file.close();


    std::cout << "EnvironmentFile::EnvironmentFile('" << filename <<"') Ok!" << std::endl;
}


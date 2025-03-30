#include "tools.hpp"


EnvironmentFile::EnvironmentFile(std::string solidFilename, std::string liquidFilename, std::string brushesFilename) {
    this->solidFilename = solidFilename;
    this->liquidFilename = liquidFilename;
    this->brushesFilename = brushesFilename;
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

	std::istringstream inputStream(decompressed.str());
 	gzipCompressToOfstream(inputStream, file);
	file.close();
};


EnvironmentFile::EnvironmentFile(std::string filename) {
	std::ifstream file = std::ifstream(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return;
    }

    std::stringstream decompressed = gzipDecompressFromIfstream(file);

    char str[256];
    size_t sizeOfData;
	decompressed.read(reinterpret_cast<char*>(&sizeOfData), sizeof(size_t) );
	decompressed.read(reinterpret_cast<char*>(str), sizeOfData);
    solidFilename = std::string(str);

	decompressed.read(reinterpret_cast<char*>(&sizeOfData), sizeof(size_t) );
	decompressed.read(reinterpret_cast<char*>(str), sizeOfData);
    liquidFilename = std::string(str);

	decompressed.read(reinterpret_cast<char*>(&sizeOfData), sizeof(size_t) );
	decompressed.read(reinterpret_cast<char*>(str), sizeOfData);
    brushesFilename = std::string(str);

    file.close();
}


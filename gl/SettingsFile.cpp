#include "gl.hpp"



SettingsFile::SettingsFile(Settings * settings, std::string filename) {
	this->settings = settings;
	this->filename = filename;
}


void SettingsFile::load(std::string baseFolder) {
	std::string filePath = baseFolder + "/" + filename+".bin";
	std::ifstream file = std::ifstream(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for reading: " << filePath << std::endl;
        return;
    }

    std::stringstream decompressed = gzipDecompressFromIfstream(file);


	decompressed.read(reinterpret_cast<char*>(settings), sizeof(Settings) );
    file.close();

	std::cout << "SettingsFile::load('" << filePath <<"') Ok!" << std::endl;
}

void SettingsFile::save(std::string baseFolder){
	ensureFolderExists(baseFolder);
	std::string filePath = baseFolder + "/" + filename+".bin";
	std::ofstream file = std::ofstream(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filePath << std::endl;
        return;
    }

    std::ostringstream decompressed;
	decompressed.write(reinterpret_cast<const char*>(settings), sizeof(Settings));

	std::istringstream inputStream(decompressed.str());
 	gzipCompressToOfstream(inputStream, file);
	file.close();

	std::cout << "SettingsFile::save('" << filePath <<"') Ok!" << std::endl;

}

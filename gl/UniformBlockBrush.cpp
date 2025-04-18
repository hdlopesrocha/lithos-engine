#include "gl.hpp"

UniformBlockBrush::UniformBlockBrush(){
	this->parallaxScale = 0.0;
    this->parallaxMinLayers = 0;
    this->parallaxMaxLayers = 0;
    this->parallaxFade = 0;
    this->parallaxRefine = 0;
    this->shininess = 32;
    this->specularStrength = 0.4;
    this->textureScale = glm::vec2(1.0);
    this->refractiveIndex = 0;
	this->textureIndex = 0;
}

UniformBlockBrush::UniformBlockBrush(int textureIndex, glm::vec2 textureScale){
	this->textureScale = textureScale;
	this->parallaxScale = 0.0;
    this->parallaxMinLayers = 0;
    this->parallaxMaxLayers = 0;
    this->parallaxFade = 0;
    this->parallaxRefine = 0;
    this->shininess = 32;
    this->specularStrength = 0.4;
    this->refractiveIndex = 0;
	this->textureIndex = textureIndex;
}

UniformBlockBrush::UniformBlockBrush(int textureIndex, glm::vec2 textureScale,float parallaxScale, float parallaxMinLayers, float parallaxMaxLayers, float parallaxFade, float parallaxRefine, float shininess, float specularStrength, float refractiveIndex){
	this->parallaxScale = parallaxScale;
    this->parallaxMinLayers = parallaxMinLayers;
    this->parallaxMaxLayers = parallaxMaxLayers;
    this->parallaxFade = parallaxFade;
    this->parallaxRefine = parallaxRefine;
    this->shininess = shininess;
    this->specularStrength = specularStrength;
    this->textureScale = textureScale;
    this->refractiveIndex = refractiveIndex;
	this->textureIndex = textureIndex;
}

void UniformBlockBrush::save(std::vector<UniformBlockBrush> * brushes, std::string baseFolder, std::string filename){
	std::string filePath = baseFolder + "/" + filename+".bin";
	std::ofstream file = std::ofstream(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filePath << std::endl;
        return;
    }

	size_t size = brushes->size();
	//std::cout << std::to_string(sizeof(OctreeNodeSerialized)) << " bytes/node" << std::endl;
    std::ostringstream decompressed;
	decompressed.write(reinterpret_cast<const char*>(&size), sizeof(size_t) );
	for(size_t i =0 ; i < size ; ++i) {
		decompressed.write(reinterpret_cast<const char*>(&brushes->data()[i]), sizeof(UniformBlockBrush) );
	}	
	std::istringstream inputStream(decompressed.str());
 	gzipCompressToOfstream(inputStream, file);
	file.close();
	std::cout << "UniformBlockBrush::save('" << filePath <<"'," << std::to_string(size) <<") Ok!" << std::endl;
}

void UniformBlockBrush::load(std::vector<UniformBlockBrush> * brushes, std::string baseFolder, std::string filename){
	std::string filePath = baseFolder + "/" + filename+".bin";
	std::ifstream file = std::ifstream(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for reading: " << filePath << std::endl;
        return;
    }

    std::stringstream decompressed = gzipDecompressFromIfstream(file);

	size_t size;
	decompressed.read(reinterpret_cast<char*>(&size), sizeof(size_t) );
	brushes->resize(size);
	for(size_t i =0 ; i < size ; ++i) {
		decompressed.read(reinterpret_cast<char*>(&brushes->data()[i]), sizeof(UniformBlockBrush));
	}

	file.close();
	std::cout << "UniformBlockBrush::load('" << filePath <<"'," << std::to_string(size) <<") Ok!" << std::endl;
}

void UniformBlockBrush::uniform(GLuint program, UniformBlockBrush * brush, std::string objectName) {
	glUniform1f(glGetUniformLocation(program, (objectName +".parallaxScale").c_str() ), brush->parallaxScale);
	glUniform1f(glGetUniformLocation(program, (objectName +".parallaxMinLayers").c_str()), brush->parallaxMinLayers);
	glUniform1f(glGetUniformLocation(program, (objectName +".parallaxMaxLayers").c_str()), brush->parallaxMaxLayers);
	glUniform1f(glGetUniformLocation(program, (objectName +".parallaxFade").c_str()), brush->parallaxFade);
	glUniform1f(glGetUniformLocation(program, (objectName +".parallaxRefine").c_str()), brush->parallaxRefine);
	glUniform1f(glGetUniformLocation(program, (objectName +".shininess").c_str()), brush->shininess);
	glUniform1f(glGetUniformLocation(program, (objectName +".specularStrength").c_str()), brush->specularStrength);
	glUniform1f(glGetUniformLocation(program, (objectName +".refractiveIndex").c_str()), brush->refractiveIndex);
	glUniform2fv(glGetUniformLocation(program, (objectName +".textureScale").c_str()), 1, glm::value_ptr(brush->textureScale));
}

void UniformBlockBrush::uniform(GLuint program, UniformBlockBrush * brush, std::string objectName, std::string textureMap, int index, uint textureIndex) {
	std::string propName = objectName + "[" + std::to_string(index)  +"]";
	std::string mapName = textureMap + "[" + std::to_string(index)  +"]";
	glUniform1ui(glGetUniformLocation(program, (mapName).c_str()), textureIndex);
	uniform(program, brush, propName);
}

void UniformBlockBrush::uniform(GLuint program, std::vector<UniformBlockBrush*> *brushes, std::string objectName, std::string textureMap) {	
	for(size_t i = 0; i < brushes->size() ; ++i) {
		UniformBlockBrush * brush = brushes->at(i);
	   	UniformBlockBrush::uniform(program, brush, objectName, textureMap, i,  brush->textureIndex);
	}
}

std::string UniformBlockBrush::toString(UniformBlockBrush * block) {
	uint * data = (uint *) block;
	int size = sizeof(UniformBlockBrush)/sizeof(uint);
    std::stringstream ss;

	for(int i=0; i < size ; ++i) {
        if(i%4 == 0) {
            ss << std::endl;
        }
		ss << "0x"<< std::setw(8) << std::setfill('0')  << std::hex << data[i] << " ";
	}
	ss << std::endl;
    return ss.str();
}

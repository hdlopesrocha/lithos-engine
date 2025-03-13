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

}

UniformBlockBrush::UniformBlockBrush(glm::vec2 textureScale){
	this->textureScale = textureScale;
	this->parallaxScale = 0.0;
    this->parallaxMinLayers = 0;
    this->parallaxMaxLayers = 0;
    this->parallaxFade = 0;
    this->parallaxRefine = 0;
    this->shininess = 32;
    this->specularStrength = 0.4;
    this->refractiveIndex = 0;
}

UniformBlockBrush::UniformBlockBrush(glm::vec2 textureScale,float parallaxScale, float parallaxMinLayers, float parallaxMaxLayers, float parallaxFade, float parallaxRefine, float shininess, float specularStrength, float refractiveIndex){
	this->parallaxScale = parallaxScale;
    this->parallaxMinLayers = parallaxMinLayers;
    this->parallaxMaxLayers = parallaxMaxLayers;
    this->parallaxFade = parallaxFade;
    this->parallaxRefine = parallaxRefine;
    this->shininess = shininess;
    this->specularStrength = specularStrength;
    this->textureScale = textureScale;
    this->refractiveIndex = refractiveIndex;
}


void UniformBlockBrush::uniform(GLuint program, UniformBlockBrush * brush, std::string objectName, std::string textureMap, int index, uint textureIndex) {


		std::string propName = objectName + "[" + std::to_string(index)  +"]";

		std::string mapName = textureMap + "[" + std::to_string(index)  +"]";

	
	
	    glUniform1ui(glGetUniformLocation(program, (mapName).c_str()), textureIndex);

		glUniform1f(glGetUniformLocation(program, (propName +".parallaxScale").c_str() ), brush->parallaxScale);
		glUniform1f(glGetUniformLocation(program, (propName +".parallaxMinLayers").c_str()), brush->parallaxMinLayers);
		glUniform1f(glGetUniformLocation(program, (propName +".parallaxMaxLayers").c_str()), brush->parallaxMaxLayers);
		glUniform1f(glGetUniformLocation(program, (propName +".parallaxFade").c_str()), brush->parallaxFade);
		glUniform1f(glGetUniformLocation(program, (propName +".parallaxRefine").c_str()), brush->parallaxRefine);
		glUniform1f(glGetUniformLocation(program, (propName +".shininess").c_str()), brush->shininess);
		glUniform1f(glGetUniformLocation(program, (propName +".specularStrength").c_str()), brush->specularStrength);
		glUniform1f(glGetUniformLocation(program, (propName +".refractiveIndex").c_str()), brush->refractiveIndex);
		glUniform2fv(glGetUniformLocation(program, (propName +".textureScale").c_str()), 1, glm::value_ptr(brush->textureScale));


}

void UniformBlockBrush::uniform(GLuint program, std::vector<UniformBlockBrush*> *brushes, std::string objectName, std::string textureMap,std::map<UniformBlockBrush*, GLuint > *textureMapper) {
	
	for(int i = 0; i < brushes->size() ; ++i) {
		UniformBlockBrush * brush = brushes->at(i);
       	// Correct way to find the texture index
	   	auto it = textureMapper->find(brush);
	   	if (it == textureMapper->end()) {
		   	std::cerr << "Warning: TextureBrush not found in textureMapper!" << std::endl;
			continue;  // Skip if not found
	   	}
	   	else {
	   		GLuint index = it->second;
	   		UniformBlockBrush::uniform(program, brush, objectName, textureMap, i,  index);
		}
	}
}

std::string UniformBlockBrush::toString(UniformBlockBrush * block) {
	uint * data = (uint *) block;
	size_t size = sizeof(UniformBlockBrush)/sizeof(uint);
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

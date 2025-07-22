#include "tools.hpp"

ComputeShaderInfoHandler::ComputeShaderInfoHandler() : info(NULL), computeShader(NULL) {}


ComputeShaderInfoHandler::ComputeShaderInfoHandler(
    std::unordered_map<OctreeNode*, GeometrySSBO> * info, ComputeShader * computeShader
) : computeShader(computeShader) {
    this->info = info;
};

void ComputeShaderInfoHandler::create(OctreeNode* node) {
    
};

void ComputeShaderInfoHandler::update(OctreeNode* node) {
   // setUpdate<InstanceData>(node, info , true);
    //std::cout << "ComputeShaderInfoHandler::update() for node: " << node->id << std::endl;
};

void ComputeShaderInfoHandler::erase(OctreeNode* node) {
	info->erase(node);
};

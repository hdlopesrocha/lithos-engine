#include "tools.hpp"

ComputeShaderInfoHandler::ComputeShaderInfoHandler(
    std::unordered_map<OctreeNode*, GeometrySSBO> * info, ComputeShader &computeShader
) : computeShader(computeShader) {
    this->info = info;
};

void ComputeShaderInfoHandler::create(OctreeNode* node) {
    
};

void ComputeShaderInfoHandler::update(OctreeNode* node) {
   // setUpdate<InstanceData>(node, info , true);
    auto it = info->find(node);
    if (it == info->end()) {
        GeometrySSBO ssbo;
        ssbo.allocate();
        (*info)[node] = ssbo;
    }
    node->setDirty(true);
};

void ComputeShaderInfoHandler::erase(OctreeNode* node) {
	info->erase(node);
};

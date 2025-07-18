#include "tools.hpp"

ComputeShaderInfoHandler::ComputeShaderInfoHandler(
    std::unordered_map<OctreeNode*, ComputeShaderInfo> * info
) {
    this->info = info;
};

void ComputeShaderInfoHandler::create(OctreeNode* node) {
    
};

void ComputeShaderInfoHandler::update(OctreeNode* node) {
   // setUpdate<InstanceData>(node, info , true);
    node->setDirty(true);
};

void ComputeShaderInfoHandler::erase(OctreeNode* node) {
	info->erase(node);
};

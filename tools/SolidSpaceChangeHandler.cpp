#include "tools.hpp"

SolidSpaceChangeHandler::SolidSpaceChangeHandler(
    OctreeLayer<InstanceData> * vegetationInfo,
    OctreeLayer<DebugInstanceData> * octreeWireframeInfo
) {
    this->vegetationInfo = vegetationInfo;
    this->octreeWireframeInfo = octreeWireframeInfo;
};

void SolidSpaceChangeHandler::create(OctreeNode* node) {
    
};



void SolidSpaceChangeHandler::update(OctreeNode* node) {
    //std::cout << "SolidSpaceChangeHandler::update " << node->id << std::endl;
};

void SolidSpaceChangeHandler::erase(OctreeNode* node) {
    if(node!= NULL) {
        vegetationInfo->erase(node);
        #ifdef DEBUG_OCTREE_WIREFRAME
        octreeWireframeInfo->erase(node);
        #endif
    }
};

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
        std::unique_lock<std::shared_mutex> lock(vegetationInfo->mutex);
        vegetationInfo->info.erase(node);
    }
    #ifdef DEBUG_OCTREE_WIREFRAME
    {
        std::unique_lock<std::shared_mutex> lock(octreeWireframeInfo->mutex);
        octreeWireframeInfo->info.erase(node);
    }
    #endif

};

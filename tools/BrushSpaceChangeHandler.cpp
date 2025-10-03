#include "tools.hpp"

BrushSpaceChangeHandler::BrushSpaceChangeHandler(
    OctreeLayer<InstanceData> * brushInfo,
    OctreeLayer<DebugInstanceData> * octreeWireframeInfo
) {
    this->brushInfo = brushInfo;
    this->octreeWireframeInfo = octreeWireframeInfo;
};

void BrushSpaceChangeHandler::create(OctreeNode* node) {
    
};

void BrushSpaceChangeHandler::update(OctreeNode* node) {

};

void BrushSpaceChangeHandler::erase(OctreeNode* node) {
    if(node!=NULL) {
        {
            std::unique_lock<std::shared_mutex> lock(brushInfo->mutex);
            brushInfo->info.erase(node);
        }

        #ifdef DEBUG_OCTREE_WIREFRAME
        {
            std::unique_lock<std::shared_mutex> lock(octreeWireframeInfo->mutex);
            octreeWireframeInfo->info.erase(node);
        }
        #endif
    }
};

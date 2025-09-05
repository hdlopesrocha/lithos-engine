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
    brushInfo->mutex.lock();
	brushInfo->info.erase(node);
    brushInfo->mutex.unlock();


    #ifdef DEBUG_OCTREE_WIREFRAME
    octreeWireframeInfo->mutex.lock();
    octreeWireframeInfo->erase(node);
    octreeWireframeInfo->mutex.unlock();
    #endif
};

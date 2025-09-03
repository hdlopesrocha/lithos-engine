#include "tools.hpp"

BrushSpaceChangeHandler::BrushSpaceChangeHandler() : brushInfo(NULL) {

};

BrushSpaceChangeHandler::BrushSpaceChangeHandler(
    std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * brushInfo,
    std::unordered_map<OctreeNode*, NodeInfo<DebugInstanceData>> * octreeWireframeInfo
) {
    this->brushInfo = brushInfo;
    this->octreeWireframeInfo = octreeWireframeInfo;
};

void BrushSpaceChangeHandler::create(OctreeNode* node) {
    
};

void BrushSpaceChangeHandler::update(OctreeNode* node) {

};

void BrushSpaceChangeHandler::erase(OctreeNode* node) {
    mtx.lock();
	brushInfo->erase(node);
    #ifdef DEBUG_OCTREE_WIREFRAME
    octreeWireframeInfo->erase(node);
    #endif
    mtx.unlock();
};

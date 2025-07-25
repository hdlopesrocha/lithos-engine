#include "tools.hpp"

BrushSpaceChangeHandler::BrushSpaceChangeHandler() : brushInfo(NULL) {

};

BrushSpaceChangeHandler::BrushSpaceChangeHandler(
    std::unordered_map<OctreeNode*, GeometrySSBO> * brushInfo,
    std::unordered_map<OctreeNode*, NodeInfo<DebugInstanceData>> * debugInfo
) {
    this->brushInfo = brushInfo;
    this->debugInfo = debugInfo;
};

void BrushSpaceChangeHandler::create(OctreeNode* node) {
    
};

void BrushSpaceChangeHandler::update(OctreeNode* node) {

};

void BrushSpaceChangeHandler::erase(OctreeNode* node) {
	brushInfo->erase(node);
    debugInfo->erase(node);
};

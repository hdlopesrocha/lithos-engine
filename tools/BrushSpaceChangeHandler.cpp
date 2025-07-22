#include "tools.hpp"

BrushSpaceChangeHandler::BrushSpaceChangeHandler() : brushInfo(NULL) {

};

BrushSpaceChangeHandler::BrushSpaceChangeHandler(
    std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * brushInfo
) {
    this->brushInfo = brushInfo;
};

void BrushSpaceChangeHandler::create(OctreeNode* node) {
    
};

void BrushSpaceChangeHandler::update(OctreeNode* node) {

};

void BrushSpaceChangeHandler::erase(OctreeNode* node) {
	brushInfo->erase(node);
};

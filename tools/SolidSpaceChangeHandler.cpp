#include "tools.hpp"

SolidSpaceChangeHandler::SolidSpaceChangeHandler() :
    solidInfo(nullptr), vegetationInfo(nullptr), debugInfo(nullptr) {
};

SolidSpaceChangeHandler::SolidSpaceChangeHandler(
    std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * solidInfo,
    std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * vegetationInfo,
    std::unordered_map<OctreeNode*, NodeInfo<DebugInstanceData>> * debugInfo
) {
    this->solidInfo = solidInfo;
    this->vegetationInfo = vegetationInfo;
    this->debugInfo = debugInfo;
};

void SolidSpaceChangeHandler::create(OctreeNode* node) {
    
};



void SolidSpaceChangeHandler::update(OctreeNode* node) {
    
};

void SolidSpaceChangeHandler::erase(OctreeNode* node) {
    solidInfo->erase(node);
	debugInfo->erase(node);
	vegetationInfo->erase(node);
};

#include "tools.hpp"

SolidSpaceChangeHandler::SolidSpaceChangeHandler() :
    vegetationInfo(nullptr), debugInfo(nullptr) {
};

SolidSpaceChangeHandler::SolidSpaceChangeHandler(
    std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * vegetationInfo,
    std::unordered_map<OctreeNode*, NodeInfo<DebugInstanceData>> * debugInfo,
    std::unordered_map<OctreeNode*, GeometrySSBO> * computeInfo
) {
    this->vegetationInfo = vegetationInfo;
    this->debugInfo = debugInfo;
    this->computeInfo = computeInfo;
};

void SolidSpaceChangeHandler::create(OctreeNode* node) {
    
};



void SolidSpaceChangeHandler::update(OctreeNode* node) {
    //std::cout << "SolidSpaceChangeHandler::update " << node->id << std::endl;
};

void SolidSpaceChangeHandler::erase(OctreeNode* node) {
	debugInfo->erase(node);
	vegetationInfo->erase(node);
    computeInfo->erase(node);
};

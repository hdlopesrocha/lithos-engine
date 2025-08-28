#include "tools.hpp"

SolidSpaceChangeHandler::SolidSpaceChangeHandler() :
    vegetationInfo(nullptr) {
};

SolidSpaceChangeHandler::SolidSpaceChangeHandler(
    std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * vegetationInfo,
    std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * computeInfo
) {
    this->vegetationInfo = vegetationInfo;
    this->computeInfo = computeInfo;
};

void SolidSpaceChangeHandler::create(OctreeNode* node) {
    
};



void SolidSpaceChangeHandler::update(OctreeNode* node) {
    //std::cout << "SolidSpaceChangeHandler::update " << node->id << std::endl;
};

void SolidSpaceChangeHandler::erase(OctreeNode* node) {
    mtx.lock();
	vegetationInfo->erase(node);
    mtx.unlock();
};

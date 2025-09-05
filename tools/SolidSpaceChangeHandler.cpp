#include "tools.hpp"

SolidSpaceChangeHandler::SolidSpaceChangeHandler(
    OctreeLayer<InstanceData> * vegetationInfo,
    OctreeLayer<InstanceData> * computeInfo
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
    vegetationInfo->mutex.lock();
	vegetationInfo->info.erase(node);
    vegetationInfo->mutex.unlock();
};

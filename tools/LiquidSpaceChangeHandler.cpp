#include "tools.hpp"

LiquidSpaceChangeHandler::LiquidSpaceChangeHandler(
    OctreeLayer<InstanceData> * liquidInfo
) {
    this->liquidInfo = liquidInfo;
};

void LiquidSpaceChangeHandler::create(OctreeNode* node) {

};

void LiquidSpaceChangeHandler::update(OctreeNode* node) {
    
};

void LiquidSpaceChangeHandler::erase(OctreeNode* node) {
    liquidInfo->mutex.lock();
    liquidInfo->info.erase(node);
    liquidInfo->mutex.unlock();
};

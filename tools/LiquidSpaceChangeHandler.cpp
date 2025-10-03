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
    if(node!= NULL) {
        std::unique_lock<std::shared_mutex> lock(liquidInfo->mutex);
        liquidInfo->info.erase(node);
    }
};

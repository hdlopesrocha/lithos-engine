#include "tools.hpp"

BrushSpaceChangeHandler::BrushSpaceChangeHandler(
    OctreeLayer<InstanceData> * brushInfo
) {
    this->brushInfo = brushInfo;
};

void BrushSpaceChangeHandler::create(OctreeNode* node) {
    
};

void BrushSpaceChangeHandler::update(OctreeNode* node) {

};

void BrushSpaceChangeHandler::erase(OctreeNode* node) {
    if(node!=NULL) {
        {
            std::unique_lock<std::shared_mutex> lock(brushInfo->mutex);
            brushInfo->info.erase(node);
        }
    }
};

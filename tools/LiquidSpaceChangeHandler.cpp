#include "tools.hpp"

LiquidSpaceChangeHandler::LiquidSpaceChangeHandler() : liquidInfo(NULL) {}

LiquidSpaceChangeHandler::LiquidSpaceChangeHandler(
    std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * liquidInfo
) {
    this->liquidInfo = liquidInfo;
};

void LiquidSpaceChangeHandler::create(OctreeNode* node) {

};

void LiquidSpaceChangeHandler::update(OctreeNode* node) {
    
};

void LiquidSpaceChangeHandler::erase(OctreeNode* node) {
    liquidInfo->erase(node);
};

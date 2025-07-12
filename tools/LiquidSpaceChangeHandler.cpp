#include "tools.hpp"

LiquidSpaceChangeHandler::LiquidSpaceChangeHandler(
    std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * liquidInfo
) {
    this->liquidInfo = liquidInfo;
};

template <typename T> void setUpdate(OctreeNode * node, std::unordered_map<OctreeNode*, NodeInfo<T>> * map, bool value) {
    auto infoIter = map->find(node);
	if(infoIter != map->end()) {
        NodeInfo<T>& info = infoIter->second;
		info.update = value;
	}
}

void LiquidSpaceChangeHandler::create(OctreeNode* node) {
    setUpdate<InstanceData>(node, liquidInfo , true);
    node->setDirty(true);
};

void LiquidSpaceChangeHandler::update(OctreeNode* node) {
    
};

void LiquidSpaceChangeHandler::erase(OctreeNode* node) {
    liquidInfo->erase(node);
};

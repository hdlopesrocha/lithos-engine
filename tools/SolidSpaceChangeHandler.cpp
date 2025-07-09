#include "tools.hpp"

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

template <typename T> void setUpdate(OctreeNode * node, std::unordered_map<OctreeNode*, NodeInfo<T>> * map, bool value) {
    auto infoIter = map->find(node);
	if(infoIter != map->end()) {
        NodeInfo<T>& info = infoIter->second;
		info.update = value;
	}
}

void SolidSpaceChangeHandler::update(OctreeNode* node) {
    setUpdate<InstanceData>(node, solidInfo , true);
    setUpdate<InstanceData>(node, vegetationInfo , true);
    setUpdate<DebugInstanceData>(node, debugInfo , true);
};

void SolidSpaceChangeHandler::erase(OctreeNode* node) {
    solidInfo->erase(node);
	debugInfo->erase(node);
	vegetationInfo->erase(node);
};

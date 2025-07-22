#include "tools.hpp"

BrushSpaceChangeHandler::BrushSpaceChangeHandler(
    std::unordered_map<OctreeNode*, NodeInfo<InstanceData>> * brushInfo
) {
    this->brushInfo = brushInfo;
};

void BrushSpaceChangeHandler::create(OctreeNode* node) {
    
};

template <typename T> void setUpdate(OctreeNode * node, std::unordered_map<OctreeNode*, NodeInfo<T>> * map, bool value) {
    auto infoIter = map->find(node);
	if(infoIter != map->end()) {
        NodeInfo<T>& info = infoIter->second;
		info.update = value;
	}
}

void BrushSpaceChangeHandler::update(OctreeNode* node) {
    setUpdate<InstanceData>(node, brushInfo , true);
};

void BrushSpaceChangeHandler::erase(OctreeNode* node) {
	brushInfo->erase(node);
};

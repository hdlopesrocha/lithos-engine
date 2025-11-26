#include "space.hpp"


Processor::Processor(long * count, ThreadPool &threadPool, ThreadContext * context, std::vector<OctreeNodeTriangleHandler*> * handlers): threadPool(threadPool), context(context), handlers(handlers) {

}

bool Processor::test(const Octree &tree, OctreeNodeData &params) {
    if(params.context != NULL) {
        return false;
    }
    else {	
        if(params.node->isLeaf()) {
            params.context = params.node;
        }
        return params.node->getType() == SpaceType::Surface;
    }
	return false;
}

void Processor::before(const Octree &tree, OctreeNodeData &params) {		

}
//std::mutex processorMutex;
void Processor::after(const Octree &tree, OctreeNodeData &params) {
    if(params.context != NULL) {
        bool nodeIterated = false;
        tree.iterateBorder(params.node, params.cube, params.sdf, params.level, tree.root, tree, tree.root->sdf, 0, nodeIterated,
            [this, &tree, params](const BoundingCube &cube, const float sdf[8], uint level){
                tree.handleQuadNodes(cube, level, sdf, handlers, true, context);
            }
        );
    }
}

void Processor::getOrder(const Octree &tree, OctreeNodeData &params, uint8_t * order){
    for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}

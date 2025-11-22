#include "space.hpp"


Processor::Processor(long * count, ThreadContext * context, std::vector<OctreeNodeTriangleHandler*> * handlers): context(context), handlers(handlers) {

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
        tree.iterateNeighbor(params.node, params.cube, params.sdf, params.level, tree.root, tree, tree.root->sdf, 0, 
            [this, &tree, params](const BoundingCube &cube, const float sdf[8], uint level){
                //std::lock_guard<std::mutex> lock(processorMutex);
                //std::cout << "Neighbor " << cube.getMin().x << "," << cube.getMin().y << "," << cube.getMin().z << " size " << cube.getLengthX() << std::endl;
                tree.handleQuadNodes(cube, level, sdf, handlers, true, context);
            }
        );
        //virtualize(tree, data->cube, data->sdf, data->level, levels);

    }
}

void Processor::getOrder(const Octree &tree, OctreeNodeData &params, uint8_t * order){
    for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}

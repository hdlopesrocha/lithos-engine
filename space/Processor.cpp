#include "space.hpp"


Processor::Processor(long * count, ThreadContext * context, std::vector<OctreeNodeTriangleHandler*> * handlers): context(context), handlers(handlers) {

}

bool Processor::test(Octree * tree, OctreeNodeData *data) {
    if(data == NULL) {
        return false;
    }
    if(data->context != NULL) {
        return false;
    }
    else {	
        if(data->node->isLeaf()) {
            data->context = data->node;
        }
        return data->node->getType() == SpaceType::Surface;
    }
	return false;
}

void Processor::before(Octree * tree, OctreeNodeData *data) {		

}
//std::mutex processorMutex;
void Processor::after(Octree * tree, OctreeNodeData *data) {
    
    
    if(data->context != NULL) {
        tree->iterateNeighbor(data->node, data->cube, data->sdf, data->level, tree->root, *tree, tree->root->sdf, 0, 
            [this, tree, data](const BoundingCube &cube, const float sdf[8], uint level){
                //std::lock_guard<std::mutex> lock(processorMutex);
                //std::cout << "Neighbor " << cube.getMin().x << "," << cube.getMin().y << "," << cube.getMin().z << " size " << cube.getLengthX() << std::endl;
                tree->handleQuadNodes(cube, level, sdf, handlers, true, context);
            }
        );
        //virtualize(tree, data->cube, data->sdf, data->level, levels);

    }
}

void Processor::getOrder(Octree * tree, OctreeNodeData *params, uint8_t * order){
    for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}

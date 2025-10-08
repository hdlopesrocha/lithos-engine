#include "space.hpp"


Processor::Processor(long * count, ThreadContext * context, std::vector<OctreeNodeTriangleHandler*> * handlers): context(context), handlers(handlers) {

}

void Processor::virtualize(Octree * tree, const OctreeNodeData &data, uint levels) {
     //std::cout << "Virtualize " << " " << std::to_string((long) &data)  << " " <<data.level << "/" << levels << std::endl;

    if(data.node == NULL){
        //std::cout << "Virtualize empty node " << " " <<data.level << "/" << levels << std::endl;
        return;
    }
    if(data.level >= levels) {
        tree->handleQuadNodes(data, data.sdf, handlers, true, context);
        return;
    } else {
        ChildBlock * block = data.node->getBlock(*tree->allocator);

        for(int i = 0 ; i < 8 ; ++i) {
            float childSDF[8];
            SDF::getChildSDF(data.sdf, i, childSDF);
            OctreeNode * childNode = block ? block->get(i, *tree->allocator) : NULL;

            OctreeNodeData childData = OctreeNodeData(
                data.level + 1, 
                childNode, 
                data.cube.getChild(i), 
                data.context,
                childSDF
            );
            virtualize(tree, childData, levels);                  
        }
        return;
    }
}


bool Processor::test(Octree * tree, OctreeNodeData *data) {
    if(data == NULL){
        return false;
    } 
    if(data->context != NULL) {
        return false;
    }
    else {	
        if(data->node->isSimplified()) {
            data->context = data->node;
        }
        return true;
    }
	return false;
}

void Processor::before(Octree * tree, OctreeNodeData *data) {		

}

void Processor::after(Octree * tree, OctreeNodeData *data) {
    if(data->context != NULL) {
        uint levels = tree->getMaxLevel(data->cube);
        virtualize(tree, *data, levels);
    }
}

void Processor::getOrder(Octree * tree, OctreeNodeData *params, uint8_t * order){
    for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}

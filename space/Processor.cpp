#include "space.hpp"


Processor::Processor(long * count, ThreadContext * context, std::vector<OctreeNodeTriangleHandler*> * handlers): context(context), handlers(handlers) {

}

void Processor::virtualize(Octree * tree, OctreeNodeData &data, uint levels) {
     //std::cout << "Virtualize " << " " << std::to_string((long) &data)  << " " <<data.level << "/" << levels << std::endl;

    if(data.node == NULL){
        //std::cout << "Virtualize empty node " << " " <<data.level << "/" << levels << std::endl;
        return;
    }
    if(data.level >= levels) {
        tree->handleQuadNodes(data, data.sdf, handlers, true, context);
        return;
    } else {
        ChildBlock * block = data.node->getBlock(tree->allocator);

        for(int i = 0 ; i < 8 ; ++i) {
            float childSDF[8];
            SDF::getChildSDF(data.sdf, i, childSDF);
            OctreeNode * childNode = block ? block->get(i, tree->allocator) : NULL;

            OctreeNodeData childData(
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

bool Processor::test(Octree * tree, OctreeNodeData &data) {
    bool shouldContinue =  data.node != NULL && !data.node->isLeaf() && !data.node->isEmpty() && !data.node->isSolid();
	return shouldContinue;
}

void Processor::before(Octree * tree, OctreeNodeData &data) {		
    bool shouldContinue = test(tree, data);

    if(!shouldContinue && data.node != NULL) {
        uint levels = tree->getMaxLevel(data.cube);
        if(data.level < levels) {
            //  std::cout << "Virtualize at level " << data.node << " " <<data.level << "/" << levels << std::endl;
        }
        //tree->handleQuadNodes(data, data.sdf, handlers, true, context);
        virtualize(tree, data, levels);
        
    }

}

void Processor::after(Octree * tree, OctreeNodeData &params) {
	return;
}

void Processor::getOrder(Octree * tree, OctreeNodeData &params, uint8_t * order){
    for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}

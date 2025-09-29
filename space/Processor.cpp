#include "space.hpp"


Processor::Processor(long * count, ThreadContext * context, std::vector<OctreeNodeTriangleHandler*> * handlers): context(context), handlers(handlers) {

}

void Processor::virtualize(Octree * tree, OctreeNodeData &data, uint levels) {
    if(data.node == NULL){
    //    std::cout << "Virtualize empty node " << " " <<data.level << "/" << levels << std::endl;
    }
    if(data.level >= levels) {
        tree->handleQuadNodes(data, data.sdf, handlers, true, context);
        return;
    } else {
        for(int i = 0 ; i < 8 ; ++i) {
            float childSDF[8];
            SDF::getChildSDF(data.sdf, i, childSDF);
            OctreeNodeData childData(
                data.level + 1, 
                data.node, 
                data.cube.getChild(i), 
                data.context,
                childSDF
            );
            virtualize(tree, childData, levels);                  
        }
        return;
    }
}

bool Processor::test(Octree * tree, OctreeNodeData &params) {
    bool shouldContinue = params.node != NULL && !params.node->isLeaf() && !params.node->isEmpty() && !params.node->isSolid();
	return shouldContinue;
}

void Processor::before(Octree * tree, OctreeNodeData &params) {		
    bool shouldContinue = test(tree, params);

    if(!shouldContinue || params.node == NULL) {
        uint levels = tree->getMaxLevel(params.cube);
        if(params.level < levels) {
//          std::cout << "Virtualize at level " << params.node << " " <<params.level << "/" << levels << std::endl;
        }
        virtualize(tree, params, levels);
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

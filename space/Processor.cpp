#include "space.hpp"


Processor::Processor(long * count, ThreadContext * context, std::vector<OctreeNodeTriangleHandler*> * handlers): context(context), handlers(handlers) {

}

void Processor::virtualize(Octree * tree, float * sdf, OctreeNodeData &data, uint levels) {
    if(data.level >= levels) {
        tree->handleQuadNodes(data, sdf, handlers, true, context);
    } else {
        for(int i = 0 ; i < 8 ; ++i) {
            float childSDF[8];
            SDF::getChildSDF(sdf, i, childSDF);
            SpaceType spaceType = SDF::eval(childSDF);
            if(spaceType == SpaceType::Surface) {
                OctreeNodeData childData(
                    data.level + 1, 
                    data.node, 
                    data.cube.getChild(i), 
                    data.context
                );
                virtualize(tree, childSDF, childData, levels);          
            }
        }
    }
}

bool Processor::test(Octree * tree, OctreeNodeData &params) {
    bool shouldContinue = !params.node->isEmpty() && !params.node->isSolid() && !params.node->isLeaf();
    if(!shouldContinue) {
        //uint levels = tree->getCurrentLevel(params);
        uint levels = tree->getMaxLevel(params.cube);
        if(params.level < levels) {
        //    std::cout << "Virtualize at level " << params.level << "/" << levels << std::endl;
        }
        virtualize(tree, params.node->sdf, params, levels);
    }
	return shouldContinue;
}

void Processor::before(Octree * tree, OctreeNodeData &params) {		

}

void Processor::after(Octree * tree, OctreeNodeData &params) {
	return;
}

void Processor::getOrder(Octree * tree, OctreeNodeData &params, uint8_t * order){
    for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}

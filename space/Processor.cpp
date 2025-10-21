#include "space.hpp"


Processor::Processor(long * count, ThreadContext * context, std::vector<OctreeNodeTriangleHandler*> * handlers): context(context), handlers(handlers) {

}

void Processor::virtualize(Octree * tree, const BoundingCube &cube, float * sdf, uint level, uint levels) {
     //std::cout << "Virtualize " << " " << std::to_string((long) &data)  << " " <<data.level << "/" << levels << std::endl;

    if(level >= levels) {
        tree->handleQuadNodes(cube, level, sdf, handlers, true, context);
        return;
    } else {
        float childSDF[8];
        for(int i = 0 ; i < 8 ; ++i) {
            SDF::getChildSDF(sdf, i, childSDF);
            virtualize(tree, cube.getChild(i), childSDF, level + 1, levels);                  
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
        virtualize(tree, data->cube, data->sdf, data->level, levels);
    }
}

void Processor::getOrder(Octree * tree, OctreeNodeData *params, uint8_t * order){
    for(int i = 0 ; i < 8 ; ++i) {
		order[i] = i;
	}
}

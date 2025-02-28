#include "math.hpp"

void IteratorHandler::iterate(int level, OctreeNode * node, BoundingCube cube, void * context) {
    if(node != NULL) {
        context = before(level,node, cube, context);
        if(test(level, node, cube, context)) {
            int internalOrder[8];
            getOrder(node, cube, internalOrder);
            for(int i=0; i <8 ; ++i) {
                int j = internalOrder[i];
                OctreeNode * child = node->children[j];
                if(child != NULL) {
                    this->iterate(level+1, child, Octree::getChildCube(cube,j), context);
                }
            }
        }
        after(level,node, cube, context);
    }
}
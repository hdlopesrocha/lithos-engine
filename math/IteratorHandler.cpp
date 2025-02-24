#include "math.hpp"

void IteratorHandler::iterate(int level, OctreeNode * node, BoundingCube cube, void * context) {
    if(node != NULL) {
        if(test(level, node, cube, context)) {
            context = before(level,node, cube, context);
            int internalOrder[8];
            for(int i=0 ; i < 8; ++i){
                internalOrder[i] = 0;
            }


            getOrder(node, cube, &internalOrder[0]);
            for(int i=0; i <8 ; ++i) {
                int j = internalOrder[i];
                OctreeNode * child = getChild(node, j);
                if(child != NULL) {
                    this->iterate(level+1, child, Octree::getChildCube(cube,j), context);
                }
            }
            after(level,node, cube, context);
        }
    }
}
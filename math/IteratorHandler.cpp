#include "math.hpp"

void IteratorHandler::iterate(int level, OctreeNode * node, BoundingCube cube, void * context) {
    if(node != NULL) {
        if(test(level, node, cube, context)) {
            context = before(level,node, cube, context);
            for(int i=0; i <8 ; ++i) {
                OctreeNode * child = getChild(node, i);
                if(child != NULL) {
                    this->iterate(level+1, child, Octree::getChildCube(cube,i), context);
                }
            }
            after(level,node, cube, context);
        }
    }
}
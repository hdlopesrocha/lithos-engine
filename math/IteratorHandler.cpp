#include "math.hpp"

void IteratorHandler::iterate(int level, OctreeNode * node, BoundingCube cube, void * context) {
    if(node != NULL) {
        context = before(level,node, cube, context);
        if(test(level, node, cube, context)) {
            int internalOrder[8];
            getOrder(cube, internalOrder);
            for(int i=0; i <8 ; ++i) {
                int j = internalOrder[i];
                OctreeNode * child = node->children[j];
                if(child != NULL) {
                    this->iterate(level+1, child, Octree::getChildCube(cube,j) , context);
                }
            }
            after(level,node, cube, context);
        }
    }
}

void IteratorHandler::iterateFlat(int level, OctreeNode * node, BoundingCube cube, void * context) {
    std::vector<IteratorData> data;
 
    data.push_back(IteratorData(level, node, cube, context));
    while(data.size()) {
        bool newData = false;
        IteratorData d = data[data.size() -1];
        data.pop_back();

        context = before(d.level,d.node, d.cube, d.context);
        if(test(d.level, d.node, d.cube, context)) {
            int internalOrder[8];
            getOrder(d.cube, internalOrder);
            for(int i=7; i >= 0 ; --i) {
                int j = internalOrder[i];
                OctreeNode * child = d.node->children[j];
                if(child != NULL) {
                    data.push_back(IteratorData(d.level + 1, child, Octree::getChildCube(d.cube,j), context ));
                    newData = true;
                }
            }
            after(d.level,d.node, d.cube, context);
        }
    }
}


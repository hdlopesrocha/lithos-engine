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

void IteratorHandler::iterateFlatIn(int level, OctreeNode * node, BoundingCube cube, void * context) {
    int internalOrder[8];
 
    flatData.push({level, node, cube, context});
    while(flatData.size()) {
        bool newData = false;
        IteratorData d = flatData.top();
        flatData.pop();

        context = before(d.level,d.node, d.cube, d.context);
        if(test(d.level, d.node, d.cube, context)) {
            getOrder(d.cube, internalOrder);
            for(int i=7; i >= 0 ; --i) {
                int j = internalOrder[i];
                OctreeNode * child = d.node->children[j];
                if(child != NULL) {
                    flatData.push({d.level + 1, child, Octree::getChildCube(d.cube,j), context });
                    newData = true;
                }
            }
            after(d.level,d.node, d.cube, context);
        }
    }
}

void IteratorHandler::iterateFlat(int level, OctreeNode * root, BoundingCube cube, void * context) {
    if (!root) return;

    stack.push({level, root, cube, context, 0, {0}, false});

    while (!stack.empty()) {
        StackFrame &frame = stack.top();

        if (!frame.secondVisit) {
            // First visit: Apply `before()`
            frame.context = before(frame.level, frame.node, frame.cube, frame.context);

            if (!test(frame.level, frame.node, frame.cube, frame.context)) {
                stack.pop(); // Skip children, go back up
                continue;
            }

            // Prepare to process children
            getOrder(frame.cube, frame.internalOrder);
            frame.secondVisit = true; // Mark this node for a second visit
        }

        // Process children in order
        if (frame.childIndex < 8) {
            int j = frame.internalOrder[frame.childIndex++];
            OctreeNode* child = frame.node->children[j];

            if (child) {
                stack.push({frame.level + 1, child, Octree::getChildCube(frame.cube, j), frame.context, 0, {0}, false});
            }
        } else {
            // After all children are processed, apply `after()`
            after(frame.level, frame.node, frame.cube, frame.context);
            stack.pop();
        }
    }
}

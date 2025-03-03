#include "math.hpp"
void IteratorHandler::iterate(int level, int height, OctreeNode * node, BoundingCube cube, void * context) {
    if(node != NULL) {
        context = before(level, height, node, cube, context);
        if(test(level, height, node, cube, context)) {
            int internalOrder[8];
            getOrder(cube, internalOrder);
            for(int i=0; i <8 ; ++i) {
                int j = internalOrder[i];
                OctreeNode * child = node->children[j];
                if(child != NULL) {
                    this->iterate(level+1, height-1, child, Octree::getChildCube(cube,j) , context);
                }
            }
            after(level, height,node, cube, context);
        }
    }
}

void IteratorHandler::iterateFlatIn(int level, int height, OctreeNode * node, BoundingCube cube, void * context) {
    int internalOrder[8];
 
    flatData.push({level, height, node, cube, context});
    while(flatData.size()) {
        bool newData = false;
        IteratorData d = flatData.top();
        flatData.pop();

        context = before(d.level, d.height ,d.node, d.cube, d.context);
        if(test(d.level, d.height, d.node, d.cube, context)) {
            getOrder(d.cube, internalOrder);
            for(int i=7; i >= 0 ; --i) {
                int j = internalOrder[i];
                OctreeNode * child = d.node->children[j];
                if(child != NULL) {
                    flatData.push({d.level + 1, d.height - 1,child, Octree::getChildCube(d.cube,j), context });
                    newData = true;
                }
            }
            after(d.level, d.height,d.node, d.cube, context);
        }
    }
}

void IteratorHandler::iterateFlat(int level, int height, OctreeNode * root, BoundingCube cube, void * context) {
    if (!root) return;

    stack.push({level, height, root, cube, context, 0, {0}, false});

    while (!stack.empty()) {
        StackFrame &frame = stack.top();

        if (!frame.secondVisit) {
            // First visit: Apply `before()`
            frame.context = before(frame.level, frame.height, frame.node, frame.cube, frame.context);

            if (!test(frame.level, frame.height, frame.node, frame.cube, frame.context)) {
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
                stack.push({frame.level + 1, frame.height -1, child, Octree::getChildCube(frame.cube, j), frame.context, 0, {0}, false});
            }
        } else {
            // After all children are processed, apply `after()`
            after(frame.level, frame.height, frame.node, frame.cube, frame.context);
            stack.pop();
        }
    }
}

void IteratorHandler::iterateFlatOut(int level, int height, OctreeNode* root, BoundingCube cube, void* context) {
    if (!root) return;

    stackOut.push({level, height, root, cube, context, false});

    // A single shared array to hold the child processing order.
    int internalOrder[8];

    while (!stackOut.empty()) {
        StackFrameOut &frame = stackOut.top();

        if (!frame.visited) {
            // First visit: execute before() and update context.
            frame.context = before(frame.level, frame.height, frame.node, frame.cube, frame.context);
            frame.visited = true;

            // Only process children if the test passes.
            if (!test(frame.level, frame.height, frame.node, frame.cube, frame.context)) {
                stackOut.pop();
                continue;
            }

            // Compute the child order for this node.
            getOrder(frame.cube, internalOrder);

            // Push all valid children in reverse order so that they are processed
            // in the original (correct) order when popped.
            for (int i = 7; i >= 0; --i) {
                int j = internalOrder[i];
                OctreeNode* child = frame.node->children[j];
                if (child) {
                    stackOut.push({frame.level + 1, frame.height -1, child, Octree::getChildCube(frame.cube, j), frame.context, false});
                }
            }
        } else {
            // Second visit: all children have been processed; now call after().
            after(frame.level, frame.height, frame.node, frame.cube, frame.context);
            stackOut.pop();
        }
    }
}

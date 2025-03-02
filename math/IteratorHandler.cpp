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


void IteratorHandler::iterateFlatOut(int level, OctreeNode *root, BoundingCube cube, void *context) {
    if (!root) return;

    // Array to store the order of child nodes for the entire iteration (shared across all nodes)
    int internalOrder[8];

    stackOut.push({level, root, cube, context, 0});

    while (!stackOut.empty()) {
        StackFrameOut &frame = stackOut.top();

        // Apply `before()` and `after()` at the same time for each node
        frame.context = before(frame.level, frame.node, frame.cube, frame.context);

        if (!test(frame.level, frame.node, frame.cube, frame.context)) {
            stackOut.pop(); // Skip processing if test fails
            continue;
        }
        after(frame.level, frame.node, frame.cube, frame.context);  // Execute `after()` immediately

        // Get the order for the first time (only once for the entire iteration)
        if (frame.childIndex == 0) {
            getOrder(frame.cube, internalOrder);  // Cache order once
        }

        // Process children in the correct order (using the shared `internalOrder`)
        while (frame.childIndex < 8) {
            int j = internalOrder[frame.childIndex++];
            OctreeNode* child = frame.node->children[j];
            if (child) {
                stackOut.push({frame.level + 1, child, Octree::getChildCube(frame.cube, j), frame.context, 0});
                break; // Break to ensure we process one child per iteration
            }
        }

        // Pop the stack frame after processing the node (if no more children left)
        if (frame.childIndex == 8) {
            stackOut.pop(); // Node has been fully processed
        }
    }
}

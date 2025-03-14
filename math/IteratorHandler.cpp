#include "math.hpp"
void IteratorHandler::iterate(IteratorData params) {
    if(params.node != NULL) {
        params.context = before(params);
        if(test(params)) {
            int internalOrder[8];
            getOrder(params, internalOrder);
            for(int i=0; i <8 ; ++i) {
                int j = internalOrder[i];
                OctreeNode * child = params.node->children[j];
                if(child != NULL) {
                    this->iterate(IteratorData( params.level+1, params.height-1, params.lod-1, child, Octree::getChildCube(params.cube,j) , params.context));
                }
            }
            after(params);
        }
    }
}

void IteratorHandler::iterateFlatIn(IteratorData params) {
    int internalOrder[8];
 
    flatData.push(params);
    while(flatData.size()) {
        bool newData = false;
        IteratorData data = flatData.top();
        flatData.pop();

        data.context = before(data);
        if(test(data)) {
            getOrder(data, internalOrder);
            for(int i=7; i >= 0 ; --i) {
                int j = internalOrder[i];
                OctreeNode * child = data.node->children[j];
                if(child != NULL) {
                    flatData.push(IteratorData(data.level + 1, data.height -1, data.lod - 1,child, Octree::getChildCube(data.cube,j), data.context));
                    newData = true;
                }
            }
            after(data);
        }
    }
}

void IteratorHandler::iterateFlat(IteratorData params) {
    if (!params.node) return;

    stack.push(StackFrame(params, 0, false));

    while (!stack.empty()) {
        StackFrame &frame = stack.top();

        if (!frame.secondVisit) {
            // First visit: Apply `before()`
            frame.context = before(frame);

            if (!test(frame)) {
                stack.pop(); // Skip children, go back up
                continue;
            }

            // Prepare to process children
            getOrder(frame, frame.internalOrder);
            frame.secondVisit = true; // Mark this node for a second visit
        }

        // Process children in order
        if (frame.childIndex < 8) {
            int j = frame.internalOrder[frame.childIndex++];
            OctreeNode* child = frame.node->children[j];

            if (child) {
                stack.push(StackFrame(frame, 0, false));
            }
        } else {
            // After all children are processed, apply `after()`
            after(frame);
            stack.pop();
        }
    }
}

void IteratorHandler::iterateFlatOut(IteratorData params) {
    if (!params.node) return;

    stackOut.push(StackFrameOut(params, false));

    // A single shared array to hold the child processing order.
    int internalOrder[8];

    while (!stackOut.empty()) {
        StackFrameOut &frame = stackOut.top();

        if (!frame.visited) {
            
            // First visit: execute before() and update context.
            frame.context = before(frame);
            frame.visited = true;

            // Only process children if the test passes.
            if (!test(frame)) {
                stackOut.pop();
                continue;
            }

            // Compute the child order for this node.
            getOrder(frame, internalOrder);

            // Push all valid children in reverse order so that they are processed
            // in the original (correct) order when popped.
            for (int i = 7; i >= 0; --i) {
                int j = internalOrder[i];
                OctreeNode* child = frame.node->children[j];
                if (child) {
                    stackOut.push(StackFrameOut(IteratorData(frame.level + 1, frame.height -1, frame.lod -1, child, Octree::getChildCube(frame.cube, j), frame.context), false));
                }
            }
        } else {
            // Second visit: all children have been processed; now call after().
            after(frame);
            stackOut.pop();
        }
    }
}

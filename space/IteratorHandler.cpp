#include "space.hpp"

void IteratorHandler::iterate(OctreeNodeData params) {
    if(params.node != NULL) {
        before(params);
        if(test(params)) {
            uint8_t internalOrder[8];
            getOrder(params, internalOrder);

            ChildBlock * block = params.node->getBlock(params.allocator);
            for(int i=0; i <8 ; ++i) {
                uint8_t j = internalOrder[i];
                OctreeNode * child = params.node->getChildNode(j, params.allocator, block);
                if(child != NULL) {
                    this->iterate(OctreeNodeData( params.level+1, child, params.cube.getChild(j) , params.context, params.allocator));
                }
            }
            after(params);
        }
    }
}

void IteratorHandler::iterateFlatIn(OctreeNodeData params) {
    params.context = NULL;
    uint8_t internalOrder[8];
 
    flatData.push(params);
    while(flatData.size()) {
        OctreeNodeData data = flatData.top();
        flatData.pop();

        before(data);
        if(test(data)) {
            getOrder(data, internalOrder);
            OctreeNode * node = data.node;
            ChildBlock * block = node->getBlock(params.allocator);
            for(int i=7; i >= 0 ; --i) {
                uint8_t j = internalOrder[i];
                OctreeNode * child = node->getChildNode(j, params.allocator, block);
                if(child != NULL) {
                    flatData.push(OctreeNodeData(data.level + 1,child, data.cube.getChild(j), data.context, data.allocator));
                }
            }
            after(data);
        }
    }
}

void IteratorHandler::iterateFlat(OctreeNodeData params) {
    if (!params.node) return;
    params.context = NULL;

    stack.push(StackFrame(params, 0, false));

    while (!stack.empty()) {
        StackFrame &frame = stack.top();

        if (!frame.secondVisit) {
            // First visit: Apply `before()`
            before(frame);

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
            uint8_t j = frame.internalOrder[frame.childIndex++];
            OctreeNode * node = frame.node;
            ChildBlock * block = node->getBlock(params.allocator);
            OctreeNode* child = node->getChildNode(j, params.allocator, block);

            if (child) {
                OctreeNodeData data(frame.level+1, child, frame.cube.getChild(j), frame.context, frame.allocator);
                stack.push(StackFrame(data, 0, false));
            }
        } else {
            // After all children are processed, apply `after()`
            after(frame);
            stack.pop();
        }
    }
}

void IteratorHandler::iterateFlatOut(OctreeNodeData params) {
    if (!params.node) return;
    params.context = NULL;

    stackOut.push(StackFrameOut(params, false));

    // A single shared array to hold the child processing order.
    uint8_t internalOrder[8];

    while (!stackOut.empty()) {
        StackFrameOut &frame = stackOut.top();

        if (!frame.visited) {
            
            // First visit: execute before() and update context.
            before(frame);
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
                uint8_t j = internalOrder[i];
                OctreeNode * node = frame.node;
                ChildBlock * block = node->getBlock(params.allocator);
                OctreeNode* child = node->getChildNode(j, params.allocator, block);
                if (child) {
                    stackOut.push(StackFrameOut(OctreeNodeData(frame.level + 1, child, frame.cube.getChild(j), frame.context, params.allocator), false));
                }
            }
        } else {
            // Second visit: all children have been processed; now call after().
            after(frame);
            stackOut.pop();
        }
    }
}

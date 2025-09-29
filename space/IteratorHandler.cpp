#include "space.hpp"

void IteratorHandler::iterate(Octree * tree, OctreeNodeData params) {
    if(params.node != NULL) {
        before(tree, params);
        if(test(tree, params)) {
            uint8_t internalOrder[8];
            getOrder(tree, params, internalOrder);

            ChildBlock * block = params.node->getBlock(tree->allocator);
            for(int i=0; i <8 ; ++i) {
                uint8_t j = internalOrder[i];
                OctreeNode * child = params.node->getChildNode(j, tree->allocator, block);
                if(child != NULL) {
                    this->iterate(tree, OctreeNodeData( params.level+1, child, params.cube.getChild(j) , params.context, child->sdf));
                }
            }
        }
    }
}

void IteratorHandler::iterateFlatIn(Octree * tree, OctreeNodeData params) {
    params.context = NULL;
    uint8_t internalOrder[8];
 
    flatData.push(params);
    while(flatData.size()) {
        OctreeNodeData data = flatData.top();
        flatData.pop();

        before(tree, data);
        if(test(tree, data)) {
            getOrder(tree, data, internalOrder);
            OctreeNode * node = data.node;
            ChildBlock * block = node->getBlock(tree->allocator);
            for(int i=7; i >= 0 ; --i) {
                uint8_t j = internalOrder[i];
                OctreeNode * child = node->getChildNode(j, tree->allocator, block);
                if(child != NULL) {
                    flatData.push(OctreeNodeData(data.level + 1,child, data.cube.getChild(j), data.context, child->sdf));
                }
            }
        }
    }
}

void IteratorHandler::iterateFlat(Octree * tree, OctreeNodeData params) {
    if (!params.node) return;
    params.context = NULL;

    stack.push(StackFrame(params, 0, false));

    while (!stack.empty()) {
        StackFrame &frame = stack.top();

        if (!frame.secondVisit) {
            // First visit: Apply `before()`
            before(tree, frame);

            if (!test(tree, frame)) {
                stack.pop(); // Skip children, go back up
                continue;
            }

            // Prepare to process children
            getOrder(tree, frame, frame.internalOrder);
            frame.secondVisit = true; // Mark this node for a second visit
        }

        // Process children in order
        if (frame.childIndex < 8) {
            uint8_t j = frame.internalOrder[frame.childIndex++];
            OctreeNode * node = frame.node;
            ChildBlock * block = node->getBlock(tree->allocator);
            OctreeNode* child = node->getChildNode(j, tree->allocator, block);

            if (child) {
                OctreeNodeData data(frame.level+1, child, frame.cube.getChild(j), frame.context, child->sdf);
                stack.push(StackFrame(data, 0, false));
            }
        } else {
            // After all children are processed, apply `after()`
            after(tree, frame);
            stack.pop();
        }
    }
}

void IteratorHandler::iterateFlatOut(Octree * tree, OctreeNodeData params) {
    if (!params.node) return;
    params.context = NULL;

    stackOut.push(StackFrameOut(params, false));

    // A single shared array to hold the child processing order.
    uint8_t internalOrder[8];

    while (!stackOut.empty()) {
        StackFrameOut &frame = stackOut.top();

        if (!frame.visited) {
            
            // First visit: execute before() and update context.
            before(tree, frame);
            frame.visited = true;

            // Only process children if the test passes.
            if (!test(tree, frame)) {
                stackOut.pop();
                continue;
            }

            // Compute the child order for this node.
            getOrder(tree, frame, internalOrder);

            // Push all valid children in reverse order so that they are processed
            // in the original (correct) order when popped.
            for (int i = 7; i >= 0; --i) {
                uint8_t j = internalOrder[i];
                OctreeNode * node = frame.node;
                ChildBlock * block = node->getBlock(tree->allocator);
                OctreeNode* child = node->getChildNode(j, tree->allocator, block);
                if (child) {
                    stackOut.push(StackFrameOut(OctreeNodeData(frame.level + 1, child, frame.cube.getChild(j), frame.context, child->sdf), false));
                }
            }
        } else {
            // Second visit: all children have been processed; now call after().
            after(tree, frame);
            stackOut.pop();
        }
    }
}

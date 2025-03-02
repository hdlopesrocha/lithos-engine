#include "math.hpp"
#include <stack>
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

void IteratorHandler::iterateNonRecursive(int level, OctreeNode * root, BoundingCube cube, void * context) {
    if (!root) return;

    struct StackFrame {
        int level;
        OctreeNode* node;
        BoundingCube cube;
        void* context;
        int childIndex; // Tracks which child we're processing
        int internalOrder[8]; // Stores child processing order
    };

    std::stack<StackFrame> stack;
    stack.push({level, root, cube, context, 0, {0}});

    while (!stack.empty()) {
        StackFrame &frame = stack.top();

        // When first visiting this node
        if (frame.childIndex == 0) {
            frame.context = before(frame.level, frame.node, frame.cube, frame.context);
            if (!test(frame.level, frame.node, frame.cube, frame.context)) {
                stack.pop(); // Skip children, go back up
                continue;
            }
            getOrder(frame.cube, frame.internalOrder); // Get order only once per node
        }

        // Process children in order
        while (frame.childIndex < 8) {
            int j = frame.internalOrder[frame.childIndex++];
            OctreeNode* child = frame.node->children[j];

            if (child) {
                stack.push({frame.level + 1, child, Octree::getChildCube(frame.cube, j), frame.context, 0, {0}});
                break; // Move to processing the child next iteration
            }
        }

        // If all children were processed, apply `after()` and go up
        if (frame.childIndex == 8) {
            after(frame.level, frame.node, frame.cube, frame.context);
            stack.pop();
        }
    }
}

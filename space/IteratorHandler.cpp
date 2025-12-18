#include "space.hpp"
#include <queue>

#include <atomic>
#include <functional>
#include "ConcurrentQueue.hpp"

void IteratorHandler::iterateParallelBFS(const Octree &tree, OctreeNodeData &rootParams, ThreadPool& pool)
{
    if (rootParams.node == NULL)
        return;

    ConcurrentQueue<OctreeNodeData> queue;
    std::atomic<int> activeTasks {0};

    // Push root node
    queue.push(rootParams);

    std::mutex doneMutex;
    std::condition_variable doneCV;

    auto worker = [&]() {
        OctreeNodeData params = OctreeNodeData();

        while (true) {
            // Try get work
            if (!queue.tryPop(params)) {
                // Exit condition: no work and no active tasks
                if (activeTasks.load() == 0 && queue.empty())
                    break;
                std::this_thread::yield();
                continue;
            }

            activeTasks.fetch_add(1);

            if (params.node != NULL) {
                before(tree, params);

                if (params.node != NULL && test(tree, params)) {
                    uint8_t internalOrder[8];
                    getOrder(tree, params, internalOrder);

                    OctreeNode* children[8] = {
                        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
                    };

                    params.node->getChildren(*tree.allocator, children);

                    for (int i = 0; i < 8; ++i) {
                        uint8_t j = internalOrder[i];
                        OctreeNode* child = children[j];

                        if (child == params.node) {
                            throw std::runtime_error("Wrong pointer @ iter!");
                        }

                        if (child != NULL && child != params.node) {
                            OctreeNodeData childData(
                                params.level + 1,
                                child,
                                params.cube.getChild(j), 
                                params.containmentType,
                                params.context,
                                child->sdf
                            );

                            queue.push(childData);
                        }
                    }

                    after(tree, params); // only if test() passed
                }
            }

            activeTasks.fetch_sub(1);
        }
    };

    // Launch worker threads
    size_t threads = pool.threadCount();
    for (size_t i = 0; i < threads; ++i) {
        pool.enqueue(worker);
    }

    // Wait until all work is complete
    {
        std::unique_lock<std::mutex> lock(doneMutex);
        doneCV.wait(lock, [&]() {
            return queue.empty() && activeTasks.load() == 0;
        });
    }
}


void IteratorHandler::iterateBFS(const Octree &tree, OctreeNodeData &rootParams)
{
    if (rootParams.node == NULL)
        return;


    std::vector<std::future<bool>> futures;
	futures.reserve(8);

    std::queue<OctreeNodeData> q;
    q.push(rootParams);

    while (!q.empty()) {
        OctreeNodeData params = q.front();
        q.pop();

        if (params.node == NULL)
            continue;

        // Same as recursive version
        before(tree, params);

        if (params.node != NULL && test(tree, params)) {
            uint8_t internalOrder[8];
            getOrder(tree, params, internalOrder);

            OctreeNode* children[8] = {
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
            };

            params.node->getChildren(*tree.allocator, children);

            for (int i = 0; i < 8; ++i) {
                uint8_t j = internalOrder[i];
                OctreeNode* child = children[j];

                if (child == params.node) {
                    throw std::runtime_error("Wrong pointer @ iter!");
                }

                if (child != NULL && child != params.node) {
                    OctreeNodeData childData(
                        params.level + 1,
                        child,
                        params.cube.getChild(j),
                        params.containmentType,
                        params.context,
                        child->sdf
                    );

                    // BFS: push instead of recursive call
                    q.push(childData);
                }
            }

            after(tree, params); // only if test() passed
        }
    }
}



void IteratorHandler::iterateMultiThreaded(const Octree &tree, OctreeNodeData &params) {
    if(params.node != NULL) {
        before(tree, params);
        if(params.node != NULL && test(tree, params)) {
            uint8_t internalOrder[8];
            getOrder(tree, params, internalOrder);

            OctreeNode* children[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
            params.node->getChildren(*tree.allocator, children);
            
            std::vector<std::thread> threads;
            threads.reserve(8);

            for(uint i=0; i <8 ; ++i) {
                uint8_t j = internalOrder[i];
                OctreeNode * child = children[j];
                if (child == params.node) {
                    throw std::runtime_error("Wrong pointer @ iter!");
                }                
                if(child != NULL && params.node != child) {
                    OctreeNodeData data = OctreeNodeData( params.level+1, child, params.cube.getChild(j), params.containmentType, params.context, child->sdf);
                    if(!child->isChunk()) {
                        threads.emplace_back([this, &tree, &data]() {
                            this->iterateMultiThreaded(tree, data);
                        });
                    } else {
                        this->iterate(tree, data);
                    }
                }
            }

            for(std::thread &t : threads) {
                if(t.joinable()) {
                    t.join();
                }
            }
        

            after(tree, params); // only if test() passed
        }
    }
}

void IteratorHandler::iterate(const Octree &tree, OctreeNodeData &params) {
    if(params.node != NULL) {
        before(tree, params);
        if(params.node != NULL && test(tree, params)) {
            uint8_t internalOrder[8];
            getOrder(tree, params, internalOrder);

            OctreeNode* children[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
            params.node->getChildren(*tree.allocator, children);
            for(int i=0; i <8 ; ++i) {
                uint8_t j = internalOrder[i];
                OctreeNode * child = children[j];
                if (child == params.node) {
                    throw std::runtime_error("Wrong pointer @ iter!");
                }                
                if(child != NULL && params.node != child) {
                    OctreeNodeData data = OctreeNodeData( params.level+1, child, params.cube.getChild(j), params.containmentType, params.context, child->sdf);
                    this->iterate(tree, data);
                }
            }
            after(tree, params); // only if test() passed
        }
    }
}

void IteratorHandler::iterateFlatIn(const Octree &tree, OctreeNodeData &params) {
    params.context = NULL;
    uint8_t internalOrder[8];

    flatData.push(params);
    while (!flatData.empty()) {
        OctreeNodeData data = flatData.top();
        flatData.pop();

        OctreeNode* node = data.node;
        before(tree, data);

        if (node != NULL && test(tree, data)) {
            getOrder(tree, data, internalOrder);
            OctreeNode* children[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
            node->getChildren(*tree.allocator, children);
            for (int i = 7; i >= 0; --i) {
                uint8_t j = internalOrder[i];
                OctreeNode* child = children[j];

                if (child == node) {
                    throw std::runtime_error("Wrong pointer!");
                }

                if (child != NULL) {
                    flatData.push(OctreeNodeData(
                        data.level + 1,
                        child,
                        data.cube.getChild(j),
                        data.containmentType,
                        data.context,
                        child->sdf
                    ));
                }
            }

            after(tree, data); // only if test() passed
        }
    }
}

void IteratorHandler::iterateFlat(const Octree &tree, OctreeNodeData &params) {
    if (params.node != NULL) return;
    params.context = NULL;

    stack.push(StackFrame(params, 0, false));

    while (!stack.empty()) {
        StackFrame &frame = stack.top();

        if (!frame.secondVisit) {
            // First visit: Apply `before()`
            before(tree, frame);

            if (!(frame.node !=NULL && test(tree, frame))) {
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
            ChildBlock * block = node->getBlock(*tree.allocator);
            OctreeNode* child = block->get(j, *tree.allocator);

            if (child) {
                OctreeNodeData data(frame.level+1, child, frame.cube.getChild(j), frame.containmentType, frame.context, child->sdf);
                stack.push(StackFrame(data, 0, false));
            }
        } else {
            // After all children are processed, apply `after()`
            after(tree, frame);
            stack.pop();
        }
    }
}

void IteratorHandler::iterateFlatOut(const Octree &tree, OctreeNodeData &params) {
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
            {
                OctreeNode* children[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
                frame.node->getChildren(*tree.allocator, children);
                for (int i = 7; i >= 0; --i) {
                    uint8_t j = internalOrder[i];
                    OctreeNode* child = children[j];
                    if (child) {
                        stackOut.push(StackFrameOut(OctreeNodeData(frame.level + 1, child, frame.cube.getChild(j), frame.containmentType, frame.context, child->sdf), false));
                    }
                }
            }
        } else {
            // Second visit: all children have been processed; now call after().
            after(tree, frame);
            stackOut.pop();
        }
    }
}

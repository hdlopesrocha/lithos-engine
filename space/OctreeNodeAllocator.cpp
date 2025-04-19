#include "space.hpp" // For OctreeNode

OctreeNodeAllocator::OctreeNodeAllocator() : blockSize(1024) {

}

OctreeNodeAllocator::~OctreeNodeAllocator() {
    // Free all allocated memory blocks
    for (OctreeNode* block : allocatedBlocks) {
        std::free(block);
    }
}

void OctreeNodeAllocator::allocateBlock() {
    // Allocate a new block of memory
    OctreeNode* block = static_cast<OctreeNode*>(std::malloc(blockSize * sizeof(OctreeNode)));
    assert(block != nullptr && "Failed to allocate memory for OctreeNodeAllocator");

    // Add the new block to the list of allocated blocks
    allocatedBlocks.push_back(block);

    // Add all memory blocks in the new block to the free list
    for (size_t i = 0; i < blockSize; ++i) {
        freeList.push_back(&block[i]);
    }
}

OctreeNode* OctreeNodeAllocator::allocate() {
    // If the free list is empty, allocate a new block
    if (freeList.empty()) {
        allocateBlock();
    }

    // Get a block from the free list
    OctreeNode* ptr = freeList.back();
    freeList.pop_back();
    return ptr;
}

void OctreeNodeAllocator::deallocate(OctreeNode* ptr) {
    // Ensure the pointer belongs to the allocator
    bool validPointer = false;
    for (OctreeNode* block : allocatedBlocks) {
        if (ptr >= block && ptr < block + blockSize) {
            validPointer = true;
            break;
        }
    }

    assert(validPointer && "Attempting to deallocate a pointer not owned by the allocator");

    // Return the block to the free list
    freeList.push_back(ptr);
}
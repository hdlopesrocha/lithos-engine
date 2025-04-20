#include "space.hpp" // For OctreeNode


template <typename T> class Allocator {
	std::vector<T*> freeList; 
	std::vector<T*> allocatedBlocks;
	size_t blockSize; 

    void allocateBlock() {
        // Allocate a new block of memory
        T* block = static_cast<T*>(std::malloc(blockSize * sizeof(T)));
        assert(block != nullptr && "Failed to allocate memory for Allocator");
    
        // Add the new block to the list of allocated blocks
        allocatedBlocks.push_back(block);
    
        // Add all memory blocks in the new block to the free list
        for (size_t i = 0; i < blockSize; ++i) {
            freeList.push_back(&block[i]);
        }
    }


	public:
    Allocator() : blockSize(1024) {

    }
    
    ~Allocator() {
        // Free all allocated memory blocks
        for (T* block : allocatedBlocks) {
            std::free(block);
        }
    }

    T* allocate() {
        // If the free list is empty, allocate a new block
        if (freeList.empty()) {
            allocateBlock();
        }
    
        // Get a block from the free list
        T* ptr = freeList.back();
        freeList.pop_back();
        return ptr;
    }

    void deallocate(T* ptr) {
        // Ensure the pointer belongs to the allocator
        bool validPointer = false;
        for (T* block : allocatedBlocks) {
            if (ptr >= block && ptr < block + blockSize) {
                validPointer = true;
                break;
            }
        }
    
        assert(validPointer && "Attempting to deallocate a pointer not owned by the allocator");
    
        // Return the block to the free list
        freeList.push_back(ptr);
    }
    
};



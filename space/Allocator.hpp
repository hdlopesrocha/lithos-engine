#include "space.hpp" // For OctreeNode


template <typename T> class Allocator {
	std::vector<T*> freeList; 
	std::vector<T*> allocatedBlocks;
	size_t blockSize; 
    std::mutex mtx;

    // thread safe
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

    size_t getBlockSize() const {
        return blockSize;
    }

    size_t getAllocatedBlocksCount() {
        mtx.lock();
        size_t result = allocatedBlocks.size();
        mtx.unlock();
        return result;
    }

    uint getIndex(T* ptr) {
        if(ptr == NULL) {
            return UINT_MAX;
        }
        mtx.lock();
        // Iterate through all allocated blocks to find the block containing the pointer
        for (size_t blockIndex = 0; blockIndex < allocatedBlocks.size(); ++blockIndex) {
            T* block = allocatedBlocks[blockIndex];
            if (ptr >= block && ptr < block + blockSize) {
                // Calculate the index within the block
                size_t offset = ptr - block;
                // Return the global index across all blocks
                mtx.unlock();
                return static_cast<uint>(blockIndex * blockSize + offset);
            }
        }
        mtx.unlock();
        // If the pointer is not found, throw an error
        assert(false && "Pointer does not belong to any allocated block");
        return static_cast<uint>(UINT_MAX); // Return an invalid index
    }

    T * getFromIndex(uint index) {
        if(index == UINT_MAX) {
            return NULL;
        }
        // Calculate the block index and offset within the block
        size_t blockIndex = index / blockSize;
        size_t offset = index % blockSize;
        mtx.lock();
        // Check if the block index is valid
        assert(blockIndex < allocatedBlocks.size() && "Block index out of range");
        T * result = &allocatedBlocks[blockIndex][offset];
        mtx.unlock();
        // Return the pointer to the requested element
        return result;
    }

    T* allocate() {
        mtx.lock();
        // If the free list is empty, allocate a new block
        if (freeList.empty()) {
            allocateBlock();
        }
        // Get a block from the free list
        T* ptr = freeList.back();
        freeList.pop_back();
        mtx.unlock();
        return ptr;
    }

    void deallocate(T* ptr) {
        mtx.lock();
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
        mtx.unlock();
    }
};



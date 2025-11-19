#include <vector>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <iostream>
#define NDEBUG 1

template <typename T>
class Allocator {
private:
    struct Block {
        T* data;          // ponteiro para os elementos
        size_t startIndex; // índice global do primeiro elemento deste bloco
    };

    std::vector<Block> blocks;
    std::vector<T*> freeList;
    #ifndef NDEBUG
    std::unordered_set<T*> deallocatedSet;
    #endif
    const size_t blockSize;
    size_t totalAllocated = 0;
    mutable std::shared_mutex mutex;    

    void allocateBlock() {
        T* data = static_cast<T*>(std::malloc(blockSize * sizeof(T)));
        if (!data) throw std::bad_alloc();

        blocks.push_back({ data, totalAllocated });

        for (size_t i = 0; i < blockSize; ++i) {
            freeList.push_back(&data[i]);
            #ifndef NDEBUG
            deallocatedSet.insert(&data[i]);
            #endif
        }

        totalAllocated += blockSize;
    }

public:
    Allocator(size_t blockSize) : blockSize(blockSize) {
        std::cout << "Allocator(" << blockSize << ")" << std::endl;
    }

    ~Allocator() {
        for (auto &b : blocks) std::free(b.data);
    }

    // -------------------
    // Alocação / Liberação
    // -------------------
    T* allocate() {
        std::unique_lock lock(mutex);
        if (freeList.empty()) allocateBlock();

        T* ptr = freeList.back();
        freeList.pop_back();
        
        #ifndef NDEBUG
        if (deallocatedSet.find(ptr) == deallocatedSet.end()) {
            throw std::runtime_error("Double allocate!");
        }
        deallocatedSet.erase(ptr);
        #endif
        return ptr;
    }

    void deallocate(T* ptr) {
        if (!ptr) return;

        std::unique_lock lock(mutex);
        #ifndef NDEBUG
        if (deallocatedSet.find(ptr) != deallocatedSet.end()) {
            throw std::runtime_error("Double deallocate!");
        }
        #endif
        // check pointer belongs to a block
        bool valid = false;
        for (auto &b : blocks) {
            if (ptr >= b.data && ptr < b.data + blockSize) { valid = true; break; }
        }
        assert(valid && "Pointer does not belong to allocator");
        #ifndef NDEBUG
        deallocatedSet.insert(ptr);
        #endif
        freeList.push_back(ptr);
    }

    // -------------------
    // Index <-> Pointer O(1)
    // -------------------
    uint getIndex(T* ptr) {
        if (!ptr) return UINT_MAX;

        std::shared_lock lock(mutex); // multiple allowed

        for (auto &b : blocks) {
            if (ptr >= b.data && ptr < b.data + blockSize) {
                return static_cast<uint>(b.startIndex + (ptr - b.data));
            }
        }
        throw std::runtime_error("Pointer does not belong to allocator");
    }

    T* getFromIndex(uint index) {
        if (index == UINT_MAX) return nullptr;

        std::shared_lock lock(mutex); // multiple allowed
        uint blockIdx = index / blockSize;
        if (blockIdx >= blocks.size()) {
            throw std::runtime_error("Invalid index");
        }
        uint offset = index % blockSize;

        Block& b = blocks[blockIdx];
        
        T* ptr = b.data + offset;
        
        #ifndef NDEBUG
        if (deallocatedSet.find(ptr) != deallocatedSet.end()) {
            throw std::runtime_error("Accessing deallocated pointer");
        }
        #endif
 
        return ptr;
        //throw std::runtime_error("Invalid index");
    }

    uint allocateIndex() {
        T* ptr = allocate();
        return getIndex(ptr);
    }

    size_t getAllocatedBlocksCount() {
        std::shared_lock lock(mutex);
        return blocks.size();
    }

    size_t getBlockSize() const { return blockSize; }
};

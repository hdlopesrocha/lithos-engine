#include <vector>
#include <unordered_set>
#include <mutex>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <iostream>

template <typename T>
class Allocator {
private:
    struct Block {
        T* data;          // ponteiro para os elementos
        size_t startIndex; // índice global do primeiro elemento deste bloco
    };

    std::vector<Block> blocks;
    std::vector<T*> freeList;
    std::unordered_set<T*> deallocatedSet;
    const size_t blockSize;
    size_t totalAllocated = 0;
    std::mutex mutex;

    void allocateBlock() {
        T* data = static_cast<T*>(std::malloc(blockSize * sizeof(T)));
        if (!data) throw std::bad_alloc();

        blocks.push_back({ data, totalAllocated });

        for (size_t i = 0; i < blockSize; ++i) {
            freeList.push_back(&data[i]);
            deallocatedSet.insert(&data[i]);
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
        std::lock_guard<std::mutex> lock(mutex);
        if (freeList.empty()) allocateBlock();

        T* ptr = freeList.back();
        freeList.pop_back();

        if (deallocatedSet.find(ptr) == deallocatedSet.end()) {
            throw std::runtime_error("Double allocate!");
        }
        deallocatedSet.erase(ptr);
        return ptr;
    }

    void deallocate(T* ptr) {
        if (!ptr) return;

        std::lock_guard<std::mutex> lock(mutex);
        if (deallocatedSet.find(ptr) != deallocatedSet.end()) {
            throw std::runtime_error("Double deallocate!");
        }

        // check pointer belongs to a block
        bool valid = false;
        for (auto &b : blocks) {
            if (ptr >= b.data && ptr < b.data + blockSize) { valid = true; break; }
        }
        assert(valid && "Pointer does not belong to allocator");

        deallocatedSet.insert(ptr);
        freeList.push_back(ptr);
    }

    // -------------------
    // Index <-> Pointer O(1)
    // -------------------
    uint getIndex(T* ptr) {
        if (!ptr) return UINT_MAX;

        std::lock_guard<std::mutex> lock(mutex);

        for (auto &b : blocks) {
            if (ptr >= b.data && ptr < b.data + blockSize) {
                return static_cast<uint>(b.startIndex + (ptr - b.data));
            }
        }
        throw std::runtime_error("Pointer does not belong to allocator");
    }

    T* getFromIndex(uint index) {
        if (index == UINT_MAX) return nullptr;

        std::lock_guard<std::mutex> lock(mutex);

        // encontra o bloco que contém o índice
        size_t blockIdx = index / blockSize; // simplificação se os blocos forem sequenciais
        size_t offset = index % blockSize;

        if (blockIdx >= blocks.size()) throw std::runtime_error("Invalid index");

        T* ptr = &blocks[blockIdx].data[offset];
        if (deallocatedSet.find(ptr) != deallocatedSet.end())
            throw std::runtime_error("Accessing deallocated pointer");

        return ptr;
    }

    uint allocateIndex() {
        T* ptr = allocate();
        return getIndex(ptr);
    }

    size_t getAllocatedBlocksCount() {
        std::lock_guard<std::mutex> lock(mutex);
        return blocks.size();
    }

    size_t getBlockSize() const { return blockSize; }
};

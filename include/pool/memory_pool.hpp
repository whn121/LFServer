#ifndef MEMORY_POOL_HPP
#define MEMORY_POOL_HPP

#include <vector>
#include <atomic>
#include <cstddef>

// 定长内存池：管理固定大小块
class FixedMemPool {
public:
    explicit FixedMemPool(size_t block_size, size_t block_count = 1024)
        : block_size_(block_size), block_count_(block_count), free_list_(nullptr) {
        allocate_chunk();
    }
    ~FixedMemPool() {
        for (auto ptr : chunks_) ::operator delete(ptr);
    }

    void* allocate() {
        if (!free_list_) allocate_chunk(); // 无空闲块则扩充
        void* p = free_list_;
        free_list_ = *static_cast<void**>(free_list_); // 从链表头部取
        return p;
    }

    void deallocate(void* ptr) {
        *static_cast<void**>(ptr) = free_list_;
        free_list_ = ptr;                     // 插入链表头部
    }

    size_t block_size() const { return block_size_; }

private:
    void allocate_chunk() {
        char* mem = static_cast<char*>(::operator new(block_size_ * block_count_));
        chunks_.push_back(mem);
        // 构建自由链表
        for (size_t i = 0; i < block_count_; ++i) {
            char* block = mem + i * block_size_;
            *static_cast<void**>(block) = free_list_;
            free_list_ = block;
        }
    }

    size_t block_size_;
    size_t block_count_;
    void* free_list_;
    std::vector<void*> chunks_;
};

// 多级内存池：根据请求大小选取合适的池
class MemoryPool {
public:
    static MemoryPool& instance() {
        static MemoryPool pool;
        return pool;
    }

    void* allocate(size_t size) {
        if (size > MAX_BLOCK_SIZE) return ::operator new(size);
        return find_pool(size)->allocate();
    }

    void deallocate(void* ptr, size_t size) {
        if (size > MAX_BLOCK_SIZE) {
            ::operator delete(ptr);
            return;
        }
        find_pool(size)->deallocate(ptr);
    }

private:
    MemoryPool() {
        // 初始化一系列固定大小池（8B, 16B, 32B, ..., 4096B）
        for (size_t sz = MIN_BLOCK_SIZE; sz <= MAX_BLOCK_SIZE; sz <<= 1) {
            pools_.push_back(new FixedMemPool(sz));
        }
    }
    ~MemoryPool() {
        for (auto p : pools_) delete p;
    }

    FixedMemPool* find_pool(size_t size) {
        size_t idx = 0;
        size_t sz = MIN_BLOCK_SIZE;
        while (sz < size && sz <= MAX_BLOCK_SIZE) {
            sz <<= 1;
            ++idx;
        }
        return pools_[idx];
    }

    static constexpr size_t MIN_BLOCK_SIZE = 8;
    static constexpr size_t MAX_BLOCK_SIZE = 4096;
    std::vector<FixedMemPool*> pools_;
};


#endif

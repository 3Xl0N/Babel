#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <atomic>
#include <vector>

template<typename T, size_t Size>
class RingBuffer {
public:
    RingBuffer() : head(0), tail(0) {}
    
    bool push(const T& item) {
        size_t current_head = head.load(std::memory_order_relaxed);
        size_t next_head = (current_head + 1) % Size;
        if(next_head == tail.load(std::memory_order_acquire)) return false;
        buffer[current_head] = item;
        head.store(next_head, std::memory_order_release);
        return true;
    }
    
    bool pop(T& item) {
        size_t current_tail = tail.load(std::memory_order_relaxed);
        if(current_tail == head.load(std::memory_order_acquire)) return false;
        item = buffer[current_tail];
        tail.store((current_tail + 1) % Size, std::memory_order_release);
        return true;
    }

private:
    T buffer[Size];
    std::atomic<size_t> head, tail;
};

#endif

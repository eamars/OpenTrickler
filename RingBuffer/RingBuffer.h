#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#define DEFAULT_MAX_BUFFER_SZ 64

#include <stdint.h>
#include <stdlib.h>

template <typename T>
class RingBuffer
{
private:
    const size_t buffer_size;
    size_t read_ptr;
    size_t write_ptr;
    size_t count;
    
    // mutex lock
    bool mux; 
    
    // overflow
    bool is_over_flow;
    
    // container
    T *data;
    
    
public:
    RingBuffer(const size_t size=DEFAULT_MAX_BUFFER_SZ);
    ~RingBuffer();
    
    // psudo mutex
    bool isLocked();
    void lock();
    void unlock();
    
    // enqueue and dequeue
    void enqueue(T in);
    T dequeue();
    
    // pointer operation
    size_t getReadPtr();
    size_t getWritePtr();
    size_t getCounter();
    
    // overflow
    bool getOverFlow();
    void clearOverFlow();
    
    // operation
    T first();
    T last();
    
    // random access
    T operator[](size_t idx);
};

#endif
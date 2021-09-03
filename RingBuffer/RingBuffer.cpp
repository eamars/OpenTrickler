#include "RingBuffer.h"
#include <stdio.h>
#include <stdlib.h>

template <typename T>
RingBuffer<T>::RingBuffer(const size_t size)
    :buffer_size(size)
{
    read_ptr = 0;
    write_ptr = 0;
    count = 0;
    
    // mutex lock
    mux = false; 
    
    // overflow
    is_over_flow = false;
    
    // container
    // data = new T[buffer_size];
    
    // use c styled memory allocation instead
    data = (T *)malloc(buffer_size * sizeof(T));
}

template <typename T>
RingBuffer<T>::~RingBuffer()
{
    // delete[] data;
    free(data);
}

template <typename T>
bool RingBuffer<T>::isLocked()
{
    return mux;
}

template <typename T>
void RingBuffer<T>::lock()
{
    mux = true;
}

template <typename T>
void RingBuffer<T>::unlock()
{
    mux = false;
}

template <typename T>
void RingBuffer<T>::enqueue(T in)
{
    data[write_ptr++] = in;
    write_ptr %= buffer_size;
    
    count++;
}

template <typename T>
T RingBuffer<T>::dequeue()
{
    T temp = data[read_ptr++];
    read_ptr %= buffer_size;
    
    count--;
    return temp;   
}

template <typename T>
size_t RingBuffer<T>::getReadPtr()
{
    return read_ptr;
}

template <typename T>
size_t RingBuffer<T>::getWritePtr()
{
    return write_ptr;
}

template <typename T>
size_t RingBuffer<T>::getCounter()
{
    return count;
}

template <typename T>
bool RingBuffer<T>::getOverFlow()
{
    return is_over_flow;
}

template <typename T>
void RingBuffer<T>::clearOverFlow()
{
    is_over_flow = false;  
}

template <typename T>
T RingBuffer<T>::first()
{
    return data[read_ptr];
}

template <typename T>
T RingBuffer<T>::last()
{
    return data[write_ptr];
}

template <typename T>
T RingBuffer<T>::operator[](size_t idx)
{
    return data[idx];
}

// force compiler to create code for template
template class RingBuffer<uint8_t>;

// forward declearation for AlohaFrame queue
class AlohaFrame;
template class RingBuffer<AlohaFrame *>;
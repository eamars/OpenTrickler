#ifndef FLOATRINGBUFFER_H_
#define FLOATRINGBUFFER_H_

#include <stdint.h>
#include <stdlib.h>


class FloatRingBuffer
{

private:
    
    size_t read_ptr;
    size_t write_ptr;
    size_t count;
    
    // mutex lock
    bool mux; 
    
    // overflow
    bool is_over_flow;

protected:
    const size_t buffer_size;
    
    // container
    float *data;
    
    
public:
    FloatRingBuffer(const size_t size);
    ~FloatRingBuffer();
    
    // psudo mutex
    bool isLocked();
    void lock();
    void unlock();
    
    // enqueue and dequeue
    void enqueue(float in);
    float dequeue();
    
    // pointer operation
    size_t getReadPtr();
    size_t getWritePtr();
    size_t getCounter();
    
    // overflow
    bool getOverFlow();
    void clearOverFlow();
    
    // operation
    float first();
    float last();
    
    // random access
    float operator[](size_t idx);

    // Arithmetic operatings
    double getSum(void);
    float getSd(void);
    float getMean(void);

};

#endif // FLOATRINGBUFFER_H_
#include "FloatRingBuffer.h"
#include "math.h"

// TODO: Integrate CMSIS DSP for core algorithm

float FloatRingBuffer::getSd(void){
    double sum = getSum();
    float mean = sum / buffer_size;
    double sum_of_sqre = 0.0;

    for (size_t idx=0; idx<buffer_size; idx++){
        sum_of_sqre += pow(data[idx] - mean, 2);
    }

    float sd = sqrt(sum_of_sqre / buffer_size);
    
    return sd;
}

double FloatRingBuffer::getSum(){
    double sum = 0.0;
    for (size_t idx=0; idx<buffer_size; idx++){
        sum += data[idx];
    }

    return sum;
}

float FloatRingBuffer::getMean(void){
    double sum = getSum();
    float mean = sum / buffer_size;

    return mean;
}


FloatRingBuffer::FloatRingBuffer(const size_t size)
    :buffer_size(size)
{
    reset();
    
    // container
    // data = new T[buffer_size];
    
    // use c styled memory allocation instead
    data = (float *)malloc(buffer_size * sizeof(float));
}

FloatRingBuffer::~FloatRingBuffer()
{
    // delete[] data;
    free(data);
}

bool FloatRingBuffer::isLocked()
{
    return mux;
}

void FloatRingBuffer::lock()
{
    mux = true;
}

void FloatRingBuffer::unlock()
{
    mux = false;
}

void FloatRingBuffer::enqueue(float in)
{
    data[write_ptr++] = in;
    write_ptr %= buffer_size;
    
    if (count < buffer_size){
        count++;
    }
}

float FloatRingBuffer::dequeue()
{
    float temp = data[read_ptr++];
    read_ptr %= buffer_size;

    if (count > 0) {
        count--;
    }
    
    return temp;   
}

void FloatRingBuffer::reset()
{
    read_ptr = 0;
    write_ptr = 0;
    count = 0;
    
    // mutex lock
    mux = false; 
    
    // initialize overflow
    clearOverFlow();
}


size_t FloatRingBuffer::getReadPtr()
{
    return read_ptr;
}

size_t FloatRingBuffer::getWritePtr()
{
    return write_ptr;
}

size_t FloatRingBuffer::getCounter()
{
    return count;
}

bool FloatRingBuffer::getOverFlow()
{
    return is_over_flow;
}

void FloatRingBuffer::clearOverFlow()
{
    is_over_flow = false;  
}

float FloatRingBuffer::first()
{
    return data[read_ptr];
}

float FloatRingBuffer::last()
{
    return data[write_ptr];
}

float FloatRingBuffer::operator[](size_t idx)
{
    return data[idx];
}


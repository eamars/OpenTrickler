#ifndef FLOATRINGBUFFER_H_
#define FLOATRINGBUFFER_H_

#include <RingBuffer.h>

class FloatRingBuffer : public RingBuffer<float>
{
public:
    float getSd(void);
    float getMean(void);
};

#endif // FLOATRINGBUFFER_H_
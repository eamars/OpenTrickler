#ifndef STEPPER_H_
#define STEPPER_H_


#include "mbed.h"


class Stepper {

private:
    DigitalOut _ENA;
    DigitalOut _ENB;

    DigitalOut _IN1;
    DigitalOut _IN2;
    DigitalOut _IN3;
    DigitalOut _IN4;

    int _total_steps;
    int _current_step;

public:
    Stepper(PinName ENA, PinName ENB, PinName IN1, PinName IN2, PinName IN3, PinName IN4, int total_steps);
    void step(int num_of_steps);
    void stepMotor(int encoded_step);
};


#endif  // STEPPER_H_
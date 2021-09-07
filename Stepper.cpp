#include "Stepper.h"


Stepper::Stepper(PinName ENA, PinName ENB, PinName IN1, PinName IN2, PinName IN3, PinName IN4, int total_steps)
    : _ENA(ENA), _ENB(ENB), _IN1(IN1), _IN2(IN2), _IN3(IN3), _IN4(IN4) {
    
    _total_steps = total_steps;
    _current_step = 0;
    _enable_hold = true;

    setStepDelayUs(900);
}


void Stepper::setStepDelayUs(int step_delay_us){
    _step_delay_us = step_delay_us;
}


void Stepper::enableHold(bool enable) {
    _enable_hold = enable;
}

void Stepper::setRPM(int rpm) {
    _step_delay_us = 60 * 1000 * 1000 / _total_steps / rpm;
    printf("DELAY:%d\r\n", _step_delay_us);
}


void Stepper::step(int num_of_steps) {
    _ENA.write(1);
    _ENB.write(1);

    int step_left = abs(num_of_steps);
    
    bool direction = true;
    if (num_of_steps < 0){
        direction = false;
    }

    while (step_left > 0) {
        if (direction) {
            // Positive direction
            _current_step += 1;
            if (_current_step == _total_steps){
                _current_step = 0;
            }
        }
        else{
            if (_current_step == 0) {
                _current_step = _total_steps;
            }
            _current_step -= 1;
        }

        // decreate step left
        step_left -= 1;

        stepMotor(_current_step % 4);
        // ThisThread::sleep_for(5ms);
        wait_us(_step_delay_us);
        
    }

    if (!_enable_hold){
        _ENA.write(0);
        _ENB.write(0);
    }
}

void Stepper::stepMotor(int encoded_step) {
    switch (encoded_step) {
        case 0:  // 1010
        {
            _IN1.write(1);
            _IN2.write(0);
            _IN3.write(1);
            _IN4.write(0);
            break;
        }
        case 1:  // 0110
        {
            _IN1.write(0);
            _IN2.write(1);
            _IN3.write(1);
            _IN4.write(0);
            break;
        }
        case 2:  // 0101
        {
            _IN1.write(0);
            _IN2.write(1);
            _IN3.write(0);
            _IN4.write(1);
            break;
        }
        case 3:  // 1001
        {
            _IN1.write(1);
            _IN2.write(0);
            _IN3.write(0);
            _IN4.write(1);
            break;
        }
    }
}


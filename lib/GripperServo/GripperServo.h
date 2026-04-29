#ifndef GRIPPER_SERVO_H
#define GRIPPER_SERVO_H

#include <Arduino.h>
#include <Servo.h>
#include "config.h"

class GripperServo {
public:
    enum State { OPEN, CLOSED, ERROR };

    GripperServo();
    void initialize();

    void open();
    void close();
    void emergencyStop();

    State getState() const { return _state; }
    bool  isOpen()   const { return _state == OPEN;   }
    bool  isClosed() const { return _state == CLOSED; }

private:
    Servo   _servo;
    State   _state;
    uint8_t _currentAngle;

    void moveToAngle(uint8_t target);
};

#endif
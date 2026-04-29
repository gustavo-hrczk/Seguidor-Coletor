#include "GripperServo.h"

static const uint16_t DETACH_DELAY_MS = SERVO_STABILIZATION_TIME;

GripperServo::GripperServo()
    : _state(OPEN), _currentAngle(SERVO_ANGLE_OPEN) {}

void GripperServo::initialize() {
    _servo.attach(PIN_SERVO);
    _servo.write(SERVO_ANGLE_OPEN);
    _currentAngle = SERVO_ANGLE_OPEN;
    _state        = OPEN;
    delay(DETACH_DELAY_MS);
    _servo.detach();
    if (DEBUG_MODE) Serial.println(F("[Gripper] Inicializado - ABERTO"));
}

void GripperServo::open() {
    if (_state == OPEN) return;
    moveToAngle(SERVO_ANGLE_OPEN);
    _state = OPEN;
    if (DEBUG_MODE) Serial.println(F("[Gripper] ABERTO"));
}

void GripperServo::close() {
    if (_state == CLOSED) return;
    moveToAngle(SERVO_ANGLE_CLOSED);
    _state = CLOSED;
    if (DEBUG_MODE) Serial.println(F("[Gripper] FECHADO"));
}

void GripperServo::emergencyStop() {
    if (_servo.attached()) _servo.detach();
    _state = ERROR;
    if (DEBUG_MODE) Serial.println(F("[Gripper] EMERGENCY STOP"));
}

void GripperServo::moveToAngle(uint8_t target) {
    target = constrain(target, SERVO_ANGLE_OPEN, SERVO_ANGLE_CLOSED);

    if (!_servo.attached()) {
        _servo.attach(PIN_SERVO);
        _servo.write(_currentAngle);
        delay(50);
    }

    const int8_t step = (_currentAngle < target) ? 1 : -1;
    while (_currentAngle != target) {
        _currentAngle += step;
        _servo.write(_currentAngle);
        delay(SERVO_STEP_DELAY_MS);
    }

    delay(DETACH_DELAY_MS);
    _servo.detach();
}
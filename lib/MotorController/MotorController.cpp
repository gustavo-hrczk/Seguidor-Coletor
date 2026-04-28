#include "MotorController.h"

// ============================================================================
// IMPLEMENTAÇÃO: MotorController
// ============================================================================

MotorController::MotorController()
    : currentLeftSpeed(0), currentRightSpeed(0) {}

void MotorController::initialize() {
    // Configura pinos como OUTPUT
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_IN3, OUTPUT);
    pinMode(PIN_IN4, OUTPUT);
    pinMode(PIN_ENA, OUTPUT);
    pinMode(PIN_ENB, OUTPUT);
    
    // Inicia parado
    stop();
}

void MotorController::setMotorSpeed(int speedLeft, int speedRight) {
    currentLeftSpeed = speedLeft;
    currentRightSpeed = speedRight;
    
    // Motor esquerda (pinos 3, 4, 5)
    setPinDirection(PIN_IN3, PIN_IN4, PIN_ENA, speedLeft);
    
    // Motor direita (pinos 2, 1, 6)
    setPinDirection(PIN_IN1, PIN_IN2, PIN_ENB, speedRight);
}

void MotorController::move(Direction direction, uint8_t speed) {
    speed = constrain(speed, 0, 255);
    
    switch (direction) {
        case FORWARD:
            setMotorSpeed(-speed, speed);
            break;
        case BACKWARD:
            setMotorSpeed(speed, -speed);
            break;
        case TURN_LEFT:
            setMotorSpeed(-speed, -speed);
            break;
        case TURN_RIGHT:
            setMotorSpeed(speed, speed);
            break;
        case STOP:
        default:
            stop();
            break;
    }
}

void MotorController::stop() {
    setMotorSpeed(0, 0);
}

void MotorController::curveCompensated(Direction direction, uint8_t speed, float compensationFactor) {
    uint8_t outerSpeed = speed;
    uint8_t innerSpeed = (uint8_t)(speed * compensationFactor);
    
    applyDeadzoneCorrection((int&)outerSpeed);
    applyDeadzoneCorrection((int&)innerSpeed);
    
    if (direction == TURN_LEFT) {
        setMotorSpeed(-outerSpeed, innerSpeed);
    } else if (direction == TURN_RIGHT) {
        setMotorSpeed(innerSpeed, -outerSpeed);
    }
}

void MotorController::applyDeadzoneCorrection(int& pwmValue) const {
    // Se valor é positivo e menor que DEADZONE, elevar ao mínimo
    if (pwmValue > 0 && pwmValue < PWM_MIN_DEADZONE) {
        pwmValue = PWM_MIN_DEADZONE;
    }
    // Se valor é negativo e maior que -DEADZONE (valor absoluto menor), reduzir ao mínimo negativo
    else if (pwmValue < 0 && pwmValue > -PWM_MIN_DEADZONE) {
        pwmValue = -PWM_MIN_DEADZONE;
    }
}

void MotorController::setPinDirection(int in1, int in2, int pwmPin, int speed) {
    // Normalizar PWM
    int pwmValue = abs(speed);
    pwmValue = constrain(pwmValue, 0, 255);
    
    if (speed == 0) {
        // Parado
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        analogWrite(pwmPin, 0);
    } else if (speed > 0) {
        // Movimento para frente
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        analogWrite(pwmPin, pwmValue);
    } else {
        // Movimento para trás
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        analogWrite(pwmPin, pwmValue);
    }
}

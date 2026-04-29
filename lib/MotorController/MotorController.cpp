#include "MotorController.h"

// Convenção de sinal adotada em setMotorSpeed():
//   Motor esquerdo: positivo = ré, negativo = frente  (montagem invertida)
//   Motor direito:  positivo = frente, negativo = ré
// Isso é compensado em move() — não altere os sinais sem revisar move().

MotorController::MotorController()
    : currentLeftSpeed(0), currentRightSpeed(0) {}

void MotorController::initialize() {
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_IN3, OUTPUT);
    pinMode(PIN_IN4, OUTPUT);
    pinMode(PIN_ENA, OUTPUT);
    pinMode(PIN_ENB, OUTPUT);
    stop();
}

void MotorController::setMotorSpeed(int speedLeft, int speedRight) {
    currentLeftSpeed  = speedLeft;
    currentRightSpeed = speedRight;

    setPinDirection(PIN_IN3, PIN_IN4, PIN_ENA, speedLeft);
    setPinDirection(PIN_IN1, PIN_IN2, PIN_ENB, speedRight);
}

void MotorController::move(Direction direction, uint8_t speed) {
    speed = constrain(speed, 0, 255);

    switch (direction) {
        case FORWARD:    setMotorSpeed(-speed,  speed); break;
        case BACKWARD:   setMotorSpeed( speed, -speed); break;
        case TURN_LEFT:  setMotorSpeed(-speed, -speed); break;
        case TURN_RIGHT: setMotorSpeed( speed,  speed); break;
        case STOP:
        default:         stop(); break;
    }
}

void MotorController::stop() {
    setMotorSpeed(0, 0);
}

// ---------------------------------------------------------------------
// Curva compensada: motor externo a 100%, interno reduzido pelo fator
// Usa int para evitar cast undefined behavior com uint8_t
// ---------------------------------------------------------------------
void MotorController::curveCompensated(Direction direction, uint8_t speed,
                                        float compensationFactor) {
    int outerSpeed = constrain((int)speed, 0, 255);
    int innerSpeed = constrain((int)(speed * compensationFactor), 0, 255);

    applyDeadzoneCorrection(outerSpeed);
    applyDeadzoneCorrection(innerSpeed);

    if (direction == TURN_LEFT) {
        // Esquerda interna, direita externa
        setMotorSpeed(-innerSpeed, outerSpeed);
    } else if (direction == TURN_RIGHT) {
        // Direita interna, esquerda externa
        setMotorSpeed(-outerSpeed, innerSpeed);
    }
}

void MotorController::applyDeadzoneCorrection(int& pwmValue) const {
    if      (pwmValue > 0 && pwmValue < PWM_MIN_DEADZONE)  pwmValue = PWM_MIN_DEADZONE;
    else if (pwmValue < 0 && pwmValue > -PWM_MIN_DEADZONE) pwmValue = -PWM_MIN_DEADZONE;
}

void MotorController::setPinDirection(int in1, int in2, int pwmPin, int speed) {
    int pwmValue = constrain(abs(speed), 0, 255);

    if (speed == 0) {
        digitalWrite(in1, LOW);
        digitalWrite(in2, LOW);
        analogWrite(pwmPin, 0);
    } else if (speed > 0) {
        digitalWrite(in1, HIGH);
        digitalWrite(in2, LOW);
        analogWrite(pwmPin, pwmValue);
    } else {
        digitalWrite(in1, LOW);
        digitalWrite(in2, HIGH);
        analogWrite(pwmPin, pwmValue);
    }
}
#include "MotorController.h"

// Convenção de sinal em setMotorSpeed():
//   Motor esquerdo: positivo = ré,   negativo = frente  (montagem invertida)
//   Motor direito:  positivo = frente, negativo = ré
// Compensado em move() — não altere os sinais sem revisar move().

MotorController::MotorController()
    : _currentLeft(0), _currentRight(0),
      _prevError(0.0f), _lastPDTime(0) {}

void MotorController::initialize() {
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_IN3, OUTPUT);
    pinMode(PIN_IN4, OUTPUT);
    pinMode(PIN_ENA, OUTPUT);
    pinMode(PIN_ENB, OUTPUT);
    stop();
}

// ---------------------------------------------------------------------
void MotorController::setMotorSpeed(int speedLeft, int speedRight) {
    _currentLeft  = speedLeft;
    _currentRight = speedRight;
    setPinDirection(PIN_IN3, PIN_IN4, PIN_ENA, speedLeft);
    setPinDirection(PIN_IN1, PIN_IN2, PIN_ENB, speedRight);
}

// ---------------------------------------------------------------------
void MotorController::move(Direction direction, uint8_t speed) {
    speed = constrain(speed, 0, 255);
    switch (direction) {
        case FORWARD:    setMotorSpeed(-speed,  speed); break;
        case BACKWARD:   setMotorSpeed( speed, -speed); break;
        case TURN_LEFT:  setMotorSpeed(-speed, -speed); break;
        case TURN_RIGHT: setMotorSpeed( speed,  speed); break;
        default:         stop(); break;
    }
}

// ---------------------------------------------------------------------
void MotorController::stop() { setMotorSpeed(0, 0); }

// ---------------------------------------------------------------------
void MotorController::curveCompensated(Direction direction, uint8_t speed,
                                        float compensationFactor) {
    int outer = constrain((int)speed, 0, 255);
    int inner = constrain((int)(speed * compensationFactor), 0, 255);
    applyDeadzoneCorrection(outer);
    applyDeadzoneCorrection(inner);

    if      (direction == TURN_LEFT)  setMotorSpeed(-inner,  outer);
    else if (direction == TURN_RIGHT) setMotorSpeed(-outer,  inner);
}

// ---------------------------------------------------------------------
// Controlador PD de seguimento de linha
//
// correction = Kp * erro + Kd * (erro - erroAnterior)
//
// Positivo = linha à direita → acelera esquerdo, freia direito
// Negativo = linha à esquerda → freia esquerdo, acelera direito
// ---------------------------------------------------------------------
void MotorController::followLine(float linePosition, uint8_t baseSpeed) {
    unsigned long now = millis();
    if (now - _lastPDTime < PD_SAMPLE_MS) return;
    _lastPDTime = now;

    float error      = linePosition;
    float derivative = error - _prevError;
    float correction = (PD_KP * error) + (PD_KD * derivative);
    _prevError       = error;

    // Correção normalizada: 0.0 = reto, 1.0 = curva máxima
    float corrNorm = constrain(fabs(correction), 0.0f, 1.0f);

    // Motor externo sempre a baseSpeed
    // Motor interno reduzido proporcionalmente — nunca abaixo de PD_MIN_INNER_SPEED
    int outerSpeed = baseSpeed;
    int innerSpeed = (int)(baseSpeed * (1.0f - corrNorm));
    innerSpeed     = constrain(innerSpeed, PD_MIN_INNER_SPEED, baseSpeed);

    // Aplica direção da correção
    // Positivo = linha à direita → curva direita → esquerdo é externo
    // Negativo = linha à esquerda → curva esquerda → direito é externo
    int leftSpeed, rightSpeed;
    if (correction >= 0.0f) {
        leftSpeed  = outerSpeed;
        rightSpeed = innerSpeed;
    } else {
        leftSpeed  = innerSpeed;
        rightSpeed = outerSpeed;
    }

    // Convenção de sinal (motor esquerdo montado invertido)
    setMotorSpeed(-leftSpeed, rightSpeed);

    if (DEBUG_MODE) {
        static unsigned long lastDbg = 0;
        if (now - lastDbg >= 200) {
            lastDbg = now;
            Serial.print(F("[PD] err="));  Serial.print(error, 3);
            Serial.print(F(" cor="));      Serial.print(correction, 3);
            Serial.print(F(" L="));        Serial.print(leftSpeed);
            Serial.print(F(" R="));        Serial.println(rightSpeed);
        }
    }
}

// ---------------------------------------------------------------------
void MotorController::resetPD() {
    _prevError   = 0.0f;
    _lastPDTime  = 0;
}

// ---------------------------------------------------------------------
void MotorController::applyDeadzoneCorrection(int& v) const {
    if      (v > 0 && v <  PD_MIN_INNER_SPEED) v =  PD_MIN_INNER_SPEED;
    else if (v < 0 && v > -PD_MIN_INNER_SPEED) v = -PD_MIN_INNER_SPEED;
}

// ---------------------------------------------------------------------
void MotorController::setPinDirection(int in1, int in2, int pwmPin, int speed) {
    int pwm = constrain(abs(speed), 0, 255);
    if (speed == 0) {
        digitalWrite(in1, LOW); digitalWrite(in2, LOW); analogWrite(pwmPin, 0);
    } else if (speed > 0) {
        digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  analogWrite(pwmPin, pwm);
    } else {
        digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(pwmPin, pwm);
    }
}
#include "MotorController.h"

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

// ============================================================================
// applyTrimRight() / applyTrimLeft()
//
// Aplica o fator de trim individual de cada motor.
// O trim compensa assimetria física (desgaste, tolerância de fabricação).
//
// Fluxo:
//   1. Multiplica velocidade pelo fator MOTOR_TRIM_*
//   2. Eleva ao mínimo PWM_MIN_DEADZONE se estiver abaixo (preserva sinal)
//   3. Limita ao máximo 255
//
// Exemplo com MOTOR_TRIM_DIR = 0.93 e speed = 170:
//   trimmed = 170 × 0.93 = 158 → motor direito gira ~7% mais lento
// ============================================================================
int MotorController::applyTrimRight(int speed) const {
    if (speed == 0) return 0;
    int trimmed = (int)(speed * MOTOR_TRIM_ESQ);
    trimmed = constrain(trimmed, -255, 255);
    // Aplica deadzone: garante que valor baixo não fica abaixo do limiar de movimento
    if      (trimmed > 0 && trimmed < PWM_MIN_DEADZONE)  trimmed = PWM_MIN_DEADZONE;
    else if (trimmed < 0 && trimmed > -PWM_MIN_DEADZONE) trimmed = -PWM_MIN_DEADZONE;
    return trimmed;
}

int MotorController::applyTrimLeft(int speed) const {
    if (speed == 0) return 0;
    int trimmed = (int)(speed * MOTOR_TRIM_DIR);
    trimmed = constrain(trimmed, -255, 255);
    if      (trimmed > 0 && trimmed < PWM_MIN_DEADZONE)  trimmed = PWM_MIN_DEADZONE;
    else if (trimmed < 0 && trimmed > -PWM_MIN_DEADZONE) trimmed = -PWM_MIN_DEADZONE;
    return trimmed;
}

// ============================================================================
// setMotorSpeed()
// Ponto central — todos os métodos passam por aqui.
// Aplica trim antes de enviar ao hardware.
// ============================================================================
void MotorController::setMotorSpeed(int speedRight, int speedLeft) {
    _currentLeft  = speedRight;
    _currentRight = speedLeft;
    setPinDirection(PIN_IN3, PIN_IN4, PIN_ENA, applyTrimRight(speedRight));
    setPinDirection(PIN_IN1, PIN_IN2, PIN_ENB, applyTrimLeft(speedLeft));
}

// ============================================================================
// move()
// TURN_*:  giro no eixo (motores opostos) — preciso, raio zero
// CURVE_*: arco com um motor parado — menor consumo, raio maior
// ============================================================================
void MotorController::move(Direction direction, uint8_t speed) {
    speed = constrain(speed, 0, 255);
    switch (direction) {
        case FORWARD:     setMotorSpeed( speed,  speed); break;
        case BACKWARD:    setMotorSpeed(-speed, -speed); break;
        case TURN_LEFT:   setMotorSpeed( speed, -speed); break;
        case TURN_RIGHT:  setMotorSpeed(-speed,  speed); break;
        case CURVE_LEFT:  setMotorSpeed(0,        speed); break;
        case CURVE_RIGHT: setMotorSpeed(speed,        0); break;
        default:          stop(); break;
    }
}

void MotorController::stop() { setMotorSpeed(0, 0); }

// ============================================================================
// followLine()
// Controlador PD. Usa BASE_SPEED como referência via baseSpeed.
// O trim é aplicado em setMotorSpeed() — followLine() não precisa saber disso.
// ============================================================================
void MotorController::followLine(float linePosition, uint8_t baseSpeed) {
    unsigned long now = millis();
    if (now - _lastPDTime < PD_SAMPLE_MS) return;
    _lastPDTime = now;

    float error      = linePosition;
    float derivative = error - _prevError;
    float correction = (PD_KP * error) + (PD_KD * derivative);
    _prevError       = error;

    float corrNorm = constrain(fabs(correction), 0.0f, 1.0f);

    int outerSpeed = (int)baseSpeed;
    int innerSpeed = (int)(baseSpeed * (1.0f - corrNorm));
    innerSpeed     = constrain(innerSpeed, PD_MIN_INNER_SPEED, (int)baseSpeed);

    int leftSpeed, rightSpeed;
    if (correction >= 0.0f) {
        leftSpeed  = outerSpeed;
        rightSpeed = innerSpeed;
    } else {
        leftSpeed  = innerSpeed;
        rightSpeed = outerSpeed;
    }

    setMotorSpeed(leftSpeed, rightSpeed);

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

void MotorController::resetPD() {
    _prevError  = 0.0f;
    _lastPDTime = 0;
}

// ============================================================================
// setPinDirection()
// Recebe valor já com trim aplicado — apenas traduz para L298N e PWM.
// ============================================================================
void MotorController::setPinDirection(int in1, int in2, int pwmPin, int speed) {
    int pwm = constrain(abs(speed), 0, 255);
    if (speed == 0) {
        digitalWrite(in1, LOW);  digitalWrite(in2, LOW);  analogWrite(pwmPin, 0);
    } else if (speed > 0) {
        digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  analogWrite(pwmPin, pwm);
    } else {
        digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(pwmPin, pwm);
    }
}

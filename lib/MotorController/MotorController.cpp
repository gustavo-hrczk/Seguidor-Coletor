#include "MotorController.h"

// ============================================================================
// Construtor
// ============================================================================
MotorController::MotorController()
    : _currentLeft(0), _currentRight(0),
      _appliedLeft(0),  _appliedRight(0),
      _prevError(0.0f), _integral(0.0f), _lastPDTime(0) {}

// ============================================================================
// initialize()
// ============================================================================
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
// applyTrimLeft() / applyTrimRight()
// Aplica fator de trim e deadzone individual por motor.
// ============================================================================
int MotorController::applyTrimLeft(int speed) const {
    if (speed == 0) return 0;
    int v = (int)(speed * MOTOR_TRIM_ESQ);
    v = constrain(v, -255, 255);
    if      (v > 0 && v <  PWM_MIN_DEADZONE) v =  PWM_MIN_DEADZONE;
    else if (v < 0 && v > -PWM_MIN_DEADZONE) v = -PWM_MIN_DEADZONE;
    return v;
}

int MotorController::applyTrimRight(int speed) const {
    if (speed == 0) return 0;
    int v = (int)(speed * MOTOR_TRIM_DIR);
    v = constrain(v, -255, 255);
    if      (v > 0 && v <  PWM_MIN_DEADZONE) v =  PWM_MIN_DEADZONE;
    else if (v < 0 && v > -PWM_MIN_DEADZONE) v = -PWM_MIN_DEADZONE;
    return v;
}

// ============================================================================
// _writePins()
// Envia velocidade diretamente ao hardware — sem rampa, sem trim.
// Usado apenas internamente pela rampa em passos intermediários.
// ============================================================================
void MotorController::_writePins(int in1, int in2, int pwmPin, int speed) {
    int pwm = constrain(abs(speed), 0, 255);
    if (speed == 0) {
        digitalWrite(in1, LOW);  digitalWrite(in2, LOW);  analogWrite(pwmPin, 0);
    } else if (speed > 0) {
        digitalWrite(in1, HIGH); digitalWrite(in2, LOW);  analogWrite(pwmPin, pwm);
    } else {
        digitalWrite(in1, LOW);  digitalWrite(in2, HIGH); analogWrite(pwmPin, pwm);
    }
}

// ============================================================================
// _applyRamp()
// Aplica rampa de aceleração ou inversão para um único motor.
//
// Cenário 1 — ARRANQUE (applied == 0, target != 0):
//   Sobe de 0 até target em MOTOR_RAMP_STEPS degraus.
//   Tempo total: MOTOR_RAMP_UP_MS ms.
//
// Cenário 2 — INVERSÃO (applied e target têm sinais opostos):
//   Desce de applied até 0 em MOTOR_RAMP_STEPS degraus (MOTOR_RAMP_DOWN_MS ms),
//   envia 0 por um ciclo de escrita, depois sobe para target normalmente.
//   O motor nunca inverte polaridade sem passar por zero — elimina pico de stall.
//
// Cenário 3 — MESMA DIREÇÃO (ajuste de velocidade sem inversão):
//   Envia target diretamente — sem rampa.
//   O PID faz muitos desses ajustes a cada 10ms; rampar cada um
//   introduziria latência que comprometeria o seguimento.
//
// Cenário 4 — PARADA (target == 0):
//   Envia 0 diretamente — frenagem imediata é segura pois não inverte polaridade.
// ============================================================================
void MotorController::_applyRamp(int pin1, int pin2, int pwmPin,
                                  int target, int& applied) {
    // Cenário 4: parada — direto
    if (target == 0) {
        _writePins(pin1, pin2, pwmPin, 0);
        applied = 0;
        return;
    }

    bool arranque  = (applied == 0 && target != 0);
    bool inversao  = (applied != 0) &&
                     ((applied > 0 && target < 0) || (applied < 0 && target > 0));

    // Cenário 2: inversão — desce até zero primeiro
    if (inversao) {
        int stepDelay = MOTOR_RAMP_DOWN_MS / MOTOR_RAMP_STEPS;
        int stepSize  = applied / MOTOR_RAMP_STEPS;   // sempre mesmo sinal de applied

        for (int i = MOTOR_RAMP_STEPS - 1; i >= 0; i--) {
            int intermediate = stepSize * i;
            // Mantém dentro da deadzone ou zero — nunca meia velocidade inútil
            if (intermediate != 0 && abs(intermediate) < PWM_MIN_DEADZONE) {
                intermediate = 0;
            }
            _writePins(pin1, pin2, pwmPin, intermediate);
            delay(stepDelay);
        }
        // Garante zero por um ciclo antes de inverter
        _writePins(pin1, pin2, pwmPin, 0);
        applied = 0;
        delay(2);
    }

    // Cenário 1: arranque — sobe progressivamente
    if (arranque || inversao) {
        int stepDelay = MOTOR_RAMP_UP_MS / MOTOR_RAMP_STEPS;
        int stepSize  = target / MOTOR_RAMP_STEPS;

        for (int i = 1; i <= MOTOR_RAMP_STEPS; i++) {
            int intermediate = stepSize * i;
            // Primeiro degrau já deve vencer a deadzone
            if (abs(intermediate) < PWM_MIN_DEADZONE) {
                intermediate = (target > 0) ? PWM_MIN_DEADZONE : -PWM_MIN_DEADZONE;
            }
            _writePins(pin1, pin2, pwmPin, intermediate);
            delay(stepDelay);
        }
    }

    // Cenário 3: mesma direção — ajuste direto (sem rampa — PID depende disso)
    _writePins(pin1, pin2, pwmPin, target);
    applied = target;
}

// ============================================================================
// setMotorSpeed()
// Ponto central — todos os métodos de movimento passam por aqui.
// 1. Armazena velocidades lógicas solicitadas
// 2. Aplica trim individual
// 3. Envia para _applyRamp() que protege o hardware
// ============================================================================
void MotorController::setMotorSpeed(int speedLeft, int speedRight) {
    _currentLeft  = speedLeft;
    _currentRight = speedRight;

    int trimLeft  = applyTrimLeft(speedLeft);
    int trimRight = applyTrimRight(speedRight);

    _applyRamp(PIN_IN3, PIN_IN4, PIN_ENA, trimLeft,  _appliedLeft);
    _applyRamp(PIN_IN1, PIN_IN2, PIN_ENB, trimRight, _appliedRight);
}

// ============================================================================
// move()
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
        default:          stop();                         break;
    }
}

// ============================================================================
// stop()
// ============================================================================
void MotorController::stop() {
    // Parada direta — sem rampa, não inverte polaridade
    _writePins(PIN_IN3, PIN_IN4, PIN_ENA, 0);
    _writePins(PIN_IN1, PIN_IN2, PIN_ENB, 0);
    _currentLeft = _currentRight = 0;
    _appliedLeft = _appliedRight = 0;
}

// ============================================================================
// followLine() — Controlador PID
// ============================================================================
void MotorController::followLine(float linePosition, uint8_t baseSpeed) {
    unsigned long now = millis();
    if (now - _lastPDTime < PD_SAMPLE_MS) return;

    float dt    = (now - _lastPDTime) / 1000.0f;
    _lastPDTime = now;

    float error      = linePosition;
    float derivative = error - _prevError;
    _prevError       = error;

    if (fabs(error) > PID_INTEGRAL_DEADZONE) {
        _integral += error * dt;
        _integral  = constrain(_integral, -PID_INTEGRAL_MAX, PID_INTEGRAL_MAX);
    } else {
        _integral = 0.0f;
    }

    float correction = (PD_KP * error)
                     + (PD_KD * derivative)
                     + (PD_KI * _integral);

    float corrNorm = constrain(fabs(correction), 0.0f, 1.0f);

    int outerSpeed = (int)baseSpeed;
    int innerSpeed = constrain(
        (int)(baseSpeed * (1.0f - corrNorm)),
        PD_MIN_INNER_SPEED,
        (int)baseSpeed
    );

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
            Serial.print(F("[PID] err="));  Serial.print(error, 3);
            Serial.print(F(" int="));       Serial.print(_integral, 3);
            Serial.print(F(" cor="));       Serial.print(correction, 3);
            Serial.print(F(" L="));         Serial.print(leftSpeed);
            Serial.print(F(" R="));         Serial.println(rightSpeed);
        }
    }
}

// ============================================================================
// resetPD()
// ============================================================================
void MotorController::resetPD() {
    _prevError   = 0.0f;
    _integral    = 0.0f;
    _lastPDTime  = 0;
    _appliedLeft = _appliedRight = 0;   // força rampa de arranque na retomada
}

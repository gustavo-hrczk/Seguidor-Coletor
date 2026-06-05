#include "MotorController.h"

// ============================================================================
// Construtor
// ============================================================================
MotorController::MotorController()
    : _currentLeft(0), _currentRight(0),
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
//
// Aplica fator de trim individual antes de enviar ao hardware.
// Perspectiva: robô visto de cima, frente para longe.
//   Left  → motor ESQUERDO → MOTOR_TRIM_ESQ
//   Right → motor DIREITO  → MOTOR_TRIM_DIR
//
// Fluxo:
//   1. Multiplica pelo fator de trim
//   2. Eleva ao PWM_MIN_DEADZONE se abaixo do limiar (preserva sinal)
//   3. Limita ao máximo ±255
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
// setMotorSpeed()
// Ponto central — todos os métodos passam por aqui.
// Armazena velocidades lógicas e aplica trim antes de enviar ao hardware.
// ============================================================================
void MotorController::setMotorSpeed(int speedLeft, int speedRight) {
    _currentLeft  = speedLeft;
    _currentRight = speedRight;
    setPinDirection(PIN_IN3, PIN_IN4, PIN_ENA, applyTrimLeft(speedLeft));
    setPinDirection(PIN_IN1, PIN_IN2, PIN_ENB, applyTrimRight(speedRight));
}

// ============================================================================
// move()
// TURN_*:  giro no eixo — dois motores em sentidos opostos, raio zero.
//          Usado em manobras de coleta e recuperação de linha.
// CURVE_*: arco com um motor parado — raio maior, menor consumo de corrente.
//          Disponível para ajustes suaves ou uso futuro.
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
void MotorController::stop() { setMotorSpeed(0, 0); }

// ============================================================================
// followLine() — Controlador PID de seguimento
//
// Fórmula: correction = Kp*erro + Kd*derivativo + Ki*integral
//
// Motor externo à curva: baseSpeed constante — preserva velocidade de avanço.
// Motor interno à curva: baseSpeed × (1 - |correction|), piso PD_MIN_INNER_SPEED.
//
// Termo integral — anti-deriva:
//   Acumula error × dt a cada ciclo. Quando o robô fica consistentemente
//   desviado, o acumulador cresce e a correção Ki×integral empurra
//   progressivamente para o centro.
//
// Anti-windup:
//   Zona morta (PID_INTEGRAL_DEADZONE): dentro de ±deadzone, zera o integral.
//     Ruído em reta não gera acúmulo desnecessário.
//   Clamp (PID_INTEGRAL_MAX): limita o valor absoluto do acumulador.
//     Previne overshooting após curvas longas.
//
// Temporização com dt real:
//   Ki independente da frequência do loop — usa segundos, não iterações.
// ============================================================================
void MotorController::followLine(float linePosition, uint8_t baseSpeed) {
    unsigned long now = millis();
    if (now - _lastPDTime < PD_SAMPLE_MS) return;

    float dt     = (now - _lastPDTime) / 1000.0f;
    _lastPDTime  = now;

    float error      = linePosition;
    float derivative = error - _prevError;
    _prevError       = error;

    if (fabs(error) > PID_INTEGRAL_DEADZONE) {
        _integral += error * dt;
        _integral  = constrain(_integral, -PID_INTEGRAL_MAX, PID_INTEGRAL_MAX);
    } else {
        _integral = 0.0f;   // centrado — descarta acúmulo
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

    // correction > 0 → linha à direita → esquerdo é externo
    // correction < 0 → linha à esquerda → direito é externo
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
// Zera os três estados internos do controlador.
// Sem este reset, o acúmulo da manobra anterior causa um spike de correção
// imediatamente ao retomar o seguimento ("chicote" na retomada).
// ============================================================================
void MotorController::resetPD() {
    _prevError  = 0.0f;
    _integral   = 0.0f;
    _lastPDTime = 0;
}

// ============================================================================
// setPinDirection()
// Traduz velocidade com sinal para L298N e PWM.
// Recebe valor já com trim aplicado.
//   speed > 0: IN_A=HIGH IN_B=LOW  → frente
//   speed < 0: IN_A=LOW  IN_B=HIGH → ré
//   speed = 0: IN_A=LOW  IN_B=LOW  → freio (curto-circuito no motor)
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

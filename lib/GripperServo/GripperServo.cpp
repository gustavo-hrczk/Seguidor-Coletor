#include "GripperServo.h"

// Alias local — evita repetir SERVO_STABILIZATION_TIME no código
static const uint16_t DETACH_DELAY_MS = SERVO_STABILIZATION_TIME;

// ============================================================================
// Construtor
// Estado inicial OPEN com ângulo registrado — servo não é movido aqui.
// ============================================================================
GripperServo::GripperServo()
    : _state(OPEN), _currentAngle(SERVO_ANGLE_OPEN) {}

// ============================================================================
// initialize()
// Conecta o servo, posiciona em ABERTO, aguarda estabilização e desliga PWM.
// ============================================================================
void GripperServo::initialize() {
    _servo.attach(PIN_SERVO);
    _servo.write(SERVO_ANGLE_OPEN);
    _currentAngle = SERVO_ANGLE_OPEN;
    _state        = OPEN;
    delay(DETACH_DELAY_MS);
    _servo.detach();
    if (DEBUG_MODE) Serial.println(F("[Gripper] Inicializado - ABERTO"));
}

// ============================================================================
// open() / close()
// Guards de estado evitam movimento desnecessário quando já na posição.
// ============================================================================
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

// ============================================================================
// emergencyStop()
// ============================================================================
void GripperServo::emergencyStop() {
    if (_servo.attached()) _servo.detach();
    _state = ERROR;
    if (DEBUG_MODE) Serial.println(F("[Gripper] EMERGENCY STOP"));
}

// ============================================================================
// moveToAngle()
// Move o servo grau a grau até o ângulo alvo.
//
// Por que grau a grau:
//   Movimento brusco causa tranco mecânico e pode soltar o objeto.
//   O passo unitário distribui a carga ao longo do tempo.
//
// Por que reativar attach antes de mover:
//   detach() desliga o PWM após cada movimento para eliminar aquecimento.
//   Para mover novamente é necessário reativar o sinal. O write() inicial
//   informa ao driver a posição atual antes de iniciar o novo movimento.
//
// Correção de underflow:
//   _currentAngle é uint8_t. Sem a variável local int, decrementar abaixo
//   de 0 causaria underflow para 255, prendendo o while em loop infinito.
//   A iteração usa int local — _currentAngle só é atualizado ao final.
// ============================================================================
void GripperServo::moveToAngle(uint8_t target) {
    target = constrain(target, SERVO_ANGLE_OPEN, SERVO_ANGLE_CLOSED);

    if (!_servo.attached()) {
        _servo.attach(PIN_SERVO);
        _servo.write(_currentAngle);
        delay(50);
    }

    // Iteração com int local — previne underflow de uint8_t ao decrementar
    int current = (int)_currentAngle;
    int tgt     = (int)target;
    int step    = (current < tgt) ? 1 : -1;

    while (current != tgt) {
        current += step;
        _servo.write(current);
        delay(SERVO_STEP_DELAY_MS);
    }

    _currentAngle = (uint8_t)current;   // sincroniza ângulo registrado
    delay(DETACH_DELAY_MS);
    _servo.detach();
}

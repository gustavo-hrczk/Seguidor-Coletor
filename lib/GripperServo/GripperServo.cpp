#include "GripperServo.h"

// Alias local para o tempo de estabilização definido no config.h.
// Usado após o servo atingir o ângulo alvo antes de desligar o PWM.
static const uint16_t DETACH_DELAY_MS = SERVO_STABILIZATION_TIME;

// ============================================================================
// Construtor
// Inicializa o estado lógico como OPEN e registra o ângulo inicial.
// O servo físico NÃO é movimentado aqui — isso ocorre em initialize().
// ============================================================================
GripperServo::GripperServo()
    : _state(OPEN), _currentAngle(SERVO_ANGLE_OPEN) {}

// ============================================================================
// initialize()
// Conecta o servo, posiciona em OPEN, aguarda estabilização mecânica
// e desliga o sinal PWM. Deve ser chamado uma vez no setup().
// ============================================================================
void GripperServo::initialize() {
    _servo.attach(PIN_SERVO);
    _servo.write(SERVO_ANGLE_OPEN);     // posiciona fisicamente em OPEN
    _currentAngle = SERVO_ANGLE_OPEN;
    _state        = OPEN;
    delay(DETACH_DELAY_MS);             // aguarda o servo assentar na posição
    _servo.detach();                    // desliga PWM — evita vibração em repouso
    if (DEBUG_MODE) Serial.println(F("[Gripper] Inicializado - ABERTO"));
}

// ============================================================================
// open()
// Abre a garra movendo para SERVO_ANGLE_OPEN.
// Ignorado silenciosamente se a garra já estiver aberta (evita movimento desnecessário).
// ============================================================================
void GripperServo::open() {
    if (_state == OPEN) return;         // guarda de estado — não reexecuta
    moveToAngle(SERVO_ANGLE_OPEN);
    _state = OPEN;
    if (DEBUG_MODE) Serial.println(F("[Gripper] ABERTO"));
}

// ============================================================================
// close()
// Fecha a garra movendo para SERVO_ANGLE_CLOSED.
// Ignorado silenciosamente se a garra já estiver fechada.
// ============================================================================
void GripperServo::close() {
    if (_state == CLOSED) return;       // guarda de estado — não reexecuta
    moveToAngle(SERVO_ANGLE_CLOSED);
    _state = CLOSED;
    if (DEBUG_MODE) Serial.println(F("[Gripper] FECHADO"));
}

// ============================================================================
// emergencyStop()
// Desliga o PWM imediatamente sem aguardar posição final.
// Coloca o estado em ERROR — open() e close() não funcionarão após isso.
// Use apenas em situações de falha crítica.
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
// Por que grau a grau?
//   Mover diretamente para o ângulo final causa tranco mecânico e pode
//   soltar ou danificar o objeto na garra. O movimento gradual também
//   distribui a carga no motor ao longo do tempo.
//
// Por que reativar o attach antes de mover?
//   O detach() desliga o PWM após cada movimento. Para mover novamente,
//   é necessário reativar o sinal — o write(_currentAngle) informa ao
//   driver a posição atual antes de iniciar o novo movimento.
// ============================================================================
void GripperServo::moveToAngle(uint8_t target) {
    // Garante que o alvo está dentro dos limites físicos definidos no config
    target = constrain(target, SERVO_ANGLE_OPEN, SERVO_ANGLE_CLOSED);

    // Reativa PWM se estava desligado (estado normal entre comandos)
    if (!_servo.attached()) {
        _servo.attach(PIN_SERVO);
        _servo.write(_currentAngle);    // informa posição atual ao driver
        delay(50);                      // aguarda o driver sincronizar
    }

    // Define direção do movimento: +1 para aumentar ângulo, -1 para diminuir
    const int8_t step = (_currentAngle < target) ? 1 : -1;

    // Move um grau por vez, aguardando SERVO_STEP_DELAY_MS entre cada passo
    while (_currentAngle != target) {
        _currentAngle += step;
        _servo.write(_currentAngle);
        delay(SERVO_STEP_DELAY_MS);     // controla velocidade do movimento
    }

    delay(DETACH_DELAY_MS);             // aguarda assentar na posição final
    _servo.detach();                    // desliga PWM — elimina aquecimento em repouso
}

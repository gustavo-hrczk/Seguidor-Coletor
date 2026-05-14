#ifndef GRIPPER_SERVO_H
#define GRIPPER_SERVO_H

#include <Arduino.h>
#include <Servo.h>
#include "config.h"

// ============================================================================
// GripperServo — controle da garra via servo SG90
//
// Responsabilidade: abrir e fechar a garra com movimento suave (grau a grau),
// evitando trancos mecânicos e aquecimento do motor entre comandos.
//
// Decisão de design: após atingir o ângulo alvo, o sinal PWM é desligado via
// detach(). Isso elimina a vibração contínua e o aquecimento que ocorrem
// quando o servo recebe sinal constante sem carga — problema comum no SG90.
//
// Uso típico:
//   GripperServo gripper;
//   gripper.initialize();   // posiciona em OPEN e desliga PWM
//   gripper.close();        // move grau a grau até SERVO_ANGLE_CLOSED
//   gripper.open();         // move grau a grau até SERVO_ANGLE_OPEN
// ============================================================================

class GripperServo {
public:

    // Estados possíveis da garra
    enum State {
        OPEN,    // Garra aberta (ângulo = SERVO_ANGLE_OPEN)
        CLOSED,  // Garra fechada (ângulo = SERVO_ANGLE_CLOSED)
        ERROR    // Servo desligado por emergência
    };

    // Construtor — inicializa estado como OPEN sem movimentar o servo
    GripperServo();

    // Inicializa o servo: posiciona em OPEN, aguarda estabilização e desliga PWM
    void initialize();

    // Move para ângulo aberto — ignorado se já estiver aberta
    void open();

    // Move para ângulo fechado — ignorado se já estiver fechada
    void close();

    // Para o servo imediatamente desligando o PWM — use apenas em emergências
    void emergencyStop();

    // Getters de estado
    State getState() const { return _state; }
    bool  isOpen()   const { return _state == OPEN;   }
    bool  isClosed() const { return _state == CLOSED; }

private:
    Servo   _servo;          // Objeto da biblioteca Servo
    State   _state;          // Estado lógico atual da garra
    uint8_t _currentAngle;   // Ângulo físico atual (rastreia posição real do servo)

    // Move o servo grau a grau até o ângulo alvo.
    // Privado — chamado apenas por open() e close().
    void moveToAngle(uint8_t target);
};

#endif

#ifndef GRIPPER_SERVO_H
#define GRIPPER_SERVO_H

#include <Arduino.h>
#include <Servo.h>
#include "config.h"

// ============================================================================
// CLASSE: GripperServo
// Responsável pelo controle do servo da garra com timeout de proteção
// e validação de estabilização.
// ============================================================================

class GripperServo {
public:
    // Estados da garra
    enum GripperState {
        OPEN = 0,
        OPENING = 1,
        CLOSED = 2,
        CLOSING = 3,
        ERROR = 4
    };

    // ===== CONSTRUTOR E INICIALIZAÇÃO =====
    GripperServo();
    
    /**
     * Inicializa servo no pino definido em config.h
     */
    void initialize();

    // ===== OPERAÇÕES DA GARRA =====
    
    /**
     * Fecha a garra com timeout de proteção
     * @return true se fechou com sucesso, false se timeout
     */
    bool close();

    /**
     * Abre a garra
     */
    void open();

    /**
     * Para o servo na posição atual (segurança)
     */
    void emergency_stop();

    // ===== GETTERS E ESTADO =====
    GripperState getState() const { return currentState; }
    bool isReady() const { return currentState == OPEN || currentState == CLOSED; }
    bool isClosed() const { return currentState == CLOSED; }
    bool isOpen() const { return currentState == OPEN; }
    unsigned long getLastCommandTime() const { return lastCommandTime; }

    // ===== DEBUG =====
    void printState() const;

private:
    Servo servo;
    GripperState currentState;
    unsigned long lastCommandTime;
    unsigned long stateChangeTime;
    uint8_t currentAngle;

    // Métodos privados
    void setAngle(uint8_t angle);
    void updateState();
};

#endif // GRIPPER_SERVO_H

#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// CLASSE: MotorController
// Responsável pelo controle de motores com escalação de velocidade baseada
// em VELOCITY_GLOBAL. Implementa compensação de curva e deadzone automático.
// ============================================================================

class MotorController {
public:
    // Direções de movimento
    enum Direction {
        STOP = 0,
        FORWARD = 1,
        BACKWARD = 2,
        TURN_LEFT = 3,
        TURN_RIGHT = 4
    };

    // ===== CONSTRUTOR E INICIALIZAÇÃO =====
    MotorController();
    
    /**
     * Inicializa pinos e configura PWM
     */
    void initialize();

    // ===== CONTROLE DE MOVIMENTO =====
    
    /**
     * Define ambos os motores com controle de velocidade
     * @param speedLeft Velocidade esquerda (-255 a +255)
     * @param speedRight Velocidade direita (-255 a +255)
     */
    void setMotorSpeed(int speedLeft, int speedRight);

    /**
     * Movimento predefinido com compensação automática
     * @param direction Direção desejada (FORWARD, BACKWARD, etc)
     * @param speed Velocidade em escala normalizada (0-255)
     */
    void move(Direction direction, uint8_t speed = VELOCITY_GLOBAL);

    /**
     * Parada imediata dos motores
     */
    void stop();

    /**
     * Curva compensada mantendo raio constante
     * @param direction TURN_LEFT ou TURN_RIGHT
     * @param speed Velocidade base
     * @param compensationFactor Fator de compensação (ex: 0.8 = 80% de diferença)
     */
    void curveCompensated(Direction direction, uint8_t speed, float compensationFactor);

    // ===== GETTERS =====
    int getLeftSpeed() const { return currentLeftSpeed; }
    int getRightSpeed() const { return currentRightSpeed; }
    bool isMoving() const { return (currentLeftSpeed != 0 || currentRightSpeed != 0); }

private:
    // Estado interno
    int currentLeftSpeed;
    int currentRightSpeed;

    // Métodos privados
    void applyDeadzoneCorrection(int& pwmValue) const;
    void setPinDirection(int in1, int in2, int pwmPin, int speed);
};

#endif // MOTOR_CONTROLLER_H

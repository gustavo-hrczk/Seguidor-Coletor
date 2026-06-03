#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// MotorController — controle dos motores DC via driver L298N
//
// Compensação de assimetria:
//   MOTOR_TRIM_ESQ e MOTOR_TRIM_DIR (config.h) aplicam um fator
//   multiplicativo individual por motor em setMotorSpeed().
//   Isso garante que BASE_SPEED nos dois motores produza
//   velocidades físicas iguais, corrigindo desvio de linha reta.
//
// Fonte única de velocidade:
//   Todos os métodos recebem valores derivados de BASE_SPEED (config.h).
//   Alterar BASE_SPEED recalibra o sistema inteiro proporcionalmente.
// ============================================================================

class MotorController {
public:

    enum Direction {
        STOP        = 0,
        FORWARD     = 1,
        BACKWARD    = 2,
        TURN_LEFT   = 3,   // giro no eixo — manobras e recuperação
        TURN_RIGHT  = 4,   // giro no eixo — manobras e recuperação
        CURVE_LEFT  = 5,   // arco suave — motor direito ativo, esquerdo parado
        CURVE_RIGHT = 6    // arco suave — motor esquerdo ativo, direito parado
    };

    MotorController();
    void initialize();

    // Controle direto por motor (-255..+255).
    // Aplica MOTOR_TRIM_* antes de enviar ao hardware.
    void setMotorSpeed(int speedRight, int speedLeft);

    // Movimentos predefinidos com velocidade uniforme
    void move(Direction direction, uint8_t speed = VELOCITY_GLOBAL);

    void stop();

    // Controlador PD de seguimento de linha.
    // Chame a cada ciclo do loop durante STATE_FOLLOWING.
    void followLine(float linePosition, uint8_t baseSpeed = VELOCITY_GLOBAL);

    // Zera estado PD — obrigatório ao retomar seguimento após qualquer parada
    void resetPD();

    int  getLeftSpeed()  const { return _currentLeft;  }
    int  getRightSpeed() const { return _currentRight; }
    bool isMoving()      const { return (_currentLeft != 0 || _currentRight != 0); }

private:
    int           _currentLeft;
    int           _currentRight;
    float         _prevError;
    unsigned long _lastPDTime;

    // Aplica MOTOR_TRIM e garante deadzone antes de enviar ao hardware
    int  applyTrimRight(int speed)  const;
    int  applyTrimLeft(int speed) const;

    void setPinDirection(int in1, int in2, int pwmPin, int speed);
};

#endif

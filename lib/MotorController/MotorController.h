#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

class MotorController {
public:
    enum Direction {
        STOP = 0, FORWARD = 1, BACKWARD = 2,
        TURN_LEFT = 3, TURN_RIGHT = 4
    };

    MotorController();
    void initialize();

    // Controle direto de velocidade por motor (-255..+255)
    void setMotorSpeed(int speedLeft, int speedRight);

    // Movimento predefinido
    void move(Direction direction, uint8_t speed = VELOCITY_GLOBAL);

    // Parada imediata
    void stop();

    // Curva com fator de compensação explícito
    void curveCompensated(Direction direction, uint8_t speed,
                          float compensationFactor);

    // ── SEGUIMENTO DE LINHA ──────────────────────────────────────────
    // Recebe posição normalizada (-1.0..+1.0) e aplica correção PD.
    // Deve ser chamado a cada PD_SAMPLE_MS ms no loop principal.
    // @param linePosition  saída de LineSensor::getLinePosition()
    // @param baseSpeed     velocidade base (use SPEED_ERROR_* conforme padrão)
    void followLine(float linePosition, uint8_t baseSpeed = VELOCITY_GLOBAL);

    // Reseta o estado interno do PD (chamar ao retomar seguimento)
    void resetPD();

    int  getLeftSpeed()  const { return _currentLeft;  }
    int  getRightSpeed() const { return _currentRight; }
    bool isMoving()      const { return (_currentLeft != 0 || _currentRight != 0); }

private:
    int   _currentLeft;
    int   _currentRight;

    // Estado PD
    float         _prevError;
    unsigned long _lastPDTime;

    void applyDeadzoneCorrection(int& pwmValue) const;
    void setPinDirection(int in1, int in2, int pwmPin, int speed);
};

#endif
#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// MotorController — controle dos motores DC via driver L298N
//
// Trim de assimetria:
//   MOTOR_TRIM_DIR e MOTOR_TRIM_ESQ (config.h §2) aplicam fator
//   multiplicativo individual dentro de setMotorSpeed(), garantindo
//   que BASE_SPEED produza velocidades físicas iguais nos dois lados.
//
// Fonte única de velocidade:
//   Todos os métodos recebem valores derivados de BASE_SPEED (config.h §3).
//   Nunca passar valores arbitrários — usar as constantes derivadas.
//
// Controlador PID:
//   followLine() implementa PID completo com termo integral.
//   Estado interno: _prevError, _integral, _lastPDTime.
//   resetPD() zera os três — obrigatório ao retomar seguimento.
//
// Modos de curva:
//   TURN_*:  giro no eixo (motores opostos) — raio zero, preciso
//   CURVE_*: arco com um motor parado — raio maior, menor consumo
// ============================================================================

class MotorController {
public:

    enum Direction {
        STOP        = 0,
        FORWARD     = 1,
        BACKWARD    = 2,
        TURN_LEFT   = 3,
        TURN_RIGHT  = 4,
        CURVE_LEFT  = 5,
        CURVE_RIGHT = 6
    };

    MotorController();

    // Configura pinos L298N como OUTPUT e para os motores
    void initialize();

    // Controle direto por motor (-255..+255).
    // Aplica trim e deadzone antes de enviar ao hardware.
    // Ponto de entrada de todos os outros métodos.
    void setMotorSpeed(int speedLeft, int speedRight);

    // Movimento predefinido com velocidade uniforme nos dois motores
    void move(Direction direction, uint8_t speed = VELOCITY_GLOBAL);

    // Para ambos os motores imediatamente
    void stop();

    // Controlador PID de seguimento de linha.
    // Chamar uma vez por ciclo em STATE_FOLLOWING.
    // @param linePosition  saída de LineSensor::getLinePosition() (-1.0..+1.0)
    // @param baseSpeed     BASE_SPEED ou derivada — nunca valor arbitrário
    void followLine(float linePosition, uint8_t baseSpeed = VELOCITY_GLOBAL);

    // Zera _prevError, _integral e _lastPDTime.
    // Obrigatório ao retomar seguimento após parada, coleta ou recuperação.
    void resetPD();

    int  getLeftSpeed()  const { return _currentLeft;  }
    int  getRightSpeed() const { return _currentRight; }
    bool isMoving()      const { return (_currentLeft != 0 || _currentRight != 0); }

private:
    int           _currentLeft;
    int           _currentRight;

    float         _prevError;    // erro do ciclo anterior — termo derivativo
    float         _integral;     // acumulador de erro — termo integral
    unsigned long _lastPDTime;   // timestamp da última execução do PID

    // Aplica MOTOR_TRIM_* e deadzone por motor
    int  applyTrimLeft(int speed)  const;
    int  applyTrimRight(int speed) const;

    // Traduz velocidade com sinal para pinos L298N e PWM
    void setPinDirection(int in1, int in2, int pwmPin, int speed);
};

#endif

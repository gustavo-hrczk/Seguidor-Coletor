#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// MotorController — controle dos motores DC via driver L298N
//
// Responsabilidade: abstrair o controle de dois motores DC em comandos
// de alto nível (frente, ré, curva) e implementar o controlador PD
// de seguimento de linha.
//
// Convenção de sinal em setMotorSpeed():
//   Motor esquerdo montado invertido fisicamente no chassi:
//     positivo → ré    | negativo → frente
//   Motor direito (montagem padrão):
//     positivo → frente | negativo → ré
//   Essa assimetria é compensada internamente em move() e followLine().
//   NÃO altere os sinais sem revisar todos os métodos que chamam setMotorSpeed().
//
// Controlador PD (followLine()):
//   Motor externo à curva: mantém baseSpeed constante
//   Motor interno à curva: reduzido por (1 - |correção|), mínimo PD_MIN_INNER_SPEED
//   Isso garante que sempre haja torque suficiente para mover o robô,
//   mesmo em curvas fechadas onde a correção se aproxima de 1.0.
// ============================================================================

class MotorController {
public:

    // Direções predefinidas — usadas por move()
    enum Direction {
        STOP      = 0,
        FORWARD   = 1,
        BACKWARD  = 2,
        TURN_LEFT = 3,
        TURN_RIGHT= 4
    };

    // Construtor — inicializa velocidades e estado PD como zero
    MotorController();

    // Configura pinos de direção e PWM como OUTPUT e para os motores
    void initialize();

    // Define velocidade individual de cada motor (-255..+255).
    // Sinal positivo/negativo define direção conforme convenção acima.
    // Ponto de entrada de todos os outros métodos de movimento.
    void setMotorSpeed(int speedLeft, int speedRight);

    // Executa movimento predefinido com velocidade uniforme nos dois motores.
    // @param speed  PWM de 0 a 255 (padrão: VELOCITY_GLOBAL do config.h)
    void move(Direction direction, uint8_t speed = VELOCITY_GLOBAL);

    // Para ambos os motores imediatamente (setMotorSpeed(0, 0))
    void stop();

    // Curva com diferença de velocidade entre motores.
    // Motor externo = speed | Motor interno = speed * compensationFactor
    // @param compensationFactor  0.0–1.0 (use CURVE_COMPENSATION_* do config.h)
    void curveCompensated(Direction direction, uint8_t speed,
                          float compensationFactor);

    // Controlador PD de seguimento de linha.
    // Deve ser chamado a cada PD_SAMPLE_MS ms no loop principal.
    // @param linePosition  saída de LineSensor::getLinePosition() (-1.0..+1.0)
    // @param baseSpeed     velocidade base (use SPEED_ERROR_* conforme padrão)
    void followLine(float linePosition, uint8_t baseSpeed = VELOCITY_GLOBAL);

    // Reseta erro anterior e timestamp do PD.
    // Chamar sempre ao retomar seguimento após parada ou recuperação.
    void resetPD();

    // Getters — leitura da velocidade atual de cada motor
    int  getLeftSpeed()  const { return _currentLeft;  }
    int  getRightSpeed() const { return _currentRight; }

    // true se qualquer motor estiver com velocidade diferente de zero
    bool isMoving() const { return (_currentLeft != 0 || _currentRight != 0); }

private:
    int   _currentLeft;    // Velocidade atual do motor esquerdo (-255..+255)
    int   _currentRight;   // Velocidade atual do motor direito  (-255..+255)

    // Estado interno do controlador PD
    float         _prevError;   // Erro da iteração anterior (para termo derivativo)
    unsigned long _lastPDTime;  // Timestamp da última execução do PD (ms)

    // Eleva valor absoluto ao mínimo PWM_MIN_DEADZONE se abaixo do limiar.
    // Evita que o motor receba sinal insuficiente para vencer o atrito estático.
    void applyDeadzoneCorrection(int& pwmValue) const;

    // Aplica direção e PWM nos pinos do driver L298N para um motor.
    // speed > 0 → frente | speed < 0 → ré | speed = 0 → parado
    void setPinDirection(int in1, int in2, int pwmPin, int speed);
};

#endif

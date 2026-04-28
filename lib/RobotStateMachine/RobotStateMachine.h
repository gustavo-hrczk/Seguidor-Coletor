#ifndef ROBOT_STATE_MACHINE_H
#define ROBOT_STATE_MACHINE_H

#include <Arduino.h>
#include "config.h"
#include "MotorController.h"
#include "LineSensor.h"
#include "UltrasonicSensor.h"
#include "GripperServo.h"

// ============================================================================
// CLASSE: RobotStateMachine
// Máquina de Estados Finita que coordena toda a lógica do robô.
// Estados: NAVIGATE, CURVE, DETECT_OBJECT, APPROACH, COLLECT, RELEASE, RETURN_LINE
// ============================================================================

class RobotStateMachine {
public:
    // Estados principais da máquina
    enum RobotState {
        STATE_IDLE = 0,
        STATE_NAVIGATE = 1,      // Seguindo linha em reta
        STATE_CURVE = 2,         // Fazendo curva
        STATE_OBJECT_DETECTED = 3, // Objeto detectado, preparando coleta
        STATE_APPROACH = 4,      // Aproximando do objeto
        STATE_COLLECT = 5,       // Coletando (garra fechando)
        STATE_ROTATE_90 = 6,     // Girando 90° para descartar
        STATE_RELEASE = 7,       // Descartando objeto
        STATE_RETURN_LINE = 8,   // Voltando à linha
        STATE_LINE_SEARCH = 9,   // Procurando linha perdida
        STATE_EMERGENCY_STOP = 10 // Parada de emergência
    };

    // Direções para algoritmo Round Robin
    enum TurnDirection {
        TURN_NONE = 0,
        TURN_LEFT = 1,
        TURN_RIGHT = 2
    };

    // ===== CONSTRUTOR E INICIALIZAÇÃO =====
    RobotStateMachine(MotorController* motor, LineSensor* line,
                      UltrasonicSensor* ultrasonic, GripperServo* gripper);

    /**
     * Inicializa todos os componentes
     */
    void initialize();

    // ===== LOOP PRINCIPAL =====
    
    /**
     * Executa um ciclo da máquina de estados
     * Deve ser chamado no loop() principal com período CYCLE_MAIN
     */
    void update();

    // ===== GETTERS =====
    RobotState getCurrentState() const { return currentState; }
    TurnDirection getLastTurnDirection() const { return lastTurnDirection; }
    uint8_t getObjectsCollected() const { return objectsCollected; }
    bool isRunning() const { return running; }
    unsigned long getElapsedTime() const { return millis() - startTime; }

    /**
     * Para o robô imediatamente
     */
    void stop();

    /**
     * Reinicia a máquina de estados
     */
    void restart();

    // ===== DEBUG =====
    void printState() const;
    void printStatistics() const;

private:
    // Referências aos componentes
    MotorController* motor;
    LineSensor* lineSensor;
    UltrasonicSensor* ultrasonic;
    GripperServo* gripper;

    // Estado da máquina
    RobotState currentState;
    RobotState previousState;
    TurnDirection lastTurnDirection;
    
    // Contadores e tempos
    unsigned long startTime;
    unsigned long stateChangeTime;
    unsigned long lastObjectTime;
    uint8_t objectsCollected;
    uint8_t lineSearchRotations;
    uint8_t collectionAttempts;
    bool running;

    // Métodos de transição de estado
    void transitionToState(RobotState newState);

    // Métodos para cada estado
    void handleNavigate();      // Seguir linha reta
    void handleCurve();         // Fazer curva compensada
    void handleObjectDetected(); // Preparar para coleta
    void handleApproach();      // Aproximar do objeto
    void handleCollect();       // Coletar objeto
    void handleRotate90();      // Girar 90° para descartar
    void handleRelease();       // Descartar e afastar
    void handleReturnLine();    // Voltar à linha principal
    void handleLineSearch();    // Procurar linha perdida
    void handleEmergencyStop(); // Parada de emergência

    // Validadores de transição
    bool shouldDetectObject();
    bool shouldReturnToNavigate();
    bool shouldSearchLine();

    // Utilitários de algoritmo Round Robin
    TurnDirection getNextTurnDirection();
    void saveTurnDirection();
    void loadTurnDirection();

    // Cálculos com velocidade global
    uint16_t scaleTime(uint16_t timeAtBaseSpeed) const;
    uint8_t scaleSpeed(uint8_t speedAtBaseVelocity) const;
};

#endif // ROBOT_STATE_MACHINE_H

#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// CLASSE: LineSensor
// Responsável pela leitura dos 6 sensores de linha QTR com filtro de debounce.
// Implementa validação em múltiplos ciclos para evitar ruído.
// ============================================================================

class LineSensor {
public:
    // Estados dos sensores (array de 6 booleanos)
    // true = linha detectada, false = background
    struct SensorState {
        bool sensors[6];
        uint8_t rawPattern;  // Padrão binário (6 bits)
        bool isValid;        // Se passou no filtro de debounce
    };

    // Padrões de linha para máquina de estados
    enum LinePattern {
        UNKNOWN = 0,
        STRAIGHT = 1,          // Sensores 2 e 3 ativos (centro)
        CURVE_LIGHT = 2,       // Sensor 3 ativo (curva suave)
        CURVE_MEDIUM = 3,      // Sensor 4 ativo (curva média)
        CURVE_SHARP = 4,       // Sensores extremos
        INTERSECTION = 5,      // Múltiplos sensores (cruzamento)
        LINE_LOST = 6          // Nenhum sensor ativo
    };

    // ===== CONSTRUTOR E INICIALIZAÇÃO =====
    LineSensor();
    
    /**
     * Inicializa pinos dos sensores como INPUT
     */
    void initialize();

    // ===== LEITURA E PROCESSAMENTO =====
    
    /**
     * Lê os 6 sensores e aplica filtro de debounce
     * @return SensorState com leituras validadas
     */
    SensorState readSensors();

    /**
     * Força novo ciclo de filtro (útil após mudanças bruscas)
     */
    void resetFilter();

    // ===== GETTERS =====
    SensorState getLastValidState() const { return lastValidState; }
    LinePattern getLinePattern() const { return identifyPattern(lastValidState); }
    bool isLineDetected() const { return lastValidState.rawPattern != 0; }
    uint8_t getRawPattern() const { return lastValidState.rawPattern; }

    /**
     * Identifica qual sensor (1-6) está mais ativo
     * @return Índice do sensor mais ativo (0-5), ou -1 se nenhum
     */
    int getActiveSensor() const;

    // ===== DEBUG =====
    void printSensorValues() const;

private:
    // Estado interno
    SensorState lastValidState;
    SensorState rawReadings;
    uint8_t filterCounter;
    SensorState previousReading;

    // Métodos privados
    SensorState performRawRead();
    LinePattern identifyPattern(const SensorState& state) const;
    bool validateDebounce(const SensorState& current, const SensorState& previous) const;
};

#endif // LINE_SENSOR_H

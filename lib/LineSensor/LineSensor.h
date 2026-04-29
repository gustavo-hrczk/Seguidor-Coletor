#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// LineSensor — leitura ponderada contínua + classificação de padrão
//
// Posição retornada por getLinePosition():
//   -1.0 = linha na extrema esquerda
//    0.0 = linha centralizada
//   +1.0 = linha na extrema direita
//
// Padrão retornado por getLinePattern():
//   Derivado da magnitude do erro e do número de sensores ativos.
//   Usado pela máquina de estados para decidir velocidade e comportamento.
// ============================================================================

class LineSensor {
public:
    struct SensorState {
        int     raw[6];        // Leituras analógicas brutas (0–1023)
        bool    active[6];     // true = sensor sobre a linha
        uint8_t activeCount;   // Número de sensores ativos
        float   position;      // Posição ponderada normalizada (-1.0 a +1.0)
        bool    isValid;       // Passou no filtro de estabilidade
    };

    enum LinePattern {
        UNKNOWN      = 0,
        STRAIGHT     = 1,   // |erro| < 0.3
        CURVE_LIGHT  = 2,   // |erro| 0.3–0.5
        CURVE_MEDIUM = 3,   // |erro| 0.5–0.7
        CURVE_SHARP  = 4,   // |erro| > 0.7
        TURN_LEFT_90 = 5,   // Cruzamento T — linha à esquerda
        TURN_RIGHT_90= 6,   // Cruzamento T — linha à direita
        INTERSECTION = 7,   // Cruzamento X (5+ sensores)
        LINE_LOST    = 8
    };

    // Última direção conhecida — usada na recuperação
    enum LastDirection { DIR_LEFT = -1, DIR_CENTER = 0, DIR_RIGHT = 1 };

    LineSensor();
    void initialize();

    // Lê sensores, atualiza estado interno e retorna estado atual
    SensorState readSensors();

    // Retorna posição normalizada da linha (-1.0 a +1.0)
    float getLinePosition() const { return _state.position; }

    // Classifica o padrão atual com base em posição e contagem de sensores
    LinePattern getLinePattern() const;

    // Última direção válida antes de perder a linha
    LastDirection getLastDirection() const { return _lastDirection; }

    bool    isLineDetected()    const { return _state.activeCount > 0 && _state.isValid; }
    uint8_t getActiveCount()    const { return _state.activeCount; }
    SensorState getState()      const { return _state; }

    void resetFilter();
    void printSensorValues() const;

private:
    SensorState   _state;
    SensorState   _prevState;
    uint8_t       _stableCounter;
    LastDirection _lastDirection;

    // Pesos dos 6 sensores: simétricos, normalizados para posição -1.0..+1.0
    static const int WEIGHTS[6];

    SensorState   performRead();
    float         calculatePosition(const SensorState& s) const;
    bool          isStable(const SensorState& a, const SensorState& b) const;
};

#endif
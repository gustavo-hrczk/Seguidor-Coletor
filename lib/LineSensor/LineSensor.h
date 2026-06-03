#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// LineSensor — leitura analógica + posição ponderada contínua
//
// Convenção (QTR analógico, linha BRANCA sobre fundo PRETO):
//   Linha branca → analogRead BAIXO  (≤ THRESHOLD_LINE_SENSOR)
//   Fundo preto  → analogRead ALTO   (> THRESHOLD_LINE_SENSOR)
//   Sensor ATIVO quando raw <= THRESHOLD_LINE_SENSOR
//
// Posição retornada por getLinePosition():
//   -1.0 = linha na extrema esquerda (S1 ativo)
//    0.0 = linha centralizada (S3+S4 com igual intensidade)
//   +1.0 = linha na extrema direita  (S6 ativo)
//
// Padrões classificados para percurso em formato de 8:
//   STRAIGHT    |pos| < 0.25
//   CURVE_LIGHT |pos| 0.25–0.50
//   CURVE_MEDIUM|pos| 0.50–0.75
//   CURVE_SHARP |pos| > 0.75
//   INTERSECTION 5-6 sensores — cruzamento central do 8
// ============================================================================

class LineSensor {
public:

    struct SensorState {
        int     raw[6];
        bool    active[6];
        uint8_t activeCount;
        float   position;
    };

    enum LinePattern {
        LINE_LOST    = 0,
        STRAIGHT     = 1,
        CURVE_LIGHT  = 2,
        CURVE_MEDIUM = 3,
        CURVE_SHARP  = 4,
        INTERSECTION = 5
    };

    enum LastDirection {
        DIR_LEFT   = -1,
        DIR_CENTER =  0,
        DIR_RIGHT  =  1
    };

    LineSensor();
    void initialize();

    const SensorState& readSensors();

    float              getLinePosition()  const { return _state.position;        }
    LinePattern        getLinePattern()   const;
    LastDirection      getLastDirection() const { return _lastDir;               }
    uint8_t            getActiveCount()   const { return _state.activeCount;     }
    bool               isLineDetected()   const { return _state.activeCount > 0; }
    const SensorState& getState()         const { return _state;                 }

    void printSensorValues()  const;
    void resetLastDirection()       { _lastDir = DIR_CENTER; }

private:
    SensorState   _state;
    LastDirection _lastDir;
    static const int WEIGHTS[6];
    float computePosition() const;
};

#endif
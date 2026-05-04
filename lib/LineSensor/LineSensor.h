#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// LineSensor — leitura direta + posição ponderada contínua
//
// CONVENÇÃO CONFIRMADA pelo código original funcional:
//   Linha branca → analogRead BAIXO  (≤ THRESHOLD)
//   Fundo preto  → analogRead ALTO   (> THRESHOLD)
//   Sensor ATIVO (sobre linha) quando raw <= THRESHOLD
//
// getLinePosition():  -1.0 = extrema esquerda | 0.0 = centro | +1.0 = extrema direita
// getLinePattern():   classifica comportamento esperado do robô
// ============================================================================

class LineSensor {
public:

    struct SensorState {
        int     raw[6];         // Leituras brutas 0–1023
        bool    active[6];      // true = sobre a linha branca
        uint8_t activeCount;    // Quantos sensores ativos
        float   position;       // Posição ponderada -1.0..+1.0
    };

    enum LinePattern {
        LINE_LOST     = 0,   // Nenhum sensor ativo
        STRAIGHT      = 1,   // |pos| < 0.20  — dois centrais
        CURVE_LIGHT   = 2,   // |pos| 0.20–0.45
        CURVE_MEDIUM  = 3,   // |pos| 0.45–0.70
        CURVE_SHARP   = 4,   // |pos| > 0.70
        INTERSECTION  = 5,   // 5–6 sensores ativos simultaneamente
        TURN_LEFT_90  = 6,   // 4 sensores, posição negativa (cruzamento T esq)
        TURN_RIGHT_90 = 7    // 4 sensores, posição positiva (cruzamento T dir)
    };

    enum LastDirection {
        DIR_LEFT   = -1,
        DIR_CENTER =  0,
        DIR_RIGHT  =  1
    };

    LineSensor();
    void initialize();

    // Lê todos os sensores e atualiza estado interno — chame a cada ciclo
    const SensorState& readSensors();

    float         getLinePosition()  const { return _state.position;     }
    LinePattern   getLinePattern()   const;
    LastDirection getLastDirection() const { return _lastDir;            }
    uint8_t       getActiveCount()   const { return _state.activeCount;  }
    bool          isLineDetected()   const { return _state.activeCount > 0; }
    const SensorState& getState()    const { return _state;              }

    void printSensorValues() const;
    void resetLastDirection() { _lastDir = DIR_CENTER; }

private:
    SensorState   _state;
    LastDirection _lastDir;

    // Pesos simétricos por posição física do sensor
    // S1(esq extremo)=-5  S2=-3  S3=-1  S4=+1  S5=+3  S6(dir extremo)=+5
    static const int WEIGHTS[6];

    float computePosition() const;
};

#endif
#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// LineSensor — leitura analógica + posição ponderada contínua
//
// Responsabilidade: ler os 6 sensores QTR, calcular onde a linha está
// em relação ao centro do robô e classificar o padrão de navegação.
//
// Convenção de leitura (QTR analógico, linha branca sobre fundo preto):
//   Linha branca → analogRead BAIXO  (≤ THRESHOLD_LINE_SENSOR)
//   Fundo preto  → analogRead ALTO   (> THRESHOLD_LINE_SENSOR)
//   Sensor ATIVO quando raw <= THRESHOLD_LINE_SENSOR
//
// Por que sem filtro de debounce?
//   O seguimento de linha exige reatividade máxima — qualquer atraso
//   introduzido por filtragem causa oscilação ou perda de linha em curvas.
//   Filtragem por janela deslizante é responsabilidade do UltrasonicSensor,
//   que opera em escala de tempo muito diferente (centenas de ms).
//
// Posição retornada por getLinePosition():
//   -1.0 = linha na extrema esquerda (S1 ativo)
//    0.0 = linha centralizada        (S3+S4 ativos com igual intensidade)
//   +1.0 = linha na extrema direita  (S6 ativo)
// ============================================================================

class LineSensor {
public:

    // Estado completo de uma leitura — retornado por referência para evitar cópia
    struct SensorState {
        int     raw[6];         // Leituras brutas de cada sensor (0–1023)
        bool    active[6];      // true = sensor detectou linha branca
        uint8_t activeCount;    // Número total de sensores ativos
        float   position;       // Posição ponderada normalizada (-1.0..+1.0)
    };

    // Padrão de linha — derivado da posição e da contagem de sensores ativos
    enum LinePattern {
        LINE_LOST     = 0,   // Nenhum sensor ativo — linha fora do alcance
        STRAIGHT      = 1,   // |pos| < 0.20 — robô centralizado na linha
        CURVE_LIGHT   = 2,   // |pos| 0.20–0.45 — curva suave
        CURVE_MEDIUM  = 3,   // |pos| 0.45–0.70 — curva média
        CURVE_SHARP   = 4,   // |pos| > 0.70 — curva fechada
        INTERSECTION  = 5,   // 5–6 sensores ativos — cruzamento em X
        TURN_LEFT_90  = 6,   // 4 sensores ativos, posição negativa — cruzamento T esq
        TURN_RIGHT_90 = 7    // 4 sensores ativos, posição positiva — cruzamento T dir
    };

    // Última direção conhecida antes de perder a linha — usada na recuperação
    enum LastDirection {
        DIR_LEFT   = -1,
        DIR_CENTER =  0,
        DIR_RIGHT  =  1
    };

    // Construtor — inicializa estado zerado e direção central
    LineSensor();

    // Configura pinos A0–A5 como INPUT
    void initialize();

    // Lê todos os sensores, calcula posição e atualiza estado interno.
    // Retorna referência constante ao estado — chame a cada ciclo do loop.
    const SensorState& readSensors();

    // Getters — acesso ao estado sem disparar nova leitura
    float              getLinePosition()  const { return _state.position;        }
    LinePattern        getLinePattern()   const;
    LastDirection      getLastDirection() const { return _lastDir;               }
    uint8_t            getActiveCount()   const { return _state.activeCount;     }
    bool               isLineDetected()   const { return _state.activeCount > 0; }
    const SensorState& getState()         const { return _state;                 }

    // Imprime estado atual no Serial (apenas se DEBUG_MODE = true)
    void printSensorValues() const;

    // Reseta a última direção conhecida para CENTER — use ao retomar seguimento
    void resetLastDirection() { _lastDir = DIR_CENTER; }

private:
    SensorState   _state;    // Estado da leitura mais recente
    LastDirection _lastDir;  // Última direção registrada com sensores ativos

    // Pesos físicos dos sensores, de esquerda para direita:
    // S1=-5  S2=-3  S3=-1  S4=+1  S5=+3  S6=+5
    // Simétricos ao centro — garantem que posição=0 quando S3 e S4 têm igual intensidade
    static const int WEIGHTS[6];

    // Calcula a posição ponderada com base no estado atual
    float computePosition() const;
};

#endif

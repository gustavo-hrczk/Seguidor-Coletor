#include "LineSensor.h"

const int LineSensor::WEIGHTS[6] = { -5, -3, -1, 1, 3, 5 };

LineSensor::LineSensor() : _lastDir(DIR_CENTER) {
    memset(&_state, 0, sizeof(SensorState));
}

void LineSensor::initialize() {
    for (int i = 0; i < 6; i++) {
        pinMode(A0 + i, INPUT);
    }
}

const LineSensor::SensorState& LineSensor::readSensors() {
    _state.activeCount = 0;

    for (int i = 0; i < 6; i++) {
        _state.raw[i]    = analogRead(A0 + i);
        _state.active[i] = (_state.raw[i] <= THRESHOLD_LINE_SENSOR);
        if (_state.active[i]) _state.activeCount++;
    }

    _state.position = computePosition();

    if (_state.activeCount > 0) {
        if      (_state.position < -0.10f) _lastDir = DIR_LEFT;
        else if (_state.position >  0.10f) _lastDir = DIR_RIGHT;
        else                               _lastDir  = DIR_CENTER;
    }

    return _state;
}

float LineSensor::computePosition() const {
    if (_state.activeCount == 0) {
        if (_lastDir == DIR_RIGHT) return  1.0f;
        if (_lastDir == DIR_LEFT)  return -1.0f;
        return 0.0f;
    }

    long weightedSum = 0;
    int  totalWeight = 0;

    for (int i = 0; i < 6; i++) {
        if (_state.active[i]) {
            int intensity = THRESHOLD_LINE_SENSOR - _state.raw[i];
            if (intensity < 1) intensity = 1;
            weightedSum += (long)WEIGHTS[i] * intensity;
            totalWeight += intensity;
        }
    }

    if (totalWeight == 0) return 0.0f;

    float pos = (float)weightedSum / (float)totalWeight / 5.0f;
    if (pos < -1.0f) pos = -1.0f;
    if (pos >  1.0f) pos =  1.0f;
    return pos;
}

// ============================================================================
// getLinePattern()
// Classifica o padrão de navegação para um percurso em formato de 8.
//
// Prioridade de classificação:
//   1. INTERSECTION (5-6 sensores) — cruzamento central do 8
//      Resposta: passar reto em velocidade reduzida
//   2. Seguimento por magnitude do desvio (1-4 sensores)
//      STRAIGHT:      |pos| < 0.25 — limiar ligeiramente alargado para
//                     absorver ruído de leitura em retas sem mascarar curvas
//      CURVE_LIGHT:   |pos| 0.25–0.50
//      CURVE_MEDIUM:  |pos| 0.50–0.75
//      CURVE_SHARP:   |pos| > 0.75 — curva fechada do 8
// ============================================================================
LineSensor::LinePattern LineSensor::getLinePattern() const {
    if (_state.activeCount == 0)
        return LINE_LOST;

    // Cruzamento central do percurso em 8 — todos ou quase todos os sensores ativos
    if (_state.activeCount >= CROSS_MIN_SENSORS_X)
        return INTERSECTION;

    float absPos = (_state.position < 0.0f) ? -_state.position : _state.position;

    if      (absPos < 0.25f) return STRAIGHT;
    else if (absPos < 0.50f) return CURVE_LIGHT;
    else if (absPos < 0.75f) return CURVE_MEDIUM;
    else                     return CURVE_SHARP;
}

void LineSensor::printSensorValues() const {
    if (!DEBUG_MODE) return;

    Serial.print(F("[Line] "));
    for (int i = 0; i < 6; i++) {
        Serial.print(_state.active[i] ? '1' : '0');
    }

    char bar[22];
    int  idx = (int)((_state.position + 1.0f) * 10.0f);
    if (idx < 0)  idx = 0;
    if (idx > 20) idx = 20;
    for (int i = 0; i < 21; i++) bar[i] = (i == 10) ? '|' : '-';
    bar[idx] = '#';
    bar[21]  = '\0';

    Serial.print(F(" ["));  Serial.print(bar);  Serial.print(F("] "));
    Serial.print(_state.position, 3);
    Serial.print(F(" cnt=")); Serial.print(_state.activeCount);
    Serial.print(F(" raw:"));
    for (int i = 0; i < 6; i++) {
        Serial.print(F(" ")); Serial.print(_state.raw[i]);
    }
    Serial.println();
}
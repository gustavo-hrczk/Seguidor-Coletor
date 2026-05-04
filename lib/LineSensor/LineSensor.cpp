#include "LineSensor.h"

// Pesos: sensor mais à esquerda = -5, mais à direita = +5
// Centro = 0 quando S3+S4 ativos com intensidade igual
const int LineSensor::WEIGHTS[6] = { -5, -3, -1, 1, 3, 5 };

// ─────────────────────────────────────────────────────────────────────────────
LineSensor::LineSensor() : _lastDir(DIR_CENTER) {
    memset(&_state, 0, sizeof(SensorState));
}

// ─────────────────────────────────────────────────────────────────────────────
void LineSensor::initialize() {
    // Pinos A0–A5 são INPUT por padrão no Arduino — pinMode explícito para clareza
    for (int i = 0; i < 6; i++) {
        pinMode(A0 + i, INPUT);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Leitura direta sem filtro — LineSensor precisa de reatividade máxima.
// Filtragem pertence ao UltrasonicSensor, não aqui.
// ─────────────────────────────────────────────────────────────────────────────
const LineSensor::SensorState& LineSensor::readSensors() {
    _state.activeCount = 0;

    for (int i = 0; i < 6; i++) {
        _state.raw[i]    = analogRead(A0 + i);

        // Linha branca = valor BAIXO → ativo quando raw <= THRESHOLD
        _state.active[i] = (_state.raw[i] <= THRESHOLD_LINE_SENSOR);

        if (_state.active[i]) _state.activeCount++;
    }

    _state.position = computePosition();

    // Atualiza última direção conhecida — usado na recuperação de linha perdida
    if (_state.activeCount > 0) {
        if      (_state.position < -0.10f) _lastDir = DIR_LEFT;
        else if (_state.position >  0.10f) _lastDir = DIR_RIGHT;
        else                               _lastDir = DIR_CENTER;
    }

    return _state;
}

// ─────────────────────────────────────────────────────────────────────────────
// Centro de massa ponderado por intensidade
//
// Intensidade = THRESHOLD - raw
//   raw próximo de 0   → muito sobre a linha → intensidade alta → mais peso
//   raw próximo de THRESHOLD → borda da linha → intensidade baixa → menos peso
//
// Resultado normalizado para -1.0..+1.0 dividindo pelo peso máximo (5)
// ─────────────────────────────────────────────────────────────────────────────
float LineSensor::computePosition() const {
    if (_state.activeCount == 0) {
        // Linha perdida: retorna extremo na última direção conhecida
        // Isso mantém o PD corrigindo na direção certa durante recuperação
        if (_lastDir == DIR_RIGHT) return  1.0f;
        if (_lastDir == DIR_LEFT)  return -1.0f;
        return 0.0f;
    }

    long weightedSum = 0;
    int  totalWeight = 0;

    for (int i = 0; i < 6; i++) {
        if (_state.active[i]) {
            int intensity = THRESHOLD_LINE_SENSOR - _state.raw[i];
            if (intensity < 1) intensity = 1;   // garante peso mínimo não-zero
            weightedSum += (long)WEIGHTS[i] * intensity;
            totalWeight += intensity;
        }
    }

    if (totalWeight == 0) return 0.0f;

    // Divide por totalWeight → posição média ponderada em escala dos pesos (-5..+5)
    // Divide por 5 → normaliza para -1.0..+1.0
    float pos = (float)weightedSum / (float)totalWeight / 5.0f;

    // constrain por segurança numérica
    if (pos < -1.0f) pos = -1.0f;
    if (pos >  1.0f) pos =  1.0f;

    return pos;
}

// ─────────────────────────────────────────────────────────────────────────────
LineSensor::LinePattern LineSensor::getLinePattern() const {

    if (_state.activeCount == 0)
        return LINE_LOST;

    // Cruzamento X: 5 ou 6 sensores — linha larga ou intersecção real
    if (_state.activeCount >= 5)
        return INTERSECTION;

    // Cruzamento T: exatamente 4 sensores — detecta direção dominante
    if (_state.activeCount == 4) {
        return (_state.position < 0.0f) ? TURN_LEFT_90 : TURN_RIGHT_90;
    }

    // Seguimento normal: classifica por magnitude da posição
    float absPos = (_state.position < 0) ? -_state.position : _state.position;

    if      (absPos < 0.20f) return STRAIGHT;
    else if (absPos < 0.45f) return CURVE_LIGHT;
    else if (absPos < 0.70f) return CURVE_MEDIUM;
    else                     return CURVE_SHARP;
}

// ─────────────────────────────────────────────────────────────────────────────
void LineSensor::printSensorValues() const {
    if (!DEBUG_MODE) return;

    // Padrão binário
    Serial.print(F("[Line] "));
    for (int i = 0; i < 6; i++) {
        Serial.print(_state.active[i] ? '1' : '0');
    }

    // Barra visual de posição [-1.0 ---|--- +1.0]
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

    // Valores brutos — essenciais para diagnóstico de threshold
    Serial.print(F(" raw:"));
    for (int i = 0; i < 6; i++) {
        Serial.print(F(" ")); Serial.print(_state.raw[i]);
    }
    Serial.println();
}
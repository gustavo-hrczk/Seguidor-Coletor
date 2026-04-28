#include "LineSensor.h"

// ============================================================================
// IMPLEMENTAÇÃO: LineSensor
// ============================================================================

LineSensor::LineSensor()
    : filterCounter(0) {
    memset(&lastValidState, 0, sizeof(SensorState));
    memset(&rawReadings, 0, sizeof(SensorState));
    memset(&previousReading, 0, sizeof(SensorState));
}

void LineSensor::initialize() {
    pinMode(PIN_S1, INPUT);
    pinMode(PIN_S2, INPUT);
    pinMode(PIN_S3, INPUT);
    pinMode(PIN_S4, INPUT);
    pinMode(PIN_S5, INPUT);
    pinMode(PIN_S6, INPUT);
}

LineSensor::SensorState LineSensor::readSensors() {
    // Realizar leitura bruta
    rawReadings = performRawRead();
    
    // Incrementar contador de filtro
    filterCounter++;
    
    // Se atingiu número de ciclos de validação, validar
    if (filterCounter >= SENSOR_FILTER_CYCLES) {
        if (validateDebounce(rawReadings, previousReading)) {
            lastValidState = rawReadings;
            lastValidState.isValid = true;
        } else {
            lastValidState.isValid = false;
        }
        previousReading = rawReadings;
        filterCounter = 0;
    }
    
    return lastValidState;
}

LineSensor::SensorState LineSensor::performRawRead() {
    SensorState state;
    int readings[6];
    
    readings[0] = analogRead(PIN_S1);
    readings[1] = analogRead(PIN_S2);
    readings[2] = analogRead(PIN_S3);
    readings[3] = analogRead(PIN_S4);
    readings[4] = analogRead(PIN_S5);
    readings[5] = analogRead(PIN_S6);
    
    // Converter para padrão binário baseado em THRESHOLD
    state.rawPattern = 0;
    for (int i = 0; i < 6; i++) {
        if (readings[i] <= THRESHOLD_LINE_SENSOR) {
            state.sensors[i] = true;  // Linha detectada
            state.rawPattern |= (1 << i);
        } else {
            state.sensors[i] = false; // Background
        }
    }
    
    state.isValid = false;
    return state;
}

bool LineSensor::validateDebounce(const SensorState& current, const SensorState& previous) const {
    // Verificar se o padrão é consistente entre leituras
    // Tolerância: até 1 bit pode ser diferente (ruído)
    
    if (filterCounter < SENSOR_FILTER_CYCLES) {
        return false;
    }
    
    // Contar bits diferentes
    uint8_t xorPattern = current.rawPattern ^ previous.rawPattern;
    uint8_t bitsChanged = 0;
    
    for (int i = 0; i < 8; i++) {
        if (xorPattern & (1 << i)) bitsChanged++;
    }
    
    // Aceitar se mudança ≤ 1 sensor (debounce básico)
    return bitsChanged <= 1;
}

LineSensor::LinePattern LineSensor::identifyPattern(const SensorState& state) const {
    if (state.rawPattern == 0) {
        return LINE_LOST;
    }
    
    // Contar sensores ativos
    uint8_t activeCount = 0;
    for (int i = 0; i < 6; i++) {
        if (state.sensors[i]) activeCount++;
    }
    
    // Cruzamento/intersecção: 4+ sensores ativos
    if (activeCount >= 4) {
        return INTERSECTION;
    }
    
    // Sensores específicos para identificar tipo de curva
    bool s1 = state.sensors[0], s2 = state.sensors[1];
    bool s3 = state.sensors[2], s4 = state.sensors[3];
    bool s5 = state.sensors[4], s6 = state.sensors[5];
    
    // Reta: apenas sensores centrais
    if (!s1 && !s2 && s3 && s4 && !s5 && !s6) {
        return STRAIGHT;
    }
    
    // Curva suave à direita: sensor 4 proeminente
    if (s4 && !s5 && !s6) {
        return CURVE_LIGHT;
    }
    
    // Curva suave à esquerda: sensor 3 proeminente
    if (s3 && !s1 && !s2) {
        return CURVE_LIGHT;
    }
    
    // Curva acentuada: sensores laterais
    if ((s5 || s6) || (s1 || s2)) {
        return CURVE_SHARP;
    }
    
    return UNKNOWN;
}

int LineSensor::getActiveSensor() const {
    for (int i = 0; i < 6; i++) {
        if (lastValidState.sensors[i]) {
            return i;
        }
    }
    return -1;
}

void LineSensor::resetFilter() {
    filterCounter = 0;
    memset(&previousReading, 0, sizeof(SensorState));
}

void LineSensor::printSensorValues() const {
    if (!DEBUG_MODE) return;
    
    Serial.print("Sensores: ");
    for (int i = 0; i < 6; i++) {
        Serial.print(lastValidState.sensors[i] ? "1" : "0");
    }
    Serial.print(" | Padrão: ");
    Serial.println(lastValidState.rawPattern, BIN);
}

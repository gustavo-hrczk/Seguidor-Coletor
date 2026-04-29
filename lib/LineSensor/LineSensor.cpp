#include "LineSensor.h"

LineSensor::LineSensor() : filterCounter(0), stableCounter(0) {
    memset(&lastValidState,  0, sizeof(SensorState));
    memset(&rawReadings,     0, sizeof(SensorState));
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

// ---------------------------------------------------------------------
// Lê sensores e aplica janela de debounce por contagem de ciclos estáveis
// ---------------------------------------------------------------------
LineSensor::SensorState LineSensor::readSensors() {
    rawReadings = performRawRead();

    if (validateDebounce(rawReadings, previousReading)) {
        stableCounter++;
        if (stableCounter >= SENSOR_FILTER_CYCLES) {
            lastValidState         = rawReadings;
            lastValidState.isValid = true;
        }
    } else {
        // Padrão mudou: reinicia contagem de estabilidade
        stableCounter              = 0;
        lastValidState.isValid     = false;
    }

    previousReading  = rawReadings;
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

    state.rawPattern = 0;
    for (int i = 0; i < 6; i++) {
        // Linha = leitura ABAIXO do threshold (sensor refletivo: linha clara = baixo)
        state.sensors[i] = (readings[i] <= THRESHOLD_LINE_SENSOR);
        if (state.sensors[i]) state.rawPattern |= (1 << i);
    }

    state.isValid = false;
    return state;
}

// ---------------------------------------------------------------------
// Debounce: aceita leitura se diferença <= 1 bit em relação à anterior
// ---------------------------------------------------------------------
bool LineSensor::validateDebounce(const SensorState& current,
                                   const SensorState& previous) const {
    uint8_t xorPattern  = current.rawPattern ^ previous.rawPattern;
    uint8_t bitsChanged = 0;

    for (int i = 0; i < 6; i++) {
        if (xorPattern & (1 << i)) bitsChanged++;
    }

    return bitsChanged <= 1;
}

// ---------------------------------------------------------------------
// Identifica padrão de linha com todos os casos do enum mapeados
// Índices:  s1=extrema esq | s2=esq | s3=centro-esq | s4=centro-dir | s5=dir | s6=extrema dir
// ---------------------------------------------------------------------
LineSensor::LinePattern LineSensor::identifyPattern(const SensorState& state) const {
    if (state.rawPattern == 0) return LINE_LOST;

    uint8_t activeCount = 0;
    for (int i = 0; i < 6; i++) {
        if (state.sensors[i]) activeCount++;
    }

    if (activeCount >= 4) return INTERSECTION;

    bool s1 = state.sensors[0], s2 = state.sensors[1];
    bool s3 = state.sensors[2], s4 = state.sensors[3];
    bool s5 = state.sensors[4], s6 = state.sensors[5];

    // Reta: apenas os dois centrais
    if (!s1 && !s2 && s3 && s4 && !s5 && !s6) return STRAIGHT;

    // Curva suave: um central + adjacente imediato
    if (!s1 && !s2 && s3 && !s4 && !s5 && !s6) return CURVE_LIGHT;  // levemente esq
    if (!s1 && !s2 && !s3 && s4 && !s5 && !s6) return CURVE_LIGHT;  // levemente dir

    // Curva média: sensor intermediário ativo
    if (!s1 && s2 && !s3 && !s4 && !s5 && !s6) return CURVE_MEDIUM; // médio esq
    if (!s1 && !s2 && !s3 && !s4 && s5 && !s6) return CURVE_MEDIUM; // médio dir
    if (!s1 && s2 && s3 && !s4 && !s5 && !s6)  return CURVE_MEDIUM;
    if (!s1 && !s2 && !s3 && s4 && s5 && !s6)  return CURVE_MEDIUM;

    // Curva acentuada: sensores extremos ativos
    if (s1 || s6) return CURVE_SHARP;

    return UNKNOWN;
}

int LineSensor::getActiveSensor() const {
    for (int i = 0; i < 6; i++) {
        if (lastValidState.sensors[i]) return i;
    }
    return -1;
}

void LineSensor::resetFilter() {
    filterCounter = 0;
    stableCounter = 0;
    memset(&previousReading, 0, sizeof(SensorState));
}

void LineSensor::printSensorValues() const {
    if (!DEBUG_MODE) return;
    Serial.print(F("[Line] "));
    for (int i = 0; i < 6; i++) {
        Serial.print(lastValidState.sensors[i] ? '1' : '0');
    }
    Serial.print(F(" | pat=0b"));
    Serial.print(lastValidState.rawPattern, BIN);
    Serial.print(F(" | valid="));
    Serial.println(lastValidState.isValid ? F("S") : F("N"));
}
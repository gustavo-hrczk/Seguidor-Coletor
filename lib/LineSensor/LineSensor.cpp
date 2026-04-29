#include "LineSensor.h"

// Pesos simétricos: S1=-5 S2=-3 S3=-1 S4=+1 S5=+3 S6=+5
// Divisor máximo = 5+3+1+1+3+5 = 18 → normaliza para -1.0..+1.0
const int LineSensor::WEIGHTS[6] = { -5, -3, -1, 1, 3, 5 };

LineSensor::LineSensor()
    : _stableCounter(0), _lastDirection(DIR_CENTER) {
    memset(&_state,     0, sizeof(SensorState));
    memset(&_prevState, 0, sizeof(SensorState));
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
LineSensor::SensorState LineSensor::readSensors() {
    SensorState current = performRead();

    if (isStable(current, _prevState)) {
        if (_stableCounter < SENSOR_FILTER_CYCLES) _stableCounter++;
    } else {
        _stableCounter = 0;
    }

    current.isValid = (_stableCounter >= SENSOR_FILTER_CYCLES);

    if (current.isValid && current.activeCount > 0) {
        // Atualiza última direção conhecida
        if (current.position < -0.15f)      _lastDirection = DIR_LEFT;
        else if (current.position > 0.15f)  _lastDirection = DIR_RIGHT;
        else                                 _lastDirection = DIR_CENTER;
    }

    _prevState = current;
    _state     = current;
    return _state;
}

// ---------------------------------------------------------------------
LineSensor::SensorState LineSensor::performRead() {
    SensorState s;
    s.activeCount = 0;
    s.isValid     = false;

    for (int i = 0; i < 6; i++) {
        s.raw[i]    = analogRead(PIN_S1 + i);   // A0..A5 são contíguos
        // Linha clara sobre fundo escuro: valor baixo = linha
        s.active[i] = (s.raw[i] <= THRESHOLD_LINE_SENSOR);
        if (s.active[i]) s.activeCount++;
    }

    s.position = calculatePosition(s);
    return s;
}

// ---------------------------------------------------------------------
// Centro de massa ponderado normalizado
// Se nenhum sensor ativo: mantém última posição conhecida (não zera)
// ---------------------------------------------------------------------
float LineSensor::calculatePosition(const SensorState& s) const {
    if (s.activeCount == 0) {
        // Linha perdida: retorna posição extrema na última direção
        return (_lastDirection == DIR_RIGHT) ? 1.0f :
               (_lastDirection == DIR_LEFT)  ? -1.0f : _state.position;
    }

    long  weightedSum = 0;
    int   totalWeight = 0;

    for (int i = 0; i < 6; i++) {
        if (s.active[i]) {
            // Usa o inverso da leitura como intensidade:
            // leitura baixa = mais sobre a linha = mais peso
            int intensity = THRESHOLD_LINE_SENSOR - s.raw[i];
            intensity     = max(intensity, 1);   // evita peso zero
            weightedSum  += (long)WEIGHTS[i] * intensity;
            totalWeight  += intensity;
        }
    }

    if (totalWeight == 0) return 0.0f;

    // Normaliza: divisor máximo teórico = 5 * THRESHOLD_LINE_SENSOR
    float raw = (float)weightedSum / (float)totalWeight;

    // Clamp e normalização para -1.0..+1.0
    float normalized = raw / 5.0f;
    return constrain(normalized, -1.0f, 1.0f);
}

// ---------------------------------------------------------------------
LineSensor::LinePattern LineSensor::getLinePattern() const {
    if (!_state.isValid || _state.activeCount == 0) return LINE_LOST;

    // Cruzamento X: 5 ou 6 sensores ativos
    if (_state.activeCount >= CROSS_MIN_SENSORS_X) return INTERSECTION;

    // Cruzamento T: 4 sensores ativos — detecta direção dominante
    if (_state.activeCount >= CROSS_MIN_SENSORS_T) {
        return (_state.position <= 0.0f) ? TURN_LEFT_90 : TURN_RIGHT_90;
    }

    // Seguimento normal por magnitude de erro
    float absPos = fabs(_state.position);

    if      (absPos < 0.3f) return STRAIGHT;
    else if (absPos < 0.5f) return CURVE_LIGHT;
    else if (absPos < 0.7f) return CURVE_MEDIUM;
    else                    return CURVE_SHARP;
}

// ---------------------------------------------------------------------
bool LineSensor::isStable(const SensorState& a, const SensorState& b) const {
    // Estável se o padrão binário difere em no máximo 1 bit
    uint8_t patA = 0, patB = 0;
    for (int i = 0; i < 6; i++) {
        if (a.active[i]) patA |= (1 << i);
        if (b.active[i]) patB |= (1 << i);
    }
    uint8_t diff     = patA ^ patB;
    uint8_t changed  = 0;
    for (int i = 0; i < 6; i++) {
        if (diff & (1 << i)) changed++;
    }
    return changed <= 1;
}

// ---------------------------------------------------------------------
void LineSensor::resetFilter() {
    _stableCounter = 0;
    memset(&_prevState, 0, sizeof(SensorState));
}

// ---------------------------------------------------------------------
void LineSensor::printSensorValues() const {
    if (!DEBUG_MODE) return;
    Serial.print(F("[Line] "));
    for (int i = 0; i < 6; i++) Serial.print(_state.active[i] ? '1' : '0');
    Serial.print(F(" | pos="));
    Serial.print(_state.position, 3);
    Serial.print(F(" | cnt="));
    Serial.print(_state.activeCount);
    Serial.print(F(" | valid="));
    Serial.println(_state.isValid ? F("S") : F("N"));
}
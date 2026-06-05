#include "UltrasonicSensor.h"

// ============================================================================
// Construtor
// Inicializa tudo como inválido (-1) — força a primeira leitura real
// a passar pelo processo de validação completo antes de ser usada.
// ============================================================================
UltrasonicSensor::UltrasonicSensor()
    : currentDistance(-1), lastValidDistance(-1), previousDistance(-1),
      validationCounter(0), recentChange(false), readingStable(false),
      lastChangeTime(0) {}

// ============================================================================
// initialize()
// TRIGGER em LOW como estado de repouso — HIGH acidental dispararia medição.
// ============================================================================
void UltrasonicSensor::initialize() {
    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO,    INPUT);
    digitalWrite(PIN_TRIGGER, LOW);
}

// ============================================================================
// readDistance()
// Interface principal — medição + validação em uma chamada.
// Sempre retorna lastValidDistance (confirmada), nunca a leitura bruta.
// ============================================================================
int UltrasonicSensor::readDistance() {
    currentDistance = measureDistance();
    validateReading();
    return lastValidDistance;
}

// ============================================================================
// measureDistance()
// Protocolo HC-SR04:
//   1. LOW por 2µs — limpa pulso anterior
//   2. HIGH por 10µs — dispara burst ultrassônico
//   3. pulseIn() mede o tempo de retorno do eco
//   4. Converte: dist = (duração × 0.0343) / 2
//      (velocidade do som ≈ 0.0343 cm/µs, divido por 2 pois é ida + volta)
//
// Retorna -1 se: timeout, eco ausente, ou distância fora de 2–400 cm.
// ============================================================================
int UltrasonicSensor::measureDistance() {
    digitalWrite(PIN_TRIGGER, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIGGER, LOW);

    long duration = pulseIn(PIN_ECHO, HIGH, ULTRASONIC_TIMEOUT_US);
    if (duration == 0) return -1;

    int distance = (int)(duration * 0.0343f / 2.0f);
    if (distance < 2 || distance > 400) return -1;

    return distance;
}

// ============================================================================
// validateReading()
// Janela deslizante com degradação gradual:
//
// Inválida (-1):
//   Reduz contador suavemente. Só marca unstable quando zera.
//   Um eco perdido não invalida uma detecção estável.
//
// Primeira válida:
//   Inicializa referência. Ainda não estável — aguarda SENSOR_FILTER_CYCLES.
//
// Dentro da tolerância:
//   Incrementa contador. Quando >= SENSOR_FILTER_CYCLES: estável.
//
// Fora da tolerância:
//   Reduz contador. Se zerar: adota nova distância como referência e
//   reinicia — permite rastrear objeto em movimento.
// ============================================================================
bool UltrasonicSensor::validateReading() {
    recentChange = false;

    if (currentDistance < 0) {
        if (validationCounter > 0) validationCounter--;
        if (validationCounter == 0) readingStable = false;
        return false;
    }

    if (lastValidDistance < 0) {
        lastValidDistance = currentDistance;
        previousDistance  = currentDistance;
        validationCounter = 1;
        readingStable     = false;
        return false;
    }

    if (isWithinTolerance(currentDistance, lastValidDistance)) {
        if (validationCounter < SENSOR_FILTER_CYCLES) validationCounter++;

        if (validationCounter >= SENSOR_FILTER_CYCLES) {
            if (abs(currentDistance - lastValidDistance) > 1) {
                recentChange   = true;
                lastChangeTime = millis();
            }
            lastValidDistance = currentDistance;
            readingStable     = true;
            return true;
        }
    } else {
        if (validationCounter > 0) validationCounter--;

        if (validationCounter == 0) {
            readingStable     = false;
            recentChange      = true;
            lastChangeTime    = millis();
            lastValidDistance = currentDistance;
            validationCounter = 1;
        }
    }

    previousDistance = currentDistance;
    return false;
}

// ============================================================================
// isWithinTolerance()
// Tolerância adaptativa ao comportamento real do HC-SR04:
//   ≤ 20 cm: ±2 cm fixo — ruído absoluto constante em curta distância
//   > 20 cm: percentual (ULTRASONIC_NOISE_TOLERANCE%), teto de ±4 cm
//            evita janela excessivamente larga em longas distâncias
// ============================================================================
bool UltrasonicSensor::isWithinTolerance(int dist1, int dist2) const {
    if (dist1 < 0 || dist2 < 0) return false;

    int diff = abs(dist1 - dist2);
    if (dist2 <= 20) return diff <= 2;

    int tol = min((int)((float)dist2 * (ULTRASONIC_NOISE_TOLERANCE / 100.0f)), 4);
    return diff <= tol;
}

// ============================================================================
// getApproachPhase()
// Classifica distância válida em fase de aproximação para controle de
// velocidade e tomada de decisão de coleta na máquina de estados.
// ============================================================================
UltrasonicSensor::ApproachPhase UltrasonicSensor::getApproachPhase() const {
    if (lastValidDistance < 0 || !isObjectDetected())
        return OBJECT_NOT_DETECTED;

    if      (lastValidDistance >= ULTRASONIC_DISTANCE_LONG)  return PHASE_1_DISTANT;
    else if (lastValidDistance >= ULTRASONIC_DISTANCE_SHORT) return PHASE_2_APPROACHING;
    else                                                      return PHASE_3_CONTACT;
}

// ============================================================================
// isObjectDetected()
// Três condições para detecção confiável:
//   1. Distância positiva
//   2. Dentro do alcance útil (< 300 cm)
//   3. Leitura validada (passou pela janela deslizante)
// ============================================================================
bool UltrasonicSensor::isObjectDetected() const {
    return (lastValidDistance > 0 && lastValidDistance < 300 && readingStable);
}

// ============================================================================
// resetValidation()
// Limpa todo o estado de validação.
// Chamar após cada coleta concluída — a próxima detecção começa do zero,
// sem herdar estado da detecção anterior.
// ============================================================================
void UltrasonicSensor::resetValidation() {
    validationCounter = 0;
    lastValidDistance = -1;
    previousDistance  = -1;
    recentChange      = false;
    readingStable     = false;
}

// ============================================================================
// printDistance()
// ============================================================================
void UltrasonicSensor::printDistance() const {
    if (!DEBUG_MODE) return;
    Serial.print(F("[Ultrasonic] Atual: "));   Serial.print(currentDistance);
    Serial.print(F("cm | Validado: "));         Serial.print(lastValidDistance);
    Serial.print(F("cm | Estavel: "));          Serial.print(readingStable ? F("SIM") : F("NAO"));
    Serial.print(F(" | Fase: "));               Serial.println(getApproachPhase());
}

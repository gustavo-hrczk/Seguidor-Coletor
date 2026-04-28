#include "UltrasonicSensor.h"

// ============================================================================
// IMPLEMENTAÇÃO: UltrasonicSensor
// ============================================================================

UltrasonicSensor::UltrasonicSensor()
    : currentDistance(-1), lastValidDistance(-1), previousDistance(-1),
      validationCounter(0), recentChange(false), lastChangeTime(0) {}

void UltrasonicSensor::initialize() {
    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    digitalWrite(PIN_TRIGGER, LOW);
}

int UltrasonicSensor::readDistance() {
    currentDistance = measureDistance();
    return currentDistance;
}

int UltrasonicSensor::measureDistance() {
    // Enviar pulso no trigger
    digitalWrite(PIN_TRIGGER, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIGGER, LOW);

    // Medir duração do echo
    long duration = pulseIn(PIN_ECHO, HIGH, 30000); // timeout 30ms
    
    // Converter para cm (velocidade do som = 343 m/s)
    // distância = (duração em us * velocidade em cm/us) / 2
    // 343 m/s = 0.0343 cm/us
    int distance = (int)(duration * 0.0343 / 2);
    
    // Filtro básico: descartar leituras anormais
    if (distance < 2 || distance > 400) {
        return -1;
    }
    
    return distance;
}

bool UltrasonicSensor::validateReading() {
    // Descartar leituras inválidas
    if (currentDistance < 0) {
        validationCounter = 0;
        recentChange = false;
        return false;
    }

    // Primeira leitura válida
    if (lastValidDistance < 0) {
        lastValidDistance = currentDistance;
        previousDistance = currentDistance;
        validationCounter = 1;
        recentChange = true;
        lastChangeTime = millis();
        return false; // Aguardar próximas validações
    }

    // Verificar se está dentro da tolerância de 5%
    if (isWithinTolerance(currentDistance, lastValidDistance)) {
        validationCounter++;
        
        if (validationCounter >= SENSOR_FILTER_CYCLES) {
            // Leitura validada
            if (currentDistance != lastValidDistance) {
                recentChange = true;
                lastChangeTime = millis();
            } else {
                recentChange = false;
            }
            
            lastValidDistance = currentDistance;
            validationCounter = 0;
            return true;
        }
    } else {
        // Fora da tolerância, resetar contador
        validationCounter = 0;
        
        // Detectar mudança significativa
        if (!isWithinTolerance(currentDistance, previousDistance)) {
            recentChange = true;
            lastChangeTime = millis();
        }
    }
    
    previousDistance = currentDistance;
    return false;
}

bool UltrasonicSensor::isWithinTolerance(int dist1, int dist2) const {
    if (dist1 < 0 || dist2 < 0) return false;
    
    // Calcular percentual de diferença
    int diff = abs(dist1 - dist2);
    float tolerance = (float)dist2 * (ULTRASONIC_NOISE_TOLERANCE / 100.0);
    
    return diff <= (int)tolerance;
}

bool UltrasonicSensor::checkOutlier(int current, int previous) const {
    // Outlier: mudança > 50% em uma única medição
    if (previous < 0) return false;
    
    int diff = abs(current - previous);
    float threshold = (float)previous * 0.5;
    
    return diff > (int)threshold;
}

UltrasonicSensor::ApproachPhase UltrasonicSensor::getApproachPhase() const {
    if (lastValidDistance < 0 || !isObjectDetected()) {
        return OBJECT_NOT_DETECTED;
    }
    
    if (lastValidDistance >= ULTRASONIC_DISTANCE_LONG) {
        return PHASE_1_DISTANT;
    } else if (lastValidDistance >= ULTRASONIC_DISTANCE_SHORT) {
        return PHASE_2_APPROACHING;
    } else if (lastValidDistance >= ULTRASONIC_DISTANCE_CONTACT) {
        return PHASE_3_CONTACT;
    }
    
    return PHASE_3_CONTACT;  // Muito perto, considerar como contato
}

bool UltrasonicSensor::isObjectDetected() const {
    // Objeto detectado se distância validada está em range válido
    return (lastValidDistance > 0 && lastValidDistance < 300);
}

void UltrasonicSensor::resetValidation() {
    validationCounter = 0;
    lastValidDistance = -1;
    previousDistance = -1;
    recentChange = false;
}

void UltrasonicSensor::printDistance() const {
    if (!DEBUG_MODE) return;
    
    Serial.print("Ultrassônico - Atual: ");
    Serial.print(currentDistance);
    Serial.print(" cm | Validado: ");
    Serial.print(lastValidDistance);
    Serial.print(" cm | Fase: ");
    Serial.println(getApproachPhase());
}

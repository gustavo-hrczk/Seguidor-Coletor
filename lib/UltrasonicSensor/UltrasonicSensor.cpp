#include "UltrasonicSensor.h"

UltrasonicSensor::UltrasonicSensor()
    : currentDistance(-1), lastValidDistance(-1), previousDistance(-1),
      validationCounter(0), recentChange(false), readingStable(false),
      lastChangeTime(0) {}

void UltrasonicSensor::initialize() {
    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    digitalWrite(PIN_TRIGGER, LOW);
}

int UltrasonicSensor::readDistance() {
    currentDistance = measureDistance();
    validateReading();
    return lastValidDistance;
}

int UltrasonicSensor::measureDistance() {
    digitalWrite(PIN_TRIGGER, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIGGER, LOW);

    long duration = pulseIn(PIN_ECHO, HIGH, 30000);

    if (duration == 0) return -1;

    int distance = (int)(duration * 0.0343f / 2.0f);

    if (distance < 2 || distance > 400) return -1;

    return distance;
}

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
        if (validationCounter < SENSOR_FILTER_CYCLES) {
            validationCounter++;
        }

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

bool UltrasonicSensor::isWithinTolerance(int dist1, int dist2) const {
    if (dist1 < 0 || dist2 < 0) return false;

    int diff = abs(dist1 - dist2);

    if (dist2 <= 20) return diff <= 2;

    int percentTol = (int)((float)dist2 * (ULTRASONIC_NOISE_TOLERANCE / 100.0f));
    int tol        = min(percentTol, 4);

    return diff <= tol;
}

UltrasonicSensor::ApproachPhase UltrasonicSensor::getApproachPhase() const {
    if (lastValidDistance < 0 || !isObjectDetected())
        return OBJECT_NOT_DETECTED;

    if      (lastValidDistance >= ULTRASONIC_DISTANCE_LONG)  return PHASE_1_DISTANT;
    else if (lastValidDistance >= ULTRASONIC_DISTANCE_SHORT) return PHASE_2_APPROACHING;
    else                                                      return PHASE_3_CONTACT;
}

bool UltrasonicSensor::isObjectDetected() const {
    return (lastValidDistance > 0 && lastValidDistance < 300 && readingStable);
}

void UltrasonicSensor::resetValidation() {
    validationCounter = 0;
    lastValidDistance = -1;
    previousDistance  = -1;
    recentChange      = false;
    readingStable     = false;
}

void UltrasonicSensor::printDistance() const {
    if (!DEBUG_MODE) return;
    Serial.print(F("[Ultrasonic] Atual: "));
    Serial.print(currentDistance);
    Serial.print(F(" cm | Validado: "));
    Serial.print(lastValidDistance);
    Serial.print(F(" cm | Estável: "));
    Serial.print(readingStable ? F("SIM") : F("NÃO"));
    Serial.print(F(" | Fase: "));
    Serial.println(getApproachPhase());
}
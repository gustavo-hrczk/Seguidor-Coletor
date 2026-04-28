#include "GripperServo.h"

// ============================================================================
// IMPLEMENTAÇÃO: GripperServo
// ============================================================================

GripperServo::GripperServo()
    : currentState(OPEN), lastCommandTime(0), stateChangeTime(0),
      currentAngle(SERVO_ANGLE_OPEN) {}

void GripperServo::initialize() {
    servo.attach(PIN_SERVO);
    servo.write(SERVO_ANGLE_OPEN);
    currentState = OPEN;
    currentAngle = SERVO_ANGLE_OPEN;
    lastCommandTime = millis();
}

bool GripperServo::close() {
    if (currentState == CLOSED || currentState == CLOSING) {
        return currentState == CLOSED;
    }

    currentState = CLOSING;
    stateChangeTime = millis();
    lastCommandTime = millis();
    setAngle(SERVO_ANGLE_CLOSED);

    // Aguardar fechamento com timeout
    unsigned long startTime = millis();
    while (millis() - startTime < SERVO_TIMEOUT) {
        // Servo está ainda movendo
        delay(50);
        
        // Se timeout expira durante fechamento, liberar
        if (millis() - startTime >= SERVO_TIMEOUT) {
            if (DEBUG_MODE) {
                Serial.println(F("ERRO: Servo travou! Liberando..."));
            }
            currentState = ERROR;
            emergency_stop();
            return false;
        }
    }

    // Após estabilização, confirmar fechamento
    delay(SERVO_STABILIZATION_TIME);
    currentState = CLOSED;
    currentAngle = SERVO_ANGLE_CLOSED;
    
    if (DEBUG_MODE) {
        Serial.println(F("Garra fechada com sucesso"));
    }
    
    return true;
}

void GripperServo::open() {
    if (currentState == OPEN) {
        return;
    }

    currentState = OPENING;
    stateChangeTime = millis();
    lastCommandTime = millis();
    setAngle(SERVO_ANGLE_OPEN);

    // Aguardar abertura
    delay(SERVO_STABILIZATION_TIME);
    
    currentState = OPEN;
    currentAngle = SERVO_ANGLE_OPEN;
    
    if (DEBUG_MODE) {
        Serial.println(F("Garra aberta"));
    }
}

void GripperServo::emergency_stop() {
    // Desabilitar servo para proteção
    servo.detach();
    currentState = ERROR;
    
    if (DEBUG_MODE) {
        Serial.println(F("PARADA DE EMERGÊNCIA DO SERVO"));
    }
}

void GripperServo::setAngle(uint8_t angle) {
    angle = constrain(angle, 0, 180);
    servo.write(angle);
    currentAngle = angle;
}

void GripperServo::updateState() {
    // Verificar timeout se estava em CLOSING
    if (currentState == CLOSING) {
        if (millis() - stateChangeTime >= SERVO_TIMEOUT) {
            currentState = ERROR;
            emergency_stop();
        }
    }
}

void GripperServo::printState() const {
    if (!DEBUG_MODE) return;

    Serial.print("Garra - Estado: ");
    switch (currentState) {
        case OPEN:
            Serial.println(F("ABERTA"));
            break;
        case OPENING:
            Serial.println(F("ABRINDO"));
            break;
        case CLOSED:
            Serial.println(F("FECHADA"));
            break;
        case CLOSING:
            Serial.println(F("FECHANDO"));
            break;
        case ERROR:
            Serial.println(F("ERRO/TRAVADA"));
            break;
        default:
            Serial.println(F("DESCONHECIDO"));
    }
}

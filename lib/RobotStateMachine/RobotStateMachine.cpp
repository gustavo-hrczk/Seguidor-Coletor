#include "RobotStateMachine.h"
#include <EEPROM.h>

// ============================================================================
// IMPLEMENTAÇÃO: RobotStateMachine
// ============================================================================

RobotStateMachine::RobotStateMachine(MotorController* motor, LineSensor* line,
                                     UltrasonicSensor* ultrasonic, GripperServo* gripper)
    : motor(motor), lineSensor(line), ultrasonic(ultrasonic), gripper(gripper),
      currentState(STATE_IDLE), previousState(STATE_IDLE),
      lastTurnDirection(TURN_NONE), startTime(0), stateChangeTime(0),
      lastObjectTime(0), objectsCollected(0), lineSearchRotations(0),
      collectionAttempts(0), running(false) {}

void RobotStateMachine::initialize() {
    motor->initialize();
    lineSensor->initialize();
    ultrasonic->initialize();
    gripper->initialize();
    
    // Carregar direção de curva anterior da EEPROM
    loadTurnDirection();
    
    running = true;
    startTime = millis();
    stateChangeTime = millis();
    
    if (DEBUG_MODE) {
        Serial.println(F("=== ROBÔ INICIALIZADO ==="));
        Serial.print("Velocidade Global: "));
        Serial.println(F(VELOCITY_GLOBAL);
    }
    
    transitionToState(STATE_NAVIGATE);
}

void RobotStateMachine::update() {
    if (!running) return;
    
    // Verificar timeout geral
    if (millis() - startTime >= RUNTIME_TOTAL) {
        transitionToState(STATE_EMERGENCY_STOP);
        return;
    }

    // Executar lógica do estado atual
    switch (currentState) {
        case STATE_NAVIGATE:
            handleNavigate();
            break;
        case STATE_CURVE:
            handleCurve();
            break;
        case STATE_OBJECT_DETECTED:
            handleObjectDetected();
            break;
        case STATE_APPROACH:
            handleApproach();
            break;
        case STATE_COLLECT:
            handleCollect();
            break;
        case STATE_ROTATE_90:
            handleRotate90();
            break;
        case STATE_RELEASE:
            handleRelease();
            break;
        case STATE_RETURN_LINE:
            handleReturnLine();
            break;
        case STATE_LINE_SEARCH:
            handleLineSearch();
            break;
        case STATE_EMERGENCY_STOP:
            handleEmergencyStop();
            break;
        case STATE_IDLE:
        default:
            motor->stop();
            break;
    }
}

// ============================================================================
// HANDLERS DE ESTADO
// ============================================================================

void RobotStateMachine::handleNavigate() {
    // Ler sensores de linha
    LineSensor::SensorState sensorState = lineSensor->readSensors();

    // Verificar detecção de objeto (válida apenas em reta ou curva suave)
    if (shouldDetectObject() && ultrasonic->isObjectDetected()) {
        ultrasonic->validateReading();
        if (ultrasonic->isObjectDetected()) {
            transitionToState(STATE_OBJECT_DETECTED);
            return;
        }
    }

    // Determinando padrão de movimento
    LineSensor::LinePattern pattern = lineSensor->getLinePattern();

    switch (pattern) {
        case LineSensor::STRAIGHT: {
            // Movimento em reta - velocidade máxima
            motor->move(MotorController::FORWARD, scaleSpeed(SPEED_STRAIGHT));
            break;
        }
        
        case LineSensor::CURVE_LIGHT: {
            transitionToState(STATE_CURVE);
            break;
        }
        
        case LineSensor::CURVE_SHARP:
        case LineSensor::CURVE_MEDIUM: {
            transitionToState(STATE_CURVE);
            break;
        }
        
        case LineSensor::INTERSECTION: {
            // Implementar algoritmo Round Robin
            TurnDirection nextTurn = getNextTurnDirection();
            MotorController::Direction turnDir = 
                (nextTurn == TURN_LEFT) ? MotorController::TURN_LEFT : MotorController::TURN_RIGHT;
            motor->move(turnDir, scaleSpeed(SPEED_CURVE_RECOVERY));
            break;
        }
        
        case LineSensor::LINE_LOST: {
            transitionToState(STATE_LINE_SEARCH);
            break;
        }
        
        default:
            motor->move(MotorController::FORWARD, scaleSpeed(SPEED_STRAIGHT));
    }
}

void RobotStateMachine::handleCurve() {
    LineSensor::SensorState sensorState = lineSensor->readSensors();
    LineSensor::LinePattern pattern = lineSensor->getLinePattern();

    // Verificar se deve retornar à navegação normal
    if (pattern == LineSensor::STRAIGHT) {
        transitionToState(STATE_NAVIGATE);
        return;
    }

    // BLOQUEIO: Não permitir coleta durante curva acentuada
    bool isSharpCurve = (pattern == LineSensor::CURVE_SHARP || pattern == LineSensor::CURVE_MEDIUM);
    
    if (isSharpCurve && ultrasonic->isObjectDetected()) {
        // IGNORAR objeto durante curva acentuada
        ultrasonic->resetValidation();
    }

    // Aplicar compensação de curva
    int activeSensor = lineSensor->getActiveSensor();
    
    if (activeSensor >= 4) {
        // Curva à direita
        motor->curveCompensated(MotorController::TURN_RIGHT, 
                               scaleSpeed(SPEED_CURVE_MEDIUM),
                               CURVE_COMPENSATION_MEDIUM);
        lastTurnDirection = TURN_RIGHT;
    } else if (activeSensor <= 1) {
        // Curva à esquerda
        motor->curveCompensated(MotorController::TURN_LEFT,
                               scaleSpeed(SPEED_CURVE_MEDIUM),
                               CURVE_COMPENSATION_MEDIUM);
        lastTurnDirection = TURN_LEFT;
    } else {
        // Curva suave
        motor->move(MotorController::FORWARD, scaleSpeed(SPEED_CURVE_LIGHT));
    }
}

void RobotStateMachine::handleObjectDetected() {
    // Centralizar objeto usando sensores de linha
    LineSensor::SensorState sensorState = lineSensor->readSensors();

    // Verificar se sensores centrais estão alinhados
    if (sensorState.sensors[2] && sensorState.sensors[3] &&
        !sensorState.sensors[0] && !sensorState.sensors[1] &&
        !sensorState.sensors[4] && !sensorState.sensors[5]) {
        
        // Alinhado! Seguir para aproximação
        transitionToState(STATE_APPROACH);
        return;
    }

    // Centralizar o objeto (fazer curva suave se necessário)
    if (sensorState.sensors[0] || sensorState.sensors[1]) {
        motor->move(MotorController::TURN_LEFT, scaleSpeed(SPEED_CURVE_LIGHT));
    } else if (sensorState.sensors[4] || sensorState.sensors[5]) {
        motor->move(MotorController::TURN_RIGHT, scaleSpeed(SPEED_CURVE_LIGHT));
    } else {
        motor->move(MotorController::FORWARD, scaleSpeed(PWM_MEDIUM));
    }

    // Timeout: se não conseguir centralizar em tempo
    if (millis() - stateChangeTime > scaleTime(2000)) {
        transitionToState(STATE_NAVIGATE);
    }
}

void RobotStateMachine::handleApproach() {
    UltrasonicSensor::ApproachPhase phase = ultrasonic->getApproachPhase();

    switch (phase) {
        case UltrasonicSensor::PHASE_1_DISTANT:
            // Desacelerar gradualmente
            motor->move(MotorController::FORWARD, scaleSpeed(PWM_MEDIUM));
            break;

        case UltrasonicSensor::PHASE_2_APPROACHING:
            // Velocidade lenta, alinhamento fino
            motor->move(MotorController::FORWARD, scaleSpeed(PWM_SLOW));
            break;

        case UltrasonicSensor::PHASE_3_CONTACT:
            // Parada de coleta
            motor->stop();
            delay(scaleTime(100));
            transitionToState(STATE_COLLECT);
            return;

        case UltrasonicSensor::OBJECT_NOT_DETECTED:
            // Objeto perdido!
            transitionToState(STATE_NAVIGATE);
            return;
    }

    // Timeout de aproximação
    if (millis() - stateChangeTime > scaleTime(5000)) {
        transitionToState(STATE_NAVIGATE);
    }
}

void RobotStateMachine::handleCollect() {
    // Fechar garra com timeout
    if (gripper->close()) {
        lastObjectTime = millis();
        objectsCollected++;
        
        if (DEBUG_MODE) {
            Serial.print("Objeto coletado! Total: "));
            Serial.println(F(objectsCollected);
        }

        // Aguardar estabilização da garra
        delay(SERVO_STABILIZATION_TIME);
        
        transitionToState(STATE_ROTATE_90);
    } else {
        // Garra travou - tentar novamente ou abortar
        collectionAttempts++;
        
        if (collectionAttempts >= MAX_COLLECTION_ATTEMPTS) {
            gripper->open();
            transitionToState(STATE_NAVIGATE);
        } else {
            motor->stop();
            delay(scaleTime(500));
        }
    }
}

void RobotStateMachine::handleRotate90() {
    // Girar 90° na direção oposta ao sensor lateral que detectou
    // Se objeto veio da direita, girar para esquerda e vice-versa
    MotorController::Direction turnDir =
        (lastTurnDirection == TURN_RIGHT) ? MotorController::TURN_LEFT : MotorController::TURN_RIGHT;

    motor->move(turnDir, scaleSpeed(SPEED_CURVE_RECOVERY));

    // Executar rotação de 90°
    uint16_t rotationTime = scaleTime(ROTATION_90_DEGREES_TIME);
    unsigned long rotationStart = millis();

    while (millis() - rotationStart < rotationTime) {
        // Monitorar se linha é perdida durante rotação
        LineSensor::SensorState sensorState = lineSensor->readSensors();
        
        if (!sensorState.isValid) {
            delay(scaleTime(200));
            // Se linha desaparece por muito tempo, pausar
            continue;
        }

        delay(CYCLE_MAIN);
    }

    transitionToState(STATE_RELEASE);
}

void RobotStateMachine::handleRelease() {
    // Abrir garra
    gripper->open();

    // Recuar distância calibrada
    motor->move(MotorController::BACKWARD, scaleSpeed(PWM_MEDIUM));
    delay(scaleTime(500));

    motor->stop();
    delay(scaleTime(200));

    // Girar 90° de volta para linha
    MotorController::Direction turnDir =
        (lastTurnDirection == TURN_RIGHT) ? MotorController::TURN_LEFT : MotorController::TURN_RIGHT;

    motor->move(turnDir, scaleSpeed(SPEED_CURVE_RECOVERY));

    transitionToState(STATE_RETURN_LINE);
}

void RobotStateMachine::handleReturnLine() {
    LineSensor::SensorState sensorState = lineSensor->readSensors();

    // Assim que qualquer sensor tocar a linha, parar giro
    if (sensorState.isValid && sensorState.rawPattern != 0) {
        // Centralizar
        motor->stop();
        delay(scaleTime(200));
        
        transitionToState(STATE_NAVIGATE);
        return;
    }

    // Continuar girando procurando linha
    MotorController::Direction turnDir =
        (lastTurnDirection == TURN_RIGHT) ? MotorController::TURN_LEFT : MotorController::TURN_RIGHT;
    
    motor->move(turnDir, scaleSpeed(SPEED_CURVE_RECOVERY));

    // Timeout: se não encontrar linha em tempo razoável
    if (millis() - stateChangeTime > scaleTime(3000)) {
        transitionToState(STATE_NAVIGATE);
    }
}

void RobotStateMachine::handleLineSearch() {
    // Girar para procurar linha
    MotorController::Direction turnDir =
        (lastTurnDirection == TURN_RIGHT) ? MotorController::TURN_LEFT : MotorController::TURN_RIGHT;

    motor->move(turnDir, scaleSpeed(SPEED_CURVE_RECOVERY));

    LineSensor::SensorState sensorState = lineSensor->readSensors();

    // Se encontrou linha
    if (sensorState.isValid && sensorState.rawPattern != 0) {
        motor->stop();
        delay(scaleTime(200));
        transitionToState(STATE_NAVIGATE);
        lineSearchRotations = 0;
        return;
    }

    // Timeout de busca
    if (millis() - stateChangeTime > scaleTime(LINE_SEARCH_TIMEOUT)) {
        lineSearchRotations++;

        if (lineSearchRotations >= LINE_SEARCH_MAX_ROTATIONS) {
            // Desistir após N rotações
            transitionToState(STATE_EMERGENCY_STOP);
            return;
        }

        // Continuar rotando
        stateChangeTime = millis();
    }
}

void RobotStateMachine::handleEmergencyStop() {
    motor->stop();
    gripper->open();
    running = false;

    if (DEBUG_MODE) {
        Serial.println(F("=== PARADA DE EMERGÊNCIA ==="));
        printStatistics();
    }
}

// ============================================================================
// VALIDADORES DE TRANSIÇÃO
// ============================================================================

bool RobotStateMachine::shouldDetectObject() {
    // Não detectar durante curva acentuada (sensores 5 ou 6 ativos)
    LineSensor::SensorState sensorState = lineSensor->readSensors();
    
    if (sensorState.sensors[4] || sensorState.sensors[5]) {
        return false;  // Curva extrema - ignorar ultrassônico
    }

    // Verificar tempo mínimo desde última detecção
    if (millis() - lastObjectTime < ULTRASONIC_DEBOUNCE_TIME) {
        return false;
    }

    return true;
}

bool RobotStateMachine::shouldReturnToNavigate() {
    return lineSensor->isLineDetected();
}

bool RobotStateMachine::shouldSearchLine() {
    return !lineSensor->isLineDetected();
}

// ============================================================================
// ALGORITMO ROUND ROBIN
// ============================================================================

RobotStateMachine::TurnDirection RobotStateMachine::getNextTurnDirection() {
    // Alternar entre ESQUERDA e DIREITA a cada cruzamento
    TurnDirection next = (lastTurnDirection == TURN_LEFT) ? TURN_RIGHT : TURN_LEFT;
    lastTurnDirection = next;
    saveTurnDirection();
    return next;
}

void RobotStateMachine::saveTurnDirection() {
    EEPROM.write(EEPROM_ADDR_LAST_TURN, (uint8_t)lastTurnDirection);
}

void RobotStateMachine::loadTurnDirection() {
    uint8_t saved = EEPROM.read(EEPROM_ADDR_LAST_TURN);
    if (saved == TURN_LEFT || saved == TURN_RIGHT) {
        lastTurnDirection = (TurnDirection)saved;
    } else {
        lastTurnDirection = TURN_RIGHT;  // Padrão: começar com direita
    }
}

// ============================================================================
// ESCALAÇÃO COM VELOCIDADE GLOBAL
// ============================================================================

uint16_t RobotStateMachine::scaleTime(uint16_t timeAtBaseSpeed) const {
    // Fórmula: Tempo_Ação = K / velocidade_global
    // Onde K = constante calibrada para VELOCITY_GLOBAL = 255
    
    if (VELOCITY_GLOBAL == 0) return timeAtBaseSpeed;
    
    uint16_t scaledTime = (uint16_t)((timeAtBaseSpeed * TIME_SCALE_FACTOR) / VELOCITY_GLOBAL);
    return scaledTime;
}

uint8_t RobotStateMachine::scaleSpeed(uint8_t speedAtBaseVelocity) const {
    // Escalar PWM proportional à velocidade global
    uint16_t scaledSpeed = ((uint16_t)speedAtBaseVelocity * VELOCITY_GLOBAL) / 255;
    return (uint8_t)constrain(scaledSpeed, PWM_MIN_DEADZONE, 255);
}

// ============================================================================
// CONTROLE E UTILITÁRIOS
// ============================================================================

void RobotStateMachine::transitionToState(RobotState newState) {
    if (currentState == newState) return;

    previousState = currentState;
    currentState = newState;
    stateChangeTime = millis();

    if (DEBUG_MODE) {
        Serial.print("Estado: "));
        Serial.print(previousState);
        Serial.print(" -> "));
        Serial.println(F(currentState);
    }
}

void RobotStateMachine::stop() {
    motor->stop();
    running = false;
    transitionToState(STATE_IDLE);
}

void RobotStateMachine::restart() {
    initialize();
}

void RobotStateMachine::printState() const {
    if (!DEBUG_MODE) return;

    Serial.print("Estado atual: "));
    Serial.println(F((int)currentState);
}

void RobotStateMachine::printStatistics() const {
    if (!DEBUG_MODE) return;

    Serial.println(F("\n=== ESTATÍSTICAS FINAIS ==="));
    Serial.print("Tempo de execução: "));
    Serial.print(getElapsedTime());
    Serial.println(F(" ms"));
    Serial.print("Objetos coletados: "));
    Serial.println(F(objectsCollected);
    Serial.print("Estado final: "));
    Serial.println(F((int)currentState);
}

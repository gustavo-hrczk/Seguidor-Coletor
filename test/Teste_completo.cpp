// ============================================================================
// main.cpp — Seguidor de Linha + Coletor Autônomo
//
// Máquina de quatro estados:
//   STATE_FOLLOWING  → segue linha com PD, detecta objeto
//   STATE_COLLECTING → coleta e manobra (bloqueante, sensores ignorados)
//   STATE_RECOVERING → busca linha perdida em dois estágios
//   STATE_STOPPED    → aguarda intervenção manual
//
// Velocidades: todas derivadas de BASE_SPEED (config.h seção 3)
// Trim:        MOTOR_TRIM_ESQ / MOTOR_TRIM_DIR (config.h seção 2)
// ============================================================================

#include <Arduino.h>
#include "config.h"
#include "LineSensor.h"
#include "MotorController.h"
#include "UltrasonicSensor.h"
#include "GripperServo.h"

// ── Estados ──────────────────────────────────────────────────────────────────
enum RobotState {
    STATE_FOLLOWING,
    STATE_COLLECTING,
    STATE_RECOVERING,
    STATE_STOPPED
};

// ── Instâncias globais ────────────────────────────────────────────────────────
LineSensor       lineSensor;
MotorController  motor;
UltrasonicSensor ultrasonic;
GripperServo     gripper;

// ── Variáveis de estado ───────────────────────────────────────────────────────
RobotState    robotState    = STATE_FOLLOWING;
unsigned long recoveryStart = 0;
unsigned long programStart  = 0;
bool          maneuverLeft  = true;
int           collectCount  = 0;

// ── Protótipos ────────────────────────────────────────────────────────────────
void handleFollowing();
void handleCollecting();
void handleRecovering();
void logTransition(const __FlashStringHelper* from, const __FlashStringHelper* to);
void printFollowStatus(float pos, uint8_t spd, LineSensor::LinePattern pat);

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    Serial.begin(BAUD_RATE);
    delay(500);

    lineSensor.initialize();
    motor.initialize();
    ultrasonic.initialize();
    gripper.initialize();

    motor.stop();
    gripper.open();

    // Banner de inicialização com parâmetros ativos
    Serial.println(F("\n╔══════════════════════════════════════════╗"));
    Serial.println(F("║     SEGUIDOR COLETOR — PROGRAMA FINAL    ║"));
    Serial.println(F("╠══════════════════════════════════════════╣"));
    Serial.print  (F("║  BASE_SPEED="));  Serial.print(BASE_SPEED);
    Serial.print  (F("  Kp="));           Serial.print(PD_KP, 1);
    Serial.print  (F("  Kd="));           Serial.println(PD_KD, 1);
    Serial.print  (F("║  TrimESQ="));     Serial.print(MOTOR_TRIM_ESQ, 2);
    Serial.print  (F("  TrimDIR="));      Serial.println(MOTOR_TRIM_DIR, 2);
    Serial.print  (F("║  Threshold="));   Serial.print(THRESHOLD_LINE_SENSOR);
    Serial.print  (F("  Contato="));      Serial.print(ULTRASONIC_DISTANCE_CONTACT);
    Serial.println(F("cm"));
    Serial.println(F("╠══════════════════════════════════════════╣"));
    Serial.println(F("║  Posicione o robo sobre a linha.         ║"));
    Serial.println(F("║  Iniciando em 3 segundos...              ║"));
    Serial.println(F("╚══════════════════════════════════════════╝"));

    delay(3000);

    programStart = millis();
    motor.resetPD();
    lineSensor.resetLastDirection();
    robotState = STATE_FOLLOWING;

    Serial.println(F("[Main] INICIADO"));
}

// ============================================================================
// LOOP
// ============================================================================
void loop() {
    switch (robotState) {
        case STATE_FOLLOWING:  handleFollowing();  break;
        case STATE_COLLECTING: handleCollecting(); break;
        case STATE_RECOVERING: handleRecovering(); break;
        case STATE_STOPPED:    motor.stop();       break;
    }
}

// ============================================================================
// STATE_FOLLOWING
//
// Velocidade única BASE_SPEED para todo o seguimento normal.
// Reduz apenas quando o ultrassônico detecta objeto próximo,
// preparando a parada antes da coleta.
// ============================================================================
void handleFollowing() {
    lineSensor.readSensors();
    float                   pos     = lineSensor.getLinePosition();
    LineSensor::LinePattern pattern = lineSensor.getLinePattern();

    int dist = ultrasonic.readDistance();
    UltrasonicSensor::ApproachPhase fase = ultrasonic.getApproachPhase();

    // Objeto na zona de contato → coleta
    if (ultrasonic.isReadingStable() && dist > 0 && dist <= ULTRASONIC_DISTANCE_CONTACT) {
        motor.stop();
        logTransition(F("FOLLOWING"), F("COLLECTING"));
        robotState = STATE_COLLECTING;
        return;
    }

    // Linha perdida → recuperação
    if (pattern == LineSensor::LINE_LOST) {
        motor.stop();
        recoveryStart = millis();
        logTransition(F("FOLLOWING"), F("RECOVERING"));
        robotState = STATE_RECOVERING;
        return;
    }

    // Cruzamento central do 8 → passa reto levemente mais devagar
    if (pattern == LineSensor::INTERSECTION) {
        motor.move(MotorController::FORWARD, SPEED_INTERSECTION);
        delay(PD_SAMPLE_MS);
        return;
    }

    // Seleciona velocidade base pela fase do ultrassônico.
    // No seguimento normal usa BASE_SPEED puro — sem degraus por padrão de linha,
    // o que elimina o "coice" ao mudar de SPEED_ERROR_LOW para MEDIUM.
    uint8_t baseSpeed;
    switch (fase) {
        case UltrasonicSensor::PHASE_2_APPROACHING:
            baseSpeed = APPROACH_SPEED_MEDIUM;  // objeto entre 20–25cm
            break;
        case UltrasonicSensor::PHASE_3_CONTACT:
            baseSpeed = APPROACH_SPEED_SLOW;    // objeto entre 4–20cm
            break;
        default:
            baseSpeed = BASE_SPEED;             // sem objeto — velocidade plena
            break;
    }

    motor.followLine(pos, baseSpeed);
    printFollowStatus(pos, baseSpeed, pattern);
    delay(PD_SAMPLE_MS);
}

// ============================================================================
// STATE_COLLECTING
// Bloqueante — sensores ignorados durante toda a manobra.
// ============================================================================
void handleCollecting() {
    collectCount++;
    Serial.print(F("\n[Coleta #")); Serial.print(collectCount);
    Serial.println(maneuverLeft ? F("] Lado ESQUERDO") : F("] Lado DIREITO"));

    gripper.close();
    delay(COLLECT_STOP_DELAY);

    if (maneuverLeft) {
        // COM CARGA
        motor.move(MotorController::TURN_LEFT, MANEUVER_SPEED_LOADED);
        delay(TURN_90_LOADED_MS);
        motor.stop(); delay(50);

        motor.move(MotorController::FORWARD, MANEUVER_SPEED_LOADED);
        delay(STRAFE_LOADED_MS);
        motor.stop(); delay(100);

        gripper.open(); delay(200);

        // SEM CARGA — retorno
        motor.move(MotorController::BACKWARD, MANEUVER_SPEED_UNLOADED);
        delay(STRAFE_UNLOADED_MS);
        motor.stop(); delay(50);

        motor.move(MotorController::TURN_RIGHT, MANEUVER_SPEED_UNLOADED);
        delay(TURN_90_UNLOADED_MS);
        motor.stop();

    } else {
        // COM CARGA
        motor.move(MotorController::TURN_RIGHT, MANEUVER_SPEED_LOADED);
        delay(TURN_90_LOADED_MS);
        motor.stop(); delay(50);

        motor.move(MotorController::FORWARD, MANEUVER_SPEED_LOADED);
        delay(STRAFE_LOADED_MS);
        motor.stop(); delay(100);

        gripper.open(); delay(200);

        // SEM CARGA — retorno
        motor.move(MotorController::BACKWARD, MANEUVER_SPEED_UNLOADED);
        delay(STRAFE_UNLOADED_MS);
        motor.stop(); delay(50);

        motor.move(MotorController::TURN_LEFT, MANEUVER_SPEED_UNLOADED);
        delay(TURN_90_UNLOADED_MS);
        motor.stop();
    }

    // Prepara próximo ciclo
    maneuverLeft = !maneuverLeft;
    ultrasonic.resetValidation();
    motor.resetPD();
    lineSensor.resetLastDirection();
    delay(300);

    logTransition(F("COLLECTING"), F("FOLLOWING"));
    robotState = STATE_FOLLOWING;
}

// ============================================================================
// STATE_RECOVERING
// Estágio 1: gira na última direção conhecida por RECOVERY_SPIN_MS
// Estágio 2: gira na direção oposta até RECOVERY_TIMEOUT_MS
// Timeout: STATE_STOPPED
// ============================================================================
void handleRecovering() {
    unsigned long elapsed = millis() - recoveryStart;

    lineSensor.readSensors();
    if (lineSensor.getLinePattern() != LineSensor::LINE_LOST) {
        motor.stop();
        motor.resetPD();
        lineSensor.resetLastDirection();
        logTransition(F("RECOVERING"), F("FOLLOWING"));
        robotState = STATE_FOLLOWING;
        return;
    }

    bool lastWasRight = (lineSensor.getLastDirection() == LineSensor::DIR_RIGHT);

    if (elapsed < RECOVERY_SPIN_MS) {
        motor.move(lastWasRight
            ? MotorController::TURN_RIGHT
            : MotorController::TURN_LEFT,  PWM_SLOW);
        return;
    }

    if (elapsed < RECOVERY_TIMEOUT_MS) {
        motor.move(lastWasRight
            ? MotorController::TURN_LEFT
            : MotorController::TURN_RIGHT, PWM_SLOW);
        return;
    }

    motor.stop();
    logTransition(F("RECOVERING"), F("STOPPED"));
    Serial.println(F("[Main] TIMEOUT — reposicione e reinicie"));
    robotState = STATE_STOPPED;
}

// ============================================================================
// Utilitários de log
// ============================================================================
void logTransition(const __FlashStringHelper* from, const __FlashStringHelper* to) {
    if (!DEBUG_MODE) return;
    Serial.print(F("["));
    Serial.print((millis() - programStart) / 1000);
    Serial.print(F("s] "));
    Serial.print(from); Serial.print(F(" -> ")); Serial.println(to);
}

void printFollowStatus(float pos, uint8_t spd, LineSensor::LinePattern pat) {
    if (!DEBUG_MODE) return;
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint < 300) return;
    lastPrint = millis();

    const __FlashStringHelper* label;
    switch (pat) {
        case LineSensor::STRAIGHT:     label = F("RETA");    break;
        case LineSensor::CURVE_LIGHT:  label = F("C-SUAVE"); break;
        case LineSensor::CURVE_MEDIUM: label = F("C-MEDIA"); break;
        case LineSensor::CURVE_SHARP:  label = F("C-AGUDA"); break;
        default:                       label = F("?");       break;
    }

    char bar[22];
    int idx = constrain((int)((pos + 1.0f) * 10.0f), 0, 20);
    for (int i = 0; i < 21; i++) bar[i] = (i == 10) ? '|' : '-';
    bar[idx] = '#'; bar[21] = '\0';

    int dist = ultrasonic.getLastValidDistance();

    Serial.print(F("["));     Serial.print(bar);   Serial.print(F("] "));
    Serial.print(pos, 2);
    Serial.print(F(" | "));   Serial.print(label);
    Serial.print(F(" | spd=")); Serial.print(spd);
    Serial.print(F(" | dist=")); Serial.print(dist > 0 ? dist : -1);
    Serial.println(F("cm"));
}

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
    // Força o robô a andar em linha reta usando a BASE_SPEED unificada
    motor.setMotorSpeed(BASE_SPEED, BASE_SPEED);
    delay(2000);
    motor.stop();
    while(true); // Trava o robô após 5 segundos
}
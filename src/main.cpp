// ============================================================================
// test_collect.cpp — Ciclo autônomo de coleta sem seguidor de linha
//
// Fluxo de cada ciclo:
//   1. SEARCHING   — avança com velocidade adaptativa até objeto na zona de contato
//   2. COLLECTING  — para e fecha a garra
//   3. MANEUVERING — gira 90° e avança lateralmente (alternando esq/dir)
//   4. RELEASING   — abre a garra
//   5. RETURNING   — recua e gira de volta para o eixo original
//   Repete até COLLECT_CYCLE_DURATION ms
//
// Módulos ativos: MotorController, UltrasonicSensor, GripperServo
// Sensores ignorados durante manobra — não há leitura entre COLLECTING e RETURNING
// ============================================================================

#include <Arduino.h>
#include "config.h"
#include "MotorController.h"
#include "UltrasonicSensor.h"
#include "GripperServo.h"

// ============================================================================
// Função auxiliar — log de etapa com timestamp relativo ao início do ciclo
// ============================================================================
static void logStep(unsigned long startTime, const __FlashStringHelper* msg) {
    if (!DEBUG_MODE) return;
    Serial.print(F("["));
    Serial.print((millis() - startTime) / 1000);
    Serial.print(F("s] "));
    Serial.println(msg);
}

// ============================================================================
// testCollector()
// Executa o ciclo completo de coleta por COLLECT_CYCLE_DURATION ms.
// Todos os módulos são instanciados aqui — setup() não precisa recriar.
// ============================================================================
void testCollector() {
    MotorController  motor;
    UltrasonicSensor ultrasonic;
    GripperServo     gripper;

    motor.initialize();
    ultrasonic.initialize();
    gripper.initialize();
    gripper.open();

    bool          sideLeft  = true;   // alterna esq/dir a cada ciclo
    int           cycleNum  = 0;
    unsigned long startTime = millis();

    Serial.println(F("\n=== INICIO DO CICLO DE COLETA ==="));
    Serial.print(F("Duracao: ")); Serial.print(COLLECT_CYCLE_DURATION / 1000);
    Serial.println(F("s"));

    while (millis() - startTime < COLLECT_CYCLE_DURATION) {

        // ──────────────────────────────────────────────────────────────
        // FASE 1: SEARCHING — aproximação com velocidade adaptativa
        // Velocidade reduz conforme o robô se aproxima, minimizando inércia
        // ao entrar na zona de contato.
        // Sensores ativos nesta fase.
        // ──────────────────────────────────────────────────────────────
        logStep(startTime, F("[SEARCHING] Buscando objeto..."));
        bool objetoDetectado = false;

        while (!objetoDetectado && (millis() - startTime < COLLECT_CYCLE_DURATION)) {
            int dist = ultrasonic.readDistance();

            // Seleciona velocidade pela distância atual
            uint8_t spd;
            if (dist <= 0 || !ultrasonic.isReadingStable()) {
                spd = APPROACH_SPEED_FAST;          // sem leitura → avança rápido
            } else if (dist <= APPROACH_DIST_MEDIUM) {
                spd = APPROACH_SPEED_SLOW;          // zona crítica — mínimo de inércia
            } else if (dist <= APPROACH_DIST_LONG) {
                spd = APPROACH_SPEED_MEDIUM;        // zona média — desacelerando
            } else {
                spd = APPROACH_SPEED_FAST;          // longe — velocidade plena
            }
            motor.move(MotorController::FORWARD, spd);

            // Gatilho de coleta: leitura estável dentro da zona de contato
            if (dist > 0
                && dist <= ULTRASONIC_DISTANCE_CONTACT
                && ultrasonic.isReadingStable()) {
                objetoDetectado = true;
            }

            if (DEBUG_MODE) {
                static unsigned long lastPrint = 0;
                if (millis() - lastPrint >= 200) {
                    lastPrint = millis();
                    Serial.print(F("  dist=")); Serial.print(dist);
                    Serial.print(F("cm | vel=")); Serial.print(spd);
                    Serial.print(F(" | estavel="));
                    Serial.println(ultrasonic.isReadingStable() ? F("S") : F("N"));
                }
            }

            delay(15);   // intervalo curto — mantém leitura responsiva
        }

        if (millis() - startTime >= COLLECT_CYCLE_DURATION) break;

        // ──────────────────────────────────────────────────────────────
        // FASE 2: COLLECTING — para e fecha a garra
        // A partir daqui os sensores são ignorados até RETURNING concluir.
        // ──────────────────────────────────────────────────────────────
        motor.stop();
        delay(COLLECT_STOP_DELAY);   // estabiliza antes de fechar

        logStep(startTime, F("[COLLECTING] Fechando garra..."));
        gripper.close();
        delay(COLLECT_STOP_DELAY);

        // ──────────────────────────────────────────────────────────────
        // FASE 3 + 4 + 5: MANOBRA → SOLTAR → RETORNO
        // Sensores ignorados durante todo este bloco.
        // Lados alternam a cada ciclo para distribuir objetos.
        // ──────────────────────────────────────────────────────────────
        if (sideLeft) {

            // — COM CARGA (garra fechada) —
            logStep(startTime, F("[MANOBRA ESQ] Girando esquerda (carregado)"));
            motor.move(MotorController::TURN_LEFT, MANEUVER_SPEED_LOADED);
            delay(TURN_90_LOADED_MS);
            motor.stop();
            delay(50);

            logStep(startTime, F("[MANOBRA ESQ] Avancando lateral (carregado)"));
            motor.move(MotorController::FORWARD, MANEUVER_SPEED_LOADED);
            delay(STRAFE_LOADED_MS);
            motor.stop();
            delay(100);

            // — SOLTA O OBJETO —
            logStep(startTime, F("[RELEASING] Abrindo garra"));
            gripper.open();
            delay(200);

            // — SEM CARGA (garra aberta) —
            logStep(startTime, F("[RETURNING ESQ] Re lateral (vazio)"));
            motor.move(MotorController::BACKWARD, MANEUVER_SPEED_UNLOADED);
            delay(STRAFE_UNLOADED_MS);
            motor.stop();
            delay(50);

            logStep(startTime, F("[RETURNING ESQ] Girando direita (vazio)"));
            motor.move(MotorController::TURN_RIGHT, MANEUVER_SPEED_UNLOADED);
            delay(TURN_90_UNLOADED_MS);
            motor.stop();

        } else {

            // — COM CARGA (garra fechada) —
            logStep(startTime, F("[MANOBRA DIR] Girando direita (carregado)"));
            motor.move(MotorController::TURN_RIGHT, MANEUVER_SPEED_LOADED);
            delay(TURN_90_LOADED_MS);
            motor.stop();
            delay(50);

            logStep(startTime, F("[MANOBRA DIR] Avancando lateral (carregado)"));
            motor.move(MotorController::FORWARD, MANEUVER_SPEED_LOADED);
            delay(STRAFE_LOADED_MS);
            motor.stop();
            delay(100);

            // — SOLTA O OBJETO —
            logStep(startTime, F("[RELEASING] Abrindo garra"));
            gripper.open();
            delay(200);

            // — SEM CARGA (garra aberta) —
            logStep(startTime, F("[RETURNING DIR] Re lateral (vazio)"));
            motor.move(MotorController::BACKWARD, MANEUVER_SPEED_UNLOADED);
            delay(STRAFE_UNLOADED_MS);
            motor.stop();
            delay(50);

            logStep(startTime, F("[RETURNING DIR] Girando esquerda (vazio)"));
            motor.move(MotorController::TURN_LEFT, MANEUVER_SPEED_UNLOADED);
            delay(TURN_90_UNLOADED_MS);
            motor.stop();
        }

        // ──────────────────────────────────────────────────────────────
        // FIM DO CICLO — prepara o próximo
        // ──────────────────────────────────────────────────────────────
        cycleNum++;
        sideLeft = !sideLeft;
        ultrasonic.resetValidation();   // leitura limpa para o próximo ciclo

        delay(300);   // pausa antes de retomar busca

        if (DEBUG_MODE) {
            Serial.print(F("[CICLO "));
            Serial.print(cycleNum);
            Serial.print(F(" concluido] Proximo lado: "));
            Serial.println(sideLeft ? F("ESQUERDA") : F("DIREITA"));
        }
    }

    motor.stop();
    gripper.open();
    Serial.print(F("\n✓ Coleta encerrada — "));
    Serial.print(cycleNum);
    Serial.println(F(" ciclos completados"));
}

// ============================================================================
// setup()
// Inicializa serial e executa o programa de coleta uma única vez.
// loop() permanece vazio — o programa termina com while(1).
// ============================================================================
void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Serial.println(F("Sistema inicializado."));
    Serial.println(F("Iniciando em 3 segundos..."));
    delay(3000);

    testCollector();

    Serial.println(F("Fim do programa."));
    while (1);   // trava após concluir — evita reinício acidental
}

void loop() {
    // vazio — programa roda inteiramente em setup() via testCollector()
}

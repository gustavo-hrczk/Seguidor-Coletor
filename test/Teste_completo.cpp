// ============================================================================
// main.cpp — Programa principal: Seguidor de Linha + Coletor Autônomo
//
// Máquina de estados principal:
//
//   STATE_FOLLOWING   → segue a linha com controlador PD
//                       detecta objeto via ultrassônico durante o trajeto
//                       transição para STATE_COLLECTING quando objeto na zona
//
//   STATE_COLLECTING  → para, fecha garra, executa manobra lateral,
//                       solta objeto, retorna ao eixo original
//                       sensores de linha e ultrassônico ignorados neste estado
//
//   STATE_RECOVERING  → linha perdida durante seguimento
//                       tenta recuperar girando na última direção conhecida
//                       transição para STATE_FOLLOWING ao reencontrar linha
//                       transição para STATE_STOPPED após timeout
//
//   STATE_STOPPED     → robô parado aguardando intervenção manual
//
// Módulos utilizados:
//   LineSensor       — leitura e posição da linha (sem filtro, reatividade máxima)
//   MotorController  — controle PD dos motores
//   UltrasonicSensor — detecção e validação de objeto por aproximação
//   GripperServo     — controle da garra com movimento suave e detach()
//
// Configuração: todos os parâmetros em config.h
// Debug:        DEBUG_MODE true → logs no Serial a 9600 baud
// ============================================================================

#include <Arduino.h>
#include "config.h"
#include "LineSensor.h"
#include "MotorController.h"
#include "UltrasonicSensor.h"
#include "GripperServo.h"

// ============================================================================
// ESTADOS DA MÁQUINA PRINCIPAL
// ============================================================================
enum RobotState {
    STATE_FOLLOWING,    // seguindo a linha normalmente
    STATE_COLLECTING,   // executando ciclo de coleta
    STATE_RECOVERING,   // buscando linha perdida
    STATE_STOPPED       // parado por timeout ou erro
};

// ============================================================================
// INSTÂNCIAS GLOBAIS
// ============================================================================
LineSensor       lineSensor;
MotorController  motor;
UltrasonicSensor ultrasonic;
GripperServo     gripper;

// ============================================================================
// VARIÁVEIS DE ESTADO
// ============================================================================
RobotState    robotState    = STATE_FOLLOWING;
unsigned long recoveryStart = 0;      // timestamp início da recuperação
unsigned long programStart  = 0;      // timestamp início do programa
bool          maneuverLeft  = true;   // alterna direção da manobra a cada coleta
int           collectCount  = 0;      // contador de coletas realizadas

// ============================================================================
// PROTÓTIPOS
// ============================================================================
void handleFollowing();
void handleCollecting();
void handleRecovering();
void logTransition(const __FlashStringHelper* from, const __FlashStringHelper* to);
void printFollowStatus(float pos, uint8_t baseSpeed, LineSensor::LinePattern pattern);

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

    Serial.println(F("\n╔══════════════════════════════════════════╗"));
    Serial.println(F("║     SEGUIDOR COLETOR — PROGRAMA FINAL    ║"));
    Serial.println(F("╠══════════════════════════════════════════╣"));
    Serial.print  (F("║  Kp="));          Serial.print(PD_KP, 2);
    Serial.print  (F("  Kd="));           Serial.print(PD_KD, 2);
    Serial.print  (F("  threshold="));    Serial.print(THRESHOLD_LINE_SENSOR);
    Serial.println(F("       ║"));
    Serial.print  (F("║  Contato: <="));  Serial.print(ULTRASONIC_DISTANCE_CONTACT);
    Serial.print  (F("cm  Longo: <="));   Serial.print(ULTRASONIC_DISTANCE_LONG);
    Serial.println(F("cm              ║"));
    Serial.println(F("╠══════════════════════════════════════════╣"));
    Serial.println(F("║  Posicionando sobre a linha...           ║"));
    Serial.println(F("║  Iniciando em 3 segundos.                ║"));
    Serial.println(F("╚══════════════════════════════════════════╝"));

    delay(3000);

    programStart = millis();
    motor.resetPD();
    lineSensor.resetLastDirection();
    robotState = STATE_FOLLOWING;

    Serial.println(F("[Main] INICIADO — STATE_FOLLOWING"));
}

// ============================================================================
// LOOP
// ============================================================================
void loop() {
    switch (robotState) {
        case STATE_FOLLOWING:  handleFollowing();  break;
        case STATE_COLLECTING: handleCollecting(); break;
        case STATE_RECOVERING: handleRecovering(); break;

        case STATE_STOPPED:
            motor.stop();
            // Aguarda sem fazer nada — intervenção manual necessária
            break;
    }
}

// ============================================================================
// STATE_FOLLOWING — Seguimento de linha com detecção de objeto
//
// A cada ciclo:
//   1. Lê sensor de linha e calcula posição
//   2. Lê ultrassônico e verifica fase de aproximação
//   3. Se objeto na zona de contato → transição para STATE_COLLECTING
//   4. Se linha perdida → transição para STATE_RECOVERING
//   5. Caso normal → followLine() com baseSpeed conforme magnitude do erro
//
// O ultrassônico reduz a velocidade de seguimento quando o objeto está próximo,
// preparando o robô para a parada suave antes da coleta.
// ============================================================================
void handleFollowing() {
    lineSensor.readSensors();
    float                   pos     = lineSensor.getLinePosition();
    float                   absPos  = fabs(pos);
    LineSensor::LinePattern pattern = lineSensor.getLinePattern();

    // Leitura do ultrassônico — ocorre a cada ciclo durante o seguimento
    int dist = ultrasonic.readDistance();
    UltrasonicSensor::ApproachPhase fase = ultrasonic.getApproachPhase();

    // ── Detecção de objeto na zona de contato ──────────────────────────────
    if (ultrasonic.isReadingStable()
        && dist > 0
        && dist <= ULTRASONIC_DISTANCE_CONTACT) {

        motor.stop();
        logTransition(F("FOLLOWING"), F("COLLECTING"));
        robotState = STATE_COLLECTING;
        return;
    }

    // ── Linha perdida → recuperação ────────────────────────────────────────
    if (pattern == LineSensor::LINE_LOST) {
        motor.stop();
        recoveryStart = millis();
        logTransition(F("FOLLOWING"), F("RECOVERING"));
        Serial.print(F("[Recovering] Ultima dir="));
        Serial.println(lineSensor.getLastDirection() == LineSensor::DIR_LEFT
                       ? F("ESQ") : F("DIR"));
        robotState = STATE_RECOVERING;
        return;
    }

    // ── Cruzamentos: passa reto em velocidade baixa ────────────────────────
    if (pattern == LineSensor::INTERSECTION  ||
        pattern == LineSensor::TURN_LEFT_90  ||
        pattern == LineSensor::TURN_RIGHT_90) {
        motor.move(MotorController::FORWARD, SPEED_ERROR_LOW);
        delay(PD_SAMPLE_MS);
        return;
    }

    // ── Velocidade base pela magnitude do erro e fase do ultrassônico ───────
    // Objeto próximo reduz velocidade máxima mesmo em reta — prepara a parada
    uint8_t baseSpeed;
    if (fase == UltrasonicSensor::PHASE_2_APPROACHING) {
        // Objeto entre LONG e SHORT: limita velocidade máxima
        baseSpeed = (absPos < 0.3f) ? APPROACH_SPEED_MEDIUM : SPEED_ERROR_HIGH;
    } else if (fase == UltrasonicSensor::PHASE_3_CONTACT) {
        // Objeto muito próximo mas ainda não na zona de gatilho
        baseSpeed = APPROACH_SPEED_SLOW;
    } else {
        // Sem objeto próximo: velocidade normal pelo erro da linha
        if      (absPos < 0.3f) baseSpeed = SPEED_ERROR_LOW;
        else if (absPos < 0.6f) baseSpeed = SPEED_ERROR_MEDIUM;
        else                    baseSpeed = SPEED_ERROR_HIGH;
    }

    motor.followLine(pos, baseSpeed);

    printFollowStatus(pos, baseSpeed, pattern);
    delay(PD_SAMPLE_MS);
}

// ============================================================================
// STATE_COLLECTING — Ciclo completo de coleta
//
// Sequência bloqueante — sensores de linha e ultrassônico são ignorados
// durante toda a execução para evitar interferência nos movimentos.
//
// Fluxo:
//   1. Fecha garra
//   2. Gira 90° para o lado (alternando esq/dir)
//   3. Avança deslocando objeto lateralmente
//   4. Abre garra — solta objeto
//   5. Recua
//   6. Gira 90° de volta — retorna ao eixo original
//   7. Reseta sensores e retoma seguimento
// ============================================================================
void handleCollecting() {
    collectCount++;

    Serial.print(F("\n[Collecting] Ciclo #")); Serial.println(collectCount);
    Serial.println(F("[Collecting] Fechando garra..."));

    // Fecha a garra para segurar o objeto
    gripper.close();
    delay(COLLECT_STOP_DELAY);

    // ── Manobra lateral (direção alterna a cada ciclo) ─────────────────────
    if (maneuverLeft) {

        // COM CARGA — gira esquerda e avança
        Serial.println(F("[Manobra ESQ] Girando esquerda (carregado)"));
        motor.move(MotorController::TURN_LEFT, MANEUVER_SPEED_LOADED);
        delay(TURN_90_LOADED_MS);
        motor.stop();
        delay(50);

        Serial.println(F("[Manobra ESQ] Avancando lateral"));
        motor.move(MotorController::FORWARD, MANEUVER_SPEED_LOADED);
        delay(STRAFE_LOADED_MS);
        motor.stop();
        delay(100);

        // SOLTA o objeto
        Serial.println(F("[Releasing] Abrindo garra"));
        gripper.open();
        delay(200);

        // SEM CARGA — recua e gira de volta
        Serial.println(F("[Returning ESQ] Re lateral"));
        motor.move(MotorController::BACKWARD, MANEUVER_SPEED_UNLOADED);
        delay(STRAFE_UNLOADED_MS);
        motor.stop();
        delay(50);

        Serial.println(F("[Returning ESQ] Girando direita (retorno)"));
        motor.move(MotorController::TURN_RIGHT, MANEUVER_SPEED_UNLOADED);
        delay(TURN_90_UNLOADED_MS);
        motor.stop();

    } else {

        // COM CARGA — gira direita e avança
        Serial.println(F("[Manobra DIR] Girando direita (carregado)"));
        motor.move(MotorController::TURN_RIGHT, MANEUVER_SPEED_LOADED);
        delay(TURN_90_LOADED_MS);
        motor.stop();
        delay(50);

        Serial.println(F("[Manobra DIR] Avancando lateral"));
        motor.move(MotorController::FORWARD, MANEUVER_SPEED_LOADED);
        delay(STRAFE_LOADED_MS);
        motor.stop();
        delay(100);

        // SOLTA o objeto
        Serial.println(F("[Releasing] Abrindo garra"));
        gripper.open();
        delay(200);

        // SEM CARGA — recua e gira de volta
        Serial.println(F("[Returning DIR] Re lateral"));
        motor.move(MotorController::BACKWARD, MANEUVER_SPEED_UNLOADED);
        delay(STRAFE_UNLOADED_MS);
        motor.stop();
        delay(50);

        Serial.println(F("[Returning DIR] Girando esquerda (retorno)"));
        motor.move(MotorController::TURN_LEFT, MANEUVER_SPEED_UNLOADED);
        delay(TURN_90_UNLOADED_MS);
        motor.stop();
    }

    // ── Prepara o próximo ciclo ────────────────────────────────────────────
    maneuverLeft = !maneuverLeft;
    ultrasonic.resetValidation();   // leitura limpa — evita re-disparo imediato
    motor.resetPD();                // zera erro PD — evita spike derivativo na retomada
    lineSensor.resetLastDirection();

    delay(300);

    logTransition(F("COLLECTING"), F("FOLLOWING"));
    Serial.print(F("[Main] Proxima manobra: "));
    Serial.println(maneuverLeft ? F("ESQUERDA") : F("DIREITA"));

    robotState = STATE_FOLLOWING;
}

// ============================================================================
// STATE_RECOVERING — Recuperação de linha perdida
//
// Estágio 1 (0 → RECOVERY_SPIN_MS):
//   Gira na última direção conhecida antes de perder a linha
//
// Estágio 2 (RECOVERY_SPIN_MS → RECOVERY_TIMEOUT_MS):
//   Gira na direção oposta — tenta o outro lado
//
// Retorno ao seguimento:
//   Assim que qualquer sensor detectar a linha, para e retoma
//
// Timeout:
//   Se não encontrar a linha em RECOVERY_TIMEOUT_MS → STATE_STOPPED
// ============================================================================
void handleRecovering() {
    unsigned long elapsed = millis() - recoveryStart;

    // Verifica se a linha foi reencontrada a cada iteração
    lineSensor.readSensors();
    if (lineSensor.getLinePattern() != LineSensor::LINE_LOST) {
        motor.stop();
        motor.resetPD();
        lineSensor.resetLastDirection();
        logTransition(F("RECOVERING"), F("FOLLOWING"));
        robotState = STATE_FOLLOWING;
        return;
    }

    // Estágio 1: gira na última direção conhecida
    if (elapsed < RECOVERY_SPIN_MS) {
        if (lineSensor.getLastDirection() == LineSensor::DIR_RIGHT) {
            motor.move(MotorController::TURN_RIGHT, PWM_SLOW);
        } else {
            motor.move(MotorController::TURN_LEFT, PWM_SLOW);
        }
        return;
    }

    // Estágio 2: tenta a direção oposta
    if (elapsed < RECOVERY_TIMEOUT_MS) {
        if (lineSensor.getLastDirection() == LineSensor::DIR_RIGHT) {
            motor.move(MotorController::TURN_LEFT, PWM_SLOW);
        } else {
            motor.move(MotorController::TURN_RIGHT, PWM_SLOW);
        }
        return;
    }

    // Timeout — para e aguarda intervenção
    motor.stop();
    logTransition(F("RECOVERING"), F("STOPPED"));
    Serial.println(F("[Main] TIMEOUT — reposicione o robo sobre a linha e reinicie"));
    robotState = STATE_STOPPED;
}

// ============================================================================
// Utilitários de log
// ============================================================================

// Imprime transição de estado com timestamp
void logTransition(const __FlashStringHelper* from, const __FlashStringHelper* to) {
    if (!DEBUG_MODE) return;
    Serial.print(F("["));
    Serial.print((millis() - programStart) / 1000);
    Serial.print(F("s] "));
    Serial.print(from);
    Serial.print(F(" -> "));
    Serial.println(to);
}

// Imprime status do seguimento a 3Hz (não saturar o serial)
void printFollowStatus(float pos, uint8_t baseSpeed, LineSensor::LinePattern pattern) {
    if (!DEBUG_MODE) return;
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint < 300) return;
    lastPrint = millis();

    const __FlashStringHelper* rotPad;
    switch (pattern) {
        case LineSensor::STRAIGHT:     rotPad = F("RETA");    break;
        case LineSensor::CURVE_LIGHT:  rotPad = F("C-SUAVE"); break;
        case LineSensor::CURVE_MEDIUM: rotPad = F("C-MEDIA"); break;
        case LineSensor::CURVE_SHARP:  rotPad = F("C-AGUDA"); break;
        default:                       rotPad = F("?");       break;
    }

    // Barra visual de posição [-1.0 ---|--- +1.0]
    char bar[22];
    int  idx = constrain((int)((pos + 1.0f) * 10.0f), 0, 20);
    for (int i = 0; i < 21; i++) bar[i] = (i == 10) ? '|' : '-';
    bar[idx] = '#';
    bar[21]  = '\0';

    int dist = ultrasonic.getLastValidDistance();

    Serial.print(F("["));        Serial.print(bar);       Serial.print(F("] "));
    Serial.print(pos, 2);
    Serial.print(F(" | "));      Serial.print(rotPad);
    Serial.print(F(" | spd="));  Serial.print(baseSpeed);
    Serial.print(F(" | dist="));
    Serial.print(dist > 0 ? dist : -1);
    Serial.println(F("cm"));
}

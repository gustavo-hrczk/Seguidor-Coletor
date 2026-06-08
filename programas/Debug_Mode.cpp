// ============================================================================
// test_components.cpp — Testes isolados por módulo
//
// Objetivo: validar cada módulo individualmente antes de executar o main.cpp.
// Executar na ordem 6 → 2 → 1 → 3 → 4 → 5 na primeira configuração.
//
// Menu:
//   1 — Motores          (direções, velocidades, TURN e CURVE)
//   2 — Sensor de linha  (leitura, posição ponderada, padrões)
//   3 — Ultrassônico     (fases de aproximação, estabilidade)
//   4 — Servo garra      (abertura, fechamento, guards de estado)
//   5 — Coleta simulada  (sequência completa do STATE_COLLECTING sem linha)
//   6 — Calibrar threshold (procedimento de 2 etapas: linha → fundo)
// ============================================================================

#include <Arduino.h>
#include "config.h"
#include "MotorController.h"
#include "LineSensor.h"
#include "UltrasonicSensor.h"
#include "GripperServo.h"

// Constantes locais do teste 5 — não pertencem ao config.h pois são
// específicas do procedimento de teste, não do comportamento de produção
static const uint16_t TEST5_STABLE_MS = 400;    // ms contínuos na zona para disparar coleta
static const uint16_t TEST5_DURATION  = 30000;  // ms de duração do teste

// Protótipos
void printMenu();
void testMotors();
void testLineSensors();
void testUltrasonic();
void testGripper();
void testCollectSequence();
void calibrateThreshold();

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Serial.println(F("\n╔════════════════════════════════════════╗"));
    Serial.println(F("║    TESTES DE COMPONENTES               ║"));
    Serial.println(F("╠════════════════════════════════════════╣"));
    Serial.print  (F("║  BASE_SPEED="));  Serial.print(BASE_SPEED);
    Serial.print  (F("  Kp="));           Serial.print(PD_KP, 2);
    Serial.print  (F("  Ki="));           Serial.println(PD_KI, 3);
    Serial.print  (F("║  TrimESQ="));     Serial.print(MOTOR_TRIM_ESQ, 2);
    Serial.print  (F("  TrimDIR="));      Serial.println(MOTOR_TRIM_DIR, 2);
    Serial.print  (F("║  Threshold="));   Serial.print(THRESHOLD_LINE_SENSOR);
    Serial.print  (F("  Contato="));      Serial.print(ULTRASONIC_DISTANCE_CONTACT);
    Serial.println(F("cm"));
    Serial.println(F("╚════════════════════════════════════════╝"));

    printMenu();
}

void loop() {
    if (!Serial.available()) { delay(50); return; }

    char cmd = Serial.read();
    while (Serial.available()) Serial.read();   // descarta caracteres extras (ex: \r\n)

    Serial.println();
    switch (cmd - '0') {
        case 1: testMotors();          break;
        case 2: testLineSensors();     break;
        case 3: testUltrasonic();      break;
        case 4: testGripper();         break;
        case 5: testCollectSequence(); break;
        case 6: calibrateThreshold();  break;
        default:
            Serial.print(F("Opção '")); Serial.print(cmd);
            Serial.println(F("' inválida."));
    }

    Serial.println();
    printMenu();
}

// ============================================================================
// Utilitário interno
// ============================================================================
void printMenu() {
    Serial.println(F("\n1-Motores  2-LineSensor  3-Ultrasson  4-Garra  5-Coleta  6-Threshold"));
    Serial.println(F("Digite o número do teste:"));
}

// ============================================================================
// TESTE 1 — MOTORES
//
// Valida todas as direções com VELOCITY_GLOBAL e os dois modos de curva.
// Observar: o robô deve executar cada movimento sem desvio perceptível.
// Se desviar em FORWARD, ajustar MOTOR_TRIM_DIR/ESQ no config.h.
// ============================================================================
void testMotors() {
    Serial.println(F("=== TESTE 1: MOTORES ==="));
    Serial.print  (F("BASE_SPEED=")); Serial.print(BASE_SPEED);
    Serial.print  (F("  TrimESQ=")); Serial.print(MOTOR_TRIM_ESQ, 2);
    Serial.print  (F("  TrimDIR=")); Serial.println(MOTOR_TRIM_DIR, 2);

    MotorController motor;
    motor.initialize();

    // Direções básicas — 2s cada
    const uint8_t spd = VELOCITY_GLOBAL;

    Serial.println(F("\n-- Movimentos básicos (2s cada) --"));

    Serial.println(F("[1/4] FRENTE"));
    motor.move(MotorController::FORWARD, spd);    delay(2000); motor.stop(); delay(500);

    Serial.println(F("[2/4] RE"));
    motor.move(MotorController::BACKWARD, spd);   delay(2000); motor.stop(); delay(500);

    Serial.println(F("[3/4] GIRO ESQUERDA (eixo)"));
    motor.move(MotorController::TURN_LEFT, spd);  delay(2000); motor.stop(); delay(500);

    Serial.println(F("[4/4] GIRO DIREITA (eixo)"));
    motor.move(MotorController::TURN_RIGHT, spd); delay(2000); motor.stop(); delay(500);

    // Curvas suaves — 1.5s cada, 60% da velocidade
    const uint8_t spdCurve = (uint8_t)(BASE_SPEED * 0.6f);
    Serial.println(F("\n-- Curvas suaves / um motor (1.5s cada) --"));

    Serial.print(F("[1/2] ARCO ESQUERDA  spd=")); Serial.println(spdCurve);
    motor.move(MotorController::CURVE_LEFT, spdCurve);  delay(1500); motor.stop(); delay(300);

    Serial.print(F("[2/2] ARCO DIREITA   spd=")); Serial.println(spdCurve);
    motor.move(MotorController::CURVE_RIGHT, spdCurve); delay(1500); motor.stop(); delay(300);

    Serial.println(F("\n✓ Teste 1 concluído"));
}

// ============================================================================
// TESTE 2 — SENSOR DE LINHA
//
// Coleta leituras por 20s a 10 Hz.
// Mova o robô lentamente sobre a linha para verificar posição e padrões.
// Checklist pós-teste:
//   [ ] raw sobre linha < THRESHOLD?
//   [ ] raw sobre fundo > THRESHOLD?
//   [ ] pos ≈ 0.000 com S3+S4 ativos?
//   [ ] pos negativa ao desviar para esquerda?
//   [ ] pos positiva ao desviar para direita?
// ============================================================================
void testLineSensors() {
    Serial.println(F("=== TESTE 2: SENSOR DE LINHA ==="));
    Serial.print  (F("Threshold=")); Serial.println(THRESHOLD_LINE_SENSOR);
    Serial.println(F("Convencao: raw <= threshold = ATIVO (linha branca)"));
    Serial.println(F("Posicao:  -1.0=esq extremo  0.0=centro  +1.0=dir extremo"));
    Serial.println(F("Posicione o robo sobre a linha. Movimente por 20s."));
    delay(2000);

    LineSensor sensor;
    sensor.initialize();

    unsigned long start     = millis();
    unsigned long lastPrint = 0;

    while (millis() - start < 20000) {
        sensor.readSensors();

        if (millis() - lastPrint < 100) continue;
        lastPrint = millis();

        const __FlashStringHelper* patLabel;
        switch (sensor.getLinePattern()) {
            case LineSensor::STRAIGHT:     patLabel = F("RETA");        break;
            case LineSensor::CURVE_LIGHT:  patLabel = F("CURVA-SUAVE"); break;
            case LineSensor::CURVE_MEDIUM: patLabel = F("CURVA-MEDIA"); break;
            case LineSensor::CURVE_SHARP:  patLabel = F("CURVA-AGUDA"); break;
            case LineSensor::INTERSECTION: patLabel = F("CRUZAMENTO");  break;
            case LineSensor::LINE_LOST:    patLabel = F("!! PERDIDA");  break;
            default:                       patLabel = F("?");           break;
        }

        const __FlashStringHelper* dirLabel;
        switch (sensor.getLastDirection()) {
            case LineSensor::DIR_LEFT:   dirLabel = F("ESQ"); break;
            case LineSensor::DIR_RIGHT:  dirLabel = F("DIR"); break;
            default:                     dirLabel = F("CTR"); break;
        }

        sensor.printSensorValues();
        Serial.print(F("       pat="));  Serial.print(patLabel);
        Serial.print(F("  dir="));       Serial.println(dirLabel);
    }

    Serial.println(F("\n✓ Teste 2 concluído"));
}

// ============================================================================
// TESTE 3 — SENSOR ULTRASSÔNICO
//
// Leitura contínua por 20s com diagnóstico completo de validação.
// Campos:
//   bruta    = measureDistance() — pode ser espúria
//   val      = lastValidDistance — confirmada pela janela deslizante
//   estav    = isReadingStable() — true após SENSOR_FILTER_CYCLES consistentes
//   fase     = ApproachPhase atual
// ============================================================================
void testUltrasonic() {
    Serial.println(F("=== TESTE 3: SENSOR ULTRASSÔNICO ==="));
    Serial.print  (F("LONG="));    Serial.print(ULTRASONIC_DISTANCE_LONG);
    Serial.print  (F("cm  SHORT=")); Serial.print(ULTRASONIC_DISTANCE_SHORT);
    Serial.print  (F("cm  CONTATO=")); Serial.print(ULTRASONIC_DISTANCE_CONTACT);
    Serial.println(F("cm"));
    Serial.println(F("Aproxime um objeto. Lendo por 20s..."));
    delay(1500);

    UltrasonicSensor us;
    us.initialize();

    unsigned long start = millis();

    while (millis() - start < 20000) {
        int raw = us.getCurrentDistance();   // antes de readDistance atualizar
        int val = us.readDistance();

        const __FlashStringHelper* faseLabel;
        switch (us.getApproachPhase()) {
            case UltrasonicSensor::PHASE_1_DISTANT:     faseLabel = F("F1-DISTANTE");    break;
            case UltrasonicSensor::PHASE_2_APPROACHING: faseLabel = F("F2-APROX");       break;
            case UltrasonicSensor::PHASE_3_CONTACT:     faseLabel = F("F3-CONTATO");     break;
            default:                                     faseLabel = F("SEM-OBJETO");     break;
        }

        Serial.print(F("bruta="));  Serial.print(us.getCurrentDistance());
        Serial.print(F("cm  val=")); Serial.print(val);
        Serial.print(F("cm  estav=")); Serial.print(us.isReadingStable() ? F("S") : F("N"));
        Serial.print(F("  fase="));  Serial.println(faseLabel);

        delay(300);
    }

    Serial.println(F("\n✓ Teste 3 concluído"));
}

// ============================================================================
// TESTE 4 — SERVO GARRA
//
// Executa a sequência: open → close → open → close
// Verifica guards de estado (segunda chamada idêntica é ignorada).
// Observar: movimento suave grau a grau sem tranco.
// ============================================================================
void testGripper() {
    Serial.println(F("=== TESTE 4: SERVO GARRA ==="));
    Serial.print  (F("Angulos: OPEN=")); Serial.print(SERVO_ANGLE_OPEN);
    Serial.print  (F("  CLOSED="));      Serial.print(SERVO_ANGLE_CLOSED);
    Serial.print  (F("  step_ms="));     Serial.println(SERVO_STEP_DELAY_MS);

    GripperServo gripper;
    gripper.initialize();

    // Sequência de 4 movimentos
    for (int i = 0; i < 2; i++) {
        Serial.print(F("[")); Serial.print(i * 2 + 1);
        Serial.println(F("/4] Fechando..."));
        gripper.close();
        Serial.print(F("  estado="));
        Serial.println(gripper.isClosed() ? F("FECHADA ✓") : F("ERRO — esperava FECHADA"));
        delay(500);

        Serial.print(F("["));  Serial.print(i * 2 + 2);
        Serial.println(F("/4] Abrindo..."));
        gripper.open();
        Serial.print(F("  estado="));
        Serial.println(gripper.isOpen() ? F("ABERTA ✓") : F("ERRO — esperava ABERTA"));
        delay(500);
    }

    // Testa guard de estado: segunda chamada deve ser ignorada (sem movimento)
    Serial.println(F("\n-- Guard de estado (sem movimento esperado) --"));
    Serial.println(F("open() quando já ABERTA:"));
    gripper.open();
    Serial.println(F("close() quando já FECHADA:"));
    gripper.close();
    gripper.close();

    Serial.println(F("\n✓ Teste 4 concluído"));
}

// ============================================================================
// TESTE 5 — COLETA SIMULADA
//
// Simula o ciclo completo do STATE_COLLECTING sem seguidor de linha ativo.
// Útil para calibrar TURN_90_LOADED_MS, STRAFE_LOADED_MS etc.
//
// Fluxo:
//   Aguarda objeto na zona de contato por TEST5_STABLE_MS contínuos
//   → fecha garra → manobra esquerda → solta → retorna → repete pelo direito
//   Repete até TEST5_DURATION ms
// ============================================================================
void testCollectSequence() {
    Serial.println(F("=== TESTE 5: COLETA SIMULADA ==="));
    Serial.println(F("Aproxime um objeto e mantenha na zona de contato."));
    Serial.print  (F("Tempo estavel: ")); Serial.print(TEST5_STABLE_MS);
    Serial.print  (F("ms  Duracao: "));   Serial.print(TEST5_DURATION / 1000);
    Serial.println(F("s"));
    delay(1500);

    MotorController  motor;
    UltrasonicSensor us;
    GripperServo     gripper;

    motor.initialize();
    us.initialize();
    gripper.initialize();
    gripper.open();

    bool          naZona       = false;
    unsigned long entradaZona  = 0;
    bool          ladoEsquerdo = true;
    int           ciclos       = 0;
    unsigned long start        = millis();

    while (millis() - start < TEST5_DURATION) {
        int dist = us.readDistance();

        bool objetoDetectado = us.isReadingStable()
                               && dist > 0
                               && dist <= ULTRASONIC_DISTANCE_CONTACT;

        if (objetoDetectado) {
            if (!naZona) {
                naZona      = true;
                entradaZona = millis();
                Serial.print(F("[Coleta] Objeto a ")); Serial.print(dist);
                Serial.println(F("cm — aguardando estabilidade..."));
            }

            if (millis() - entradaZona >= TEST5_STABLE_MS) {
                ciclos++;
                Serial.print(F("\n[Ciclo #")); Serial.print(ciclos);
                Serial.println(ladoEsquerdo ? F("] Lado ESQUERDO") : F("] Lado DIREITO"));

                // Fecha garra
                motor.stop();
                gripper.close();
                delay(COLLECT_STOP_DELAY);

                if (ladoEsquerdo) {
                    motor.move(MotorController::TURN_LEFT, MANEUVER_SPEED_LOADED);
                    delay(TURN_90_LOADED_MS);
                    motor.stop(); delay(50);

                    motor.move(MotorController::FORWARD, MANEUVER_SPEED_LOADED);
                    delay(STRAFE_LOADED_MS);
                    motor.stop(); delay(100);

                    gripper.open(); delay(200);

                    motor.move(MotorController::BACKWARD, MANEUVER_SPEED_UNLOADED);
                    delay(STRAFE_UNLOADED_MS);
                    motor.stop(); delay(50);

                    motor.move(MotorController::TURN_RIGHT, MANEUVER_SPEED_UNLOADED);
                    delay(TURN_90_UNLOADED_MS);
                    motor.stop();

                } else {
                    motor.move(MotorController::TURN_RIGHT, MANEUVER_SPEED_LOADED);
                    delay(TURN_90_LOADED_MS);
                    motor.stop(); delay(50);

                    motor.move(MotorController::FORWARD, MANEUVER_SPEED_LOADED);
                    delay(STRAFE_LOADED_MS);
                    motor.stop(); delay(100);

                    gripper.open(); delay(200);

                    motor.move(MotorController::BACKWARD, MANEUVER_SPEED_UNLOADED);
                    delay(STRAFE_UNLOADED_MS);
                    motor.stop(); delay(50);

                    motor.move(MotorController::TURN_LEFT, MANEUVER_SPEED_UNLOADED);
                    delay(TURN_90_UNLOADED_MS);
                    motor.stop();
                }

                ladoEsquerdo = !ladoEsquerdo;
                us.resetValidation();
                naZona = false;

                Serial.print(F("[Ciclo #")); Serial.print(ciclos);
                Serial.println(F("] Concluído — retornando à busca"));
                delay(300);
            }

        } else {
            if (naZona) {
                Serial.println(F("[Coleta] Objeto saiu da zona — contagem cancelada"));
                naZona = false;
            }
        }

        // Log periódico de diagnóstico
        if (DEBUG_MODE) {
            static unsigned long lastLog = 0;
            if (millis() - lastLog >= 500) {
                lastLog = millis();
                Serial.print(F("  dist=")); Serial.print(dist);
                Serial.print(F("cm  estav=")); Serial.print(us.isReadingStable() ? F("S") : F("N"));
                Serial.print(F("  zona="));   Serial.print(objetoDetectado ? F("S") : F("N"));
                if (naZona) {
                    unsigned long conta = millis() - entradaZona;
                    Serial.print(F("  conta=")); Serial.print(conta);
                    Serial.print(F("ms  falta="));
                    Serial.print(conta < TEST5_STABLE_MS ? TEST5_STABLE_MS - conta : 0);
                    Serial.print(F("ms"));
                }
                Serial.println();
            }
        }

        delay(20);
    }

    motor.stop();
    gripper.open();
    Serial.print(F("\n✓ Teste 5 concluído — ")); Serial.print(ciclos);
    Serial.println(F(" ciclos realizados"));
}

// ============================================================================
// TESTE 6 — CALIBRAÇÃO DE THRESHOLD
//
// Procedimento de 2 etapas — coleta 200 amostras de cada superfície.
// Etapa 1: todos os sensores sobre a LINHA BRANCA
// Etapa 2: todos os sensores sobre o FUNDO PRETO
//
// Calcula threshold recomendado = 50% do intervalo [max_linha, min_fundo].
//
// Resultado confiável exige:
//   separação > 80 por sensor
//   altura do sensor: 3–6 mm da pista
//   iluminação uniforme sobre toda a pista
// ============================================================================
void calibrateThreshold() {
    Serial.println(F("=== TESTE 6: CALIBRAÇÃO DE THRESHOLD ==="));
    Serial.println(F("Linha branca = valores BAIXOS"));
    Serial.println(F("Fundo preto  = valores ALTOS"));

    // ── Etapa 1: linha branca ────────────────────────────────────────────────
    Serial.println(F("\nEtapa 1/2: TODOS os sensores sobre a LINHA BRANCA."));
    Serial.println(F("Aguardando 4s..."));
    delay(4000);

    int maxLine[6] = {};
    int minLine[6] = {1023, 1023, 1023, 1023, 1023, 1023};

    Serial.println(F("Coletando 200 amostras na linha..."));
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 6; j++) {
            int v = analogRead(A0 + j);
            if (v > maxLine[j]) maxLine[j] = v;
            if (v < minLine[j]) minLine[j] = v;
        }
        delay(10);
    }

    Serial.println(F("Linha branca — min / max por sensor:"));
    for (int i = 0; i < 6; i++) {
        Serial.print(F("  S")); Serial.print(i + 1);
        Serial.print(F(": ")); Serial.print(minLine[i]);
        Serial.print(F(" / ")); Serial.println(maxLine[i]);
    }

    // ── Etapa 2: fundo preto ─────────────────────────────────────────────────
    Serial.println(F("\nEtapa 2/2: TODOS os sensores sobre o FUNDO PRETO."));
    Serial.println(F("Aguardando 4s..."));
    delay(4000);

    int maxBack[6] = {};
    int minBack[6] = {1023, 1023, 1023, 1023, 1023, 1023};

    Serial.println(F("Coletando 200 amostras no fundo..."));
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 6; j++) {
            int v = analogRead(A0 + j);
            if (v > maxBack[j]) maxBack[j] = v;
            if (v < minBack[j]) minBack[j] = v;
        }
        delay(10);
    }

    Serial.println(F("Fundo preto — min / max por sensor:"));
    for (int i = 0; i < 6; i++) {
        Serial.print(F("  S")); Serial.print(i + 1);
        Serial.print(F(": ")); Serial.print(minBack[i]);
        Serial.print(F(" / ")); Serial.println(maxBack[i]);
    }

    // ── Análise por sensor ───────────────────────────────────────────────────
    Serial.println(F("\n=== ANÁLISE POR SENSOR ==="));

    int  sumMaxLine = 0, sumMinBack = 0;
    bool allOk      = true;

    for (int i = 0; i < 6; i++) {
        int sep = minBack[i] - maxLine[i];
        int thr = maxLine[i] + sep / 2;

        Serial.print(F("  S")); Serial.print(i + 1);
        Serial.print(F(": max_linha="));  Serial.print(maxLine[i]);
        Serial.print(F("  min_fundo="));  Serial.print(minBack[i]);
        Serial.print(F("  sep="));        Serial.print(sep);
        Serial.print(F("  thr="));        Serial.print(thr);

        if (sep < 80) {
            Serial.print(F("  << SEP BAIXA"));
            allOk = false;
        }
        Serial.println();

        sumMaxLine += maxLine[i];
        sumMinBack += minBack[i];
    }

    int avgMaxLine  = sumMaxLine / 6;
    int avgMinBack  = sumMinBack / 6;
    int recommended = avgMaxLine + (avgMinBack - avgMaxLine) / 2;

    // ── Resultado ────────────────────────────────────────────────────────────
    Serial.println(F("\n=== RESULTADO GLOBAL ==="));
    Serial.print  (F("  Média max linha : ")); Serial.println(avgMaxLine);
    Serial.print  (F("  Média min fundo : ")); Serial.println(avgMinBack);
    Serial.print  (F("  Separação média : ")); Serial.println(avgMinBack - avgMaxLine);
    Serial.print  (F("  THRESHOLD REC.  : ")); Serial.println(recommended);

    if (!allOk) {
        Serial.println(F("\n  ATENÇÃO: separação baixa em um ou mais sensores."));
        Serial.println(F("  Verifique altura (ideal 3–6mm) e iluminação uniforme."));
    } else {
        Serial.println(F("  Todos os sensores com separação adequada."));
    }

    Serial.println(F("\nAtualize config.h:"));
    Serial.print  (F("  #define THRESHOLD_LINE_SENSOR  "));
    Serial.println(recommended);

    Serial.println(F("\n✓ Teste 6 concluído"));
}

// ============================================================================
// ARQUIVO: test_components.cpp
// Testes para componentes individuais e lógica reativa da garra
// ============================================================================

#include <Arduino.h>
#include "config.h"
#include "MotorController.h"
#include "LineSensor.h"
#include "UltrasonicSensor.h"
#include "GripperServo.h"

// ============================================================================
// TESTE 1: MOTORES
// ============================================================================
void testMotors() {
    Serial.println(F("\n=== TESTE DE MOTORES ==="));
    Serial.print(F("VELOCITY_GLOBAL=")); Serial.println(VELOCITY_GLOBAL);

    MotorController motor;
    motor.initialize();

    struct { const __FlashStringHelper* label; MotorController::Direction dir; } moves[] = {
        { F("FRENTE"),   MotorController::FORWARD    },
        { F("RE"),       MotorController::BACKWARD   },
        { F("ESQUERDA"), MotorController::TURN_LEFT  },
        { F("DIREITA"),  MotorController::TURN_RIGHT },
    };

    for (auto& m : moves) {
        Serial.print(F("[Motor] ")); Serial.print(m.label); Serial.println(F(" por 2s..."));
        motor.move(m.dir, VELOCITY_GLOBAL);
        delay(2000);
        motor.stop();
        delay(500);
    }

    // Testes de curvas (arco suave com um motor)
    struct { const __FlashStringHelper* label; MotorController::Direction dir; } curveTests[] = {
        { F("CURVA SUAVE (50% vel)"),  MotorController::CURVE_LEFT  },
        { F("CURVA SUAVE (50% vel)"),  MotorController::CURVE_RIGHT },
    };

    Serial.println(F("\n--- Testando curvas (um motor ativo) ---"));
    for (auto& c : curveTests) {
        Serial.print(F("[Motor] ")); Serial.print(c.label); Serial.println(F("..."));
        motor.move(c.dir, (uint8_t)(VELOCITY_GLOBAL * 0.5f));
        delay(1500);
        motor.stop();
        delay(300);
    }

    Serial.println(F("\n✓ Teste de motores concluido"));
}

// ============================================================================
// TESTE 2: SENSORES DE LINHA
// Valida leitura, posição ponderada e classificação de padrão.
// Execute com o robô parado: primeiro sobre a linha, depois movendo manualmente.
// ============================================================================
void testLineSensors() {
    Serial.println(F("\n=== TESTE 2: SENSORES DE LINHA ==="));
    Serial.print  (F("Threshold: "));   Serial.println(THRESHOLD_LINE_SENSOR);
    Serial.println(F("Convencao: raw <= threshold = LINHA BRANCA (ativo=1)"));
    Serial.println(F("Pesos:     S1=-5  S2=-3  S3=-1  S4=+1  S5=+3  S6=+5"));
    Serial.println(F("Posicao:  -1.0=extrema esq | 0.0=centro | +1.0=extrema dir"));
    Serial.println(F("------------------------------------------------------"));
    Serial.println(F("Posicione sobre a linha. Movimente lentamente por 20s."));
    delay(2000);

    LineSensor sensor;
    sensor.initialize();

    unsigned long startTime = millis();
    unsigned long lastPrint = 0;

    while (millis() - startTime < 20000) {

        const LineSensor::SensorState& state = sensor.readSensors();

        if (millis() - lastPrint >= 100) {   // 10Hz de impressão
            lastPrint = millis();

            // Rótulo do padrão
            const __FlashStringHelper* label;
            switch (sensor.getLinePattern()) {
                case LineSensor::STRAIGHT:      label = F("RETA");           break;
                case LineSensor::CURVE_LIGHT:   label = F("CURVA-SUAVE");    break;
                case LineSensor::CURVE_MEDIUM:  label = F("CURVA-MEDIA");    break;
                case LineSensor::CURVE_SHARP:   label = F("CURVA-AGUDA");    break;
                case LineSensor::INTERSECTION:  label = F("INTERSECCAO");    break;
                case LineSensor::LINE_LOST:     label = F("!! PERDIDA !!");  break;
                default:                        label = F("?");              break;
            }

            sensor.printSensorValues();
            Serial.print(F("       pad="));  Serial.print(label);
            Serial.print(F("  dir="));
            Serial.println(sensor.getLastDirection() == LineSensor::DIR_LEFT  ? F("ESQ") :
                           sensor.getLastDirection() == LineSensor::DIR_RIGHT ? F("DIR") : F("CTR"));
        }
    }

    Serial.println(F("\n--- Checklist pos-teste ---"));
    Serial.println(F("  [ ] raw sobre linha < threshold?"));
    Serial.println(F("  [ ] raw sobre fundo > threshold?"));
    Serial.println(F("  [ ] pos=0.000 com S3+S4 ativos (centrais)?"));
    Serial.println(F("  [ ] pos negativa ao mover para esquerda?"));
    Serial.println(F("  [ ] pos positiva ao mover para direita?"));
    Serial.println(F("\n✓ Teste 2 concluido"));
}

// ============================================================================
// TESTE 3: SENSOR ULTRASSÔNICO
// ============================================================================
void testUltrasonic() {
    Serial.println(F("\n=== TESTE DE SENSOR ULTRASSÔNICO ==="));
    Serial.println(F("Aproxime um objeto do sensor..."));
    delay(2000);

    UltrasonicSensor ultrasonic;
    ultrasonic.initialize();

    Serial.println(F("Lendo ultrassônico por 15 segundos..."));
    unsigned long startTime = millis();

    while (millis() - startTime < 15000) {
        int distance = ultrasonic.readDistance();

        Serial.print("Distância: ");
        Serial.print(distance);
        Serial.print(" cm | Fase: ");

        if (!ultrasonic.isObjectDetected()) {
            Serial.println(F("SEM OBJETO"));
        } else {
            switch (ultrasonic.getApproachPhase()) {
                case UltrasonicSensor::PHASE_1_DISTANT:   Serial.println(F("FASE 1 (Distante)")); break;
                case UltrasonicSensor::PHASE_2_APPROACHING: Serial.println(F("FASE 2 (Aproximando)")); break;
                case UltrasonicSensor::PHASE_3_CONTACT:   Serial.println(F("FASE 3 (Contato)")); break;
                default:                                  Serial.println(F("DESCONHECIDO"));
            }
        }
        delay(500);
    }
    Serial.println(F("✓ Teste de ultrassônico concluído"));
}

// ============================================================================
// TESTE 4: SERVO GARRA (NÃO BLOQUEANTE)
// ============================================================================
void testGripper() {
    Serial.println(F("\n=== TESTE DE SERVO GARRA (BLOQUEANTE) ==="));

    GripperServo gripper;
    gripper.initialize();               // posiciona em OPEN e aguarda
    delay(100);

    Serial.println(F("Abrindo garra..."));
    gripper.open();
    Serial.print("Estado: "); Serial.println(gripper.isOpen() ? "ABERTA" : "FECHADA");
    delay(500);

    Serial.println(F("Fechando garra..."));
    gripper.close();
    Serial.print("Estado: "); Serial.println(gripper.isClosed() ? "FECHADA" : "ABERTA");
    delay(500);

    Serial.println(F("Abrindo novamente..."));
    gripper.open();
    Serial.print("Estado: "); Serial.println(gripper.isOpen() ? "ABERTA" : "FECHADA");
    delay(500);

    Serial.println(F("Fechando novamente..."));
    gripper.close();
    Serial.print("Estado: "); Serial.println(gripper.isClosed() ? "FECHADA" : "ABERTA");

    Serial.println(F("\n✓ Teste de servo garra concluído"));
}

// ============================================================================
// TESTE 5: GARRA REATIVA COM DETECÇÃO CONTÍNUA
// Preparado para acoplamento futuro do módulo de movimento (fases de aproximação)
// ============================================================================
void testGripperReactive() {

    Serial.println(F("\n=== TESTE DE GARRA REATIVA ==="));
    Serial.println(F("--- Parâmetros de distância ---"));
    Serial.print(F("  Fase 1 (distante):    > ")); Serial.print(ULTRASONIC_DISTANCE_LONG);    Serial.println(F(" cm"));
    Serial.print(F("  Fase 2 (aproximando): > ")); Serial.print(ULTRASONIC_DISTANCE_SHORT);   Serial.println(F(" cm  <- inicio da contagem"));
    Serial.print(F("  Fase 3 (coleta):     <= ")); Serial.print(ULTRASONIC_DISTANCE_CONTACT); Serial.println(F(" cm  <- gatilho da garra"));
    Serial.println(F("--- Parâmetros de tempo ---"));
    Serial.print(F("  Estavel por: ")); Serial.print(GRIPPER_STABLE_TIME_MS); Serial.println(F(" ms continuos para fechar"));
    Serial.print(F("  Hold:        ")); Serial.print(GRIPPER_HOLD_TIME_MS);   Serial.println(F(" ms com garra fechada"));
    Serial.println(F("-------------------------------"));
    Serial.println(F("Aproxime um objeto e mantenha-o dentro da distancia de coleta."));

    UltrasonicSensor ultrasonic;
    GripperServo     gripper;

    ultrasonic.initialize();
    gripper.initialize();
    gripper.open();

    bool          emZonaDeColeta = false;
    unsigned long entradaNaZona  = 0;
    unsigned long lastPrint      = 0;
    unsigned long startTime      = millis();

    const unsigned long TEST_DURATION = 30000UL;

    while (millis() - startTime < TEST_DURATION) {

        int distance = ultrasonic.readDistance();
        UltrasonicSensor::ApproachPhase fase = ultrasonic.getApproachPhase();

        bool objetoNaZona = ultrasonic.isReadingStable()
                            && (distance > 0)
                            && (distance <= ULTRASONIC_DISTANCE_CONTACT);

        // --- Máquina de estados da zona de coleta ---
        if (objetoNaZona) {
            if (!emZonaDeColeta) {
                emZonaDeColeta = true;
                entradaNaZona  = millis();
                Serial.print(F("[Garra] Objeto na zona de coleta ("));
                Serial.print(distance);
                Serial.println(F(" cm) -> contagem iniciada"));
            }

            unsigned long tempoNaZona = millis() - entradaNaZona;

            if (tempoNaZona >= GRIPPER_STABLE_TIME_MS) {
                Serial.print(F("[Garra] COLETA em "));
                Serial.print(distance);
                Serial.println(F(" cm -> fechando"));

                gripper.close();

                Serial.print(F("[Garra] Fechada | hold por "));
                Serial.print(GRIPPER_HOLD_TIME_MS);
                Serial.println(F(" ms"));
                delay(GRIPPER_HOLD_TIME_MS);

                gripper.open();
                Serial.println(F("[Garra] Reaberta -> aguardando proximo objeto"));

                ultrasonic.resetValidation();
                emZonaDeColeta = false;
                entradaNaZona  = 0;
                delay(200);
            }

        } else {
            if (emZonaDeColeta) {
                Serial.print(F("[Garra] Objeto saiu da zona ("));
                Serial.print(distance);
                Serial.println(F(" cm) -> contagem cancelada"));
                emZonaDeColeta = false;
                entradaNaZona  = 0;
            }
        }

        // --- Debug periódico ---
        if (DEBUG_MODE && (millis() - lastPrint >= 500)) {
            lastPrint = millis();

            const __FlashStringHelper* rotFase;
            switch (fase) {
                case UltrasonicSensor::PHASE_1_DISTANT:     rotFase = F("F1-DISTANTE");    break;
                case UltrasonicSensor::PHASE_2_APPROACHING: rotFase = F("F2-APROXIMANDO"); break;
                case UltrasonicSensor::PHASE_3_CONTACT:     rotFase = F("F3-CONTATO");     break;
                default:                                     rotFase = F("SEM-OBJETO");     break;
            }

            unsigned long tempoNaZona   = emZonaDeColeta ? (millis() - entradaNaZona) : 0;
            unsigned long tempoRestante = (emZonaDeColeta && tempoNaZona < GRIPPER_STABLE_TIME_MS)
                                          ? GRIPPER_STABLE_TIME_MS - tempoNaZona : 0;

            Serial.print(F("  ["));        Serial.print((millis() - startTime) / 1000);
            Serial.print(F("s] dist="));   Serial.print(distance);
            Serial.print(F("cm | fase=")); Serial.print(rotFase);
            Serial.print(F(" | estavel=")); Serial.print(ultrasonic.isReadingStable() ? F("S") : F("N"));
            Serial.print(F(" | zona="));   Serial.print(objetoNaZona ? F("S") : F("N"));
            Serial.print(F(" | conta="));  Serial.print(tempoNaZona);
            Serial.print(F("ms | faltam=")); Serial.print(tempoRestante);
            Serial.print(F("ms | garra=")); Serial.println(gripper.isClosed() ? F("FECHADA") : F("ABERTA"));
        }

        delay(20);
    }

    gripper.open();
    Serial.println(F("\n✓ Teste de garra reativa concluido"));
}

// ============================================================================
// TESTE 6: CALIBRAÇÃO DE THRESHOLD
//
// Procedimento:
//   Etapa 1 — posicione TODOS os sensores sobre a LINHA BRANCA
//   Etapa 2 — posicione TODOS os sensores sobre o FUNDO PRETO
//
// O código coleta o pior caso de cada superfície e calcula o ponto
// médio com margem conservadora para o lado do fundo.
//
// Resultado esperado (linha branca = BAIXO, fundo preto = ALTO):
//   max_linha < min_fundo   → separação positiva → threshold confiável
// ============================================================================
void calibrateThreshold() {
    Serial.println(F("\n=== TESTE 6: CALIBRACAO DE THRESHOLD ==="));
    Serial.println(F("Linha branca = valores BAIXOS"));
    Serial.println(F("Fundo preto  = valores ALTOS"));
    Serial.println(F(""));
    Serial.println(F("Etapa 1/2: posicione TODOS os sensores sobre a LINHA BRANCA."));
    Serial.println(F("Aguardando 4 segundos..."));
    delay(4000);

    // Coleta máximo da linha branca (pior caso — mais alto que a linha chega)
    int maxLine[6] = {0, 0, 0, 0, 0, 0};
    int minLine[6] = {1023, 1023, 1023, 1023, 1023, 1023};

    Serial.println(F("Coletando 200 amostras na LINHA..."));
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 6; j++) {
            int v = analogRead(A0 + j);
            if (v > maxLine[j]) maxLine[j] = v;
            if (v < minLine[j]) minLine[j] = v;
        }
        delay(10);
    }

    Serial.println(F("Linha branca — min/max por sensor:"));
    for (int i = 0; i < 6; i++) {
        Serial.print(F("  S")); Serial.print(i + 1);
        Serial.print(F(": min=")); Serial.print(minLine[i]);
        Serial.print(F("  max=")); Serial.println(maxLine[i]);
    }

    Serial.println(F(""));
    Serial.println(F("Etapa 2/2: posicione TODOS os sensores sobre o FUNDO PRETO."));
    Serial.println(F("Aguardando 4 segundos..."));
    delay(4000);

    // Coleta mínimo do fundo preto (pior caso — mais baixo que o fundo chega)
    int maxBack[6] = {0, 0, 0, 0, 0, 0};
    int minBack[6] = {1023, 1023, 1023, 1023, 1023, 1023};

    Serial.println(F("Coletando 200 amostras no FUNDO..."));
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 6; j++) {
            int v = analogRead(A0 + j);
            if (v > maxBack[j]) maxBack[j] = v;
            if (v < minBack[j]) minBack[j] = v;
        }
        delay(10);
    }

    Serial.println(F("Fundo preto — min/max por sensor:"));
    for (int i = 0; i < 6; i++) {
        Serial.print(F("  S")); Serial.print(i + 1);
        Serial.print(F(": min=")); Serial.print(minBack[i]);
        Serial.print(F("  max=")); Serial.println(maxBack[i]);
    }

    // Calcula separação e threshold por sensor
    Serial.println(F(""));
    Serial.println(F("=== ANALISE POR SENSOR ==="));

    int  sumMaxLine = 0, sumMinBack = 0;
    bool allOk      = true;

    for (int i = 0; i < 6; i++) {
        int sep       = minBack[i] - maxLine[i];   // deve ser positivo
        int threshold = maxLine[i] + (int)(sep * 0.5f);

        Serial.print(F("  S")); Serial.print(i + 1);
        Serial.print(F(": linha_max=")); Serial.print(maxLine[i]);
        Serial.print(F("  fundo_min=")); Serial.print(minBack[i]);
        Serial.print(F("  sep="));       Serial.print(sep);
        Serial.print(F("  thr="));       Serial.print(threshold);

        if (sep < 80) {
            Serial.print(F("  << AVISO: separacao baixa"));
            allOk = false;
        }
        Serial.println();

        sumMaxLine += maxLine[i];
        sumMinBack += minBack[i];
    }

    int avgMaxLine = sumMaxLine / 6;
    int avgMinBack = sumMinBack / 6;
    int avgSep     = avgMinBack - avgMaxLine;

    // Threshold global: 50% do caminho entre max_linha e min_fundo
    // 50% é conservador — equidistante, sem favorecimento
    int recommended = avgMaxLine + (int)(avgSep * 0.50f);

    Serial.println(F(""));
    Serial.println(F("=== RESULTADO GLOBAL ==="));
    Serial.print(F("  Media max linha:  ")); Serial.println(avgMaxLine);
    Serial.print(F("  Media min fundo:  ")); Serial.println(avgMinBack);
    Serial.print(F("  Separacao media:  ")); Serial.println(avgSep);
    Serial.print(F("  THRESHOLD REC.:   ")); Serial.println(recommended);

    if (!allOk) {
        Serial.println(F(""));
        Serial.println(F("  ATENCAO: um ou mais sensores com separacao baixa."));
        Serial.println(F("  Verifique altura dos sensores (ideal: 3–6mm da pista)"));
        Serial.println(F("  e iluminacao uniforme sobre toda a pista."));
    } else {
        Serial.println(F("  Todos os sensores com separacao adequada."));
    }

    Serial.println(F(""));
    Serial.println(F("Atualize config.h:"));
    Serial.print  (F("  #define THRESHOLD_LINE_SENSOR  ")); Serial.println(recommended);
    Serial.println(F("\n✓ Teste 6 concluido"));
}

// ============================================================================
// SETUP E LOOP — menu atualizado com teste 7
// ============================================================================
void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Serial.println(F("\n╔════════════════════════════════════════╗"));
    Serial.println(F("║   PROGRAMA DE TESTE DE COMPONENTES     ║"));
    Serial.println(F("╚════════════════════════════════════════╝"));
    Serial.println(F("1 - Testar Motores"));
    Serial.println(F("2 - Testar Sensores de Linha"));
    Serial.println(F("3 - Testar Ultrassonico"));
    Serial.println(F("4 - Testar Servo Garra"));
    Serial.println(F("5 - Testar Garra Reativa"));
    Serial.println(F("6 - Calibrar Threshold de Linha"));
}

int selectedTest = 0;

void loop() {
    if (Serial.available()) {
        char cmd    = Serial.read();
        selectedTest = cmd - '0';

        switch (selectedTest) {
            case 1: testMotors();          break;
            case 2: testLineSensors();     break;
            case 3: testUltrasonic();      break;
            case 4: testGripper();         break;
            case 5: testGripperReactive(); break;
            case 6: calibrateThreshold();  break;
            default: Serial.println(F("Opcao invalida!")); break;
        }

        Serial.println(F("\n╔════════════════════════════════════════╗"));
        Serial.println(F("║   Teste concluido. Proximo teste?      ║"));
        Serial.println(F("╚════════════════════════════════════════╝"));
    }
    delay(100);
}
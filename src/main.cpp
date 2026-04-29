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

    // Curvas compensadas com os três fatores do config
    struct { const __FlashStringHelper* label; float factor; } curves[] = {
        { F("CURVA SUAVE  (0.9)"), CURVE_COMPENSATION_LIGHT  },
        { F("CURVA MEDIA  (0.8)"), CURVE_COMPENSATION_MEDIUM },
        { F("CURVA AGUDA  (0.6)"), CURVE_COMPENSATION_SHARP  },
    };

    for (auto& c : curves) {
        Serial.print(F("[Motor] ESQ ")); Serial.print(c.label); Serial.println(F("..."));
        motor.curveCompensated(MotorController::TURN_LEFT, VELOCITY_GLOBAL, c.factor);
        delay(1500);
        motor.stop();
        delay(300);

        Serial.print(F("[Motor] DIR ")); Serial.print(c.label); Serial.println(F("..."));
        motor.curveCompensated(MotorController::TURN_RIGHT, VELOCITY_GLOBAL, c.factor);
        delay(1500);
        motor.stop();
        delay(300);
    }

    Serial.println(F("\n✓ Teste de motores concluido"));
}

// ============================================================================
// TESTE 2: SENSORES DE LINHA (posição ponderada contínua)
// ============================================================================
void testLineSensors() {
    Serial.println(F("\n=== TESTE DE SENSORES DE LINHA ==="));
    Serial.print(F("Threshold: "));      Serial.println(THRESHOLD_LINE_SENSOR);
    Serial.print(F("Filtro: "));         Serial.print(SENSOR_FILTER_CYCLES);
    Serial.println(F(" leituras estaveis"));
    Serial.println(F("Pesos: S1=-5 S2=-3 S3=-1 S4=+1 S5=+3 S6=+5"));
    Serial.println(F("Posicione o robo sobre a linha e aguarde..."));
    delay(2000);

    LineSensor sensor;
    sensor.initialize();

    Serial.println(F("Lendo por 20 segundos — mova o robo sobre curvas e retas:"));
    unsigned long startTime = millis();
    unsigned long lastPrint = 0;

    while (millis() - startTime < 20000) {
        LineSensor::SensorState state = sensor.readSensors();

        if (millis() - lastPrint >= 200) {
            lastPrint = millis();

            const __FlashStringHelper* rotPad;
            switch (sensor.getLinePattern()) {
                case LineSensor::STRAIGHT:      rotPad = F("RETA");          break;
                case LineSensor::CURVE_LIGHT:   rotPad = F("CURVA SUAVE");   break;
                case LineSensor::CURVE_MEDIUM:  rotPad = F("CURVA MEDIA");   break;
                case LineSensor::CURVE_SHARP:   rotPad = F("CURVA AGUDA");   break;
                case LineSensor::TURN_LEFT_90:  rotPad = F("CRUZAMENTO-ESQ");break;
                case LineSensor::TURN_RIGHT_90: rotPad = F("CRUZAMENTO-DIR");break;
                case LineSensor::INTERSECTION:  rotPad = F("INTERSECCAO-X"); break;
                case LineSensor::LINE_LOST:     rotPad = F("LINHA PERDIDA"); break;
                default:                        rotPad = F("DESCONHECIDO");  break;
            }

            Serial.print(F("  ["));
            Serial.print((millis() - startTime) / 1000);
            Serial.print(F("s] ativos="));
            for (int i = 0; i < 6; i++) Serial.print(state.active[i] ? '1' : '0');
            Serial.print(F(" | pos="));

            // Barra visual de posição  [-1.0 ... 0.0 ... +1.0]
            char bar[22];
            int  idx = (int)((state.position + 1.0f) * 10.0f);
            idx = constrain(idx, 0, 20);
            for (int i = 0; i < 21; i++) bar[i] = (i == 10) ? '|' : '-';
            bar[idx] = '#';
            bar[21]  = '\0';
            Serial.print('['); Serial.print(bar); Serial.print(F("] "));

            Serial.print(state.position, 3);
            Serial.print(F(" | cnt="));  Serial.print(state.activeCount);
            Serial.print(F(" | valid=")); Serial.print(state.isValid ? F("S") : F("N"));
            Serial.print(F(" | pad="));  Serial.println(rotPad);
        }
    }

    Serial.println(F("\n✓ Teste de sensores de linha concluido"));
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
// TESTE 6: CALIBRAÇÃO DE THRESHOLD DE LINHA
// ============================================================================
void calibrateThreshold() {
    Serial.println(F("\n=== CALIBRACAO DE THRESHOLD ==="));
    Serial.println(F("Etapa 1/2: posicione o robo SOBRE A LINHA branca."));
    Serial.println(F("Aguardando 3 segundos..."));
    delay(3000);

    LineSensor lineSensor;
    lineSensor.initialize();

    int maxLineValues[6]       = {0};
    int minBackgroundValues[6] = {1023, 1023, 1023, 1023, 1023, 1023};
    int readings[6];

    Serial.println(F("Coletando 100 amostras na LINHA..."));
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 6; j++) {
            readings[j] = analogRead(A0 + j);
            if (readings[j] > maxLineValues[j]) maxLineValues[j] = readings[j];
        }
        delay(20);
    }

    Serial.println(F("Valores maximos na linha:"));
    for (int i = 0; i < 6; i++) {
        Serial.print(F("  S")); Serial.print(i + 1);
        Serial.print(F(": ")); Serial.println(maxLineValues[i]);
    }

    Serial.println(F("\nEtapa 2/2: posicione o robo FORA DA LINHA (fundo)."));
    Serial.println(F("Aguardando 3 segundos..."));
    delay(3000);

    Serial.println(F("Coletando 100 amostras no FUNDO..."));
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 6; j++) {
            readings[j] = analogRead(A0 + j);
            if (readings[j] < minBackgroundValues[j]) minBackgroundValues[j] = readings[j];
        }
        delay(20);
    }

    Serial.println(F("Valores minimos no fundo:"));
    for (int i = 0; i < 6; i++) {
        Serial.print(F("  S")); Serial.print(i + 1);
        Serial.print(F(": ")); Serial.println(minBackgroundValues[i]);
    }

    int avgLine = 0, avgBackground = 0;
    for (int i = 0; i < 6; i++) {
        avgLine       += maxLineValues[i];
        avgBackground += minBackgroundValues[i];
    }
    avgLine       /= 6;
    avgBackground /= 6;

    int recommended = (avgLine + avgBackground) / 2;

    Serial.println(F("\n=== RESULTADO ==="));
    Serial.print(F("  Media na linha:  ")); Serial.println(avgLine);
    Serial.print(F("  Media no fundo:  ")); Serial.println(avgBackground);
    Serial.print(F("  Separacao:       ")); Serial.println(abs(avgLine - avgBackground));
    Serial.print(F("  THRESHOLD REC.:  ")); Serial.println(recommended);

    if (abs(avgLine - avgBackground) < 100) {
        Serial.println(F("  AVISO: separacao baixa (<100). Verifique iluminacao e posicionamento."));
    }

    Serial.println(F("\nAtualize em config.h:"));
    Serial.print(F("  #define THRESHOLD_LINE_SENSOR ")); Serial.println(recommended);
    Serial.println(F("\n✓ Calibracao concluida"));
}

// ============================================================================
// TESTE 7: SEGUIMENTO PD (motor + sensor integrados)
// ============================================================================
void testLineFollowing() {
    Serial.println(F("\n=== TESTE DE SEGUIMENTO PD ==="));
    Serial.print(F("Kp=")); Serial.print(PD_KP);
    Serial.print(F(" Kd=")); Serial.print(PD_KD);
    Serial.print(F(" sample=")); Serial.print(PD_SAMPLE_MS); Serial.println(F("ms"));
    Serial.println(F("Velocidades: erro<0.3 -> FAST | 0.3-0.6 -> MEDIUM | >0.6 -> SLOW"));
    Serial.println(F("Posicione o robo sobre a linha. Iniciando em 3s..."));
    delay(3000);

    LineSensor      sensor;
    MotorController motor;

    sensor.initialize();
    motor.initialize();
    motor.resetPD();

    const unsigned long RUN_DURATION = 20000UL;
    unsigned long startTime = millis();
    unsigned long lastPrint = 0;

    while (millis() - startTime < RUN_DURATION) {
        sensor.readSensors();
        float    pos     = sensor.getLinePosition();
        float    absPos  = fabs(pos);
        LineSensor::LinePattern pattern = sensor.getLinePattern();

        // Seleciona velocidade base pela magnitude do erro
        uint8_t baseSpeed;
        if      (absPos < 0.3f) baseSpeed = SPEED_ERROR_LOW;
        else if (absPos < 0.6f) baseSpeed = SPEED_ERROR_MEDIUM;
        else                    baseSpeed = SPEED_ERROR_HIGH;

        // Linha perdida: gira na última direção conhecida
        if (pattern == LineSensor::LINE_LOST) {
            motor.stop();
            if (DEBUG_MODE) Serial.println(F("[PD] LINHA PERDIDA — aguardando"));
            delay(50);
            continue;
        }

        // Cruzamentos: manter frente por enquanto (expansão futura)
        if (pattern == LineSensor::INTERSECTION   ||
            pattern == LineSensor::TURN_LEFT_90   ||
            pattern == LineSensor::TURN_RIGHT_90) {
            motor.move(MotorController::FORWARD, SPEED_ERROR_LOW);
            if (DEBUG_MODE) Serial.println(F("[PD] CRUZAMENTO — passando reto"));
            delay(PD_SAMPLE_MS);
            continue;
        }

        motor.followLine(pos, baseSpeed);

        // Debug periódico
        if (DEBUG_MODE && (millis() - lastPrint >= 300)) {
            lastPrint = millis();

            const __FlashStringHelper* rotPad;
            switch (pattern) {
                case LineSensor::STRAIGHT:     rotPad = F("RETA");         break;
                case LineSensor::CURVE_LIGHT:  rotPad = F("C-SUAVE");      break;
                case LineSensor::CURVE_MEDIUM: rotPad = F("C-MEDIA");      break;
                case LineSensor::CURVE_SHARP:  rotPad = F("C-AGUDA");      break;
                default:                       rotPad = F("?");            break;
            }

            Serial.print(F("  ["));
            Serial.print((millis() - startTime) / 1000);
            Serial.print(F("s] pos="));    Serial.print(pos, 3);
            Serial.print(F(" | base="));   Serial.print(baseSpeed);
            Serial.print(F(" | pad="));    Serial.print(rotPad);
            Serial.print(F(" | L="));      Serial.print(motor.getLeftSpeed());
            Serial.print(F(" | R="));      Serial.println(motor.getRightSpeed());
        }

        delay(PD_SAMPLE_MS);
    }

    motor.stop();
    Serial.println(F("\n✓ Teste de seguimento PD concluido"));
}
// ============================================================================
// SETUP E LOOP — menu atualizado com teste 7
// ============================================================================
void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Serial.println(F("\n╔════════════════════════════════════════╗"));
    Serial.println(F("║   PROGRAMA DE TESTE DE COMPONENTES    ║"));
    Serial.println(F("╚════════════════════════════════════════╝"));
    Serial.println(F("1 - Testar Motores"));
    Serial.println(F("2 - Testar Sensores de Linha"));
    Serial.println(F("3 - Testar Ultrassonico"));
    Serial.println(F("4 - Testar Servo Garra"));
    Serial.println(F("5 - Testar Garra Reativa"));
    Serial.println(F("6 - Calibrar Threshold de Linha"));
    Serial.println(F("7 - Testar Seguimento PD (motor+sensor)"));
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
            case 7: testLineFollowing();   break;
            default: Serial.println(F("Opcao invalida!")); break;
        }

        Serial.println(F("\n╔════════════════════════════════════════╗"));
        Serial.println(F("║   Teste concluido. Proximo teste?     ║"));
        Serial.println(F("╚════════════════════════════════════════╝"));
    }
    delay(100);
}
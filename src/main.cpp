// ============================================================================
// ARQUIVO: test_components.cpp (OPCIONAL)
// Utilize este arquivo para testar cada componente isoladamente
// Renomeie para main.cpp se quiser usar no lugar do programa principal
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
    
    MotorController motor;
    motor.initialize();

    // Frente
    Serial.println(F("Movendo para frente..."));
    motor.move(MotorController::FORWARD, VELOCITY_GLOBAL);
    delay(2000);

    // Parar
    motor.stop();
    delay(500);

    // Trás
    Serial.println(F("Movendo para trás..."));
    motor.move(MotorController::BACKWARD, VELOCITY_GLOBAL);
    delay(2000);

    // Parar
    motor.stop();
    delay(500);

    // Esquerda
    Serial.println(F("Virando para esquerda..."));
    motor.move(MotorController::TURN_LEFT, VELOCITY_GLOBAL);
    delay(2000);

    // Parar
    motor.stop();
    delay(500);

    // Direita
    Serial.println(F("Virando para direita..."));
    motor.move(MotorController::TURN_RIGHT, VELOCITY_GLOBAL);
    delay(2000);

    // Parar
    motor.stop();
    
    Serial.println(F("✓ Teste de motores concluído"));
}

// ============================================================================
// TESTE 2: SENSORES DE LINHA
// ============================================================================

void testLineSensors() {
    Serial.println(F("\n=== TESTE DE SENSORES DE LINHA ==="));
    Serial.println(F("Posicione o robô sobre a linha..."));
    delay(2000);
    
    LineSensor lineSensor;
    lineSensor.initialize();

    Serial.println(F("Lendo sensores por 10 segundos..."));
    unsigned long startTime = millis();

    while (millis() - startTime < 10000) {
        LineSensor::SensorState state = lineSensor.readSensors();
        
        // Imprimir valores brutos
        Serial.print("Padrão: ");
        for (int i = 0; i < 6; i++) {
            Serial.print(state.sensors[i] ? "█" : "·");
        }
        Serial.print(" | Binário: ");
        Serial.print(state.rawPattern, BIN);
        Serial.print(" | Padrão: ");
        
        switch (lineSensor.getLinePattern()) {
            case LineSensor::STRAIGHT:
                Serial.println(F("RETA"));
                break;
            case LineSensor::CURVE_LIGHT:
                Serial.println(F("CURVA SUAVE"));
                break;
            case LineSensor::CURVE_SHARP:
                Serial.println(F("CURVA ACENTUADA"));
                break;
            case LineSensor::INTERSECTION:
                Serial.println(F("INTERSECÇÃO"));
                break;
            case LineSensor::LINE_LOST:
                Serial.println(F("LINHA PERDIDA"));
                break;
            default:
                Serial.println(F("DESCONHECIDO"));
        }
        
        delay(500);
    }
    
    Serial.println(F("✓ Teste de sensores de linha concluído"));
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
                case UltrasonicSensor::PHASE_1_DISTANT:
                    Serial.println(F("FASE 1 (Distante)"));
                    break;
                case UltrasonicSensor::PHASE_2_APPROACHING:
                    Serial.println(F("FASE 2 (Aproximando)"));
                    break;
                case UltrasonicSensor::PHASE_3_CONTACT:
                    Serial.println(F("FASE 3 (Contato)"));
                    break;
                default:
                    Serial.println(F("DESCONHECIDO"));
            }
        }
        
        delay(500);
    }
    
    Serial.println(F("✓ Teste de ultrassônico concluído"));
}

// ============================================================================
// TESTE 4: SERVO GARRA
// ============================================================================

void testGripper() {
    Serial.println(F("\n=== TESTE DE SERVO GARRA ==="));
    
    GripperServo gripper;
    gripper.initialize();

    // Teste de abertura
    Serial.println(F("Abrindo garra..."));
    gripper.open();
    delay(1000);
    gripper.printState();

    // Teste de fechamento
    Serial.println(F("Fechando garra..."));
    bool closedSuccessfully = gripper.close();
    
    if (closedSuccessfully) {
        Serial.println(F("✓ Garra fechou com sucesso"));
    } else {
        Serial.println(F("✗ ERRO: Garra não fechou ou timeout"));
    }
    
    gripper.printState();

    // Teste de abertura novamente
    delay(2000);
    Serial.println(F("Abrindo garra novamente..."));
    gripper.open();
    delay(1000);

    Serial.println(F("✓ Teste de servo garra concluído"));
}

// ============================================================================
// CALIBRAÇÃO: THRESHOLD DE SENSORES
// ============================================================================

void calibrateThreshold() {
    Serial.println(F("\n=== CALIBRAÇÃO DE THRESHOLD ==="));
    Serial.println(F("Posicione o robô:"));
    Serial.println(F("1. Sobre a linha branca"));
    delay(3000);
    
    LineSensor lineSensor;
    lineSensor.initialize();

    int readings[6];
    int maxLineValues[6] = {0};

    // Coletar valores máximos sobre a linha
    Serial.println(F("Coletando valores na LINHA..."));
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 6; j++) {
            readings[j] = analogRead(A0 + j);
            if (readings[j] > maxLineValues[j]) {
                maxLineValues[j] = readings[j];
            }
        }
        delay(20);
    }

    Serial.println(F("\nValores MÁXIMOS na linha:"));
    for (int i = 0; i < 6; i++) {
        Serial.print(F("S"));
        Serial.print(i + 1);
        Serial.print(F(": "));
        Serial.println(maxLineValues[i]);
    }

    // Agora sobre o fundo
    Serial.println(F("\nAgora posicione o robô sobre o fundo (NÃO na linha)"));
    delay(3000);

    int minBackgroundValues[6] = {1023};
    
    Serial.println(F("Coletando valores no FUNDO..."));
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 6; j++) {
            readings[j] = analogRead(A0 + j);
            if (readings[j] < minBackgroundValues[j]) {
                minBackgroundValues[j] = readings[j];
            }
        }
        delay(20);
    }

    Serial.println(F("\nValores MÍNIMOS no fundo:"));
    for (int i = 0; i < 6; i++) {
        Serial.print(F("S"));
        Serial.print(i + 1);
        Serial.print(F(": "));
        Serial.println(minBackgroundValues[i]);
    }

    // Calcular threshold recomendado
    Serial.println(F("\n=== THRESHOLD RECOMENDADO ==="));
    int avgLine = 0, avgBackground = 0;
    
    for (int i = 0; i < 6; i++) {
        avgLine += maxLineValues[i];
        avgBackground += minBackgroundValues[i];
    }
    
    avgLine /= 6;
    avgBackground /= 6;
    
    int recommendedThreshold = (avgLine + avgBackground) / 2;
    
    Serial.print("Valor médio na LINHA: ");
    Serial.println(avgLine);
    Serial.print("Valor médio no FUNDO: ");
    Serial.println(avgBackground);
    Serial.print("THRESHOLD RECOMENDADO: ");
    Serial.println(recommendedThreshold);

    Serial.println(F("\nAtualize em config.h:"));
    Serial.print("#define THRESHOLD_LINE_SENSOR ");
    Serial.println(recommendedThreshold);
}

// ============================================================================
// SETUP E LOOP
// ============================================================================

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Serial.println(F("\n╔════════════════════════════════════════╗"));
    Serial.println(F("║   PROGRAMA DE TESTE DE COMPONENTES    ║"));
    Serial.println(F("╚════════════════════════════════════════╝"));
    Serial.println(F("\nSelecione um teste:"));
    Serial.println(F("1 - Testar Motores"));
    Serial.println(F("2 - Testar Sensores de Linha"));
    Serial.println(F("3 - Testar Ultrassônico"));
    Serial.println(F("4 - Testar Servo Garra"));
    Serial.println(F("5 - Calibrar Threshold"));
}

int selectedTest = 0;

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        selectedTest = cmd - '0';

        switch (selectedTest) {
            case 1:
                testMotors();
                break;
            case 2:
                testLineSensors();
                break;
            case 3:
                testUltrasonic();
                break;
            case 4:
                testGripper();
                break;
            case 5:
                calibrateThreshold();
                break;
            default:
                Serial.println(F("Opção inválida!"));
        }

        Serial.println(F("\n╔════════════════════════════════════════╗"));
        Serial.println(F("║   Teste concluído. Próximo teste?     ║"));
        Serial.println(F("╚════════════════════════════════════════╝"));
    }

    delay(100);
}

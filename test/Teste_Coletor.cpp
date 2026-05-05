#include <Arduino.h>
#include "config.h"
#include "MotorController.h"
#include "UltrasonicSensor.h"
#include "GripperServo.h"

void testCollector()
{

    MotorController motor;
    UltrasonicSensor ultrasonic;
    GripperServo gripper;

    motor.initialize();
    ultrasonic.initialize();
    gripper.initialize();

    bool sideLeft = true;
    unsigned long startTime = millis();

    while (millis() - startTime < COLLECT_CYCLE_DURATION)
    {

        // ----- Fase 1: Aproximação com velocidade variável -----
        motor.move(MotorController::FORWARD, APPROACH_SPEED_FAST);
        bool objetoDetectado = false;
        uint8_t contadorDetect = 0;

        while (!objetoDetectado && (millis() - startTime < COLLECT_CYCLE_DURATION))
        {
            int dist = ultrasonic.readDistance();

            // Ajuste dinâmico de velocidade
            uint8_t spd = APPROACH_SPEED_FAST;
            if (dist > 0)
            {
                if (dist <= APPROACH_DIST_MEDIUM)
                    spd = APPROACH_SPEED_SLOW;
                else if (dist <= APPROACH_DIST_LONG)
                    spd = APPROACH_SPEED_MEDIUM;
            }
            motor.move(MotorController::FORWARD, spd);

            // Detecção com histerese
            if (dist > 0 && dist <= ULTRASONIC_DISTANCE_CONTACT && ultrasonic.isReadingStable())
            {
                contadorDetect++;
                if (contadorDetect >= 3)
                    objetoDetectado = true;
            }
            else
            {
                contadorDetect = 0;
            }
            delay(20);
        }

        if (millis() - startTime >= COLLECT_CYCLE_DURATION)
            break;

        motor.stop();
        Serial.println(F("[COLETA] Objeto detectado! Fechando garra..."));
        gripper.close();
        delay(COLLECT_STOP_DELAY);

        // ----- Fase 2: Manobra lateral (sensores ignorados) -----
if (sideLeft) {
    // ------- Com carga (garra fechada) -------
    Serial.println(F("[MANOBRA] Girando ESQUERDA (carregado)"));
    motor.move(MotorController::TURN_LEFT, MANEUVER_SPEED_LOADED);
    delay(TURN_90_LOADED_MS);
    motor.stop();
    delay(50);

    Serial.println(F("[MANOBRA] Avançando LADO (carregado)"));
    motor.move(MotorController::FORWARD, MANEUVER_SPEED_LOADED);
    delay(STRAFE_LOADED_FORWARD_MS);
    motor.stop();
    delay(100);

    // Soltar objeto
    gripper.open();
    Serial.println(F("[COLETA] Garra aberta. Retornando..."));
    delay(200);

    // ------- Sem carga (garra aberta) -------
    Serial.println(F("[MANOBRA] Dando RÉ (vazio)"));
    motor.move(MotorController::BACKWARD, MANEUVER_SPEED_UNLOADED);
    delay(STRAFE_UNLOADED_BACKWARD_MS);
    motor.stop();
    delay(50);

    Serial.println(F("[MANOBRA] Girando DIREITA (vazio, retorno)"));
    motor.move(MotorController::TURN_RIGHT, MANEUVER_SPEED_UNLOADED);
    delay(TURN_90_UNLOADED_MS);
    motor.stop();
} else {
    // Lado direito – mesma lógica
    Serial.println(F("[MANOBRA] Girando DIREITA (carregado)"));
    motor.move(MotorController::TURN_RIGHT, MANEUVER_SPEED_LOADED);
    delay(TURN_90_LOADED_MS);
    motor.stop();
    delay(50);

    motor.move(MotorController::FORWARD, MANEUVER_SPEED_LOADED);
    delay(STRAFE_LOADED_FORWARD_MS);
    motor.stop();
    delay(100);

    gripper.open();
    delay(200);

    motor.move(MotorController::BACKWARD, MANEUVER_SPEED_UNLOADED);
    delay(STRAFE_UNLOADED_BACKWARD_MS);
    motor.stop();
    delay(50);

    motor.move(MotorController::TURN_LEFT, MANEUVER_SPEED_UNLOADED);
    delay(TURN_90_UNLOADED_MS);
    motor.stop();
}

        sideLeft = !sideLeft;
        ultrasonic.resetValidation();
        Serial.print(F("[COLETA] Ciclo concluido. Proximo lado: "));
        Serial.println(sideLeft ? F("ESQUERDA") : F("DIREITA"));
        delay(300);
    }

    motor.stop();
    gripper.open();
    Serial.println(F("\n✓ Teste de coleta concluido"));
}

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    // Inicializa módulos e força parada total
    MotorController motor;
    GripperServo gripper;
    motor.initialize();
    gripper.initialize();
    motor.stop();
    gripper.open();
    delay(300);
    // ----------------------------------------------------------------

    Serial.println(F("Sistema reiniciado com segurança."));
    Serial.println(F("Iniciando coleta autonoma..."));
    testCollector();
    Serial.println(F("Fim do programa."));
    while (1);
}

void loop()
{
    // vazio
}
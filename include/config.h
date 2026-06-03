#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// CONFIGURAÇÃO CENTRAL DO ROBÔ — SEGUIDOR COLETOR
//
// Organização das seções:
//   1. Pinos de hardware
//   2. Calibração dos motores (EDITAR AQUI para corrigir assimetria)
//   3. Velocidade base única
//   4. Sensor de linha
//   5. Controlador PD
//   6. Recuperação de linha
//   7. Detecção de cruzamentos
//   8. Sensor ultrassônico
//   9. Garra (servo)
//  10. Coleta autônoma
//  11. Sistema
// ============================================================================


// ============================================================================
// 1. PINOS DE HARDWARE
// ============================================================================

#define PIN_IN1 5    // Motor esquerdo  — sentido 1
#define PIN_IN2 7    // Motor esquerdo  — sentido 2
#define PIN_IN3 2    // Motor direito — sentido 1
#define PIN_IN4 4    // Motor direito — sentido 2
#define PIN_ENA 6    // PWM motor esquerdo
#define PIN_ENB 3    // PWM motor direito

#define PIN_S1 A0
#define PIN_S2 A1
#define PIN_S3 A2
#define PIN_S4 A3
#define PIN_S5 A4
#define PIN_S6 A5

#define PIN_TRIGGER 12
#define PIN_ECHO    13
#define PIN_SERVO   9


// ============================================================================
// 2. CALIBRAÇÃO DOS MOTORES
//
// Problema: motores TT com assimetria física giram em velocidades diferentes
// mesmo recebendo o mesmo PWM, causando desvio em linha reta.
//
// Solução: fatores de trim individuais por motor.
//   1.00 = sem correção (neutro)
//   > 1.0 = aumenta velocidade deste motor
//   < 1.0 = reduz velocidade deste motor
//
// Como calibrar:
//   1. Coloque o robô em superfície plana sem linha
//   2. Ajuste MOTOR_TRIM_ESQ e MOTOR_TRIM_DIR até andar reto
//   3. Comece com pequenos ajustes (ex: 0.95 ou 1.05)
//   4. Salve os valores que funcionaram — não altere depois
//
// Exemplo: robô desvia para a direita → motor direito está mais rápido
//   → reduza MOTOR_TRIM_DIR para 0.93 ou aumente MOTOR_TRIM_ESQ para 1.07
// ============================================================================

#define MOTOR_TRIM_DIR  0.90f   // fator de correção do motor direito (0.80–1.20)
#define MOTOR_TRIM_ESQ  1.05f   // fator de correção do motor esquerdo  (0.80–1.20)


// ============================================================================
// 3. VELOCIDADE BASE ÚNICA
//
// FONTE DA VERDADE para todas as velocidades do sistema.
// Todos os outros valores de velocidade são derivados deste.
//
//   BASE_SPEED        → velocidade padrão de operação
//   BASE_SPEED * 0.xx → velocidades derivadas (não editar as derivadas)
//
// Alterar BASE_SPEED recalibra todo o sistema proporcionalmente.
// Para ajuste fino de situações específicas, use os OFFSETS abaixo.
// ============================================================================

#define BASE_SPEED  180   // PWM base — ajuste único para todo o sistema

// Derivadas automáticas — NÃO editar, ajustar apenas BASE_SPEED
#define VELOCITY_GLOBAL      BASE_SPEED
#define SPEED_ERROR_LOW      BASE_SPEED -10
#define SPEED_ERROR_MEDIUM   BASE_SPEED -20
#define SPEED_ERROR_HIGH     BASE_SPEED -30

// Offsets opcionais para situações específicas (relativo ao BASE_SPEED)
// Positivo = mais rápido | Negativo = mais lento | 0 = igual ao BASE
#define SPEED_OFFSET_RECOVERY    -10   // recuperação: um pouco mais lento
#define SPEED_OFFSET_INTERSECTION -20  // cruzamento: mais lento para controle
#define SPEED_OFFSET_APPROACH_MED -40  // objeto médio: desacelera
#define SPEED_OFFSET_APPROACH_SLW -50  // objeto perto: desacelera mais

// Derivadas com offset — NÃO editar
#define PWM_SLOW             (BASE_SPEED + SPEED_OFFSET_RECOVERY)
#define SPEED_INTERSECTION   (BASE_SPEED + SPEED_OFFSET_INTERSECTION)
#define APPROACH_SPEED_MEDIUM (BASE_SPEED + SPEED_OFFSET_APPROACH_MED)
#define APPROACH_SPEED_SLOW   (BASE_SPEED + SPEED_OFFSET_APPROACH_SLW)

// PWM mínimo para vencer atrito estático — independente do BASE_SPEED
#define PWM_MIN_DEADZONE  140


// ============================================================================
// 4. SENSOR DE LINHA
// ============================================================================

#define THRESHOLD_LINE_SENSOR  635


// ============================================================================
// 5. CONTROLADOR PD DE SEGUIMENTO DE LINHA
//
// Fórmula: correction = Kp * erro + Kd * (erro - erro_anterior)
//
// Motor externo à curva: mantém baseSpeed constante
// Motor interno à curva: baseSpeed × (1 - |correction|), mínimo PD_MIN_INNER_SPEED
//
// Referência de ajuste por tamanho de pista:
//   Pista pequena / baixa velocidade: Kp=0.4–0.6  Kd=0.1–0.2
//   Pista normal:                     Kp=0.8–1.0  Kd=0.3–0.4
//   Pista grande / alta velocidade:   Kp=1.2–1.5  Kd=0.4–0.6
// ============================================================================

#define PD_KP          1.0f
#define PD_KD          0.7f
#define PD_SAMPLE_MS    10

#define PD_MIN_INNER_SPEED  100


// ============================================================================
// 6. RECUPERAÇÃO DE LINHA PERDIDA
// ============================================================================

#define RECOVERY_SPIN_MS     300
#define RECOVERY_TIMEOUT_MS 1500


// ============================================================================
// 7. DETECÇÃO DE CRUZAMENTOS
// ============================================================================

#define CROSS_MIN_SENSORS_X  5

// ============================================================================
// 8. SENSOR ULTRASSÔNICO (HC-SR04)
// ============================================================================

#define ULTRASONIC_DISTANCE_LONG     25
#define ULTRASONIC_DISTANCE_SHORT    20
#define ULTRASONIC_DISTANCE_CONTACT   4
#define ULTRASONIC_NOISE_TOLERANCE   20
#define ULTRASONIC_TIMEOUT_US     10000
#define SENSOR_FILTER_CYCLES          2


// ============================================================================
// 9. GARRA (SG90)
// ============================================================================

#define SERVO_ANGLE_OPEN          20
#define SERVO_ANGLE_CLOSED       170
#define SERVO_STEP_DELAY_MS       10
#define SERVO_STABILIZATION_TIME 400
#define GRIPPER_STABLE_TIME_MS   300
#define GRIPPER_HOLD_TIME_MS    3000


// ============================================================================
// 10. COLETA AUTÔNOMA
//
// Velocidades de manobra também derivadas do BASE_SPEED.
// ============================================================================

#define APPROACH_SPEED_FAST   BASE_SPEED

#define APPROACH_DIST_LONG    20
#define APPROACH_DIST_MEDIUM  10

#define MANEUVER_SPEED_LOADED    140
#define MANEUVER_SPEED_UNLOADED  100

#define STRAFE_LOADED_MS    700
#define STRAFE_UNLOADED_MS  350
#define TURN_90_LOADED_MS   700
#define TURN_90_UNLOADED_MS 300
#define COLLECT_STOP_DELAY  300
#define COLLECT_CYCLE_DURATION  120000


// ============================================================================
// 11. SISTEMA
// ============================================================================

#define BAUD_RATE   9600
#define DEBUG_MODE  true

#endif // CONFIG_H

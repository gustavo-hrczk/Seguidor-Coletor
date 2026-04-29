#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// CONFIGURAÇÃO CENTRAL DO ROBÔ - SEGUIDOR COLETOR
// ============================================================================

// --- PINOS - MOTOR ---
#define PIN_IN1 2
#define PIN_IN2 4
#define PIN_IN3 5
#define PIN_IN4 7
#define PIN_ENA 3
#define PIN_ENB 6

// --- PINOS - SENSORES DE LINHA (QTR-6) ---
#define PIN_S1 A0   // Extrema esquerda  peso -5
#define PIN_S2 A1   // Esquerda          peso -3
#define PIN_S3 A2   // Centro-esquerda   peso -1
#define PIN_S4 A3   // Centro-direita    peso +1
#define PIN_S5 A4   // Direita           peso +3
#define PIN_S6 A5   // Extrema direita   peso +5

// --- PINOS - SENSOR ULTRASSÔNICO ---
#define PIN_TRIGGER 12
#define PIN_ECHO    13

// --- PINOS - SERVO ---
#define PIN_SERVO 8

// --- VELOCIDADE GLOBAL ---
#define VELOCITY_GLOBAL    200
#define PWM_SLOW            85
#define PWM_MEDIUM         170
#define PWM_FAST           255
#define PWM_MIN_DEADZONE    60

// --- CONFIGURAÇÃO DE TEMPOS ---
#define CYCLE_MAIN               100
#define SENSOR_FILTER_CYCLES       3
#define LINE_SEARCH_TIMEOUT     2000
#define LINE_SEARCH_MAX_ROTATIONS  5
#define ULTRASONIC_DEBOUNCE_TIME 300

// --- ESCALAÇÃO TEMPORAL ---
#define TIME_SCALE_FACTOR      255.0
#define ROTATION_90_DEGREES_TIME 600

// --- LIMIARES ---
#define THRESHOLD_LINE_SENSOR      800
#define ULTRASONIC_DISTANCE_LONG    30
#define ULTRASONIC_DISTANCE_SHORT   15
#define ULTRASONIC_DISTANCE_CONTACT  5
#define ULTRASONIC_NOISE_TOLERANCE  30

// --- CONTROLADOR PD DE SEGUIMENTO ---
// Kp: ganho proporcional — aumentar deixa o robô mais reativo à posição
// Kd: ganho derivativo   — aumentar amortece oscilações
// Aumentar ambos torna o seguimento mais agressivo mas pode causar instabilidade
#define PD_KP          0.8f   // Ganho proporcional (calibrar em pista)
#define PD_KD          0.3f   // Ganho derivativo
#define PD_SAMPLE_MS    20    // Intervalo mínimo entre cálculos PD (ms)

// --- DETECÇÃO DE CRUZAMENTOS ---
// Sensores simultaneamente ativos para classificar como cruzamento
#define CROSS_MIN_SENSORS_T  4   // Cruzamento em T (4 sensores)
#define CROSS_MIN_SENSORS_X  5   // Cruzamento em X (5+ sensores)

// --- RECUPERAÇÃO DE LINHA PERDIDA ---
// Ao perder a linha o robô gira na última direção conhecida por este tempo
// antes de escalar para busca completa
#define RECOVERY_SPIN_MS     400   // Tempo de giro inicial de recuperação (ms)
#define RECOVERY_TIMEOUT_MS 2000   // Timeout total de recuperação

// --- VELOCIDADES POR MAGNITUDE DE ERRO PD ---
// Mapeiam o erro normalizado (0.0–1.0) para PWM
// Erro pequeno  → velocidade alta (reta)
// Erro grande   → velocidade reduzida (curva)
#define SPEED_ERROR_LOW    255   // Erro < 0.3  → reta
#define SPEED_ERROR_MEDIUM 180   // Erro 0.3–0.6 → curva suave/média
#define SPEED_ERROR_HIGH   110   // Erro > 0.6  → curva acentuada/aguda

// --- COMPENSAÇÕES DE CURVA (fator aplicado ao motor interno) ---
#define CURVE_COMPENSATION_LIGHT  0.9f
#define CURVE_COMPENSATION_MEDIUM 0.8f
#define CURVE_COMPENSATION_SHARP  0.6f

// --- CONFIGURAÇÃO DA GARRA ---
#define SERVO_STEP_DELAY_MS      10
#define SERVO_ANGLE_OPEN          5
#define SERVO_ANGLE_CLOSED       90
#define SERVO_STABILIZATION_TIME 400

// --- GARRA REATIVA ---
#define GRIPPER_STABLE_TIME_MS   300
#define GRIPPER_HOLD_TIME_MS    3000

// --- DISTÂNCIAS DE MOVIMENTO ---
#define REVERSE_DISTANCE_AFTER_COLLECT 5

// --- SISTEMA ---
#define BAUD_RATE            9600
#define DEBUG_MODE           true
#define MAX_COLLECTION_ATTEMPTS  3
#define RUNTIME_TOTAL       600000

// --- EEPROM ---
#define EEPROM_ADDR_LAST_TURN 0

#endif
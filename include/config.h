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

// --- VELOCIDADES ---
// VELOCITY_GLOBAL: usado por move() e testes manuais
// SPEED_ERROR_*:   usados exclusivamente por followLine()
#define VELOCITY_GLOBAL       150    // Velocidade para movimentos manuais
#define PWM_SLOW               70    // Velocidade de recuperação de linha
#define PWM_MIN_DEADZONE       60    // PWM mínimo para vencer atrito estático

// --- THRESHOLD DE LINHA ---
// Fundo preto  → valores ALTOS (~1012)
// Linha branca → valores BAIXOS (~779)
// Sensor ativo quando leitura ABAIXO do threshold
// Threshold = fundo_max - (separacao * 0.4) = 1012 - (233*0.4) ≈ 919
// Margem conservadora: usa 870 para tolerar variação de iluminação
#define THRESHOLD_LINE_SENSOR  700
#define LINE_HYSTERESIS 15

// --- FILTRO --- (exclusivo do UltrasonicSensor)
#define SENSOR_FILTER_CYCLES     3

// --- SENSOR ULTRASSÔNICO ---
#define ULTRASONIC_DISTANCE_LONG    30
#define ULTRASONIC_DISTANCE_SHORT   15
#define ULTRASONIC_DISTANCE_CONTACT  5
#define ULTRASONIC_NOISE_TOLERANCE  30
#define ULTRASONIC_DEBOUNCE_TIME   300

// --- CONTROLADOR PD ---
// Kp: reatividade à posição — aumentar para reagir mais rápido à linha
// Kd: amortecimento — aumentar para reduzir oscilação
// PD_SAMPLE_MS: intervalo entre cálculos — menor = mais responsivo
#define PD_KP             0.8f
#define PD_KD             0.3f
#define PD_SAMPLE_MS        10   // 10ms = 100Hz de atualização

// --- VELOCIDADES DE SEGUIMENTO PD ---
// Motor externo sempre em SPEED_ERROR_*, motor interno reduzido pelo PD
// Aumentar todos proporcionalmente para velocidade maior em pista grande
#define SPEED_ERROR_LOW    150   // Reta         (|erro| < 0.3)
#define SPEED_ERROR_MEDIUM 130   // Curva suave  (|erro| 0.3–0.6)
#define SPEED_ERROR_HIGH   110   // Curva aguda  (|erro| > 0.6)

// Motor interno nunca abaixo deste valor (garante torque mínimo em curvas)
#define PD_MIN_INNER_SPEED  70

// --- RECUPERAÇÃO DE LINHA ---
#define RECOVERY_SPIN_MS    300   // Gira na última direção por este tempo
#define RECOVERY_TIMEOUT_MS 1500  // Timeout total antes de parar

// --- DETECÇÃO DE CRUZAMENTOS ---
#define CROSS_MIN_SENSORS_T  4
#define CROSS_MIN_SENSORS_X  5

// --- COMPENSAÇÕES DE CURVA (para curveCompensated()) ---
#define CURVE_COMPENSATION_LIGHT  0.9f
#define CURVE_COMPENSATION_MEDIUM 0.8f
#define CURVE_COMPENSATION_SHARP  0.6f

// --- GARRA ---
#define SERVO_STEP_DELAY_MS      10
#define SERVO_ANGLE_OPEN          5
#define SERVO_ANGLE_CLOSED       90
#define SERVO_STABILIZATION_TIME 400
#define GRIPPER_STABLE_TIME_MS   300
#define GRIPPER_HOLD_TIME_MS    3000

// --- SISTEMA ---
#define BAUD_RATE        9600
#define DEBUG_MODE       true
#define RUNTIME_TOTAL    600000
#define EEPROM_ADDR_LAST_TURN 0

#endif
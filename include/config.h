#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// CONFIGURAÇÃO CENTRAL DO ROBÔ - SEGUIDOR COLETOR
// ============================================================================
// Este arquivo centraliza TODAS as constantes e configurações do sistema.
// Modificar aqui afeta todo o comportamento do robô.
// ============================================================================

// --- VELOCIDADE GLOBAL (PWM base para todas as operações) ---
#define VELOCITY_GLOBAL 200          // Base de PWM (0-255). Ajuste principal do robô.
#define PWM_SLOW 85                  // Velocidade lenta (~33% de VELOCITY_GLOBAL)
#define PWM_MEDIUM 170               // Velocidade média (~67% de VELOCITY_GLOBAL)
#define PWM_FAST 255                 // Velocidade rápida (~100% de VELOCITY_GLOBAL)
#define PWM_MIN_DEADZONE 60          // PWM mínimo para vencer atrito estático

// --- CONFIGURAÇÃO DE TEMPOS (em ms) ---
#define CYCLE_MAIN 100               // Ciclo principal de atualização
#define SENSOR_FILTER_CYCLES 3       // Ciclos de filtro/debounce para validação
#define SERVO_TIMEOUT 3000           // Timeout para liberar servo se travar (ms)
#define LINE_SEARCH_TIMEOUT 2000     // Timeout para busca de linha (200ms de perda máx)
#define LINE_SEARCH_MAX_ROTATIONS 5  // Máximo de rotações de 360° para buscar linha
#define ULTRASONIC_DEBOUNCE_TIME 300 // Tempo mínimo entre detecções (ms)

// --- ESCALAÇÃO TEMPORAL (inversamente proporcional à velocidade) ---
// Fórmula: Tempo_Acao = K / (velocidade_global / 255)
// K é calibrado empiricamente para velocidade = 255
#define TIME_SCALE_FACTOR 255.0      // Constante de calibração
#define ROTATION_90_DEGREES_TIME 600 // Tempo para girar 90° a VELOCITY_GLOBAL

// --- CONFIGURAÇÃO DE PINOS - MOTOR ---
#define PIN_IN1 2                    // Motor direita - sentido 1
#define PIN_IN2 4                    // Motor direita - sentido 2
#define PIN_IN3 5                    // Motor esquerda - sentido 1
#define PIN_IN4 7                    // Motor esquerda - sentido 2
#define PIN_ENA 3                    // PWM motor esquerda (Timer 2)
#define PIN_ENB 6                    // PWM motor direita (Timer 0)

// --- CONFIGURAÇÃO DE PINOS - SENSORES DE LINHA (QTR-6) ---
#define PIN_S1 A0                    // Sensor extrema esquerda
#define PIN_S2 A1                    // Sensor esquerda
#define PIN_S3 A2                    // Sensor centro-esquerda
#define PIN_S4 A3                    // Sensor centro-direita
#define PIN_S5 A4                    // Sensor direita
#define PIN_S6 A5                    // Sensor extrema direita

// --- CONFIGURAÇÃO DE PINOS - LED RGB ---
//#define PIN_LED_R 9                  // LED RGB - Vermelho
//#define PIN_LED_G 11                 // LED RGB - Verde
//#define PIN_LED_B 10                 // LED RGB - Azul

// --- CONFIGURAÇÃO DE PINOS - SENSOR ULTRASSÔNICO ---
#define PIN_TRIGGER 13               // Pino Trigger HC-SR04
#define PIN_ECHO 12                  // Pino Echo HC-SR04

// --- CONFIGURAÇÃO DE PINOS - SERVO (GARRA) ---
#define PIN_SERVO 8                  // Pino servo da garra

// --- LIMIARES E THRESHOLDS ---
#define THRESHOLD_LINE_SENSOR 700    // Limite para distinguir linha branca de fundo
#define ULTRASONIC_DISTANCE_LONG 30  // Distância longa (fase 1 de aproximação) - cm
#define ULTRASONIC_DISTANCE_SHORT 15 // Distância curta (fase 2 de aproximação) - cm
#define ULTRASONIC_DISTANCE_CONTACT 5 // Distância de contato (fase 3 - coleta) - cm
#define ULTRASONIC_NOISE_TOLERANCE 5 // Tolerância de variação para debounce (%)

// --- VELOCIDADES ESPECÍFICAS POR PADRÃO DE SENSOR ---
// Array de velocidades indexado pelo padrão de sensor (6 bits)
// Valores são aplicados como ganhos proporcionais à velocidade global
#define SPEED_STRAIGHT 255           // Reta nos 2 sensores centrais
#define SPEED_CURVE_LIGHT 220        // Curva suave (sensor 3)
#define SPEED_CURVE_MEDIUM 150       // Curva média (sensor 4)
#define SPEED_CURVE_SHARP_1 100      // Curva acentuada (fase 1)
#define SPEED_CURVE_SHARP_2 80       // Curva acentuada (fase 2)
#define SPEED_CURVE_EXTREME_LEFT 50  // Curva extrema (esquerda)
#define SPEED_CURVE_EXTREME_RIGHT 50 // Curva extrema (direita)
#define SPEED_CURVE_RECOVERY 200     // Velocidade de recuperação em curva

// --- DISTÂNCIAS E COMPENSAÇÕES DE MOTOR ---
#define CURVE_COMPENSATION_LIGHT 0.9 // Compensação suave (10% de diferença)
#define CURVE_COMPENSATION_MEDIUM 0.8 // Compensação média (20% de diferença)
#define CURVE_COMPENSATION_SHARP 0.6  // Compensação acentuada (40% de diferença)

// --- CONFIGURAÇÃO DA GARRA ---
#define SERVO_ANGLE_OPEN 180         // Ângulo servo - garra aberta
#define SERVO_ANGLE_CLOSED 0         // Ângulo servo - garra fechada
#define SERVO_STABILIZATION_TIME 500 // Tempo para estabilizar garra após fechar

// --- DISTÂNCIAS DE MOVIMENTO ---
#define REVERSE_DISTANCE_AFTER_COLLECT 5 // Distância em cm para recuar após coleta

// --- CONFIGURAÇÃO GERAL DO SISTEMA ---
#define BAUD_RATE 9600              // Taxa de comunicação serial
#define DEBUG_MODE true              // Ativar logs via Serial
#define MAX_COLLECTION_ATTEMPTS 3    // Máximo de tentativas de coleta por objeto
#define RUNTIME_TOTAL 600000         // Tempo total de execução (10 minutos em ms)

// --- ENDEREÇOS DE EEPROM (para persistência) ---
#define EEPROM_ADDR_LAST_TURN 0      // Endereço para armazenar último giro (ESQUERDA=0, DIREITA=1)

#endif // CONFIG_H

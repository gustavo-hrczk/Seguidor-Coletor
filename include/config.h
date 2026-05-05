#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// CONFIGURAÇÃO CENTRAL DO ROBÔ — SEGUIDOR COLETOR
// ============================================================================
// Arquivo único de configuração. Alterar aqui afeta todos os módulos.
// Consulte docs/CONFIG_INDEX.md para descrição completa de cada constante.
// ============================================================================

// ----------------------------------------------------------------------------
// PINOS — MOTOR (L298N)
// Convenção de sinal em MotorController::setMotorSpeed():
//   Motor esquerdo montado invertido: positivo = ré, negativo = frente
//   Motor direito:                    positivo = frente, negativo = ré
// ----------------------------------------------------------------------------
#define PIN_IN1 2    // Motor direito  — sentido 1
#define PIN_IN2 4    // Motor direito  — sentido 2
#define PIN_IN3 5    // Motor esquerdo — sentido 1
#define PIN_IN4 7    // Motor esquerdo — sentido 2
#define PIN_ENA 3    // PWM motor esquerdo (Timer 2)
#define PIN_ENB 6    // PWM motor direito  (Timer 0)

// ----------------------------------------------------------------------------
// PINOS — SENSORES DE LINHA (QTR-6 analógico)
// Pesos do centro de massa ponderado: S1=-5  S2=-3  S3=-1  S4=+1  S5=+3  S6=+5
// ----------------------------------------------------------------------------
#define PIN_S1 A0    // Extrema esquerda  (peso -5)
#define PIN_S2 A1    // Esquerda          (peso -3)
#define PIN_S3 A2    // Centro-esquerda   (peso -1)
#define PIN_S4 A3    // Centro-direita    (peso +1)
#define PIN_S5 A4    // Direita           (peso +3)  ← sensor fraco, funcional
#define PIN_S6 A5    // Extrema direita   (peso +5)

// ----------------------------------------------------------------------------
// PINOS — SENSOR ULTRASSÔNICO (HC-SR04)
// ----------------------------------------------------------------------------
#define PIN_TRIGGER 12
#define PIN_ECHO    13

// ----------------------------------------------------------------------------
// PINOS — SERVO (SG90)
// ----------------------------------------------------------------------------
#define PIN_SERVO 9

// ----------------------------------------------------------------------------
// VELOCIDADES
// VELOCITY_GLOBAL e PWM_SLOW: usados por move() e handleRecovery()
// SPEED_ERROR_*: usados EXCLUSIVAMENTE por followLine() via handleFollowing()
// Não misturar — são escalas independentes
// ----------------------------------------------------------------------------
#define VELOCITY_GLOBAL      200   // PWM para movimentos manuais e testes
#define PWM_SLOW              70   // PWM de recuperação de linha perdida
#define PWM_MIN_DEADZONE      60   // PWM mínimo para vencer atrito estático

// ----------------------------------------------------------------------------
// SENSOR DE LINHA — THRESHOLD E MÁSCARA
//
// Convenção (confirmada pelo hardware):
//   Linha branca → analogRead BAIXO (~50–200)
//   Fundo preto  → analogRead ALTO  (~800–1023)
//   Sensor ATIVO quando raw <= THRESHOLD_LINE_SENSOR
//
// Calibrar com Teste 6 (test_components.cpp) antes de operar.
// Valor atual baseado em: max_linha≈200, min_fundo≈900, threshold=550
//
// LINE_SENSOR_MASK: bitmask de sensores habilitados (1=ativo, 0=ignorado)
//   Bit 0 = S1, Bit 1 = S2, ..., Bit 5 = S6
//   S1 (A0) desabilitado por defeito físico — substituir e setar 0b111111
// ----------------------------------------------------------------------------
#define THRESHOLD_LINE_SENSOR  700

// ----------------------------------------------------------------------------
// CONTROLADOR PD DE SEGUIMENTO DE LINHA
//
// correction = Kp * erro + Kd * (erro - erro_anterior)
// Motor externo = baseSpeed (constante)
// Motor interno = baseSpeed * (1 - |correction|), mínimo PD_MIN_INNER_SPEED
//
// Para pista pequena: Kp=0.5 Kd=0.1
// Para pista grande:  Kp=1.2 Kd=0.4
// ----------------------------------------------------------------------------
#define PD_KP          0.8f   // Ganho proporcional
#define PD_KD          0.3f   // Ganho derivativo (amortecimento)
#define PD_SAMPLE_MS    10    // Intervalo entre cálculos PD (ms) — 10ms = 100Hz

// ----------------------------------------------------------------------------
// VELOCIDADES DE SEGUIMENTO PD
// Selecionadas por magnitude do erro em handleFollowing():
//   |erro| < 0.3  → SPEED_ERROR_LOW   (reta)
//   |erro| 0.3–0.6 → SPEED_ERROR_MEDIUM (curva suave)
//   |erro| > 0.6  → SPEED_ERROR_HIGH  (curva aguda)
// ----------------------------------------------------------------------------
#define SPEED_ERROR_LOW    150   // Reta
#define SPEED_ERROR_MEDIUM 130   // Curva suave/média
#define SPEED_ERROR_HIGH   110   // Curva aguda

// Motor interno nunca cai abaixo deste valor — garante torque mínimo em curvas
#define PD_MIN_INNER_SPEED  70

// ----------------------------------------------------------------------------
// RECUPERAÇÃO DE LINHA PERDIDA
// Estágio 1: gira na última direção conhecida por RECOVERY_SPIN_MS
// Estágio 2: gira na direção oposta até RECOVERY_TIMEOUT_MS
// Após timeout: para e entra em STATE_STOPPED
// ----------------------------------------------------------------------------
#define RECOVERY_SPIN_MS     300   // ms girando na última direção
#define RECOVERY_TIMEOUT_MS 1500   // ms total antes de parar

// ----------------------------------------------------------------------------
// DETECÇÃO DE CRUZAMENTOS
// Baseada em contagem de sensores ativos simultâneos:
//   4 sensores → cruzamento T (TURN_LEFT_90 ou TURN_RIGHT_90)
//   5+ sensores → cruzamento X (INTERSECTION)
// ----------------------------------------------------------------------------
#define CROSS_MIN_SENSORS_T  4
#define CROSS_MIN_SENSORS_X  5

// ----------------------------------------------------------------------------
// COMPENSAÇÕES DE CURVA
// Usadas por MotorController::curveCompensated() nos testes.
// NÃO afetam followLine() — o PD calcula sua própria compensação.
// ----------------------------------------------------------------------------
#define CURVE_COMPENSATION_LIGHT  0.9f   // 10% de diferença entre motores
#define CURVE_COMPENSATION_MEDIUM 0.8f   // 20%
#define CURVE_COMPENSATION_SHARP  0.6f   // 40%

// ----------------------------------------------------------------------------
// SENSOR ULTRASSÔNICO (HC-SR04)
// Fases de aproximação para acionamento da garra:
//   PHASE_1_DISTANT:     distância >= LONG  (desacelerar)
//   PHASE_2_APPROACHING: distância >= SHORT (velocidade lenta)
//   PHASE_3_CONTACT:     distância <  SHORT (parar e coletar)
// Gatilho da garra: distância <= ULTRASONIC_DISTANCE_CONTACT
// ----------------------------------------------------------------------------
#define ULTRASONIC_DISTANCE_LONG    30   // cm — fase 1
#define ULTRASONIC_DISTANCE_SHORT   15   // cm — fase 2
#define ULTRASONIC_DISTANCE_CONTACT  4   // cm — gatilho de coleta
#define ULTRASONIC_NOISE_TOLERANCE  30   // % de variação tolerada
#define ULTRASONIC_DEBOUNCE_TIME   300   // ms mínimo entre detecções
#define SENSOR_FILTER_CYCLES         3   // leituras consecutivas para validar

// ----------------------------------------------------------------------------
// GARRA (GripperServo / SG90)
// moveToAngle() move grau a grau com SERVO_STEP_DELAY_MS entre cada passo.
// Após atingir o ângulo: aguarda SERVO_STABILIZATION_TIME e faz detach()
// para eliminar aquecimento e vibração do motor entre comandos.
// ----------------------------------------------------------------------------
#define SERVO_ANGLE_OPEN          5    // graus — evitar 0° (limite mecânico)
#define SERVO_ANGLE_CLOSED       170    // graus
#define SERVO_STEP_DELAY_MS      10    // ms entre cada grau (menor = mais rápido)
#define SERVO_STABILIZATION_TIME 400   // ms aguardando antes do detach()

// Lógica reativa de coleta (UltrasonicSensor + GripperServo):
#define GRIPPER_STABLE_TIME_MS   300   // ms contínuos dentro da zona para fechar
#define GRIPPER_HOLD_TIME_MS    3000   // ms com garra fechada antes de reabrir

// ----------------------------------------------------------------------------
// SISTEMA
// ----------------------------------------------------------------------------
#define BAUD_RATE        9600    // Taxa serial — monitor deve usar o mesmo valor
#define DEBUG_MODE       true    // false remove todo código de log (libera memória)
#define RUNTIME_TOTAL    600000  // ms — reservado para uso futuro
#define EEPROM_ADDR_LAST_TURN 0  // endereço EEPROM para última direção de giro

// ----------------------------------------------------------------------------
// CONFIGURAÇÕES PARA TESTE DE COLETA AUTÔNOMA // REMOVIVEIS APÓS ARQUIVO FINAL
// ----------------------------------------------------------------------------
#define MANEUVER_SPEED           200   // giro e deslocamento lateral
#define COLLECT_STOP_DELAY       300   // ms para estabilizar antes da manobra
#define COLLECT_CYCLE_DURATION   30000 // duração total do teste (2 minutos)

// Tempos de deslocamento lateral (ms) – diferentes para garra fechada e aberta
#define STRAFE_LOADED_FORWARD_MS    300   // ida (com objeto)
#define STRAFE_UNLOADED_BACKWARD_MS 250   // volta sem objeto – usada para retornar

// Giros (se o peso afetar, pode ter versões carregado/vazio)
#define TURN_90_LOADED_MS   750   // giro com carga
#define TURN_90_UNLOADED_MS 650   // giro sem carga

#define MANEUVER_SPEED_LOADED   220   // potência maior com carga
#define MANEUVER_SPEED_UNLOADED 200   // normal sem carga

#define APPROACH_SPEED_FAST      200   // > 20 cm
#define APPROACH_SPEED_MEDIUM    130   // 10-20 cm
#define APPROACH_SPEED_SLOW       80   // < 10 cm
#define APPROACH_DIST_LONG        20   // cm
#define APPROACH_DIST_MEDIUM      10   // cm

#endif // CONFIG_H

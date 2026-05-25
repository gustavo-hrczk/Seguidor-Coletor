#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// CONFIGURAÇÃO CENTRAL DO ROBÔ — SEGUIDOR COLETOR
//
// Arquivo único de configuração do projeto. Alterar um valor aqui afeta
// todos os módulos que o consomem — consulte docs/CONFIG_INDEX.md para
// a descrição completa de cada constante e qual módulo a utiliza.
//
// Organização das seções:
//   1. Pinos de hardware
//   2. Velocidades gerais
//   3. Sensor de linha
//   4. Controlador PD
//   5. Recuperação de linha
//   6. Detecção de cruzamentos
//   7. Compensações de curva
//   8. Sensor ultrassônico
//   9. Garra (servo)
//  10. Teste de coleta autônoma
//  11. Sistema
// ============================================================================


// ============================================================================
// 1. PINOS DE HARDWARE
// ============================================================================

// --- Motor DC (driver L298N) ---
// Convenção de sinal adotada em MotorController::setMotorSpeed():
//   Motor esquerdo montado invertido no chassi:
//     valor positivo = ré | valor negativo = frente
//   Motor direito (montagem padrão):
//     valor positivo = frente | valor negativo = ré
//   Essa assimetria é compensada internamente — não altere sem revisar move()
#define PIN_IN1 4    // Motor direito  — sentido 1 (L298N: IN1)
#define PIN_IN2 2    // Motor direito  — sentido 2 (L298N: IN2)
#define PIN_IN3 7    // Motor esquerdo — sentido 1 (L298N: IN3)
#define PIN_IN4 6    // Motor esquerdo — sentido 2 (L298N: IN4)
#define PIN_ENA 3    // PWM motor esquerdo — Timer 2 do UNO
#define PIN_ENB 5    // PWM motor direito  — Timer 0 do UNO
// Portas PWM suportadas no UNO: 3, 5, 6, 9, 10, 11

// --- Sensores de linha (QTR-1 analógico × 6) ---
// Disposição física da esquerda para a direita do robô.
// Pesos do centro de massa ponderado: S1=-5  S2=-3  S3=-1  S4=+1  S5=+3  S6=+5
#define PIN_S1 A0    // Extrema esquerda  (peso -5)
#define PIN_S2 A1    // Esquerda          (peso -3)
#define PIN_S3 A2    // Centro-esquerda   (peso -1)
#define PIN_S4 A3    // Centro-direita    (peso +1)
#define PIN_S5 A4    // Direita           (peso +3)
#define PIN_S6 A5    // Extrema direita   (peso +5)

// --- Sensor ultrassônico (HC-SR04) ---
#define PIN_TRIGGER 12
#define PIN_ECHO    13

// --- Servo da garra (SG90) ---
#define PIN_SERVO 9


// ============================================================================
// 2. VELOCIDADES GERAIS
//
// VELOCITY_GLOBAL e PWM_SLOW: usados por move() e pela recuperação de linha.
// SPEED_ERROR_*: usados EXCLUSIVAMENTE por followLine() — não misturar.
// ============================================================================

// Velocidade padrão para movimentos manuais e testes isolados
#define VELOCITY_GLOBAL      220

// Velocidade de recuperação de linha perdida — baixa para maior controle
#define PWM_SLOW              170

// PWM mínimo para vencer o atrito estático dos motores TT.
// Abaixo desse valor o motor recebe sinal mas não gira com carga.
#define PWM_MIN_DEADZONE      60


// ============================================================================
// 3. SENSOR DE LINHA
//
// Convenção (QTR analógico, linha BRANCA sobre fundo PRETO):
//   Linha branca → analogRead BAIXO (~50–200)   ← superfície refletiva
//   Fundo preto  → analogRead ALTO  (~800–1023) ← superfície absorvente
//   Sensor ATIVO quando raw <= THRESHOLD_LINE_SENSOR
//
// Calibração: execute o Teste 6 (test_components.cpp) na pista real
// com a iluminação do local de uso. O valor abaixo é um ponto de partida.
// ============================================================================

// Ponto de corte entre linha e fundo — ajustar via Teste 6
#define THRESHOLD_LINE_SENSOR  700


// ============================================================================
// 4. CONTROLADOR PD DE SEGUIMENTO DE LINHA
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

#define PD_KP         0.8f   // Ganho proporcional — reatividade à posição da linha
#define PD_KD         0.3f   // Ganho derivativo   — amortece oscilações
#define PD_SAMPLE_MS   10    // Intervalo entre cálculos (ms) — 10ms = 100 Hz

// Velocidades base selecionadas em handleFollowing() pela magnitude do erro:
//   |erro| < 0.3              → SPEED_ERROR_LOW   (reta)
//   |erro| entre 0.3 e 0.6   → SPEED_ERROR_MEDIUM (curva suave)
//   |erro| > 0.6              → SPEED_ERROR_HIGH  (curva aguda)
#define SPEED_ERROR_LOW    150
#define SPEED_ERROR_MEDIUM 130
#define SPEED_ERROR_HIGH   110

// Piso de velocidade do motor interno — garante torque mínimo em curvas fechadas
#define PD_MIN_INNER_SPEED  70


// ============================================================================
// 5. RECUPERAÇÃO DE LINHA PERDIDA
//
// Estratégia em dois estágios quando LINE_LOST é detectado:
//   Estágio 1: gira na última direção conhecida por RECOVERY_SPIN_MS
//   Estágio 2: gira na direção oposta até RECOVERY_TIMEOUT_MS
//   Timeout:   para motores e entra em STATE_STOPPED
// ============================================================================

#define RECOVERY_SPIN_MS     300   // ms no estágio 1
#define RECOVERY_TIMEOUT_MS 1500   // ms total antes de parar completamente


// ============================================================================
// 6. DETECÇÃO DE CRUZAMENTOS
//
// Classificação pelo número de sensores ativos simultaneamente:
//   == CROSS_MIN_SENSORS_T → cruzamento T (TURN_LEFT_90 ou TURN_RIGHT_90)
//   >= CROSS_MIN_SENSORS_X → cruzamento X (INTERSECTION)
// ============================================================================

#define CROSS_MIN_SENSORS_T  4
#define CROSS_MIN_SENSORS_X  5


// ============================================================================
// 7. COMPENSAÇÕES DE CURVA
//
// Usadas por MotorController::curveCompensated() nos testes manuais.
// NÃO afetam followLine() — o PD calcula sua própria redução.
//
// Motor externo = speed | Motor interno = speed × fator
// ============================================================================

#define CURVE_COMPENSATION_LIGHT  0.9f   // diferença de 10% entre motores
#define CURVE_COMPENSATION_MEDIUM 0.8f   // diferença de 20%
#define CURVE_COMPENSATION_SHARP  0.6f   // diferença de 40%


// ============================================================================
// 8. SENSOR ULTRASSÔNICO (HC-SR04)
//
// Fases de aproximação — usadas para controle de velocidade e coleta:
//   PHASE_1_DISTANT     : dist >= ULTRASONIC_DISTANCE_LONG  → desacelerar
//   PHASE_2_APPROACHING : dist >= ULTRASONIC_DISTANCE_SHORT → velocidade lenta
//   PHASE_3_CONTACT     : dist <  ULTRASONIC_DISTANCE_SHORT → coletar
//
// O gatilho real da garra usa ULTRASONIC_DISTANCE_CONTACT, que pode ser
// igual ou menor que ULTRASONIC_DISTANCE_SHORT conforme a física da garra.
// ============================================================================

#define ULTRASONIC_DISTANCE_LONG    30   // cm — limite da fase 1
#define ULTRASONIC_DISTANCE_SHORT   15   // cm — limite da fase 2
#define ULTRASONIC_DISTANCE_CONTACT  4   // cm — gatilho de fechamento da garra

// Tolerância de variação entre leituras consecutivas para validação (%)
// Acima desse percentual a leitura é considerada divergente
#define ULTRASONIC_NOISE_TOLERANCE  30

// Timeout do pulseIn: 15ms cobre até ~250cm — 2× mais rápido que 30ms
// em ausência de eco, reduzindo latência do loop de aproximação
#define ULTRASONIC_TIMEOUT_US   15000

// Leituras consecutivas dentro da tolerância para marcar readingStable.
// Valor 2 equilibra velocidade de resposta e proteção contra espúrios.
#define SENSOR_FILTER_CYCLES     2


// ============================================================================
// 9. GARRA (GripperServo / SG90)
//
// O servo move grau a grau com SERVO_STEP_DELAY_MS entre cada passo.
// Após atingir o ângulo alvo, aguarda SERVO_STABILIZATION_TIME e executa
// detach() — elimina o aquecimento causado por PWM contínuo sem carga.
// ============================================================================

// Ângulos físicos — evitar 0° pois é o limite mecânico do SG90
#define SERVO_ANGLE_OPEN      5    // graus — posição aberta
#define SERVO_ANGLE_CLOSED  170    // graus — posição fechada

// Velocidade do movimento: menor valor = movimento mais rápido
#define SERVO_STEP_DELAY_MS  10    // ms entre cada grau de movimento

// Tempo de espera após atingir ângulo alvo, antes de desligar o PWM
#define SERVO_STABILIZATION_TIME 400   // ms

// Lógica reativa de coleta (UltrasonicSensor → GripperServo):
#define GRIPPER_STABLE_TIME_MS   300   // ms contínuos dentro da zona para fechar
#define GRIPPER_HOLD_TIME_MS    3000   // ms com garra fechada antes de reabrir


// ============================================================================
// 10. TESTE DE COLETA AUTÔNOMA (test_collect.cpp)
//
// Constantes exclusivas do ciclo de coleta sem seguidor de linha.
// Ajustar aqui sem alterar os parâmetros dos demais módulos.
//
// Fluxo do ciclo:
//   SEARCHING   → avança até objeto a <= ULTRASONIC_DISTANCE_CONTACT cm
//   COLLECTING  → fecha garra
//   MANEUVERING → gira 90° e avança lateralmente (alternando esq/dir)
//   RELEASING   → abre garra
//   RETURNING   → recua e gira de volta ao eixo original
//   Repete até COLLECT_CYCLE_DURATION ms
// ============================================================================

// Velocidades de aproximação — reduzem progressivamente conforme distância
#define APPROACH_SPEED_FAST    200   // PWM quando dist > APPROACH_DIST_LONG
#define APPROACH_SPEED_MEDIUM  170   // PWM quando dist entre MEDIUM e LONG
#define APPROACH_SPEED_SLOW    140   // PWM quando dist <= APPROACH_DIST_MEDIUM
// APPROACH_SPEED_SLOW deve ser maior que PWM_MIN_DEADZONE (60) para
// garantir que o motor gire com carga mesmo com bateria em baixa tensão

// Limiares de distância para troca de velocidade de aproximação (cm)
#define APPROACH_DIST_LONG    20
#define APPROACH_DIST_MEDIUM  10

// Velocidades da manobra — distintas para compensar diferença de inércia com/sem carga
#define MANEUVER_SPEED_LOADED    220   // PWM com objeto na garra
#define MANEUVER_SPEED_UNLOADED  200   // PWM no retorno sem objeto

// Tempo de deslocamento lateral (ms)
// O retorno usa o mesmo tempo na direção oposta para voltar ao eixo original
#define STRAFE_LOADED_MS    300   // avanço lateral com objeto
#define STRAFE_UNLOADED_MS  250   // recuo lateral sem objeto

// Tempo de giro de 90° (ms) — calibrar na pista com o peso real do robô.
// Valores distintos compensam a diferença de inércia rotacional com/sem carga.
#define TURN_90_LOADED_MS    750   // giro com carga
#define TURN_90_UNLOADED_MS  550   // giro sem carga

// Pausa entre etapas do ciclo — permite estabilização mecânica do chassi
#define COLLECT_STOP_DELAY   300   // ms

// Duração total do programa de coleta
#define COLLECT_CYCLE_DURATION  120000   // ms (2 minutos)


// ============================================================================
// 11. SISTEMA
// ============================================================================

#define BAUD_RATE  9600   // Taxa serial — monitor deve usar o mesmo valor

// DEBUG_MODE: true  → todos os módulos imprimem no Serial
//             false → código de log removido em compilação (libera memória)
#define DEBUG_MODE  true

// Endereço EEPROM para persistir última direção de giro entre reinicializações
#define EEPROM_ADDR_LAST_TURN  0

#endif // CONFIG_H

# Seguidor Coletor — Arduino UNO

Robô seguidor de linha com módulo de coleta de objetos via garra servo.
Desenvolvido em C++ para Arduino UNO com arquitetura modular orientada a classes.

---

## Hardware

| Componente          | Modelo           | Qtd |
|---------------------|------------------|-----|
| Microcontrolador    | Arduino UNO      | 1   |
| Driver de motor     | L298N            | 1   |
| Motores DC          | TT Motor 3–6V    | 2   |
| Sensor de linha     | QTR-1 analógico  | 6   |
| Sensor ultrassônico | HC-SR04          | 1   |
| Servo               | SG90             | 1   |

### Pinagem

```
MOTOR (L298N)
  IN1 → D2    IN2 → D4    ENA → D3   (PWM motor direito)
  IN3 → D5    IN4 → D7    ENB → D6   (PWM motor esquerdo)

SENSOR DE LINHA (QTR-6, esquerda → direita)
  S1 → A0   S2 → A1   S3 → A2
  S4 → A3   S5 → A4   S6 → A5

ULTRASSÔNICO (HC-SR04)
  TRIGGER → D12   ECHO → D13

SERVO (SG90)
  SINAL → D9
```

---

## Estrutura do projeto

```
/
├── src/
│   ├── main.cpp              # Máquina de estados principal (seguidor) — EM DESENVOLVIMENTO
│   ├── test_components.cpp   # Menu de testes isolados por módulo
│   ├── test_collect.cpp      # Teste de ciclo de coleta sem seguidor de linha
│   └── config.h              # Configuração central — edite aqui
├── lib/
│   ├── LineSensor/
│   │   ├── LineSensor.h
│   │   └── LineSensor.cpp
│   ├── MotorController/
│   │   ├── MotorController.h
│   │   └── MotorController.cpp
│   ├── UltrasonicSensor/
│   │   ├── UltrasonicSensor.h
│   │   └── UltrasonicSensor.cpp
│   └── GripperServo/
│       ├── GripperServo.h
│       └── GripperServo.cpp
└── docs/
    ├── README.md          # Este arquivo
    ├── CALIBRATION.md     # Guia de calibração passo a passo
    └── CONFIG_INDEX.md    # Índice de todas as constantes do config.h
```

---

## Módulos

### `LineSensor`
Lê os 6 sensores QTR analógicos e calcula a posição da linha pelo algoritmo
de centro de massa ponderado. Retorna posição normalizada de -1.0 (extrema
esquerda) a +1.0 (extrema direita) e classifica o padrão de navegação
(STRAIGHT, CURVE_LIGHT, CURVE_MEDIUM, CURVE_SHARP, INTERSECTION, LINE_LOST).

Sem filtro de debounce — reatividade máxima é necessária para seguimento em
alta velocidade. Filtragem pertence ao `UltrasonicSensor`.

### `MotorController`
Controla dois motores DC via L298N. Implementa o controlador PD de seguimento
em `followLine()`: motor externo à curva mantém velocidade base, motor interno
é reduzido proporcionalmente ao erro, nunca abaixo de `PD_MIN_INNER_SPEED`.

### `UltrasonicSensor`
Lê o HC-SR04 com validação por janela deslizante (SENSOR_FILTER_CYCLES leituras
consecutivas consistentes). Classifica distância em 3 fases: DISTANT (>30 cm),
APPROACHING (>15 cm), CONTACT (≤5 cm). Tolerância adaptativa: ±2 cm fixo para
distâncias curtas, percentual com teto de ±4 cm para distâncias longas.

### `GripperServo`
Controla o SG90 com movimento gradual grau a grau e `detach()` após cada
posicionamento, eliminando aquecimento e micro-vibrações em repouso — problema
comum quando o servo recebe PWM contínuo sem carga.

---

## Fluxo de operação (main.cpp — em desenvolvimento)

```
SETUP → inicialização dos módulos → delay 3s → STATE_FOLLOWING

STATE_FOLLOWING
  ├─ Lê LineSensor
  ├─ Seleciona baseSpeed pela magnitude do erro (SPEED_ERROR_*)
  ├─ Chama motor.followLine(pos, baseSpeed)
  └─ Se LINE_LOST → STATE_RECOVERING

STATE_RECOVERING
  ├─ Estágio 1: gira na última direção por RECOVERY_SPIN_MS
  ├─ Estágio 2: gira na direção oposta até RECOVERY_TIMEOUT_MS
  ├─ Se linha encontrada → STATE_FOLLOWING
  └─ Se timeout → STATE_STOPPED

STATE_STOPPED
  └─ Para motores, aguarda intervenção manual
```

> `UltrasonicSensor` e `GripperServo` serão acoplados como `STATE_COLLECTING`
> entre `STATE_FOLLOWING` e `STATE_RECOVERING`.

---

## Início rápido

1. Grave `test_components.cpp` e abra o monitor serial em 9600 baud
2. Execute o **Teste 6** para calibrar o threshold da linha na pista real
3. Atualize `THRESHOLD_LINE_SENSOR` no `config.h`
4. Execute o **Teste 2** para validar leitura e posição dos sensores
5. Execute o **Teste 1** para validar direções dos motores
6. Execute o **Teste 3** para validar fases do ultrassônico
7. Execute o **Teste 4** para validar abertura/fechamento da garra
8. Grave `main.cpp` e posicione o robô sobre a linha

---

## Debug

Com `DEBUG_MODE true` no `config.h`, todos os módulos imprimem no Serial
a 9600 baud. Setar `false` remove todo o código de log em tempo de compilação,
liberando memória de programa no Arduino UNO.

| Prefixo serial    | Módulo                        |
|-------------------|-------------------------------|
| `[Line]`          | LineSensor                    |
| `[PD]`            | MotorController::followLine   |
| `[Ultrasonic]`    | UltrasonicSensor              |
| `[Gripper]`       | GripperServo                  |
| `[Main]`          | main.cpp / test_collect.cpp   |

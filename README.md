# Seguidor Coletor — Arduino UNO

Robô seguidor de linha com módulo de coleta de objetos via garra servo. Desenvolvido em C++ para Arduino UNO com arquitetura modular orientada a classes.

---

## Hardware

| Componente | Modelo | Quantidade |
|---|---|---|
| Microcontrolador | Arduino UNO | 1 |
| Driver de motor | L298N | 1 |
| Motores DC | TT Motor 3–6V | 2 |
| Sensor de linha | QTR-1 analógico | 6 |
| Sensor ultrassônico | HC-SR04 | 1 |
| Servo | SG90 | 1 |

### Pinagem

```
MOTOR
  IN1 → D2    IN2 → D4    ENA → D3  (PWM, motor direito)
  IN3 → D5    IN4 → D7    ENB → D6  (PWM, motor esquerdo)

SENSOR DE LINHA (QTR-6)
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
│   ├── main.cpp              # Máquina de estados principal (seguidor) *AINDA NÃO IMPLEMENTADO
│   ├── test_components.cpp   # Menu de testes isolados por módulo *ATUALMENTE INCLUSO EM MAIN.CPP
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
├── docs/
│   ├── README.md             # Este arquivo
│   ├── CALIBRATION.md        # Guia de calibração passo a passo
│   └── CONFIG_INDEX.md       # Índice de todas as constantes do config.h
└── platformio.ini
```

---

## Módulos

### `LineSensor`
Lê os 6 sensores QTR analógicos e calcula a posição da linha por centro de massa ponderado. Retorna posição normalizada de -1.0 (extrema esquerda) a +1.0 (extrema direita) e classifica o padrão (STRAIGHT, CURVE_LIGHT, CURVE_MEDIUM, CURVE_SHARP, INTERSECTION, LINE_LOST).

Sem filtro de debounce — reatividade máxima é necessária para seguimento em alta velocidade.

### `MotorController`
Controla os dois motores DC via L298N. Implementa o controlador PD de seguimento de linha em `followLine()`, onde o motor externo à curva mantém velocidade base e o motor interno é reduzido proporcionalmente ao erro.

### `UltrasonicSensor`
Lê o HC-SR04 com validação por janela deslizante (3 leituras consecutivas consistentes). Classifica a distância em 3 fases: DISTANT (>30cm), APPROACHING (>15cm), CONTACT (≤5cm).

### `GripperServo`
Controla o SG90 com movimento gradual grau a grau e `detach()` após estabilização, eliminando aquecimento e micro-vibrações entre comandos.

---

## Fluxo de operação (main.cpp)

```
SETUP → inicialização dos módulos → delay 3s → STATE_FOLLOWING

STATE_FOLLOWING
  ├─ Lê LineSensor
  ├─ Calcula baseSpeed por magnitude do erro
  ├─ Chama motor.followLine(pos, baseSpeed)
  └─ Se LINE_LOST → STATE_RECOVERING

STATE_RECOVERING
  ├─ Estágio 1: gira na última direção por RECOVERY_SPIN_MS
  ├─ Estágio 2: gira na direção oposta até RECOVERY_TIMEOUT_MS
  └─ Se linha encontrada → STATE_FOLLOWING
     Se timeout → STATE_STOPPED

STATE_STOPPED
  └─ Para motores, aguarda intervenção
```

> Módulos `UltrasonicSensor` e `GripperServo` serão acoplados como `STATE_COLLECTING` entre `STATE_FOLLOWING` e `STATE_RECOVERING`.

---

## Início rápido

1. Compile e grave `test_components.cpp` (comente `main.cpp` no build)
2. Abra o monitor serial em 9600 baud
3. Execute o **Teste 6** para calibrar o threshold da linha
4. Atualize `THRESHOLD_LINE_SENSOR` no `config.h`
5. Execute o **Teste 2** para validar a leitura dos sensores
6. Execute o **Teste 1** para validar os motores e direções
7. Grave `main.cpp` e posicione o robô sobre a linha

---

## Debug

Com `DEBUG_MODE true` no `config.h`, todos os módulos imprimem no Serial a 9600 baud. Para desabilitar e liberar memória, setar `DEBUG_MODE false` — o compilador remove todo o código de debug.

Prefixos de log:

| Prefixo | Módulo |
|---|---|
| `[Line]` | LineSensor |
| `[PD]` | MotorController::followLine |
| `[Ultrasonic]` | UltrasonicSensor |
| `[Gripper]` | GripperServo |
| `[Main]` | main.cpp |
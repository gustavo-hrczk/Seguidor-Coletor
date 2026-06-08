# Seguidor Coletor — Robô Autônomo

Robô seguidor de linha com coleta autônoma de objetos via garra servo.
Desenvolvido em C++ para Arduino Nano com arquitetura modular orientada a classes.

Projeto PAC — Programa Aplicado à Comunidade  
Engenharia de Software — 2ª Fase

---

## Hardware

| Componente          | Modelo                                       | Qtd |
|---------------------|----------------------------------------------|-----|
| Microcontrolador    | Arduino Nano (ATmega328P)                    | 1   |
| Driver de motor     | L298N                                        | 1   |
| Motores DC          | Micro Motor Caixa de Redução 6V — Robocore   | 2   |
| Sensor de linha     | QTR-1 analógico                              | 6   |
| Sensor ultrassônico | HC-SR04                                      | 1   |
| Servo               | SG90                                         | 1   |

### Pinagem

```
MOTOR (L298N)
  IN1 → D5    IN2 → D7    ENB → D3   (PWM motor direito  — Timer 2)
  IN3 → D2    IN4 → D4    ENA → D6   (PWM motor esquerdo — Timer 1)

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
│   ├── main.cpp               # Programa técnico completo (máquina de estados)
│   ├── main_robo.cpp          # Programa com API de abstração (workshop)
│   ├── test_components.cpp    # Testes isolados por módulo (desenvolvimento)
│   └── main_workshop.cpp      # Ponto de partida do aluno (workshop)
├── lib/
│   ├── Robo/
│   │   ├── Robo.h             # API de abstração didática em português
│   │   └── Robo.cpp
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
│   ├── README.md              # Este arquivo
│   └── CONFIG_INDEX.md        # Referência de todas as constantes
└── config/
    ├── config.h               # Configuração técnica central (não editar no workshop)
    └── config_workshop.h      # Constantes editáveis pelo aluno
```

---

## Dois modos de uso

### Modo técnico — `main.cpp`

Programa completo com máquina de estados explícita. Usa a biblioteca diretamente
sem camada de abstração. Para desenvolvimento e manutenção da equipe.

```cpp
#include "LineSensor.h"
#include "MotorController.h"
// ...
motor.followLine(pos, baseSpeed);   // PID direto
```

### Modo workshop — `main_robo.cpp` + `Robo.h`

Mesmo comportamento do modo técnico, reescrito com a API de abstração em português.
Toda a complexidade (PID, validação ultrassônica, trim, detach do servo, rampas de
proteção dos motores) fica invisível dentro da classe `Robo`.

```cpp
#include "Robo.h"
Robo robo;
robo.seguirLinha(NORMAL);   // PID aplicado internamente
```

O aluno recebe `main_workshop.cpp` com 4 linhas e a datasheet impressa,
e constrói toda a lógica de decisão do zero.

---

## Módulos da biblioteca

### `LineSensor`
Lê os 6 sensores QTR e calcula a posição da linha pelo algoritmo de centro de massa
ponderado por intensidade. Retorna posição normalizada de -1.0 (extrema esquerda)
a +1.0 (extrema direita). Sem filtro de debounce — reatividade máxima para seguimento.
Classifica padrão: `STRAIGHT`, `CURVE_LIGHT`, `CURVE_MEDIUM`, `CURVE_SHARP`,
`INTERSECTION`, `LINE_LOST`.

### `MotorController`
Controla dois motores via L298N com três camadas de proteção aplicadas em
`setMotorSpeed()`, invisíveis para os módulos superiores:

- **Trim de assimetria** — `MOTOR_TRIM_ESQ/DIR` compensam diferença física entre motores
- **Deadzone** — eleva PWM baixo ao mínimo necessário para vencer atrito estático
- **Rampas de proteção** — evitam pico de corrente no arranque e inversão brusca de sentido
  (crítico para o Micro Motor Robocore 6V com corrente de stall de 1.6A)

Implementa controlador PID completo em `followLine()` com termo integral
anti-windup para corrigir deriva acumulada em percursos longos.

### `UltrasonicSensor`
Lê o HC-SR04 com validação por janela deslizante (SENSOR_FILTER_CYCLES leituras
consecutivas). Tolerância adaptativa: ±2 cm fixo até 20 cm, percentual com teto
de ±4 cm acima disso. Classifica distância em fases: `DISTANT`, `APPROACHING`,
`CONTACT`.

### `GripperServo`
Controla o SG90 com movimento grau a grau para evitar tranco mecânico.
Desliga o PWM via `detach()` após cada posicionamento — elimina aquecimento
e vibração em repouso. Correção de underflow de `uint8_t` na iteração garante
que o loop de movimento nunca trave.

### `Robo`
Camada de fachada que encapsula todos os módulos acima. Expõe API em português
com 11 métodos públicos. Converte escala -100..+100 para PWM real via `BASE_SPEED`.
Inclui `seguirLinha()` que aplica o PID internamente com ciclo bloqueante
sincronizado ao `PD_SAMPLE_MS`.

---

## Calibração — procedimento completo

### 1. Threshold do sensor de linha (obrigatório na pista real)

```
1. Grave test_components.cpp
2. Abra o monitor serial em 9600 baud
3. Digite 6 para iniciar o Teste 6
4. Etapa 1: posicione TODOS os sensores sobre a LINHA BRANCA — aguarde 4s
5. Etapa 2: posicione TODOS os sensores sobre o FUNDO PRETO — aguarde 4s
6. Anote o valor THRESHOLD REC. impresso no serial
7. Atualize config.h: #define THRESHOLD_LINE_SENSOR  <valor>
```

> Repetir sempre que o local ou iluminação mudar.

### 2. Trim de assimetria dos motores

```
1. Superfície plana, sem linha, ~1 metro de distância
2. Grave test_components.cpp → Teste 1 → observe FRENTE por 2s
3. Robô desvia para a DIREITA → motor direito está mais rápido
   → reduza MOTOR_TRIM_DIR em 0.02 (ex: 0.90 → 0.88)
4. Robô desvia para a ESQUERDA → motor esquerdo está mais rápido
   → reduza MOTOR_TRIM_ESQ em 0.02
5. Repita até andar reto. Registre os valores no config.h.
```

> Repetir após troca de qualquer motor.

### 3. Ganhos do PID

```
Pista pequena / baixa velocidade:  Kp=0.4–0.6  Kd=0.1–0.2  Ki=0.02
Pista normal (atual):              Kp=0.8      Kd=0.4      Ki=0.04
Pista grande / alta velocidade:    Kp=1.2–1.5  Kd=0.4–0.6  Ki=0.06

Sintomas e correções:
  Serpenteia em reta   → aumentar Kd ou diminuir Kp
  Lento nas curvas     → aumentar Kp
  Deriva em retas longas → aumentar Ki
  Oscila após curvas   → diminuir Ki (windup)
```

### 4. Delays de manobra (coleta)

```
1. Grave test_components.cpp → Teste 5 (Coleta Simulada)
2. Aproxime um objeto e mantenha na zona de contato
3. Observe o giro e o avanço lateral
4. Ajuste TEMPO_GIRO_90 até o giro ser exatamente 90°
5. Ajuste TEMPO_AVANCO até o objeto sair completamente da pista
6. Registre os valores em config_workshop.h
```

---

## Início rápido — primeira configuração

```
1. Instale VS Code + PlatformIO + driver CH340 (Nano)
2. Grave test_components.cpp
3. Execute Teste 6 → calibre THRESHOLD_LINE_SENSOR
4. Execute Teste 1 → calibre MOTOR_TRIM_ESQ/DIR
5. Execute Teste 3 → valide ultrassônico
6. Execute Teste 4 → valide garra
7. Grave main.cpp → posicione na pista e teste
8. Grave main_robo.cpp → valide comportamento equivalente
```

---

## Debug

`DEBUG_MODE true` no `config.h` ativa logs no Serial a 9600 baud.
`false` remove todo o código de log em compilação, liberando memória Flash.

| Prefixo serial | Módulo                      |
|----------------|-----------------------------|
| `[Line]`       | LineSensor                  |
| `[PID]`        | MotorController::followLine |
| `[Ultrasonic]` | UltrasonicSensor            |
| `[Gripper]`    | GripperServo                |
| `[Robo]`       | main_robo.cpp               |
| `[Main]`       | main.cpp                    |

---
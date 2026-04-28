# 🤖 Robô Seguidor de Linha com Coleta Autônoma

## 📋 Visão Geral

Este projeto implementa um robô autônomo baseado em Arduino UNO com as seguintes funcionalidades:

- ✅ **Segue linha** usando 6 sensores QTR
- ✅ **Detecta objetos** com sensor ultrassônico HC-SR04
- ✅ **Coleta objetos** com servo-garra automática
- ✅ **Máquina de Estados Finita** para controle lógico
- ✅ **Encapsulamento e POO** em C++
- ✅ **Velocidade Global** escalável
- ✅ **Algoritmo Round Robin** para cruzamentos

---

## 🏗️ Arquitetura Modular

### Estrutura de Diretórios

```
src/
├── main.cpp                 # Ponto de entrada + loop principal
include/
├── config.h                 # Configuração centralizada
lib/
├── MotorController/         # Controle de motores com PWM
├── LineSensor/             # Leitura de 6 sensores de linha
├── UltrasonicSensor/       # Sensor ultrassônico HC-SR04
├── GripperServo/           # Controle de servo-garra
└── RobotStateMachine/      # Máquina de estados finita
```

---

## 🔧 Componentes Principais

### 1. **MotorController** (`MotorController.h/.cpp`)

Responsável pelo controle de dois motores DC com PWM.

**Funcionalidades:**
- Controle de velocidade (0-255)
- Movimento em 5 direções: FORWARD, BACKWARD, TURN_LEFT, TURN_RIGHT, STOP
- Compensação de curva para manter raio constante
- Correção de deadzone automática (PWM_MIN_DEADZONE = 60)

**Uso:**
```cpp
motorController.initialize();
motorController.move(MotorController::FORWARD, 200);  // Frente a PWM 200
motorController.curveCompensated(MotorController::TURN_LEFT, 180, 0.8);  // Curva com 80% compensation
```

### 2. **LineSensor** (`LineSensor.h/.cpp`)

Leitura e processamento de 6 sensores de linha com filtro de debounce.

**Funcionalidades:**
- Leitura analógica dos 6 sensores
- Filtro de debounce em 3 ciclos (SENSOR_FILTER_CYCLES)
- Identificação de padrões: STRAIGHT, CURVE, INTERSECTION, LINE_LOST
- Retorna padrão binário (6 bits)

**Padrões Detectados:**
- `STRAIGHT`: Sensores centrais ativos (navegação reta)
- `CURVE_LIGHT`: Sensor 3 ou 4 (curva suave)
- `CURVE_SHARP`: Sensores extremos (curva acentuada)
- `INTERSECTION`: 4+ sensores (cruzamento)
- `LINE_LOST`: Nenhum sensor (linha perdida)

**Uso:**
```cpp
lineSensor.initialize();
LineSensor::SensorState state = lineSensor.readSensors();
LineSensor::LinePattern pattern = lineSensor.getLinePattern();
```

### 3. **UltrasonicSensor** (`UltrasonicSensor.h/.cpp`)

Leitura de sensor ultrassônico com validação de debounce (3 leituras com tolerância 5%).

**Funcionalidades:**
- Medição em centímetros
- Filtro de outliers (mudanças > 50% em uma leitura)
- Validação com tolerância de 5%
- 3 fases de aproximação:
  1. **PHASE_1_DISTANT** (>30cm): Desacelerar
  2. **PHASE_2_APPROACHING** (15-30cm): Velocidade lenta
  3. **PHASE_3_CONTACT** (<15cm): Parar e coletar

**Uso:**
```cpp
ultrasonicSensor.initialize();
int distance = ultrasonicSensor.readDistance();
UltrasonicSensor::ApproachPhase phase = ultrasonicSensor.getApproachPhase();
```

### 4. **GripperServo** (`GripperServo.h/.cpp`)

Controle de servo-motor para garra com proteção contra travamento.

**Funcionalidades:**
- Estados: OPEN, OPENING, CLOSED, CLOSING, ERROR
- Timeout de 3 segundos se servo travar
- Estabilização automática após movimento
- Proteção de emergência

**Uso:**
```cpp
gripperServo.initialize();
bool success = gripperServo.close();  // Retorna false se timeout
if (!success) {
    gripperServo.emergency_stop();
}
gripperServo.open();
```

### 5. **RobotStateMachine** (`RobotStateMachine.h/.cpp`)

Máquina de Estados Finita que coordena toda a lógica do robô.

**Estados:**
```
STATE_IDLE → STATE_NAVIGATE → STATE_CURVE
                    ↓
             STATE_OBJECT_DETECTED
                    ↓
             STATE_APPROACH
                    ↓
             STATE_COLLECT
                    ↓
             STATE_ROTATE_90
                    ↓
             STATE_RELEASE → STATE_RETURN_LINE → STATE_NAVIGATE
                    ↑
             STATE_LINE_SEARCH (se linha perdida)
```

**Transições Críticas:**
1. **NAVIGATE → OBJECT_DETECTED**: Quando ultrassônico valida objeto em reta/curva suave
2. **CURVE → NAVIGATE**: Quando sensores centrais se alinham
3. **NAVIGATE → LINE_SEARCH**: Quando nenhum sensor detecta linha
4. Algoritmo **Round Robin** em cruzamentos

---

## ⚙️ Configuração Centralizada (`config.h`)

Todos os parâmetros estão em um único arquivo:

```cpp
#define VELOCITY_GLOBAL 200           // PWM base (0-255)
#define PWM_SLOW 85                   // Velocidade lenta
#define PWM_MEDIUM 170                // Velocidade média
#define PWM_FAST 255                  // Velocidade rápida

#define PIN_IN1 2                     // Pinos do motor
#define PIN_TRIGGER 11                // Pinos do ultrassônico
#define PIN_SERVO 9                   // Pino do servo

#define THRESHOLD_LINE_SENSOR 700     // Limiar de detecção
#define ULTRASONIC_DISTANCE_LONG 30   // Fase 1 (cm)
#define ULTRASONIC_DISTANCE_SHORT 15  // Fase 2 (cm)
#define ULTRASONIC_DISTANCE_CONTACT 5 // Fase 3 (cm)
```

---

## 🎯 Escalação de Velocidade Global

**Princípio**: Um valor `VELOCITY_GLOBAL` controla todo o comportamento do robô.

### Fórmula de Escalação Temporal:
```
Tempo_Ação = K / (VELOCITY_GLOBAL / 255)
Onde K é constante de calibração empírica
```

### Exemplo:
- Se `VELOCITY_GLOBAL = 255` (máximo): Tempos normais
- Se `VELOCITY_GLOBAL = 127` (metade): Tempos 2x maiores
- Se `VELOCITY_GLOBAL = 60` (mínimo com deadzone): Tempos 4x maiores

Isso garante que **curvas, timeouts e ações mantêm proporção**.

---

## 🔌 Mapeamento de Pinos

### Motores (Driver L298N)
| Função | Pino |
|--------|------|
| IN1 (Motor Direita) | 2 |
| IN2 (Motor Direita) | 4 |
| ENA (PWM Motor Esquerda) | 3 |
| ENB (PWM Motor Direita) | 6 |
| IN3 (Motor Esquerda) | 5 |
| IN4 (Motor Esquerda) | 7 |

### Sensores de Linha (QTR-6)
| Sensor | Pino |
|--------|------|
| S1 (Extrema Esquerda) | A0 |
| S2 (Esquerda) | A1 |
| S3 (Centro-Esquerda) | A2 |
| S4 (Centro-Direita) | A3 |
| S5 (Direita) | A4 |
| S6 (Extrema Direita) | A5 |

### Outros Componentes
| Componente | Pino |
|------------|------|
| Trigger Ultrassônico | 11 |
| Echo Ultrassônico | 12 |
| Servo Garra | 9 |
| LED R | 9 |
| LED G | 11 |
| LED B | 10 |

---

## 📊 Máquina de Estados Detalhada

### STATE_NAVIGATE (Seguindo Linha)
- Lê 6 sensores de linha
- Identifica padrão (reta, curva, cruzamento)
- Movimento proporcional ao padrão
- **Verifica coleta** apenas em reta/curva suave
- Transição para curva se necessário

### STATE_CURVE (Fazendo Curva)
- Aplica compensação de PWM entre rodas
- Mantém raio de giro constante
- **BLOQUEIO**: Ignora ultrassônico durante curva acentuada
- Retorna a NAVIGATE quando alinhado

### STATE_OBJECT_DETECTED (Centralizando)
- Move-se para centralizar objeto
- Verifica alinhamento dos sensores centrais
- Timeout de 2s se não conseguir

### STATE_APPROACH (Aproximação em 3 Fases)
- **Fase 1** (>30cm): Velocidade média
- **Fase 2** (15-30cm): Velocidade lenta
- **Fase 3** (<15cm): Parada → STATE_COLLECT

### STATE_COLLECT (Coletando)
- Fecha garra com timeout (3s)
- Se sucesso: incrementa contador
- Se falha: tenta novamente (máx 3 vezes)

### STATE_ROTATE_90 (Girar para Descartar)
- Gira 90° na direção oposta à detecção
- Monitora linha durante rotação
- Tempo escalado: `ROTATION_90_DEGREES_TIME`

### STATE_RELEASE (Descartando)
- Abre garra
- Recua distância calibrada
- Retorna à linha

### STATE_RETURN_LINE (Catch-Line)
- Busca ativa pela linha
- Assim que encontra: centraliza
- Retorna a NAVIGATE

### STATE_LINE_SEARCH (Busca de Linha)
- Gira até 360° procurando linha
- Máximo 5 rotações (LINE_SEARCH_MAX_ROTATIONS)
- Se não encontra: EMERGENCY_STOP

---

## 🚀 Como Usar

### 1. **Compilar e Upload**
```bash
cd "c:\Users\Gustavo\Documents\PlatformIO\Projects\Seguidor Coletor"
pio run -e uno -t upload
```

### 2. **Configurar Parâmetros** (`include/config.h`)
```cpp
#define VELOCITY_GLOBAL 200    // Ajustar velocidade
#define THRESHOLD_LINE_SENSOR 700  // Calibrar sensores
```

### 3. **Debug via Serial**
```
Abrir Monitor Serial (9600 baud)
- S: Parar robô
- R: Reiniciar
- D: Debug info
- M: Testar motor
- L: Testar sensores de linha
- U: Testar ultrassônico
- G: Testar garra
```

---

## 🎓 Ordem de Implementação / Testes

### Etapa 1: Sensores + Motores
**META**: Seguir linha reta por 10m
```
✓ Inicializar sensores de linha
✓ Calibrar threshold
✓ Testar motores em linha reta
✓ Implementar STATE_NAVIGATE básico
```

### Etapa 2: Detecção de Objetos
**META**: Detectar objeto sem parar/girar
```
✓ Implementar UltrasonicSensor
✓ Validação de debounce (3 leituras)
✓ Teste em reta: detecta e reduz velocidade
✓ Teste em curva: IGNORA objeto (bloqueio)
```

### Etapa 3: Coleta Completa
**META**: Coleta → Rotaciona → Deposita
```
✓ Implementar GripperServo com timeout
✓ Teste de fechamento/abertura
✓ Integrar STATE_COLLECT e STATE_ROTATE_90
✓ Teste de coleta simples em reta
```

### Etapa 4: Testes Extremos
**META**: Pista completa com múltiplos objetos
```
✓ Teste em intersecção (Round Robin)
✓ Teste de busca de linha perdida
✓ Múltiplas coletas consecutivas
✓ Verificar tempos de calibração
✓ Teste final: 30m, 5 objetos, 10 min
```

---

## ✅ Checklist de Sucesso

- [ ] Robô anda 30m sem perder linha
- [ ] Coleta 5 objetos com 100% de sucesso
- [ ] Recupera de linha perdida (8/10 tentativas)
- [ ] Servo não trava
- [ ] Rotação 90° com precisão
- [ ] Sem oscilação em cruzamentos
- [ ] Executa por 10 minutos sem problemas

---

## 📝 Notas de Engenharia

### Deadzone PWM
Motores têm zona morta (~60 PWM). Se velocidade scaled < 60, será elevada para 60.

### Tolerância Ultrassônica
5% de tolerância entre 3 leituras consecutivas = filtro robusto contra ruído.

### Escalação Inversa
Velocidade mais lenta = tempos maiores (fórmula K / velocidade).

### Round Robin
Prioridade alternada em cruzamentos previne oscilação.

---

## 🐛 Troubleshooting

| Problema | Solução |
|----------|---------|
| Robô trava em curva | Ajustar `CURVE_COMPENSATION_*` em config.h |
| Sensores não detectam linha | Verificar `THRESHOLD_LINE_SENSOR` |
| Garra não fecha | Aumentar `SERVO_TIMEOUT` ou `SERVO_ANGLE_CLOSED` |
| Colisão com objeto | Aumentar `ULTRASONIC_DISTANCE_LONG` |
| Oscila em cruzamento | Verificar implementação Round Robin |

---

## 📚 Referências

- Arduino PWM: https://www.arduino.cc/en/Tutorial/Foundations/PWM
- Servo Control: https://www.arduino.cc/en/Reference/ServoWrite
- Finite State Machine: https://en.wikipedia.org/wiki/Finite-state_machine

---

**Desenvolvido em 2026** | **Arduino UNO + PlatformIO**

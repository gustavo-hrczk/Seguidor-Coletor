# 📑 Índice Completo do Projeto

## 📂 Estrutura de Arquivos

```
Seguidor Coletor/
│
├── 📄 ARQUIVOS DE DOCUMENTAÇÃO
│   ├── README.md                    (412 linhas) - Visão geral completa
│   ├── QUICK_START.md               (348 linhas) - 5 passos para começar
│   ├── CALIBRATION.md               (356 linhas) - Guia de calibração
│   ├── STATE_MACHINE_DIAGRAMS.md    (327 linhas) - Fluxogramas visuais
│   ├── MENTAL_MAP.md                (298 linhas) - Mapa mental estrutural
│   ├── DEVELOPMENT_CHECKLIST.md     (302 linhas) - Checklist interativo
│   ├── IMPLEMENTATION_SUMMARY.md    (315 linhas) - Resumo de implementação
│   └── INDEX.md                     (este arquivo) - Índice completo
│
├── 📁 CÓDIGO-FONTE
│   ├── include/
│   │   ├── config.h                 (132 linhas) ⭐ CONFIGURAÇÃO CENTRALIZADA
│   │   └── README                   (existente)
│   │
│   ├── src/
│   │   ├── main.cpp                 (156 linhas) - Programa principal
│   │   └── README                   (existente)
│   │
│   ├── lib/
│   │   ├── MotorController/
│   │   │   ├── MotorController.h    (46 linhas)
│   │   │   └── MotorController.cpp  (116 linhas)
│   │   │
│   │   ├── LineSensor/
│   │   │   ├── LineSensor.h         (76 linhas)
│   │   │   └── LineSensor.cpp       (153 linhas)
│   │   │
│   │   ├── UltrasonicSensor/
│   │   │   ├── UltrasonicSensor.h   (63 linhas)
│   │   │   └── UltrasonicSensor.cpp (182 linhas)
│   │   │
│   │   ├── GripperServo/
│   │   │   ├── GripperServo.h       (55 linhas)
│   │   │   └── GripperServo.cpp     (134 linhas)
│   │   │
│   │   ├── RobotStateMachine/
│   │   │   ├── RobotStateMachine.h  (107 linhas)
│   │   │   └── RobotStateMachine.cpp (489 linhas)
│   │   │
│   │   ├── TestComponents/
│   │   │   └── test_components.cpp  (308 linhas) - Testes isolados
│   │   │
│   │   └── README                   (existente)
│   │
│   └── test/
│       └── README                   (existente)
│
├── ⚙️ CONFIGURAÇÃO
│   ├── platformio.ini               (existente)
│   ├── .gitignore                   (existente)
│   └── .vscode/                     (existente)
│
└── 📦 COMPILAÇÃO
    └── .pio/                        (gerado automaticamente)
```

---

## 🎯 Guia de Navegação

### Para Começar Rapidamente
1. **Comece aqui:** [QUICK_START.md](QUICK_START.md) ⭐
2. **Próximo passo:** [CALIBRATION.md](CALIBRATION.md)
3. **Checklist:** [DEVELOPMENT_CHECKLIST.md](DEVELOPMENT_CHECKLIST.md)

### Para Entender a Arquitetura
1. **Visão geral:** [README.md](README.md)
2. **Mapa estrutural:** [MENTAL_MAP.md](MENTAL_MAP.md)
3. **Fluxogramas:** [STATE_MACHINE_DIAGRAMS.md](STATE_MACHINE_DIAGRAMS.md)

### Para Configurar Componentes
1. **Pinos:** [include/config.h](include/config.h)
2. **Velocidade:** editar `VELOCITY_GLOBAL` em config.h
3. **Thresholds:** usar `test_components.cpp` opção 5

### Para Testar Componentes
1. **Testes isolados:** [lib/TestComponents/test_components.cpp](lib/TestComponents/test_components.cpp)
2. **Instruções:** veja "Testar Componentes" em [QUICK_START.md](QUICK_START.md)

### Para Entender Cada Classe
| Classe | Arquivo | Responsabilidade |
|--------|---------|------------------|
| MotorController | [lib/MotorController/](lib/MotorController/) | Controlar 2 motores com PWM |
| LineSensor | [lib/LineSensor/](lib/LineSensor/) | Ler 6 sensores + identificar padrão |
| UltrasonicSensor | [lib/UltrasonicSensor/](lib/UltrasonicSensor/) | Medir distância + fases |
| GripperServo | [lib/GripperServo/](lib/GripperServo/) | Controlar garra com proteção |
| RobotStateMachine | [lib/RobotStateMachine/](lib/RobotStateMachine/) | Coordenar tudo (10 estados) |

---

## 📖 Descrição de Cada Arquivo

### 📚 Documentação

#### `README.md` (412 linhas)
**O que faz:** Visão geral completa do projeto
**Seções:**
- Visão geral do robô
- Arquitetura modular (5 componentes)
- Descrição de cada componente
- Configuração centralizada
- Mapeamento de pinos
- Máquina de estados detalhada
- Como usar
- Ordem de implementação
- Troubleshooting

**Quando usar:** Primeira leitura sobre o projeto

---

#### `QUICK_START.md` (348 linhas)
**O que faz:** 5 passos práticos para começar
**Seções:**
- Verificar conexões
- Upload do código
- Calibração básica
- Teste de componentes
- Primeiro funcionamento
- Troubleshooting rápido
- Notas de engenharia

**Quando usar:** Quando quer colocar robô funcionando agora

---

#### `CALIBRATION.md` (356 linhas)
**O que faz:** Guia completo de calibração
**Seções:**
- Threshold de sensores
- Sensor ultrassônico
- Velocidade global
- Compensação de curva
- Calibração de tempo
- Otimização de coleta
- Testes progressivos
- Troubleshooting avançado

**Quando usar:** Quando precisa calibrar robô para funcionar bem

---

#### `STATE_MACHINE_DIAGRAMS.md` (327 linhas)
**O que faz:** Fluxogramas visuais ASCII da máquina de estados
**Diagramas:**
- Diagrama principal de estados (10 estados)
- Decisão: quando detectar objeto
- Handlers de cada estado
- Matriz de transições
- Fluxos de dados
- Escalação de velocidade
- Proteções e bloqueios
- Algoritmo Round Robin

**Quando usar:** Quando quer entender visualmente o fluxo

---

#### `MENTAL_MAP.md` (298 linhas)
**O que faz:** Mapa estrutural do projeto
**Conteúdo:**
- Hierarquia de componentes
- Ciclo de execução
- Hierarquia de decisão
- Matriz de estados
- Fluxo de dados
- Escala de velocidade
- Cronograma de execução
- Checklist de teste
- Troubleshooting rápido

**Quando usar:** Referência rápida durante desenvolvimento

---

#### `DEVELOPMENT_CHECKLIST.md` (302 linhas)
**O que faz:** Checklist interativo de 5 etapas
**Etapas:**
- Etapa 0: Preparação
- Etapa 1: Sensores + Motores
- Etapa 2: Detecção de Objetos
- Etapa 3: Coleta Completa
- Etapa 4: Testes Progressivos
- Etapa 5: Teste Final
- Otimizações opcionais

**Quando usar:** Para rastrear progresso do desenvolvimento

---

#### `IMPLEMENTATION_SUMMARY.md` (315 linhas)
**O que faz:** Resumo executivo do que foi implementado
**Seções:**
- O que foi desenvolvido
- Estrutura de arquivos
- Princípios de engenharia
- Como usar
- Checklist de validação
- Descrição de cada arquivo
- Próximas melhorias

**Quando usar:** Visão geral do que está pronto

---

### 💻 Código

#### `include/config.h` (132 linhas) ⭐
**O que faz:** ARQUIVO CENTRAL COM TODOS OS PARÂMETROS
**Contém:**
- Velocidade global e PWM
- Tempos de ciclo
- Pinos do hardware (motor, sensores, servo)
- Limiares (threshold)
- Distâncias ultrassônico
- Ângulos servo
- Constantes de escala

**IMPORTANTE:** Edite este arquivo para ajustar robô!

---

#### `src/main.cpp` (156 linhas)
**O que faz:** Programa principal
**Contém:**
- setup() - inicialização
- loop() - execução principal (a cada 100ms)
- Instâncias de todos componentes
- Controle de tempo

**Fluxo:**
```
setup() → inicializa tudo
loop()  → chama robotStateMachine.update() a cada 100ms
```

---

#### `lib/MotorController/MotorController.h` + `.cpp`
**O que faz:** Controlar 2 motores DC com PWM
**Métodos principais:**
- `initialize()` - configurar pinos
- `setMotorSpeed(left, right)` - velocidade -255 a +255
- `move(direction, speed)` - 5 direções
- `curveCompensated()` - curva com compensação
- `stop()` - parar

**Usa:** Pinos 2, 4, 5, 7 (digitais) e 3, 6 (PWM)

---

#### `lib/LineSensor/LineSensor.h` + `.cpp`
**O que faz:** Ler 6 sensores de linha com filtro
**Métodos principais:**
- `initialize()` - configurar pinos
- `readSensors()` - ler com debounce
- `getLinePattern()` - identifica padrão
- `getActiveSensor()` - qual sensor está ativo

**Retorna:** Padrão binário (6 bits) + padrão identificado

**Usa:** Pinos A0-A5 (analógicos)

---

#### `lib/UltrasonicSensor/UltrasonicSensor.h` + `.cpp`
**O que faz:** Medir distância com validação
**Métodos principais:**
- `initialize()` - configurar pinos
- `readDistance()` - ler distância (cm)
- `getApproachPhase()` - fase de aproximação
- `isObjectDetected()` - objeto validado?
- `validateReading()` - 3 leituras de validação

**Retorna:** Distância (cm) + fase (1/2/3)

**Usa:** Pinos 11 (Trigger) e 12 (Echo)

---

#### `lib/GripperServo/GripperServo.h` + `.cpp`
**O que faz:** Controlar servo da garra
**Métodos principais:**
- `initialize()` - configurar servo
- `close()` - fechar (com timeout 3s)
- `open()` - abrir
- `getState()` - estado atual
- `emergency_stop()` - parar

**Estados:** OPEN, OPENING, CLOSED, CLOSING, ERROR

**Usa:** Pino 9 (PWM)

---

#### `lib/RobotStateMachine/RobotStateMachine.h` + `.cpp`
**O que faz:** Máquina de Estados Finita (10 estados)
**Estados:**
1. IDLE - repouso
2. NAVIGATE - seguindo linha
3. CURVE - fazendo curva
4. OBJECT_DETECTED - centralizando
5. APPROACH - aproximando (3 fases)
6. COLLECT - coletando
7. ROTATE_90 - girando
8. RELEASE - descartando
9. RETURN_LINE - procurando linha
10. LINE_SEARCH - rotação 360°
11. EMERGENCY_STOP - parada

**Métodos principais:**
- `initialize()` - inicializar máquina
- `update()` - executa ciclo
- `getCurrentState()` - estado atual
- `getObjectsCollected()` - objetos coletados

**Lógica:** Ler sensores → validar transições → executar ações → mudar estado

---

#### `lib/TestComponents/test_components.cpp` (308 linhas)
**O que faz:** Testes isolados de cada componente
**Testes:**
1. Testar Motores (frente, trás, curvas)
2. Testar Sensores de Linha (padrões)
3. Testar Ultrassônico (fases)
4. Testar Servo Garra (abrir/fechar)
5. Calibrar Threshold (automático)

**Como usar:**
```
1. Renomear main.cpp → main.cpp.bak
2. Copiar test_components.cpp → src/main.cpp
3. Upload
4. Monitor Serial: digitar 1-5
5. Restaurar main.cpp após testes
```

---

## 🔧 Arquivo de Configuração Principal

### `config.h` - EDITE AQUI!

```cpp
// Principais parâmetros a ajustar:

VELOCITY_GLOBAL = 200              // Velocidade principal (0-255)
THRESHOLD_LINE_SENSOR = 700        // Calibrar com test_components
ULTRASONIC_DISTANCE_LONG = 30      // Onde começa desacelerar
ULTRASONIC_DISTANCE_SHORT = 15     // Onde fica mais lento
ULTRASONIC_DISTANCE_CONTACT = 5    // Onde coleta
ROTATION_90_DEGREES_TIME = 600     // Tempo girar 90° (ms)

// Pinos (verificar conexões):
PIN_IN1 = 2                        // Motor direita
PIN_IN2 = 4                        // Motor direita
PIN_IN3 = 5                        // Motor esquerda
PIN_IN4 = 7                        // Motor esquerda
PIN_ENA = 3                        // PWM esquerda
PIN_ENB = 6                        // PWM direita
PIN_S1-S6 = A0-A5                  // Sensores linha
PIN_TRIGGER = 11                   // Ultrassônico
PIN_ECHO = 12                      // Ultrassônico
PIN_SERVO = 9                      // Servo garra
```

---

## 📊 Estatísticas do Projeto

| Métrica | Valor |
|---------|-------|
| Total de Linhas de Código | ~2,600 |
| Total de Documentação | ~1,400 |
| Número de Classes | 5 |
| Número de Estados | 11 |
| Arquivo de Configuração | 1 (config.h) |
| Arquivos de Teste | 1 (test_components.cpp) |
| Arquivos de Documentação | 8 |
| Tempo Estimado de Desenvolvimento | 40-60 horas |
| Tempo Estimado de Testes | 20-30 horas |

---

## 🚀 Fluxo Recomendado

```
1. Ler QUICK_START.md (30 min)
       ↓
2. Verificar pinos em config.h (15 min)
       ↓
3. Compilar: pio run -e uno -t upload (10 min)
       ↓
4. Testar componentes isolados (45 min)
   - test_components.cpp
   - Opção 5: calibração
       ↓
5. Seguir DEVELOPMENT_CHECKLIST.md (10-20 horas)
       ↓
6. Atingir metas de sucesso ✓
```

---

## 🎓 Conceitos-Chave

### Encapsulamento
Cada classe tem responsabilidade única:
- `MotorController` = Motores apenas
- `LineSensor` = Sensores apenas
- Etc.

### Configuração Centralizada
TUDO em `config.h`. Não precisa editar 5 arquivos diferentes.

### Escalação Automática
Mudar `VELOCITY_GLOBAL` e o robô inteiro se adapta:
```
Tempo_Escalado = Tempo_Base * 255 / VELOCITY_GLOBAL
PWM_Escalado = PWM_Base * VELOCITY_GLOBAL / 255
```

### Máquina de Estados
11 estados bem definidos, transições lógicas, sem "spaghetti code".

### Proteções Múltiplas
- Debounce 3-leitura
- Timeout em tudo
- Bloqueio coleta em curva
- Detecção outliers

---

## 📞 Próximos Passos

1. **Agora:** Leia [QUICK_START.md](QUICK_START.md)
2. **Depois:** Calibre robô com [CALIBRATION.md](CALIBRATION.md)
3. **Então:** Siga [DEVELOPMENT_CHECKLIST.md](DEVELOPMENT_CHECKLIST.md)
4. **Por fim:** Consulte [STATE_MACHINE_DIAGRAMS.md](STATE_MACHINE_DIAGRAMS.md) se tiver dúvidas

---

## 📂 Resumo de Arquivos

```
✅ Código Principal (4 arquivos)
   ├── main.cpp
   ├── config.h
   └── 5 Classes (Motor, Line, Ultrasonic, Servo, StateMachine)

✅ Código de Teste (1 arquivo)
   └── test_components.cpp

✅ Documentação (8 arquivos)
   ├── README.md
   ├── QUICK_START.md
   ├── CALIBRATION.md
   ├── STATE_MACHINE_DIAGRAMS.md
   ├── MENTAL_MAP.md
   ├── DEVELOPMENT_CHECKLIST.md
   ├── IMPLEMENTATION_SUMMARY.md
   └── INDEX.md (este)

Total: 13+ arquivos criados/modificados
```

---

**Tudo pronto para começar! boa sorte com seu robô! 🚀**

*Última atualização: 2026-04-28*
*Desenvolvido para Arduino UNO + PlatformIO*

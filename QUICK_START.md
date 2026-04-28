# 🚀 Guia de Início Rápido

## 5 Passos para Começar

### 1️⃣ Verificar Conexões de Pino

Consulte `config.h` e verifique **todos** os pinos:

```
┌─────────────────────────────────────────────┐
│ MOTORES (Driver L298N)                      │
├─────────────────────────────────────────────┤
│ IN1 = 2,  IN2 = 4  (Motor Direita)         │
│ IN3 = 5,  IN4 = 7  (Motor Esquerda)        │
│ ENA = 3,  ENB = 6  (PWM)                   │
└─────────────────────────────────────────────┘

┌─────────────────────────────────────────────┐
│ SENSORES DE LINHA (QTR-6)                  │
├─────────────────────────────────────────────┤
│ S1-S6 = A0-A5  (Pinos analógicos)          │
└─────────────────────────────────────────────┘

┌─────────────────────────────────────────────┐
│ SENSOR ULTRASSÔNICO (HC-SR04)              │
├─────────────────────────────────────────────┤
│ TRIGGER = 11  |  ECHO = 12                 │
└─────────────────────────────────────────────┘

┌─────────────────────────────────────────────┐
│ SERVO (Garra)                              │
├─────────────────────────────────────────────┤
│ SERVO = 9                                  │
└─────────────────────────────────────────────┘
```

**⚠️ Ajuste em config.h se seus pinos forem diferentes!**

---

### 2️⃣ Upload do Código

```bash
# Terminal no VS Code
cd "c:\Users\Gustavo\Documents\PlatformIO\Projects\Seguidor Coletor"

# Build
pio run -e uno

# Upload
pio run -e uno -t upload

# Monitor Serial (Debug)
pio device monitor -b 9600
```

---

### 3️⃣ Calibração Básica

**Opção A: Automática (Recomendado)**
```
1. Upload test_components.cpp (veja seção 4)
2. Selecione opção 5 no Serial Monitor
3. Siga as instruções na tela
4. Copie o THRESHOLD recomendado
5. Cole em config.h
6. Re-upload main.cpp
```

**Opção B: Manual**
```cpp
// Em config.h, ajuste:
#define THRESHOLD_LINE_SENSOR 700  // Valor entre linha e fundo

// Teste:
// - Valor muito baixo = linha muito sensível
// - Valor muito alto = não detecta linha
// - Correto = robô segue linha reta sem desvio
```

---

### 4️⃣ Teste de Componentes Isolados

**Use `test_components.cpp` para testar cada peça:**

```
Arquivo: lib/TestComponents/test_components.cpp

Passos:
1. Renomeie: main.cpp → main.cpp.bak
2. Copie: test_components.cpp → src/main.cpp
3. Upload
4. Monitor Serial (9600 baud)

Opções:
  1 = Testar Motores
  2 = Testar Sensores de Linha
  3 = Testar Ultrassônico
  4 = Testar Servo Garra
  5 = Calibrar Threshold

Restaure:
1. Delete: src/main.cpp
2. Renomeie: main.cpp.bak → main.cpp
```

**Cada teste vai imprimir:**
- Status do componente
- Valores de leitura
- Recomendações

---

### 5️⃣ Primeiro Funcionamento

**Etapa 1: Teste Simples (1 minuto)**
```
1. Posicione robô sobre linha branca
2. Reset do Arduino
3. Observe no Serial Monitor

Esperado:
✓ "Ciclo: 10 | Estado: 1 | Objetos: 0"
✓ Robô segue linha de forma estável
✓ Sem oscilações

Se não funcionar:
→ Ver Troubleshooting abaixo
```

**Etapa 2: Linha Curva (3 minutos)**
```
1. Coloque linha em forma de "S"
2. Observe se robô faz curvas suaves
3. Verifique se não sai da linha

Esperado:
✓ Robô segue curva sem perder linha
✓ Compensação de PWM funciona

Se oscila demais:
→ Diminuir VELOCITY_GLOBAL em 20-30 pontos
→ Aumentar CURVE_COMPENSATION_* em 0.05
```

**Etapa 3: Objeto Simples (5 minutos)**
```
1. Coloque objeto a 50cm do robô
2. Posicione em linha reta
3. Observe se detecta e aproxima

Esperado:
✓ Reduz velocidade em 30cm
✓ Velocidade mais lenta em 15cm
✓ Para em ~5cm
✓ Garra fecha

Se não detecta:
→ Aproximar objeto mais
→ Verificar ligação do HC-SR04
→ Aumentar ULTRASONIC_DISTANCE_LONG
```

**Etapa 4: Coleta Completa (10 minutos)**
```
1. Objeto + linha (em "T")
2. Robô deve:
   a) Detectar
   b) Aproximar
   c) Coletar
   d) Girar 90°
   e) Descartar
   f) Retornar à linha
   g) Continuar navegando

Esperado:
✓ Todas as etapas a-g funcionam
✓ Tempo total: ~30-45s

Se falha em alguma etapa:
→ Consultar Troubleshooting
```

---

## 🎯 Estrutura de Arquivos

```
src/
  └─ main.cpp                    ← Programa principal
  
include/
  └─ config.h                    ← CONFIGURAÇÃO CENTRALIZADA
  
lib/
  ├─ MotorController/
  │  ├─ MotorController.h
  │  └─ MotorController.cpp
  │
  ├─ LineSensor/
  │  ├─ LineSensor.h
  │  └─ LineSensor.cpp
  │
  ├─ UltrasonicSensor/
  │  ├─ UltrasonicSensor.h
  │  └─ UltrasonicSensor.cpp
  │
  ├─ GripperServo/
  │  ├─ GripperServo.h
  │  └─ GripperServo.cpp
  │
  ├─ RobotStateMachine/
  │  ├─ RobotStateMachine.h
  │  └─ RobotStateMachine.cpp
  │
  └─ TestComponents/
     └─ test_components.cpp     ← Testes isolados
     
Documentação/
  ├─ README.md                   ← Visão geral completa
  ├─ CALIBRATION.md              ← Guia de calibração
  ├─ STATE_MACHINE_DIAGRAMS.md   ← Diagramas de fluxo
  └─ QUICK_START.md              ← Este arquivo
```

---

## 🔧 Parâmetros Principais para Ajustar

### Velocidade
```cpp
#define VELOCITY_GLOBAL 200  // Aumentar para mais rápido
                             // Diminuir para mais preciso
```

### Detecção de Linha
```cpp
#define THRESHOLD_LINE_SENSOR 700  // Ajustar conforme calibração
```

### Detecção de Objeto
```cpp
#define ULTRASONIC_DISTANCE_LONG 30    // Onde começa a desacelerar
#define ULTRASONIC_DISTANCE_SHORT 15   // Onde fica mais lento
#define ULTRASONIC_DISTANCE_CONTACT 5  // Onde coleta
```

### Tempo de Rotação
```cpp
#define ROTATION_90_DEGREES_TIME 600   // Tempo para girar 90°
```

**💡 TIP:** Comece com valores padrão. Ajuste apenas um parâmetro por vez!

---

## 🐛 Troubleshooting Rápido

| Problema | Causa Provável | Solução |
|----------|---|---|
| Robô não se move | Motores não conectados | Verificar pinos 2,4,5,7 |
| Robô vai para trás | Pinos invertidos | Trocar IN1↔IN2 ou IN3↔IN4 |
| Não detecta linha | Threshold errado | Executar calibração (opção 5 tests) |
| Oscila demais | Velocidade alta | Reduzir VELOCITY_GLOBAL |
| Não coleta objeto | Ultrassônico desligado | Verificar pinos 11, 12 |
| Garra trava | Servo travado | Aumentar SERVO_TIMEOUT |
| Perde linha em curva | Compensação baixa | Aumentar CURVE_COMPENSATION_* |

---

## 📊 Ordem de Teste Recomendada

```
✅ 1. Verificar pinos (30 min)
        └─ Conferir conexões fisicamente

✅ 2. Testar componentes isolados (45 min)
        ├─ Motores
        ├─ Sensores
        ├─ Ultrassônico
        └─ Servo

✅ 3. Calibração (30 min)
        ├─ Threshold de sensores
        ├─ Rotação 90°
        └─ Fases de ultrassônico

✅ 4. Seguir reta (10 m)
        └─ Sem parar, sem desvio

✅ 5. Fazer curva (10 m em "S")
        └─ Sem oscilação

✅ 6. Detectar objeto
        ├─ Em reta
        ├─ Reduz velocidade
        └─ Para em ~5cm

✅ 7. Coleta simples
        ├─ Fecha garra
        ├─ Abre garra
        └─ Sem travar

✅ 8. Coleta + Rotação + Descartar
        ├─ Gira 90°
        ├─ Descarta
        └─ Retorna à linha

✅ 9. Múltiplos objetos
        ├─ 3-5 objetos
        ├─ Coleta todos
        └─ Sem falhas

✅ 10. Teste extremo
        ├─ 30m de pista
        ├─ 5+ objetos
        ├─ 10 minutos
        └─ Sem travamento
```

---

## 📱 Monitoramento Serial

Quando DEBUG_MODE = true, você vê:

```
=== ROBÔ INICIALIZADO ===
Velocidade Global: 200
Ciclo Principal: 100 ms
✓ Inicialização concluída

Ciclo: 10 | Estado: 1 | Objetos: 0
Ciclo: 20 | Estado: 1 | Objetos: 0
Ciclo: 30 | Estado: 1 | Objetos: 0

Estado: 1 -> 3  (NAVIGATE → OBJECT_DETECTED)
Objeto coletado! Total: 1

Estado: 3 -> 4  (OBJECT_DETECTED → APPROACH)
Estado: 4 -> 5  (APPROACH → COLLECT)
Estado: 5 -> 6  (COLLECT → ROTATE_90)
Estado: 6 -> 7  (ROTATE_90 → RELEASE)
...
```

**Significado dos Estados:**
- 0 = IDLE
- 1 = NAVIGATE
- 2 = CURVE
- 3 = OBJECT_DETECTED
- 4 = APPROACH
- 5 = COLLECT
- 6 = ROTATE_90
- 7 = RELEASE
- 8 = RETURN_LINE
- 9 = LINE_SEARCH
- 10 = EMERGENCY_STOP

---

## 🎓 Dicas de Engenharia

### 1. Escalação de Velocidade
Tudo escala automaticamente com `VELOCITY_GLOBAL`:
- Tempos
- PWM
- Compensações de curva

**Não precisa ajustar cada coisa individualmente!**

### 2. Algoritmo Round Robin
Em cruzamentos, o robô alterna entre esquerda e direita:
- Primeira vez: direita
- Segunda vez: esquerda
- E assim por diante...

**Evita oscilação em cruzamentos!**

### 3. Bloqueio de Coleta em Curva
Durante curvas acentuadas, o robô **ignora** detecções de objeto.

**Impede colisão!**

### 4. Debounce em 3 Ciclos
Sensores usam filtro de 3 leituras = reduz ruído.

**Estabiliza detecção!**

---

## ✨ Próximos Passos Após Funcionando

```
1. Adicionar LED RGB para feedback visual
2. Implementar compensação dinâmica de bateria
3. Adicionar buzzer para alertas
4. Criar interface serial mais amigável
5. Logging em SD card
6. Modo automático de calibração
```

---

## 📞 Suporte

Se algo não funcionar:

1. **Verificar pinos** em `config.h`
2. **Testar cada componente** isoladamente
3. **Ler logs Serial** com DEBUG_MODE = true
4. **Consultar Troubleshooting** neste guia
5. **Revisar STATE_MACHINE_DIAGRAMS.md** para entender fluxo

---

**Pronto para começar? Boa sorte! 🚀**

*Última atualização: 2026-04-28*

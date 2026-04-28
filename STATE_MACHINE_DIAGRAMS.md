# Diagramas de Fluxo - Máquina de Estados

## 1. Diagrama Principal de Estados

```
┌─────────────┐
│   IDLE      │ (Inicialização)
└──────┬──────┘
       │
       ▼
┌─────────────────────────────────────────────────────────────────┐
│                        NAVIGATE                                 │
│  (Seguindo linha em reta)                                        │
│                                                                  │
│  ├─ Ler 6 sensores de linha                                     │
│  ├─ Validar detecção de objeto (se reta/curva suave)           │
│  └─ Movimento baseado em padrão                                │
└─────────────┬────────────────────────────────────────┬──────────┘
              │                                        │
              │ Sensores extremos ativos             │ Objeto validado
              │ (Curva detectada)                     │ (Ultrassônico)
              │                                        │
              ▼                                        ▼
       ┌──────────────┐               ┌──────────────────────────┐
       │    CURVE     │               │   OBJECT_DETECTED        │
       │              │               │                          │
       │ ├─ Compensar │               │ ├─ Centralizar objeto   │
       │ │   PWM      │               │ ├─ Alinhar sensores     │
       │ ├─ Monitorar │               │ └─ Timeout: 2s          │
       │ │   linha    │               │                          │
       │ └─ BLOQUEIO: │               │ Sensores centrais OK?   │
       │    Ignorar   │               │ ▼                        │
       │    objeto em │               │ SIM                      │
       │    curva     │               │                          │
       └──────┬───────┘               └────┬────────────────────┘
              │                            │
              │ Alinhado (reta)            │ Alinhado
              │                            ▼
              │                    ┌──────────────────┐
              │                    │    APPROACH      │
              │                    │                  │
              │                    │ Fase 1: >30cm   │
              │                    │ ├─ Velocidade   │
              │                    │ │   média       │
              │                    │ ├─ Fase 2: <30cm│
              │                    │ │ ├─ Velocidade │
              │                    │ │ │   lenta     │
              │                    │ │ ├─ Fase 3: <15│
              │                    │ │ │ └─ Parar     │
              │                    │ │ ▼              │
              │                    │ COLLECT         │
              │                    │ ▼               │
              │                    └────┬────────────┘
              │                         │
              │                         │ Garra fechou OK
              │                         ▼
              │                    ┌──────────────────┐
              │                    │   ROTATE_90      │
              │                    │                  │
              │                    │ ├─ Girar 90°    │
              │                    │ ├─ Monitorar    │
              │                    │ │   linha       │
              │                    │ └─ Tempo escalado
              │                    │                  │
              │                    └────┬────────────┘
              │                         │
              │                         ▼
              │                    ┌──────────────────┐
              │                    │    RELEASE       │
              │                    │                  │
              │                    │ ├─ Abrir garra  │
              │                    │ ├─ Recuar       │
              │                    │ └─ Girar 90°    │
              │                    │    (retorno)    │
              │                    │                  │
              │                    └────┬────────────┘
              │                         │
              │                         ▼
              │                    ┌──────────────────┐
              │                    │  RETURN_LINE     │
              │                    │                  │
              │                    │ ├─ Procurar linha│
              │                    │ ├─ Ao encontrar: │
              │                    │ │   centralizar │
              │                    │ └─ Timeout: 3s   │
              │                    │                  │
              │                    └────┬────────────┘
              │                         │
              │◄────────────────────────┘
              │
              │ Nenhum sensor detecta linha
              ▼
       ┌──────────────────┐
       │  LINE_SEARCH     │
       │                  │
       │ ├─ Girar 360°    │
       │ ├─ Max 5 rotações│
       │ └─ Timeout: 2s   │
       │                  │
       │ Encontrou linha? │
       │ ├─ SIM → NAVIGATE│
       │ └─ NÃO → EMERG.  │
       └──────────────────┘

               │
               │ Timeout geral RUNTIME_TOTAL
               ▼
        ┌──────────────────────┐
        │ EMERGENCY_STOP       │
        │                      │
        │ ├─ Parar motores    │
        │ ├─ Abrir garra      │
        │ ├─ Imprimir stats   │
        │ └─ Parar sistema    │
        └──────────────────────┘
```

---

## 2. Decisão: Quando Detectar Objeto?

```
                    Ultrasom validou?
                           │
                    SIM ───┴─── NÃO
                    │            │
                    ▼            ▼
            Em curva          Em curva
         extrema?          extrema?
       (S5 ou S6)         (S5 ou S6)
          │                  │
        SIM               NÃO
        │                  │
        ▼                  ▼
    IGNORAR          ┌──────────────┐
    ✗ BLOQUEIO      │  Tempo desde  │
    │               │  última coleta│
    │               │  > DEBOUNCE?  │
    │               └───┬────────┬──┘
    │                  SIM      NÃO
    │                  │        │
    │                  ▼        ▼
    │            OBJECT_        IGNORE
    │            DETECTED    (esperar)
    │                  ▲
    └──────────────────┘
```

---

## 3. Máquina de Estados Detalhada - STATE_NAVIGATE

```
STATE_NAVIGATE
    │
    ├─ readSensors()
    │  └─ identificarPardrão()
    │     ├─ STRAIGHT      → move(FORWARD, max_speed)
    │     ├─ CURVE_LIGHT   → transição CURVE
    │     ├─ CURVE_SHARP   → transição CURVE
    │     ├─ INTERSECTION  → Algoritmo Round Robin
    │     │                  ├─ if lastTurn == LEFT
    │     │                  │  └─ próximo: RIGHT
    │     │                  └─ if lastTurn == RIGHT
    │     │                     └─ próximo: LEFT
    │     └─ LINE_LOST      → transição LINE_SEARCH
    │
    └─ shouldDetectObject()?
       ├─ Em curva extrema? → SIM = IGNORE
       ├─ Debounce OK?      → NÃO = IGNORE
       └─ Objeto validado?  → SIM = OBJECT_DETECTED
```

---

## 4. Máquina de Estados Detalhada - STATE_APPROACH

```
STATE_APPROACH
    │
    ├─ getApproachPhase()
    │  │
    │  ├─ PHASE_1_DISTANT (>30cm)
    │  │  └─ move(FORWARD, PWM_MEDIUM)
    │  │
    │  ├─ PHASE_2_APPROACHING (15-30cm)
    │  │  └─ move(FORWARD, PWM_SLOW)
    │  │
    │  └─ PHASE_3_CONTACT (<15cm)
    │     ├─ stop()
    │     ├─ delay(100ms)
    │     └─ transição → COLLECT
    │
    └─ timeout > 5s?
       └─ SIM → NAVIGATE (objeto perdido)
```

---

## 5. Algoritmo Round Robin (Cruzamentos)

```
Encontra INTERSECTION?
    │
    ├─ Carregar lastTurnDirection da EEPROM
    │
    ├─ if lastTurnDirection == TURN_LEFT
    │  └─ próximo = TURN_RIGHT
    └─ if lastTurnDirection == TURN_RIGHT
       └─ próximo = TURN_LEFT
    
    ├─ Salvar em EEPROM
    │
    └─ Executar movimento na direção calculada
```

---

## 6. Filtro de Debounce - Sensores de Linha

```
readSensors() a cada ciclo
    │
    ├─ Executar leitura bruta
    ├─ filterCounter++
    │
    └─ filterCounter == SENSOR_FILTER_CYCLES?
       │ (padrão: 3)
       │
       ├─ SIM
       │  ├─ validateDebounce(atual, anterior)
       │  │  └─ Bits mudados <= 1?
       │  │     ├─ SIM → isValid = true
       │  │     └─ NÃO → isValid = false
       │  │
       │  ├─ lastValidState = readingAtual
       │  ├─ filterCounter = 0
       │  └─ retornar
       │
       └─ NÃO
          └─ retornar
```

---

## 7. Filtro de Debounce - Ultrassônico

```
readDistance() a cada ciclo
    │
    ├─ Executar medição
    ├─ validateReading()
    │  │
    │  ├─ isWithinTolerance(atual, anterior)?
    │  │  (tolerância: 5%)
    │  │
    │  ├─ SIM
    │  │  ├─ validationCounter++
    │  │  │
    │  │  └─ validationCounter == 3?
    │  │     ├─ SIM → Distância validada!
    │  │     └─ NÃO → Aguardar próximas leituras
    │  │
    │  └─ NÃO
    │     └─ validationCounter = 0
    │        (resetar série)
    │
    └─ retornar distância validada
```

---

## 8. Escalação com Velocidade Global

```
scaleTime(timeAtBaseSpeed) é chamado para:
    │
    ├─ delay() de pause entre ações
    ├─ timeout de máquina de estados
    └─ duração de movimentos
    
Fórmula:
    Tempo_Escalado = (timeAtBase * TIME_SCALE_FACTOR) / VELOCITY_GLOBAL
    Onde TIME_SCALE_FACTOR = 255 (base)
    
Exemplo:
    ├─ VELOCITY_GLOBAL = 255
    │  └─ Tempo_Escalado = timeAtBase * 255 / 255 = timeAtBase (1:1)
    │
    ├─ VELOCITY_GLOBAL = 127 (metade)
    │  └─ Tempo_Escalado = timeAtBase * 255 / 127 ≈ 2 × timeAtBase
    │
    └─ VELOCITY_GLOBAL = 60 (muito lento)
       └─ Tempo_Escalado = timeAtBase * 255 / 60 ≈ 4 × timeAtBase
```

---

## 9. Fluxo Completo de Coleta

```
NAVIGATE (linha reta)
    ↓ (objeto detectado)
OBJECT_DETECTED (centralizar)
    ↓ (alinhado)
APPROACH (Fase 1 → Fase 2 → Fase 3)
    ↓ (distância < 5cm)
COLLECT (fechar garra)
    ├─ close() com timeout 3s
    ├─ SIM: objectsCollected++
    └─ NÃO: retry (máx 3x)
    ↓
ROTATE_90 (girar 90° para descartar)
    ├─ Direcção oposta ao sensor que detectou
    ├─ Tempo: scaleTime(600ms)
    └─ Monitorar linha durante rotação
    ↓
RELEASE (descartar e afastar)
    ├─ open() garra
    ├─ Recuar 500ms
    └─ Girar 90° de volta
    ↓
RETURN_LINE (encontrar linha)
    ├─ Girar até tocar linha
    ├─ Centralizar
    └─ Timeout: 3s
    ↓
NAVIGATE (continuar seguindo linha)
```

---

## 10. Proteções e Bloqueios

```
BLOQUEIO 1: Coleta em Curva Acentuada
    ├─ Se sensores S5 ou S6 ativos
    ├─ E ultrassônico detecta objeto
    └─ IGNORAR objeto
       (esperar realinhamento)

BLOQUEIO 2: Timeout Servo
    ├─ close() com espera máx 3s
    ├─ Se falhar
    └─ transição emergency ou retry

BLOQUEIO 3: Busca de Linha
    ├─ Máximo 5 rotações de 360°
    ├─ Se não encontra
    └─ EMERGENCY_STOP

BLOQUEIO 4: Tempo Geral
    ├─ Máximo RUNTIME_TOTAL (10 min)
    └─ EMERGENCY_STOP
```

---

Esta documentação visual ajuda a entender o fluxo completo da máquina de estados! 🔄

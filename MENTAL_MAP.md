# 🗺️ Mapa Mental - Robô Seguidor Coletor

## Hierarquia de Componentes

```
┌─────────────────────────────────────────────────────────────────┐
│                   RobotStateMachine                             │
│            (Máquina de Estados Finita - 10 estados)             │
│                                                                 │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │MotorControl  │  │ LineSensor   │  │UltrasonicSns│          │
│  └──────────────┘  └──────────────┘  └──────────────┘          │
│  ┌──────────────────────┐                                       │
│  │   GripperServo       │                                       │
│  └──────────────────────┘                                       │
│                                                                 │
│  Coordena: leitura → decisão → ação                             │
└─────────────────────────────────────────────────────────────────┘
```

---

## Ciclo de Execução

```
┌──────────┐
│  SETUP   │
│          │
│ 1. Serial│
│ 2. Init  │
│    comps │
└────┬─────┘
     │
     ▼
┌───────────────────────────────────────┐
│        LOOP (a cada 100ms)            │
│                                       │
│ robotStateMachine.update()            │
│   ├─ Ler sensores                    │
│   ├─ Validar transições              │
│   ├─ Executar ações (motores)        │
│   └─ Mudar estado se necessário      │
│                                       │
│ Debug a cada 10 ciclos                │
└───────────────┬───────────────────────┘
                │
                └─ Repete até EMERGENCY_STOP
```

---

## Hierarquia de Decisão

```
        ┌─────────────────┐
        │   NAVEGANDO?    │
        └────────┬────────┘
                 │
        ┌────────▼────────┐
        │  Detecta OBJETO?│
        └────────┬────────┘
                 │
        ┌────────▼─────────────┐
        │ Está em CURVA?       │
        ├─ SIM → IGNORAR      │
        └─ NÃO → COLETAR     │
                 │
        ┌────────▼──────────────────────────┐
        │ Localizar e Centralizar Objeto    │
        └────────┬──────────────────────────┘
                 │
        ┌────────▼──────────────────────────┐
        │ Fase 1 (Distante >30cm)           │
        │ → Desacelerar (MEDIUM)            │
        │                                   │
        │ Fase 2 (Próximo 15-30cm)          │
        │ → Velocidade lenta (SLOW)         │
        │                                   │
        │ Fase 3 (Contato <15cm)            │
        │ → Parar e COLETAR                 │
        └────────┬──────────────────────────┘
                 │
        ┌────────▼──────────────────────────┐
        │ Girar 90° (descartar)              │
        │ + Descartar                        │
        │ + Retornar à linha                 │
        └────────┬──────────────────────────┘
                 │
        ┌────────▼──────────────────────────┐
        │ Continuar navegando                │
        └────────────────────────────────────┘
```

---

## Matriz de Estados x Transições

```
De        │ Para              │ Condição
━━━━━━━━━┿───────────────────┼─────────────────────────
IDLE      │ NAVIGATE          │ always (setup)
NAVIGATE  │ CURVE             │ curva detectada
NAVIGATE  │ OBJECT_DETECTED   │ objeto validado (+ não em curva)
NAVIGATE  │ LINE_SEARCH       │ linha perdida
CURVE     │ NAVIGATE          │ sensores alinhados
OBJECT_   │ APPROACH          │ centralizado
DETECTED  │ NAVIGATE          │ timeout (2s)
APPROACH  │ COLLECT           │ fase 3 (<15cm)
APPROACH  │ NAVIGATE          │ objeto perdido
COLLECT   │ ROTATE_90         │ garra fechou OK
COLLECT   │ NAVIGATE          │ falhou 3x
ROTATE_90 │ RELEASE           │ rotação concluída
RELEASE   │ RETURN_LINE       │ descarte concluído
RETURN_   │ NAVIGATE          │ linha encontrada
LINE      │ NAVIGATE          │ timeout (3s)
LINE_     │ NAVIGATE          │ linha encontrada
SEARCH    │ EMERGENCY_STOP    │ 5 rotações sem linha
ANY       │ EMERGENCY_STOP    │ timeout geral (10 min)
```

---

## Fluxo de Dados

```
SENSORES
  │
  ├─ S1-S6 (A0-A5)
  │  └─ LineSensor → 6-bit pattern → LinePattern
  │
  ├─ Trigger/Echo (11/12)
  │  └─ UltrasonicSensor → distância (cm) → ApproachPhase
  │
  └─ Timers internos
     └─ Estado anterior → tempo decorrido

        ▼

LÓGICA (RobotStateMachine)
  │
  ├─ shouldDetectObject()?
  ├─ shouldReturnToNavigate()?
  ├─ shouldSearchLine()?
  └─ Validadores de transição

        ▼

ATUADORES
  │
  ├─ Motor (PWM 3, 6)
  │  └─ MotorController → velocidade esquerda/direita
  │
  ├─ Servo (9)
  │  └─ GripperServo → ângulo (0-180)
  │
  └─ Serial
     └─ Debug logs
```

---

## Escala de Velocidade (Exemplo com VELOCITY_GLOBAL=200)

```
VELOCITY_GLOBAL = 200

   ↓

PWM_SLOW = (85 × 200) / 255 = 67
PWM_MEDIUM = (170 × 200) / 255 = 133
PWM_FAST = (255 × 200) / 255 = 200

   ↓

Tempo escalado para ROTATION_90:
T = (600 × 255) / 200 = 765ms

   ↓

Resultado: Robô inteiro se comporta proporcionalmente
(mais rápido = tempos mais curtos = comportamento equilibrado)
```

---

## Cronograma de Execução (ciclo de 100ms)

```
T=0ms     │ Início ciclo
          │ ├─ Ler sensores de linha
          │ ├─ Ler ultrassônico
          │ ├─ Validações de debounce
          │ └─ Executar máquina de estados
T=50ms    │ (aproximado) Atualização de motores
T=100ms   │ Fim ciclo, aguardar T=100ms

T=100ms   │ Próximo ciclo começa
...
T=1000ms  │ Debug impresso (a cada 10 ciclos)
```

---

## Proteções e Bloqueios

```
BLOQUEIO 1
┌─────────────────────────────────────────┐
│ Não coletar em curva acentuada          │
│ if (S5 || S6) && ultrassônico          │
│    → IGNORE ultrassônico                │
└─────────────────────────────────────────┘

BLOQUEIO 2
┌─────────────────────────────────────────┐
│ Servo com timeout                       │
│ close() com espera máx 3 segundos       │
│ Se falhar → retry ou NAVIGATE           │
└─────────────────────────────────────────┘

BLOQUEIO 3
┌─────────────────────────────────────────┐
│ Busca de linha limitada                 │
│ Máximo 5 rotações × 360°                │
│ Se não encontra → EMERGENCY_STOP        │
└─────────────────────────────────────────┘

BLOQUEIO 4
┌─────────────────────────────────────────┐
│ Timeout geral                           │
│ Máximo RUNTIME_TOTAL = 10 minutos       │
│ → EMERGENCY_STOP                        │
└─────────────────────────────────────────┘
```

---

## Algoritmo Round Robin (Cruzamentos)

```
Encontra INTERSECTION (4+ sensores)
    │
    ├─ Carregar lastTurn de EEPROM
    │
    ├─ if lastTurn == LEFT
    │  └─ próximo = RIGHT
    │
    └─ if lastTurn == RIGHT
       └─ próximo = LEFT
    │
    ├─ Salvar próximo em EEPROM
    │
    └─ Executar girar na direção próximo
```

---

## Calibração (3 passos)

```
PASSO 1: Linha Branca
┌──────────────────────────┐
│ Posicione sobre linha    │
│ test_components opção 5  │
│ Colete 100 leituras      │
│ → valor máximo           │
└──────────────────────────┘

PASSO 2: Fundo Preto
┌──────────────────────────┐
│ Posicione no fundo       │
│ test_components opção 5  │
│ Colete 100 leituras      │
│ → valor mínimo           │
└──────────────────────────┘

PASSO 3: Calcular
┌──────────────────────────┐
│ THRESHOLD =              │
│ (máx + mín) / 2          │
│                          │
│ Cole em config.h         │
│ Re-upload                │
└──────────────────────────┘
```

---

## Checklist de Teste

```
□ Teste 1: Motores (frente/trás/curvas)
□ Teste 2: Sensores de linha (padrões)
□ Teste 3: Ultrassônico (3 fases)
□ Teste 4: Servo (abrir/fechar)
□ Teste 5: Calibração (threshold)

□ Teste 6: Linha reta 5m (sem parar)
□ Teste 7: Curva 10m em "S"
□ Teste 8: Objeto em reta (detecta + para)
□ Teste 9: Coleta simples (fecha garra)
□ Teste 10: Coleta completa (gira + descarta)
□ Teste 11: Pista 25m com 5 objetos
□ Teste 12: 10 minutos contínuos
```

---

## Troubleshooting Rápido

```
SINTOMA              │ VERIFICAR
─────────────────────┼──────────────────────
Não se move          │ Pinos motores (2,4,5,7)
Vai para trás        │ Pinos invertidos (IN1↔IN2)
Não detecta linha    │ Calibração threshold
Oscila em reta       │ VELOCITY_GLOBAL alto
Não coleta           │ HC-SR04 desligado
Garra trava          │ SERVO_TIMEOUT baixo
Perde linha curva    │ CURVE_COMPENSATION baixo
```

---

## Resumo: O que Cada Componente Faz

```
MotorController    → Controla velocidade e direção dos 2 motores
LineSensor         → Lê 6 sensores + identifica padrão de linha
UltrasonicSensor   → Mede distância + classifica fase de aproximação
GripperServo       → Controla garra com proteção contra trava
RobotStateMachine  → Coordena tudo (ler → pensar → agir)
config.h           → Parâmetros centralizados
main.cpp           → Loop principal que chama máquina de estados
test_components    → Testa cada peça isoladamente
```

---

## Fórmulas Importantes

```
ESCALAÇÃO DE TEMPO:
Tempo_Escalado = (Tempo_Base × 255) / VELOCITY_GLOBAL

ESCALAÇÃO DE PWM:
PWM_Escalado = (PWM_Base × VELOCITY_GLOBAL) / 255

THRESHOLD IDEAL:
THRESHOLD = (Valor_Linha + Valor_Fundo) / 2

COMPENSAÇÃO DE CURVA:
PWM_Interna = PWM_Externa × COMPENSATION_FACTOR
(Exemplo: 0.8 = 20% de diferença)

DEBOUNCE:
3 leituras consecutivas dentro da tolerância = validada
```

---

**Este mapa mental resume TODA a estrutura do projeto!** 🗺️

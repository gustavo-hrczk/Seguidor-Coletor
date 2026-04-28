# 🔧 Guia de Calibração e Ajustes Finos

## 📋 Índice
1. [Calibração de Sensores](#calibração-de-sensores)
2. [Ajuste de Velocidade](#ajuste-de-velocidade)
3. [Compensação de Curva](#compensação-de-curva)
4. [Calibração de Tempo](#calibração-de-tempo)
5. [Otimização de Coleta](#otimização-de-coleta)

---

## Calibração de Sensores

### 1. Threshold de Sensores de Linha

**O que é?** Limite que distingue a linha branca do fundo preto.

**Como Calibrar?**

1. Use o programa `test_components.cpp` (opção 5)
2. Posicione o robô sobre a linha
3. Anote os valores máximos
4. Posicione o robô no fundo preto
5. Anote os valores mínimos
6. O threshold recomendado é: `(máx + mín) / 2`

**Fórmula em config.h:**
```cpp
#define THRESHOLD_LINE_SENSOR 700  // Ajuste conforme calibração
```

**Exemplo:**
- Valores sobre linha: 450, 480, 460, 470, 490, 460
- Valores no fundo: 900, 920, 910, 900, 930, 910
- Média linha: 470
- Média fundo: 912
- **Threshold = (470 + 912) / 2 = 691** → Use 700

---

### 2. Sensor Ultrassônico (HC-SR04)

**O que é?** Distância máxima e mínima de detecção confiável.

**Como Calibrar?**

1. Use o programa `test_components.cpp` (opção 3)
2. Aproxime um objeto lentamente
3. Observe as leituras
4. Ajuste os limites em config.h:

```cpp
#define ULTRASONIC_DISTANCE_LONG 30    // Início da detecção
#define ULTRASONIC_DISTANCE_SHORT 15   // Transição fase 2→3
#define ULTRASONIC_DISTANCE_CONTACT 5  // Coleta
```

**Dica:** Se o robô não detecta objetos próximos, reduzir `DISTANCE_LONG`.

---

## Ajuste de Velocidade

### 1. Velocidade Global

**O que faz?** Controla **todo** o comportamento do robô.

**Valores Recomendados:**
```cpp
#define VELOCITY_GLOBAL 200          // Padrão equilibrado
// 255 = Máximo (rápido, arriscado)
// 200 = Recomendado (bom balanço)
// 150 = Mais lento (maior precisão)
// 100 = Muito lento (debug)
```

**Como Ajustar?**

| Problema | Solução |
|----------|---------|
| Robô muito lento | Aumentar VELOCITY_GLOBAL |
| Robô perde linha frequentemente | Diminuir VELOCITY_GLOBAL |
| Robô oscila em curvas | Diminuir VELOCITY_GLOBAL |
| Coleta falha no movimento | Aumentar VELOCITY_GLOBAL |

### 2. Velocidades de Movimento

```cpp
#define PWM_SLOW 85        // ~33% de VELOCITY_GLOBAL
#define PWM_MEDIUM 170     // ~67% de VELOCITY_GLOBAL
#define PWM_FAST 255       // ~100% de VELOCITY_GLOBAL
```

**Para manter proporção ao mudar VELOCITY_GLOBAL:**
```
Se VELOCITY_GLOBAL muda para V:
  PWM_SLOW = (85 * V) / 255
  PWM_MEDIUM = (170 * V) / 255
  PWM_FAST = (255 * V) / 255
```

---

## Compensação de Curva

### O que é?
Diferença de PWM entre roda interna e externa para manter raio constante.

**Valores em config.h:**
```cpp
#define CURVE_COMPENSATION_LIGHT 0.9   // 10% diferença
#define CURVE_COMPENSATION_MEDIUM 0.8  // 20% diferença
#define CURVE_COMPENSATION_SHARP 0.6   // 40% diferença
```

### Como Ajustar?

**Teste 1: Curva Suave**
1. Posicione em linha reta
2. Ative STATE_CURVE
3. Observe o raio de giro
4. Se raio > desejado → diminuir compensação (ex: 0.85)
5. Se raio < desejado → aumentar compensação (ex: 0.95)

**Fórmula:**
```
Roda_Interna = Roda_Externa * COMPENSATION_FACTOR
```

---

## Calibração de Tempo

### 1. Rotação de 90°

**O que é?** Tempo necessário para girar exatamente 90°.

**Como Calibrar?**

1. Marque uma linha no chão
2. Posicione robô perpendicular à linha
3. Execute `motor.move(TURN_LEFT, VELOCITY_GLOBAL)` por tempo X
4. Meça o ângulo girado
5. Calcule: `Tempo_90_graus = (X * 90) / ângulo_medido`

**Fórmula em config.h:**
```cpp
#define ROTATION_90_DEGREES_TIME 600  // em ms a VELOCITY_GLOBAL = 200
```

**Exemplo:**
- Tempo executado: 1000ms → ângulo: 150°
- Tempo para 90° = (1000 * 90) / 150 = 600ms ✓

### 2. Escalação Temporal

A fórmula automática já compensa:
```cpp
uint16_t RobotStateMachine::scaleTime(uint16_t timeAtBaseSpeed) const {
    return (timeAtBaseSpeed * TIME_SCALE_FACTOR) / VELOCITY_GLOBAL;
}
```

**Nenhum ajuste necessário aqui!** É automático.

---

## Otimização de Coleta

### 1. Fases de Aproximação

```cpp
#define ULTRASONIC_DISTANCE_LONG 30    // Desacelera
#define ULTRASONIC_DISTANCE_SHORT 15   // Velocidade lenta
#define ULTRASONIC_DISTANCE_CONTACT 5  // Coleta
```

**Teste:**
- Coloque objeto a 50cm do robô
- Observe se desacelera em 30cm ✓
- Observe se reduz mais em 15cm ✓
- Observe se para em 5cm ✓

### 2. Timeout da Garra

```cpp
#define SERVO_TIMEOUT 3000                  // 3 segundos
#define SERVO_STABILIZATION_TIME 500        // 500ms estabilização
```

**Se garra trava frequentemente:**
1. Aumentar SERVO_TIMEOUT (ex: 4000)
2. Ou aumentar SERVO_STABILIZATION_TIME (ex: 1000)

### 3. Tentativas de Coleta

```cpp
#define MAX_COLLECTION_ATTEMPTS 3  // Máximo de tentativas por objeto
```

---

## 📊 Checklist de Calibração

- [ ] **Sensores de Linha**
  - [ ] Threshold calibrado
  - [ ] Robô segue linha reta por 5m
  - [ ] Detecta curvas corretamente

- [ ] **Velocidade Global**
  - [ ] Robô não perde linha em alta velocidade
  - [ ] Sem oscilações em curvas
  - [ ] Aceleração/frenagem suave

- [ ] **Compensação de Curva**
  - [ ] Raio de giro constante
  - [ ] Sem alargamento nas curvas

- [ ] **Rotação de 90°**
  - [ ] Gira exatamente 90° (±5°)
  - [ ] Tempo está calibrado corretamente

- [ ] **Sensor Ultrassônico**
  - [ ] Detecta objetos a 30cm
  - [ ] Reduz velocidade progressiva
  - [ ] Para precisamente a 5cm

- [ ] **Servo Garra**
  - [ ] Fecha sem travar
  - [ ] Abre completamente
  - [ ] Timeout funciona

---

## 🎯 Testes Progressivos

### Teste 1: Linha Reta (5m)
```
✓ Robô segue linha reta sem desvio
✓ Tempo: ~20-30s (dependendo VELOCITY_GLOBAL)
```

### Teste 2: Curvas (10m de pista com curvas)
```
✓ Robô faz curvas suaves
✓ Sem oscilação em cruzamentos
✓ Tempo: ~45-60s
```

### Teste 3: Coleta Simples (objeto em reta)
```
✓ Detecta objeto a 30cm+
✓ Reduz velocidade progressiva
✓ Para e coleta
✓ Abre garra
```

### Teste 4: Coleta com Retorno (objeto + giro 90° + descartar)
```
✓ Coleta objeto
✓ Gira 90° (±5°)
✓ Descarta
✓ Retorna à linha
✓ Continua navegando
```

### Teste 5: Pista Completa (25-30m, múltiplos objetos)
```
✓ Coleta 5 objetos com 100% sucesso
✓ Recupera de linha perdida (8/10 tentativas)
✓ Executa por 10 minutos
✓ Sem travamentos
```

---

## 🐛 Troubleshooting Avançado

### Problema: Robô se move para trás quando deveria ir para frente

**Causa:** Pinos de direção invertidos

**Solução:** Em config.h, trocar PIN_IN1 com PIN_IN2 (ou IN3 com IN4)

### Problema: Sensores alternam loucamente entre 0 e 1

**Causa:** Threshold muito perto do valor médio

**Solução:** Aumentar margem entre linha e fundo
- Aumentar THRESHOLD_LINE_SENSOR em 10-20 pontos
- Reposicionar sensores para captar diferença maior

### Problema: Robô detecta linha onde não há (falso positivo)

**Causa:** Threshold muito baixo, sensores captando reflexo

**Solução:** Aumentar THRESHOLD_LINE_SENSOR

### Problema: Garra não fecha completamente

**Causa:** SERVO_ANGLE_CLOSED muito alto (servo não consegue)

**Solução:** 
```cpp
#define SERVO_ANGLE_CLOSED 10  // Reduzir de 0 para 10-30
```

### Problema: Oscila muito em cruzamentos

**Causa:** Compensação de curva inadequada

**Solução:** Aumentar CURVE_COMPENSATION_* (ex: 0.95 em vez de 0.9)

---

## 📱 Monitoramento via Serial

Ative DEBUG_MODE em config.h:
```cpp
#define DEBUG_MODE true
```

Monitorar com Monitor Serial (9600 baud):
- Estados da máquina
- Leituras de sensores
- Distância do ultrassônico
- Tentativas de coleta

---

**Pronto? Comece com o Teste 1 e aumente a dificuldade gradualmente!** 🚀

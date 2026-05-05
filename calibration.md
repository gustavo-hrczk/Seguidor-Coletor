# CALIBRATION — Guia de calibração

Procedimento completo para calibrar o robô do zero em uma nova pista.

---

## Pré-requisitos

- Arduino IDE ou PlatformIO instalado
- Monitor serial configurado para 9600 baud
- `test_components.cpp` compilado e gravado
- `DEBUG_MODE true` no `config.h`

---

## Etapa 1 — Calibração do threshold de linha (Teste 6)

O threshold define o ponto de corte entre linha branca e fundo preto.

**Procedimento:**

1. Grave `test_components.cpp`, abra o monitor serial
2. Envie `6` para iniciar o Teste 6
3. **Etapa 1/2:** posicione TODOS os 6 sensores diretamente sobre a fita branca
   - Garanta altura de 3–6mm da superfície
   - Aguarde os 4 segundos e a coleta de 200 amostras
4. **Etapa 2/2:** mova o robô para o fundo preto (fora da fita)
   - Aguarde os 4 segundos e a coleta
5. Anote o `THRESHOLD REC.` exibido no resultado
6. Atualize `config.h`:
   ```cpp
   #define THRESHOLD_LINE_SENSOR  <valor_recomendado>
   ```

**Resultado esperado:**
```
Linha branca — max por sensor: ~50–200
Fundo preto  — min por sensor: ~800–1023
Separação:  > 200 pontos por sensor
```

**Aviso de separação baixa** significa que os LEDs IR não estão iluminando corretamente a superfície. Verifique:
- Altura dos sensores (ideal: 3–6mm)
- Alimentação VCC do QTR (5V)
- Câmera do celular apontada para os LEDs — deve aparecer luz roxa/branca

---

## Etapa 2 — Validação da leitura (Teste 2)

Confirma que threshold e pesos estão produzindo posição correta.

**Procedimento:**

1. Envie `2` no monitor serial
2. Posicione S3+S4 (centrais) sobre a linha — deve mostrar:
   ```
   ativos=001100  pos=0.000  pad=RETA
   ```
3. Deslize lentamente para a esquerda — posição deve ir de 0.0 → -1.0
4. Deslize para a direita — posição deve ir de 0.0 → +1.0
5. Retire da linha — deve mostrar `pad=!! PERDIDA !!`

**Checklist pós-teste:**
- [ ] `raw` sobre linha < `THRESHOLD_LINE_SENSOR`
- [ ] `raw` sobre fundo > `THRESHOLD_LINE_SENSOR`
- [ ] `pos=0.000` com S3+S4 ativos
- [ ] Posição negativa ao mover para esquerda
- [ ] Posição positiva ao mover para direita

---

## Etapa 3 — Validação dos motores (Teste 1)

Confirma direções e que todos os movimentos estão corretos.

**Procedimento:**

1. Envie `1` no monitor serial — robô executa sequência automática
2. Verifique cada movimento:

| Esperado | Se errado |
|---|---|
| FRENTE: avança para frente | Inverter sinal em `move(FORWARD)` |
| RÉ: recua | Inverter sinal em `move(BACKWARD)` |
| ESQUERDA: gira no eixo para esquerda | Trocar `TURN_LEFT`/`TURN_RIGHT` |
| DIREITA: gira no eixo para direita | Trocar `TURN_LEFT`/`TURN_RIGHT` |

> Se um motor girar no sentido errado individualmente: trocar os fios desse motor no L298N, ou inverter o sinal apenas daquele motor em `setMotorSpeed()`.

---

## Etapa 4 — Ajuste do PD (Teste 7 / main.cpp)

O controlador PD precisa de ajuste fino na pista real.

**Procedimento:**

1. Grave `main.cpp`, posicione o robô sobre a linha
2. Ligue o robô e observe o comportamento
3. Ajuste os parâmetros conforme a tabela:

| Sintoma | Ajuste |
|---|---|
| Serpenteia em reta | Reduzir `PD_KP` (ex: 0.8 → 0.5) |
| Não reage rápido nas curvas | Aumentar `PD_KP` (ex: 0.8 → 1.2) |
| Oscila ao corrigir | Aumentar `PD_KD` (ex: 0.3 → 0.5) |
| Trava após correção | Reduzir `PD_KD` (ex: 0.3 → 0.1) |
| Perde linha nas curvas | Reduzir `SPEED_ERROR_HIGH` |
| Muito lento em reta | Aumentar `SPEED_ERROR_LOW` |
| Para em curvas fechadas | Aumentar `PD_MIN_INNER_SPEED` |

**Valores iniciais recomendados por cenário:**

```cpp
// Pista pequena / validação inicial
PD_KP = 0.5   PD_KD = 0.1
SPEED_ERROR_LOW = 120   SPEED_ERROR_MEDIUM = 100   SPEED_ERROR_HIGH = 80

// Pista média / operação normal
PD_KP = 0.8   PD_KD = 0.3
SPEED_ERROR_LOW = 180   SPEED_ERROR_MEDIUM = 150   SPEED_ERROR_HIGH = 110

// Pista grande / alta velocidade
PD_KP = 1.2   PD_KD = 0.4
SPEED_ERROR_LOW = 255   SPEED_ERROR_MEDIUM = 200   SPEED_ERROR_HIGH = 150
```

---

## Etapa 5 — Calibração da garra (Testes 4 e 5)

**Teste 4** — verifica abertura e fechamento mecânico:
1. Envie `4` — a garra abre, fecha, abre e fecha novamente
2. Verifique se o objeto é preso com força suficiente
3. Se fraca: reduzir `SERVO_ANGLE_CLOSED` (ex: 90 → 100)
4. Se emperrar: aumentar `SERVO_ANGLE_OPEN` (ex: 5 → 10)

**Teste 5** — garra reativa com sensor ultrassônico:
1. Envie `5` — coloque um objeto a menos de `ULTRASONIC_DISTANCE_CONTACT` cm
2. Mantenha parado por `GRIPPER_STABLE_TIME_MS` ms
3. A garra deve fechar automaticamente

**Ajustes comuns:**

| Comportamento | Ajuste |
|---|---|
| Fecha muito cedo | Reduzir `ULTRASONIC_DISTANCE_CONTACT` |
| Não fecha mesmo com objeto próximo | Aumentar `ULTRASONIC_DISTANCE_CONTACT` |
| Fecha com objeto instável | Aumentar `GRIPPER_STABLE_TIME_MS` |
| Demora muito para fechar | Reduzir `GRIPPER_STABLE_TIME_MS` |

---

## Referência rápida de valores por pista

| Parâmetro | Pista pequena | Pista média | Pista grande |
|---|---|---|---|
| `SPEED_ERROR_LOW` | 100–130 | 150–180 | 200–255 |
| `SPEED_ERROR_HIGH` | 70–90 | 100–120 | 140–180 |
| `PD_MIN_INNER_SPEED` | 60–70 | 70–90 | 90–110 |
| `PD_KP` | 0.4–0.6 | 0.7–1.0 | 1.0–1.5 |
| `PD_KD` | 0.1–0.2 | 0.3–0.4 | 0.4–0.6 |
| `RECOVERY_SPIN_MS` | 200–300 | 300–400 | 400–600 |

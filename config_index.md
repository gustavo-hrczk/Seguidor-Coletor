# CONFIG_INDEX — Índice de constantes do config.h

Referência completa de todas as constantes configuráveis. Organizado por categoria com indicação de impacto e quem consome cada valor.

---

## Pinos

| Constante | Pino | Função | Módulo |
|---|---|---|---|
| `PIN_IN1` | D2 | Motor direito — sentido 1 | MotorController |
| `PIN_IN2` | D4 | Motor direito — sentido 2 | MotorController |
| `PIN_IN3` | D5 | Motor esquerdo — sentido 1 | MotorController |
| `PIN_IN4` | D7 | Motor esquerdo — sentido 2 | MotorController |
| `PIN_ENA` | D3 | PWM motor esquerdo (Timer 2) | MotorController |
| `PIN_ENB` | D6 | PWM motor direito (Timer 0) | MotorController |
| `PIN_S1` | A0 | Sensor linha — extrema esquerda (peso -5) | LineSensor |
| `PIN_S2` | A1 | Sensor linha — esquerda (peso -3) | LineSensor |
| `PIN_S3` | A2 | Sensor linha — centro-esquerda (peso -1) | LineSensor |
| `PIN_S4` | A3 | Sensor linha — centro-direita (peso +1) | LineSensor |
| `PIN_S5` | A4 | Sensor linha — direita (peso +3) | LineSensor |
| `PIN_S6` | A5 | Sensor linha — extrema direita (peso +5) | LineSensor |
| `PIN_TRIGGER` | D12 | HC-SR04 trigger | UltrasonicSensor |
| `PIN_ECHO` | D13 | HC-SR04 echo | UltrasonicSensor |
| `PIN_SERVO` | D9 | SG90 sinal PWM | GripperServo |

---

## Velocidades

| Constante | Valor atual | Descrição | Consumido por |
|---|---|---|---|
| `VELOCITY_GLOBAL` | 150 | PWM base para movimentos manuais e testes | `move()` |
| `PWM_SLOW` | 70 | Velocidade de recuperação de linha perdida | `handleRecovery()` |
| `PWM_MIN_DEADZONE` | 60 | PWM mínimo para vencer atrito estático | `applyDeadzoneCorrection()` |
| `SPEED_ERROR_LOW` | 150 | Velocidade em reta (\|erro\| < 0.3) | `followLine()` via main |
| `SPEED_ERROR_MEDIUM` | 130 | Velocidade em curva suave (\|erro\| 0.3–0.6) | `followLine()` via main |
| `SPEED_ERROR_HIGH` | 110 | Velocidade em curva aguda (\|erro\| > 0.6) | `followLine()` via main |
| `PD_MIN_INNER_SPEED` | 70 | Piso do motor interno em curva (garante torque) | `followLine()` |

> Para aumentar a velocidade máxima em pista grande: aumente `SPEED_ERROR_LOW`, `SPEED_ERROR_MEDIUM` e `SPEED_ERROR_HIGH` proporcionalmente. Manter a diferença entre eles para que a redução em curvas continue proporcional.

---

## Sensor de linha

| Constante | Valor atual | Descrição |
|---|---|---|
| `THRESHOLD_LINE_SENSOR` | 700 | Sensor ativo quando `raw <= threshold`. Calibrar com Teste 6. |
| `LINE_SENSOR_MASK` | `0b111110` | Bitmask de sensores ativos. Bit 0 = S1 (desabilitado por defeito). |
| `LINE_HYSTERESIS` | 15 | Reservado — não utilizado na versão atual. |

> **Convenção:** linha branca = raw BAIXO (~50), fundo preto = raw ALTO (~950–1023).
> Threshold recomendado = ponto médio entre `max_linha` e `min_fundo`.

### Pesos dos sensores

```
S1 = -5   S2 = -3   S3 = -1   S4 = +1   S5 = +3   S6 = +5
```

Posição normalizada = soma ponderada / totalWeight / 5.0

---

## Controlador PD

| Constante | Valor atual | Efeito de aumentar | Efeito de diminuir |
|---|---|---|---|
| `PD_KP` | 0.8 | Mais reativo à posição da linha | Resposta mais lenta, pode perder linha em curvas |
| `PD_KD` | 0.3 | Amortece oscilações | Pode oscilar (serpenteado) |
| `PD_SAMPLE_MS` | 10 | — | Ciclo mais rápido (100Hz → aumentar responsividade) |

> Para pista pequena e velocidade baixa: `Kp=0.5 Kd=0.1`.
> Para pista grande e velocidade alta: `Kp=1.0–1.5 Kd=0.3–0.5`.

---

## Recuperação de linha

| Constante | Valor atual | Descrição |
|---|---|---|
| `RECOVERY_SPIN_MS` | 300 | Tempo girando na última direção conhecida (ms) |
| `RECOVERY_TIMEOUT_MS` | 1500 | Timeout total antes de parar completamente (ms) |

---

## Cruzamentos

| Constante | Valor atual | Descrição |
|---|---|---|
| `CROSS_MIN_SENSORS_T` | 4 | Sensores ativos simultâneos para detectar cruzamento T |
| `CROSS_MIN_SENSORS_X` | 5 | Sensores ativos simultâneos para detectar cruzamento X |

---

## Sensor ultrassônico

| Constante | Valor atual | Descrição |
|---|---|---|
| `ULTRASONIC_DISTANCE_LONG` | 30 cm | Fase 1 — distante (desacelerar) |
| `ULTRASONIC_DISTANCE_SHORT` | 15 cm | Fase 2 — aproximando (velocidade lenta) |
| `ULTRASONIC_DISTANCE_CONTACT` | 5 cm | Fase 3 — contato (parar e coletar) |
| `ULTRASONIC_NOISE_TOLERANCE` | 30% | Tolerância de variação para validação |
| `ULTRASONIC_DEBOUNCE_TIME` | 300 ms | Tempo mínimo entre detecções |
| `SENSOR_FILTER_CYCLES` | 3 | Leituras consecutivas consistentes para validar |

---

## Compensações de curva

Usadas por `curveCompensated()` nos testes de motor. Não afetam `followLine()`.

| Constante | Valor | Diferença entre motores |
|---|---|---|
| `CURVE_COMPENSATION_LIGHT` | 0.9 | 10% |
| `CURVE_COMPENSATION_MEDIUM` | 0.8 | 20% |
| `CURVE_COMPENSATION_SHARP` | 0.6 | 40% |

---

## Garra (GripperServo)

| Constante | Valor atual | Descrição |
|---|---|---|
| `SERVO_ANGLE_OPEN` | 5° | Ângulo de abertura (evitar 0° — limite mecânico do SG90) |
| `SERVO_ANGLE_CLOSED` | 90° | Ângulo de fechamento |
| `SERVO_STEP_DELAY_MS` | 10 ms | Pausa entre cada grau — controla velocidade do movimento |
| `SERVO_STABILIZATION_TIME` | 400 ms | Aguarda após atingir ângulo antes de desligar PWM |
| `GRIPPER_STABLE_TIME_MS` | 300 ms | Tempo contínuo dentro da zona para disparar coleta |
| `GRIPPER_HOLD_TIME_MS` | 3000 ms | Tempo com garra fechada antes de reabrir |

> Reduzir `SERVO_STEP_DELAY_MS` para 8ms acelera o movimento. Abaixo de 8ms o SG90 pode não acompanhar.

---

## Sistema

| Constante | Valor | Descrição |
|---|---|---|
| `BAUD_RATE` | 9600 | Taxa serial — monitor serial deve usar o mesmo valor |
| `DEBUG_MODE` | true | `false` remove todo código de log e libera memória |
| `RUNTIME_TOTAL` | 600000 ms | Reservado para uso futuro na máquina de estados |
| `EEPROM_ADDR_LAST_TURN` | 0 | Endereço EEPROM para persistir última direção de giro |

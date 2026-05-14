# CONFIG_INDEX — Índice de constantes do config.h

Referência completa de todas as constantes configuráveis.
Organizado por seção com indicação de impacto, valor atual e qual módulo consome cada valor.

> Sempre que alterar uma constante aqui, consulte a coluna "Consumido por"
> para saber quais arquivos serão afetados.

---

## 1. Pinos de hardware

| Constante     | Pino | Função                                     | Módulo           |
| `PIN_IN1`     | D2   | Motor direito — sentido 1 (L298N IN1)      | MotorController  |
| `PIN_IN2`     | D4   | Motor direito — sentido 2 (L298N IN2)      | MotorController  |
| `PIN_IN3`     | D5   | Motor esquerdo — sentido 1 (L298N IN3)     | MotorController  |
| `PIN_IN4`     | D7   | Motor esquerdo — sentido 2 (L298N IN4)     | MotorController  |
| `PIN_ENA`     | D3   | PWM motor esquerdo — Timer 2               | MotorController  |
| `PIN_ENB`     | D6   | PWM motor direito — Timer 0                | MotorController  |
| `PIN_S1`      | A0   | Sensor linha — extrema esquerda (peso -5)  | LineSensor       |
| `PIN_S2`      | A1   | Sensor linha — esquerda (peso -3)          | LineSensor       |
| `PIN_S3`      | A2   | Sensor linha — centro-esquerda (peso -1)   | LineSensor       |
| `PIN_S4`      | A3   | Sensor linha — centro-direita (peso +1)    | LineSensor       |
| `PIN_S5`      | A4   | Sensor linha — direita (peso +3)           | LineSensor       |
| `PIN_S6`      | A5   | Sensor linha — extrema direita (peso +5)   | LineSensor       |
| `PIN_TRIGGER` | D12  | HC-SR04 — pino de disparo                  | UltrasonicSensor |
| `PIN_ECHO`    | D13  | HC-SR04 — pino de retorno                  | UltrasonicSensor |
| `PIN_SERVO`   | D9   | SG90 — sinal PWM                           | GripperServo     |

---

## 2. Velocidades gerais

| Constante          | Valor | Descrição                              | Consumido por               |
| `VELOCITY_GLOBAL`  | 200   | PWM para movimentos manuais e testes   | `move()`                    |
| `PWM_SLOW`         | 70    | PWM de recuperação de linha perdida    | `handleRecovery()`          |
| `PWM_MIN_DEADZONE` | 60    | PWM mínimo para vencer atrito estático | `applyDeadzoneCorrection()` |

> `VELOCITY_GLOBAL` **não** controla o seguimento de linha — para isso use `SPEED_ERROR_*`.
> Aumentar `VELOCITY_GLOBAL` afeta apenas testes manuais (Teste 1) e a recuperação de linha.

---

## 3. Sensor de linha

| Constante               | Valor | Descrição                                                   | Consumido por |
| `THRESHOLD_LINE_SENSOR` | 700   | Limiar de detecção — sensor ativo quando `raw <= threshold` | LineSensor    |

> **Convenção do hardware:** linha branca = raw BAIXO (~50–200), fundo preto = raw ALTO (~800–1023).
> Calibrar com o Teste 6 (`test_components.cpp`) na pista real com a iluminação do local.
> O valor recomendado pelo Teste 6 = ponto médio entre `max_linha` e `min_fundo`.

### Pesos dos sensores (fixos no código)

```
S1 = -5   S2 = -3   S3 = -1   S4 = +1   S5 = +3   S6 = +5
```

Fórmula da posição: `pos = soma(peso_i × intensidade_i) / totalWeight / 5.0`

Resultado: `-1.0` = linha na extrema esquerda | `0.0` = centralizado | `+1.0` = extrema direita

---

## 4. Controlador PD

| Constante      | Valor | Efeito de aumentar                            | Efeito de diminuir                           |
| `PD_KP`        | 0.8   | Mais reativo — reage mais rápido ao desvio    | Resposta lenta — pode perder linha em curvas |
| `PD_KD`        | 0.3   | Amortece oscilações                           | Robô serpenteia em reta                      |
| `PD_SAMPLE_MS` | 10 ms | Loop mais lento (menos atualizações/s)        | Mais atualizações — maior responsividade     |

> **Referência de ajuste por tamanho de pista:**
> - Pequena / baixa velocidade: `Kp=0.4–0.6  Kd=0.1–0.2`
> - Normal: `Kp=0.8–1.0  Kd=0.3–0.4`
> - Grande / alta velocidade: `Kp=1.2–1.5  Kd=0.4–0.6`

### Velocidades do seguimento PD

| Constante                 | Valor | Quando é usada                                    | Consumido por           |
| `SPEED_ERROR_LOW`         | 150   | `\|erro\| < 0.3` — reta                           | `followLine()` via main |
| `SPEED_ERROR_MEDIUM`      | 130   | `\|erro\|` entre 0.3 e 0.6 — curva suave          | `followLine()` via main |
| `SPEED_ERROR_HIGH`        | 110   | `\|erro\| > 0.6` — curva aguda                    | `followLine()` via main |
| `PD_MIN_INNER_SPEED`      | 70    | Piso do motor interno — garante torque em curvas  | `followLine()`          |

> Para aumentar a velocidade máxima: aumente os três `SPEED_ERROR_*` proporcionalmente,
> mantendo a diferença relativa entre eles para preservar a redução gradual em curvas.

---

## 5. Recuperação de linha

| Constante             | Valor   | Descrição                                             | Consumido por      |
| `RECOVERY_SPIN_MS`    | 300 ms  | Tempo girando na última direção conhecida (estágio 1) | `handleRecovery()` |
| `RECOVERY_TIMEOUT_MS` | 1500 ms | Timeout total antes de STATE_STOPPED (estágio 2)      | `handleRecovery()` |

> Se o robô parar frequentemente por timeout: aumente `RECOVERY_TIMEOUT_MS`.
> Se o robô girar demais antes de encontrar a linha: reduza `RECOVERY_SPIN_MS`.

---

## 6. Detecção de cruzamentos

| Constante             | Valor | Descrição                         | Consumido por      |
| `CROSS_MIN_SENSORS_T` | 4     | Sensores ativos para cruzamento T | `getLinePattern()` |
| `CROSS_MIN_SENSORS_X` | 5     | Sensores ativos para cruzamento X | `getLinePattern()` |

---

## 7. Compensações de curva

Usadas exclusivamente por `curveCompensated()` nos testes manuais.
**Não afetam `followLine()`** — o PD calcula sua própria compensação.

| Constante                     | Valor | Diferença entre motores | Consumido por        |
| `CURVE_COMPENSATION_LIGHT`    | 0.9   | 10%                     | `curveCompensated()` |
| `CURVE_COMPENSATION_MEDIUM`   | 0.8   | 20%                     | `curveCompensated()` |
| `CURVE_COMPENSATION_SHARP`    | 0.6   | 40%                     | `curveCompensated()` |

---

## 8. Sensor ultrassônico

| Constante                     | Valor | Descrição                                         | Consumido por                                 |
| `ULTRASONIC_DISTANCE_LONG`    | 30 cm | Limite da fase 1 — desacelerar                    | `getApproachPhase()`                          |   
| `ULTRASONIC_DISTANCE_SHORT`   | 15 cm | Limite da fase 2 — velocidade lenta               | `getApproachPhase()`                          |   
| `ULTRASONIC_DISTANCE_CONTACT` | 4 cm  | Gatilho de fechamento da garra                    | `handleFollowing()`, `testGripperReactive()`  |
| `ULTRASONIC_NOISE_TOLERANCE`  | 30%   | Tolerância de variação entre leituras             | `isWithinTolerance()`                         |
| `SENSOR_FILTER_CYCLES`        | 3     | Leituras consecutivas para marcar `readingStable` | `validateReading()`                           |

> A tolerância é **adaptativa**: abaixo de 20 cm usa ±2 cm fixo; acima usa o percentual
> com teto de ±4 cm para evitar janelas excessivamente largas.

---

## 9. Garra (GripperServo / SG90)

| Constante                 | Valor     | Descrição                                            | Consumido por                   |
| `SERVO_ANGLE_OPEN`        | 5°        | Posição aberta — evitar 0° (limite mecânico do SG90) | `GripperServo`                  |
| `SERVO_ANGLE_CLOSED`      | 170°      | Posição fechada                                      | `GripperServo`                  |
| `SERVO_STEP_DELAY_MS`     | 10 ms     | Pausa entre cada grau — controla velocidade          | `moveToAngle()`                 |
| `SERVO_STABILIZATION_TIME`| 400 ms    | Aguarda assentar antes de desligar PWM               | `moveToAngle()`, `initialize()` |
| `GRIPPER_STABLE_TIME_MS`  | 300 ms    | ms contínuos na zona de contato para fechar          | `testGripperReactive()`, main   |
| `GRIPPER_HOLD_TIME_MS`    | 3000 ms   | ms com garra fechada antes de reabrir                | `testGripperReactive()`, main   |

> `SERVO_STEP_DELAY_MS` mínimo prático: **8 ms**. Abaixo disso o SG90 não acompanha
> o sinal e pode vibrar ou não atingir o ângulo alvo.
> `SERVO_ANGLE_CLOSED` foi ajustado para 170° para a garra atual — calibrar fisicamente.

---

## 10. Teste de coleta autônoma (test_collect.cpp)

Constantes exclusivas do ciclo de coleta sem seguidor de linha.
Ajustar sem impacto nos demais módulos.

### Velocidades de aproximação

| Constante                 | Valor | Quando é usada                                                |
| `APPROACH_SPEED_FAST`     | 200   | Distância > `APPROACH_DIST_LONG`                              |
| `APPROACH_SPEED_MEDIUM`   | 130   | Distância entre `APPROACH_DIST_MEDIUM` e `APPROACH_DIST_LONG` |
| `APPROACH_SPEED_SLOW`     | 80    | Distância <= `APPROACH_DIST_MEDIUM`                           |
| `APPROACH_DIST_LONG`      | 20 cm | Limiar de troca para velocidade média                         |
| `APPROACH_DIST_MEDIUM`    | 10 cm | Limiar de troca para velocidade lenta                         |

### Manobra lateral

| Constante                 | Valor     | Descrição                                    |
| `MANEUVER_SPEED_LOADED`   | 220       | PWM da manobra com objeto na garra           |
| `MANEUVER_SPEED_UNLOADED` | 200       | PWM do retorno sem objeto                    |
| `STRAFE_LOADED_MS`        | 300 ms    | Duração do deslocamento lateral com objeto   |
| `STRAFE_UNLOADED_MS`      | 250 ms    | Duração do retorno sem objeto                |
| `TURN_90_LOADED_MS`       | 750 ms    | Tempo de giro de 90° com carga               |
| `TURN_90_UNLOADED_MS`     | 650 ms    | Tempo de giro de 90° sem carga               |
| `COLLECT_STOP_DELAY`      | 300 ms    | Pausa entre etapas do ciclo                  |
| `COLLECT_CYCLE_DURATION`  | 120000 ms | Duração total do programa (2 minutos)        |

> `STRAFE_*_MS` e `TURN_90_*_MS` devem ser calibrados na pista real —
> superfície, peso do robô e tensão da bateria afetam diretamente o deslocamento real.

---

## 11. Sistema

| Constante               | Valor   | Descrição                                        |
| `BAUD_RATE`             | 9600    | Taxa serial — monitor deve usar o mesmo valor    |
| `DEBUG_MODE`            | true    | `false` remove todo código de log em compilação  |

> `DEBUG_MODE false` é recomendado para operação final — libera memória de programa
> e elimina o overhead de formatação serial no loop principal.

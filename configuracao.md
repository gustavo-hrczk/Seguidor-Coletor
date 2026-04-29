
---

## 📄 `CONFIGURACAO.md`

```markdown
# 🔧 Configuração Detalhada – `config.h`

Este arquivo descreve cada constante definida no `config.h`, seus efeitos e como ajustá‑los para o comportamento desejado.

## 🎚️ Velocidade e PWM

| Macro                      | Valor padrão | Descrição                                                                 |
|----------------------------|--------------|---------------------------------------------------------------------------|
| `VELOCITY_GLOBAL`          | 200          | Velocidade base usada em movimentos simples (frente/ré/giro).             |
| `PWM_SLOW`                 | 85           | Velocidade reduzida para curvas muito fechadas.                          |
| `PWM_MEDIUM`               | 170          | Velocidade média para curvas suaves.                                     |
| `PWM_FAST`                 | 255          | Velocidade máxima (reta).                                                |
| `PWM_MIN_DEADZONE`         | 60           | Menor PWM que efetivamente move o robô (compensa deadzone dos motores).  |

## 🧭 Sensores de Linha

| Macro                        | Valor padrão | Descrição                                                                 |
|------------------------------|--------------|---------------------------------------------------------------------------|
| `THRESHOLD_LINE_SENSOR`      | 800          | Valor analógico abaixo do qual o sensor considera que está sobre a linha. |
| `SENSOR_FILTER_CYCLES`       | 3            | Número de leituras consecutivas estáveis para validar a posição.          |
| `CROSS_MIN_SENSORS_T`        | 4            | Quantidade de sensores ativos para classificar cruzamento em T.           |
| `CROSS_MIN_SENSORS_X`        | 5            | Quantidade para classificar cruzamento em X.                              |

## 🎛️ Controlador PD (seguimento)

| Macro            | Valor padrão | Descrição                                                                 |
|------------------|--------------|---------------------------------------------------------------------------|
| `PD_KP`          | 0.8f         | Ganho proporcional – aumenta a reação ao erro de posição.                 |
| `PD_KD`          | 0.3f         | Ganho derivativo – suaviza oscilações (amortece).                         |
| `PD_SAMPLE_MS`   | 20           | Intervalo mínimo entre cálculos do PD (ms).                               |

> 💡 **Calibração**: Comece com `KP=0.5` e `KD=0.1` e aumente gradualmente até o robô seguir reto sem oscilar demais.

## 🏎️ Velocidade por Magnitude do Erro

| Macro                      | Valor padrão | Condição de erro (|pos|)                     |
|----------------------------|--------------|-----------------------------------------------|
| `SPEED_ERROR_LOW`          | 255          | `< 0.3` (reta)                                |
| `SPEED_ERROR_MEDIUM`       | 180          | entre `0.3` e `0.6` (curva suave ou média)   |
| `SPEED_ERROR_HIGH`         | 110          | `> 0.6` (curva acentuada/aguda)              |

## 📏 Sensor Ultrassônico

| Macro                              | Valor padrão | Descrição                                                                 |
|------------------------------------|--------------|---------------------------------------------------------------------------|
| `ULTRASONIC_DISTANCE_LONG`         | 30 cm        | Limite da fase "distante".                                                |
| `ULTRASONIC_DISTANCE_SHORT`        | 15 cm        | Início da fase "aproximando".                                             |
| `ULTRASONIC_DISTANCE_CONTACT`      | 5 cm         | Distância que dispara a coleta (garra fecha).                             |
| `ULTRASONIC_NOISE_TOLERANCE`       | 30 %         | Tolerância percentual para considerar leituras estáveis.                  |
| `ULTRASONIC_DEBOUNCE_TIME`         | 300 ms       | Tempo mínimo entre mudanças de estado (evita disparos falsos).            |

## 🦾 Garra (Servo)

| Macro                          | Valor padrão | Descrição                                                                 |
|--------------------------------|--------------|---------------------------------------------------------------------------|
| `SERVO_ANGLE_OPEN`             | 5°           | Ângulo do servo quando aberto.                                            |
| `SERVO_ANGLE_CLOSED`           | 90°          | Ângulo do servo quando fechado.                                           |
| `SERVO_STABILIZATION_TIME`     | 400 ms       | Tempo que o servo espera para estabilizar antes de desativar o sinal.     |
| `SERVO_STEP_DELAY_MS`          | 10 ms        | Intervalo entre incrementos suaves (movimento não brusco).                |
| `GRIPPER_STABLE_TIME_MS`       | 300 ms       | Tempo que o objeto deve permanecer na zona de coleta para fechar a garra. |
| `GRIPPER_HOLD_TIME_MS`         | 3000 ms      | Tempo que a garra mantém o objeto fechado antes de reabrir.               |

## 🕹️ Recuperação de Linha

| Macro                          | Valor padrão | Descrição                                                                 |
|--------------------------------|--------------|---------------------------------------------------------------------------|
| `RECOVERY_SPIN_MS`             | 400 ms       | Tempo de giro na última direção conhecida ao perder a linha.              |
| `RECOVERY_TIMEOUT_MS`          | 2000 ms      | Tempo máximo de tentativa de recuperação antes de parar.                  |
| `LINE_SEARCH_TIMEOUT`          | 2000 ms      | Timeout geral para busca de linha.                                        |
| `LINE_SEARCH_MAX_ROTATIONS`    | 5            | Número máximo de rotações completas na busca.                             |

## ⏱️ Tempos Gerais

| Macro                      | Valor padrão | Descrição                                                                 |
|----------------------------|--------------|---------------------------------------------------------------------------|
| `CYCLE_MAIN`               | 100 ms       | Período do loop principal (não usado nos testes, mas reservado).          |
| `TIME_SCALE_FACTOR`        | 255.0        | Fator de escala para conversões temporais (uso interno).                  |
| `ROTATION_90_DEGREES_TIME` | 600 ms       | Tempo para girar 90° na velocidade `VELOCITY_GLOBAL`.                     |

## 🔁 Sistema

| Macro                      | Valor padrão | Descrição                                                                 |
|----------------------------|--------------|---------------------------------------------------------------------------|
| `BAUD_RATE`                | 9600         | Taxa de comunicação Serial.                                               |
| `DEBUG_MODE`               | `true`       | Habilita impressões detalhadas no Serial Monitor.                         |
| `MAX_COLLECTION_ATTEMPTS`  | 3            | Tentativas máximas de coleta consecutivas (quando integrado).             |
| `RUNTIME_TOTAL`            | 600000 ms    | Tempo total de operação (10 minutos) para modo autônomo.                  |

## 💾 EEPROM

| Macro                          | Valor padrão | Descrição                                                                 |
|--------------------------------|--------------|---------------------------------------------------------------------------|
| `EEPROM_ADDR_LAST_TURN`        | 0            | Endereço para salvar a última direção de curva (expansão futura).         |

---

✏️ **Como alterar:** Edite o arquivo `config.h` na raiz do sketch e faça o upload novamente. Sempre recalibre o threshold da linha após mudanças mecânicas ou de iluminação.
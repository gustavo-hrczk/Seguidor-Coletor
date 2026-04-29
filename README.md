# 🤖 Robô Seguidor de Linha com Garra Reativa

Robô autônomo baseado em Arduino que segue linha preta em fundo claro, detecta objetos com sensor ultrassônico e coleta-os utilizando um servo motor. O sistema inclui um controlador PD para seguimento suave, filtros de estabilidade para sensores e uma máquina de estados para aproximação e coleta.

## ✨ Funcionalidades

- **Seguimento de linha** – 6 sensores QTR com posição ponderada contínua e controle PD adaptativo.
- **Detecção de cruzamentos** – diferencia entre reta, curva suave/média/aguda, cruzamento em T e cruzamento em X.
- **Coleta reativa** – utiliza sensor ultrassônico com validação de estabilidade para acionar a garra.
- **Testes individuais** – menu serial para testar motores, sensores, servo e calibração.
- **Modo debug** – saída detalhada via Serial para ajustes finos.

## 🧰 Hardware Necessário

| Componente               | Especificação / Modelo        | Quantidade |
|--------------------------|-------------------------------|------------|
| Arduino Uno / Mega       | Atmega328P / 2560             | 1          |
| Sensor de linha QTR-6    | 6 sensores analógicos         | 1          |
| Sensor ultrassônico      | HC-SR04                       | 1          |
| Ponte H                   | L298N ou similar              | 1          |
| Motores DC com rodas     | 6V~12V                        | 2          |
| Servo motor              | 9g ou padrão                  | 1          |
| Bateria                  | 7.4V~12V                      | 1          |
| Chassi seguidor de linha | –                             | 1          |

## 🔌 Pinagem (configuração padrão)

| Componente        | Pino Arduino |
|-------------------|--------------|
| Motor A (IN1)     | 2            |
| Motor A (IN2)     | 4            |
| Motor B (IN3)     | 5            |
| Motor B (IN4)     | 7            |
| PWM Motor A (ENA) | 3            |
| PWM Motor B (ENB) | 6            |
| Sensor S1 (A0)    | A0           |
| Sensor S2 (A1)    | A1           |
| Sensor S3 (A2)    | A2           |
| Sensor S4 (A3)    | A3           |
| Sensor S5 (A4)    | A4           |
| Sensor S6 (A5)    | A5           |
| Trigger (HC-SR04) | 12           |
| Echo (HC-SR04)    | 13           |
| Servo sinal       | 8            |

> ⚠️ **Atenção:** O motor esquerdo está montado invertido mecanicamente. O código já compensa essa inversão na função `setMotorSpeed()`.

## 📦 Instalação do Software

1. Instale a [IDE Arduino](https://www.arduino.cc/en/software) (versão 1.8.19 ou superior).
2. Instale a biblioteca `Servo` (já inclusa na IDE).
3. Clone este repositório:
   ```bash
   git clone https://github.com/seu-usuario/robo-seguidor-coletor.git
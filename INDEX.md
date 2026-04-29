# 📚 Índice da Documentação – Robô Seguidor Coletor

Bem‑vindo à documentação do projeto. Utilize os links abaixo para navegar:

## 🧭 Guias Principais

- [README.md](./README.md) – Visão geral, requisitos de hardware, instalação e testes rápidos.
- [CONFIGURACAO.md](./CONFIGURACAO.md) – Descrição detalhada de todas as constantes em `config.h`.

## 📁 Estrutura do Código (Arquivos .cpp/.h)

| Arquivo                 | Descrição                                                                 |
|-------------------------|---------------------------------------------------------------------------|
| `main.cpp`              | Menu serial com testes individuais de motores, sensores, servo, PD etc.   |
| `MotorController.cpp/h` | Controle dos motores DC, incluindo `followLine()` com PD e `curveCompensated()`. |
| `LineSensor.cpp/h`      | Leitura dos 6 sensores, cálculo de posição ponderada (-1..+1) e classificação de padrão. |
| `UltrasonicSensor.cpp/h`| Medição do HC‑SR04 com validação por tolerância e filtro de estabilidade. |
| `GripperServo.cpp/h`    | Movimentação suave do servo (step‑by‑step) e desativação após estabilização. |
| `config.h`              | Todas as definições de pinos, constantes de tempo, ganhos e limiares.     |

## 🧪 Testes Disponíveis (Menu Serial)

| Opção | Nome do Teste           | Descrição resumida                           |
|-------|-------------------------|----------------------------------------------|
| 1     | Motores                 | Frente, ré, giros e curvas compensadas.     |
| 2     | Sensores de linha       | Exibe padrão binário, posição e tipo de curva. |
| 3     | Ultrassônico            | Mostra distância e fase de aproximação.      |
| 4     | Garra (bloqueante)      | Abre/fecha o servo com delays.               |
| 5     | Garra reativa           | Fecha a garra quando objeto está a ≤5 cm por 300 ms. |
| 6     | Calibrar threshold      | Calcula valor ideal para `THRESHOLD_LINE_SENSOR`. |
| 7     | Seguimento PD integrado | Robô segue linha usando o controlador PD.    |

## 🔗 Links Úteis

- [Simulador de Controlador PD (desmos)](https://www.desmos.com/calculator) – Ajuda a visualizar o efeito de KP e KD.
- [Datasheet do HC‑SR04](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)
- [Ponte H L298N](https://www.st.com/resource/en/datasheet/l298.pdf)

## 📌 Próximos Passos (Para Integração Completa)

- Criar um `loop()` que combine `LineSensor.readSensors()` e `MotorController.followLine()` com a lógica de coleta do ultrassônico.
- Implementar tratamento de cruzamentos (girar 90° quando detectar T ou X).
- Adicionar sistema de logging para cartão SD (opcional).

---

💡 **Sugestões ou problemas?** Abra uma *issue* no repositório do GitHub.
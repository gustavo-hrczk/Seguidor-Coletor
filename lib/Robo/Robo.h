#ifndef ROBO_H
#define ROBO_H

#include <Arduino.h>
#include "config.h"
#include "config_workshop.h"
#include "MotorController.h"
#include "LineSensor.h"
#include "UltrasonicSensor.h"
#include "GripperServo.h"

// ============================================================================
// Robo — camada de abstração didática
//
// Encapsula toda a biblioteca técnica (MotorController, LineSensor,
// UltrasonicSensor, GripperServo) e expõe uma API em português com
// nomes de ação diretos.
//
// O aluno nunca interage com a biblioteca técnica diretamente.
// Toda a complexidade (PID, validação de sensor, trim de motores,
// detach do servo) permanece invisível dentro desta classe.
//
// Uso mínimo:
//   #include "Robo.h"
//   Robo robo;
//   void setup() { robo.inicializar(); }
//   void loop()  { /* seu código aqui */ }
// ============================================================================

class Robo {
public:

    Robo();

    // ── Inicialização ────────────────────────────────────────────────────────
    // Configura todos os módulos internos e posiciona a garra aberta.
    // Deve ser a primeira chamada, sempre dentro de setup().
    void inicializar();

    // ── Motores ──────────────────────────────────────────────────────────────
    // Controla cada motor individualmente.
    // @param velocidade  -100 a +100, ou constante: PARADO, LENTO, NORMAL, RAPIDO
    //                    Positivo = frente | Negativo = ré | PARADO = desligado
    void motorEsquerdo(int velocidade);
    void motorDireito(int velocidade);

    // Para ambos os motores imediatamente.
    void pararMotores();

    // Segue a linha automaticamente usando o controlador PID interno.
    // @param velocidade  velocidade base de avanço — use NORMAL ou LENTO
    // O PID ajusta os dois motores para manter o robô centralizado na linha.
    // Chamar uma vez por ciclo do loop enquanto o robô deve seguir a linha.
    void seguirLinha(int velocidade = NORMAL);

    // Reseta o estado interno do PID.
    // Chamar sempre que o robô parar ou executar uma manobra,
    // antes de retomar o seguimento — evita correção brusca na retomada.
    void resetarSeguimento();

    // ── Sensor de linha ──────────────────────────────────────────────────────
    // Retorna a posição da linha em relação ao centro do robô.
    // @return  inteiro de -100 a +100
    //          -100 = linha na extrema esquerda
    //             0 = linha centralizada
    //          +100 = linha na extrema direita
    // Compare com: DESVIO_ESQ_FORTE, DESVIO_ESQ_LEVE, DESVIO_DIR_LEVE, DESVIO_DIR_FORTE
    int lerLinha();

    // Retorna true se ao menos um sensor detecta a linha.
    bool temLinha();

    // ── Sensor ultrassônico ──────────────────────────────────────────────────
    // Retorna a distância em centímetros até o objeto mais próximo.
    // @return  distância em cm, ou -1 se sem objeto / leitura instável
    // Compare com: ZONA_LONGE, ZONA_MEDIO, ZONA_PERTO, ZONA_CONTATO
    int lerDistancia();

    // Retorna true quando o sensor confirmou leituras consecutivas estáveis.
    // Use junto com lerDistancia() para evitar falsos positivos.
    bool leituraEstavelSensor();

    // ── Garra ────────────────────────────────────────────────────────────────
    // Move a garra para a posição fechada. Ignorado se já estiver fechada.
    void fecharGarra();

    // Move a garra para a posição aberta. Ignorado se já estiver aberta.
    void abrirGarra();

    // Retorna true se a garra está fechada.
    bool garrataFechada();

private:
    MotorController  _motor;
    LineSensor       _sensor;
    UltrasonicSensor _ultrasonic;
    GripperServo     _gripper;

    // Converte velocidade de -100..+100 para PWM respeitando BASE_SPEED e deadzone
    int _converterVelocidade(int velocidade) const;
};

#endif

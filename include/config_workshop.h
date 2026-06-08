#ifndef CONFIG_WORKSHOP_H
#define CONFIG_WORKSHOP_H

// ============================================================================
// config_workshop.h — Configurações do Workshop
//
// Este é o ÚNICO arquivo que você precisa editar durante a aula.
// Não mexa no config.h — ele controla o hardware interno do robô.
//
// Como usar:
//   Chame as constantes diretamente nas funções da classe Robo.
//   Exemplo: robo.motorEsquerdo(NORMAL);
//            if (robo.lerLinha() < DESVIO_ESQ_LEVE) { ... }
// ============================================================================


// ============================================================================
// VELOCIDADES DOS MOTORES
//
// Use em motorEsquerdo() e motorDireito().
// Valores positivos = frente | Negativos = ré | PARADO = para o motor.
// Você também pode usar valores numéricos diretos de -100 a +100.
// ============================================================================

#define PARADO   0    // motor desligado
#define LENTO   180    // mínimo para mover com carga
#define NORMAL  200    // velocidade de trabalho
#define RAPIDO  220    // retas longas, sem objeto próximo


// ============================================================================
// LIMIARES DO SENSOR DE LINHA
//
// Compare com o retorno de lerLinha(), que vai de -100 a +100.
//   Valor negativo = linha desviou para a ESQUERDA do centro
//   Valor zero     = linha CENTRALIZADA
//   Valor positivo = linha desviou para a DIREITA do centro
// ============================================================================

#define DESVIO_ESQ_FORTE  -60   // curva fechada para esquerda
#define DESVIO_ESQ_LEVE   -20   // desvio suave para esquerda
#define LINHA_CENTRO        0   // linha centralizada
#define DESVIO_DIR_LEVE    20   // desvio suave para direita
#define DESVIO_DIR_FORTE   60   // curva fechada para direita


// ============================================================================
// ZONAS DE DISTÂNCIA
//
// Compare com o retorno de lerDistancia(), que retorna centímetros.
// Retorna -1 se não há objeto no alcance ou leitura ainda instável.
// ============================================================================

#define ZONA_LONGE    30   // cm — objeto detectado, longe
#define ZONA_MEDIO    15   // cm — objeto se aproximando
#define ZONA_PERTO    10   // cm — reduzir velocidade
#define ZONA_CONTATO   5   // cm — parar e acionar garra


// ============================================================================
// DELAYS DE MANOBRA
//
// Use com delay() nas etapas da sequência de coleta.
// Calibre estes valores na pista antes do desafio final.
// ============================================================================

#define TEMPO_GIRO_90      700   // ms para girar 90 graus
#define TEMPO_AVANCO       400   // ms de avanço lateral
#define TEMPO_ESTABILIZAR  300   // ms de pausa entre movimentos


#endif // CONFIG_WORKSHOP_H

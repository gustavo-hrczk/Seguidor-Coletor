// ============================================================================
// main_robo.cpp — Seguidor de Linha + Coletor usando a classe Robo
//
// Equivalente funcional do main.cpp técnico, reescrito inteiramente
// com a API de abstração. Mesma máquina de quatro estados, mesma
// lógica de decisão, mesma sequência de coleta.
//
// Tudo que era complexidade interna (PID, validação de sensor, trim,
// detach do servo) está encapsulado dentro da classe Robo — invisível.
//
// Estados:
//   SEGUINDO   → segue a linha, detecta objeto
//   COLETANDO  → manobra completa (bloqueante)
//   RECUPERANDO → busca linha perdida
//   PARADO     → aguarda intervenção manual
// ============================================================================

#include "Robo.h"

// ── Instância única ───────────────────────────────────────────────────────────
Robo robo;

// ── Estados ───────────────────────────────────────────────────────────────────
// Prefixo EST_ evita colisao com #define PARADO=0 do config_workshop.h
enum Estado { EST_SEGUINDO, EST_COLETANDO, EST_RECUPERANDO, EST_PARADO };
Estado estadoAtual = EST_SEGUINDO;

// ── Variáveis de controle ─────────────────────────────────────────────────────
unsigned long inicioRecuperacao = 0;
bool          ladoEsquerdo      = true;   // alterna a cada coleta
int           totalColetas      = 0;

// ── Protótipos ────────────────────────────────────────────────────────────────
void seguirLinha();
void realizarColeta();
void recuperarLinha();

// ============================================================================
// setup()
// ============================================================================
void setup() {
    Serial.begin(BAUD_RATE);
    robo.inicializar();
    delay(3000);
    Serial.println(F("[Robo] INICIADO"));
}

// ============================================================================
// loop()
// ============================================================================
void loop() {
    switch (estadoAtual) {
        case EST_SEGUINDO:    seguirLinha();    break;
        case EST_COLETANDO:   realizarColeta(); break;
        case EST_RECUPERANDO: recuperarLinha(); break;
        case EST_PARADO:      robo.pararMotores(); break;
    }
}

// ============================================================================
// seguirLinha()
//
// A cada ciclo, o robô:
//   1. Lê a distância — objeto na zona de contato → COLETANDO
//   2. Lê a linha — linha perdida → RECUPERANDO
//   3. Linha centralizada ou em curva → corrige com motorEsquerdo/motorDireito
//
// Velocidade reduzida conforme o objeto se aproxima (ZONA_MEDIO, ZONA_PERTO).
// ============================================================================
void seguirLinha() {
    int dist  = robo.lerDistancia();
    int linha = robo.lerLinha();

    // 1. Objeto confirmado na zona de contato → inicia coleta
    if (robo.leituraEstavelSensor() && dist > 0 && dist <= ZONA_CONTATO) {
        robo.pararMotores();
        Serial.println(F("[Robo] Objeto detectado -> COLETANDO"));
        estadoAtual = EST_COLETANDO;
        return;
    }

    // 2. Linha perdida → tenta recuperar
    if (!robo.temLinha()) {
        robo.pararMotores();
        inicioRecuperacao = millis();
        Serial.println(F("[Robo] Linha perdida -> RECUPERANDO"));
        estadoAtual = EST_RECUPERANDO;
        return;
    }

    // 3. Define velocidade base conforme distância do objeto
    int vel;
    if      (dist > 0 && dist <= ZONA_PERTO)  vel = LENTO;
    else if (dist > 0 && dist <= ZONA_MEDIO)  vel = NORMAL - 15;
    else                                       vel = NORMAL;

    // 4. Corrige direção com base na posição da linha
    if (linha < DESVIO_ESQ_FORTE) {
        // Curva fechada para esquerda — motor direito pleno, esquerdo para trás
        robo.motorEsquerdo(-LENTO);
        robo.motorDireito(vel);

    } else if (linha < DESVIO_ESQ_LEVE) {
        // Desvio suave para esquerda — reduz motor esquerdo
        robo.motorEsquerdo(vel - 25);
        robo.motorDireito(vel);

    } else if (linha > DESVIO_DIR_FORTE) {
        // Curva fechada para direita — motor esquerdo pleno, direito para trás
        robo.motorEsquerdo(vel);
        robo.motorDireito(-LENTO);

    } else if (linha > DESVIO_DIR_LEVE) {
        // Desvio suave para direita — reduz motor direito
        robo.motorEsquerdo(vel);
        robo.motorDireito(vel - 25);

    } else {
        // Centralizado — ambos os motores na mesma velocidade
        robo.motorEsquerdo(vel);
        robo.motorDireito(vel);
    }
}

// ============================================================================
// realizarColeta()
//
// Sequência bloqueante — sensores ignorados durante a manobra.
// Direção alterna a cada ciclo (esquerda → direita → esquerda…).
//
// Fluxo:
//   fecha garra → gira 90° → avança → abre garra → recua → gira de volta
// ============================================================================
void realizarColeta() {
    totalColetas++;
    Serial.print(F("[Robo] Coleta #")); Serial.print(totalColetas);
    Serial.println(ladoEsquerdo ? F(" — ESQUERDA") : F(" — DIREITA"));

    // Segura o objeto
    robo.fecharGarra();
    delay(TEMPO_ESTABILIZAR);

    if (ladoEsquerdo) {

        // COM CARGA — gira esquerda, avança, solta
        robo.motorEsquerdo( NORMAL);
        robo.motorDireito( -NORMAL);
        delay(TEMPO_GIRO_90);
        robo.pararMotores(); delay(50);

        robo.motorEsquerdo(NORMAL);
        robo.motorDireito(NORMAL);
        delay(TEMPO_AVANCO);
        robo.pararMotores(); delay(100);

        robo.abrirGarra(); delay(200);

        // SEM CARGA — recua e gira de volta
        robo.motorEsquerdo(-NORMAL);
        robo.motorDireito(-NORMAL);
        delay(TEMPO_AVANCO);
        robo.pararMotores(); delay(50);

        robo.motorEsquerdo(-NORMAL);
        robo.motorDireito( NORMAL);
        delay(TEMPO_GIRO_90);
        robo.pararMotores();

    } else {

        // COM CARGA — gira direita, avança, solta
        robo.motorEsquerdo(-NORMAL);
        robo.motorDireito( NORMAL);
        delay(TEMPO_GIRO_90);
        robo.pararMotores(); delay(50);

        robo.motorEsquerdo(NORMAL);
        robo.motorDireito(NORMAL);
        delay(TEMPO_AVANCO);
        robo.pararMotores(); delay(100);

        robo.abrirGarra(); delay(200);

        // SEM CARGA — recua e gira de volta
        robo.motorEsquerdo(-NORMAL);
        robo.motorDireito(-NORMAL);
        delay(TEMPO_AVANCO);
        robo.pararMotores(); delay(50);

        robo.motorEsquerdo( NORMAL);
        robo.motorDireito(-NORMAL);
        delay(TEMPO_GIRO_90);
        robo.pararMotores();
    }

    // Prepara o próximo ciclo
    ladoEsquerdo = !ladoEsquerdo;
    delay(300);

    Serial.println(F("[Robo] Coleta concluida -> SEGUINDO"));
    estadoAtual = EST_SEGUINDO;
}

// ============================================================================
// recuperarLinha()
//
// Estágio 1 (0 → RECOVERY_SPIN_MS):
//   Gira devagar para o lado em que a linha foi vista pela última vez.
//
// Estágio 2 (→ RECOVERY_TIMEOUT_MS):
//   Gira na direção oposta — tenta o outro lado.
//
// A cada iteração: verifica se a linha voltou. Se sim, retoma SEGUINDO.
// Após timeout: PARADO.
// ============================================================================
void recuperarLinha() {
    unsigned long tempo = millis() - inicioRecuperacao;

    // Linha reencontrada — retoma seguimento
    if (robo.temLinha()) {
        robo.pararMotores();
        Serial.println(F("[Robo] Linha encontrada -> SEGUINDO"));
        estadoAtual = EST_SEGUINDO;
        return;
    }

    // Estágio 1: gira na direção onde a linha estava
    if (tempo < RECOVERY_SPIN_MS) {
        robo.motorEsquerdo( LENTO);
        robo.motorDireito( -LENTO);
        return;
    }

    // Estágio 2: tenta a direção oposta
    if (tempo < RECOVERY_TIMEOUT_MS) {
        robo.motorEsquerdo(-LENTO);
        robo.motorDireito( LENTO);
        return;
    }

    // Timeout — para e aguarda intervenção
    robo.pararMotores();
    Serial.println(F("[Robo] TIMEOUT — reposicione e reinicie"));
    estadoAtual = EST_PARADO;
}

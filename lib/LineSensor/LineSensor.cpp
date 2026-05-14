#include "LineSensor.h"

// Pesos simétricos por posição física do sensor no array QTR-6.
// Sensor mais à esquerda tem peso mais negativo, mais à direita mais positivo.
// Resultado: posição = 0 quando S3 e S4 leem a linha com mesma intensidade.
const int LineSensor::WEIGHTS[6] = { -5, -3, -1, 1, 3, 5 };

// ============================================================================
// Construtor
// Inicializa direção central e zera toda a estrutura de estado.
// memset garante que raw[], active[] e contadores começam em zero.
// ============================================================================
LineSensor::LineSensor() : _lastDir(DIR_CENTER) {
    memset(&_state, 0, sizeof(SensorState));
}

// ============================================================================
// initialize()
// Define A0–A5 como INPUT. Embora seja o padrão do Arduino,
// o pinMode explícito documenta a intenção e evita surpresas
// caso outro módulo tenha reconfigurado esses pinos.
// ============================================================================
void LineSensor::initialize() {
    for (int i = 0; i < 6; i++) {
        pinMode(A0 + i, INPUT);
    }
}

// ============================================================================
// readSensors()
// Leitura direta de todos os 6 sensores em sequência.
// Atualiza estado interno e retorna referência constante (sem cópia).
//
// Por que sem filtro?
//   O seguidor de linha precisa reagir imediatamente a cada variação.
//   Um filtro de 3 amostras introduziria ~30ms de atraso (3 × 10ms),
//   suficiente para o robô sair da linha em velocidade média.
// ============================================================================
const LineSensor::SensorState& LineSensor::readSensors() {
    _state.activeCount = 0;

    for (int i = 0; i < 6; i++) {
        _state.raw[i] = analogRead(A0 + i);

        // Linha branca reflete mais luz → menor tensão no fotodetector → raw baixo
        _state.active[i] = (_state.raw[i] <= THRESHOLD_LINE_SENSOR);

        if (_state.active[i]) _state.activeCount++;
    }

    _state.position = computePosition();

    // Atualiza direção apenas quando há sensores ativos (linha visível)
    // Mantém a última direção conhecida quando a linha desaparece,
    // permitindo que o controlador PD saiba para qual lado buscar
    if (_state.activeCount > 0) {
        if      (_state.position < -0.10f) _lastDir = DIR_LEFT;
        else if (_state.position >  0.10f) _lastDir = DIR_RIGHT;
        else                               _lastDir  = DIR_CENTER;
    }

    return _state;
}

// ============================================================================
// computePosition()
// Calcula onde a linha está em relação ao centro do array de sensores.
//
// Algoritmo: centro de massa ponderado por intensidade
//   intensity = THRESHOLD - raw
//     → sensor no centro da linha: raw mínimo → intensidade máxima → mais peso
//     → sensor na borda da linha:  raw alto   → intensidade mínima → menos peso
//
// Resultado: weightedSum / totalWeight produz valor em escala dos pesos (-5..+5)
//            dividir por 5 normaliza para -1.0..+1.0
//
// Caso sem sensores ativos (linha perdida):
//   Retorna ±1.0 na última direção conhecida — mantém o PD corrigindo
//   na direção correta enquanto o robô executa a manobra de recuperação.
// ============================================================================
float LineSensor::computePosition() const {
    if (_state.activeCount == 0) {
        if (_lastDir == DIR_RIGHT) return  1.0f;
        if (_lastDir == DIR_LEFT)  return -1.0f;
        return 0.0f;
    }

    long weightedSum = 0;
    int  totalWeight = 0;

    for (int i = 0; i < 6; i++) {
        if (_state.active[i]) {
            int intensity = THRESHOLD_LINE_SENSOR - _state.raw[i];
            if (intensity < 1) intensity = 1;   // mínimo 1 — evita peso zero na borda
            weightedSum += (long)WEIGHTS[i] * intensity;
            totalWeight += intensity;
        }
    }

    if (totalWeight == 0) return 0.0f;

    float pos = (float)weightedSum / (float)totalWeight / 5.0f;

    // Clamp de segurança numérica — garante saída dentro do intervalo esperado
    if (pos < -1.0f) pos = -1.0f;
    if (pos >  1.0f) pos =  1.0f;

    return pos;
}

// ============================================================================
// getLinePattern()
// Classifica o padrão de navegação com base em dois critérios:
//   1. Contagem de sensores ativos  → detecta cruzamentos
//   2. Magnitude da posição         → classifica tipo de curva
//
// Cruzamentos têm prioridade pois indicam decisão de rota,
// não apenas intensidade de curvatura.
// ============================================================================
LineSensor::LinePattern LineSensor::getLinePattern() const {
    if (_state.activeCount == 0)
        return LINE_LOST;

    // 5 ou 6 sensores: linha muito larga ou cruzamento em X
    if (_state.activeCount >= CROSS_MIN_SENSORS_X)
        return INTERSECTION;

    // Exatamente 4 sensores: cruzamento em T — direção pela posição dominante
    if (_state.activeCount == CROSS_MIN_SENSORS_T)
        return (_state.position < 0.0f) ? TURN_LEFT_90 : TURN_RIGHT_90;

    // Seguimento normal: 1–3 sensores, classifica pela magnitude do desvio
    float absPos = (_state.position < 0) ? -_state.position : _state.position;

    if      (absPos < 0.20f) return STRAIGHT;
    else if (absPos < 0.45f) return CURVE_LIGHT;
    else if (absPos < 0.70f) return CURVE_MEDIUM;
    else                     return CURVE_SHARP;
}

// ============================================================================
// printSensorValues()
// Saída de diagnóstico no Serial. Produz três elementos em uma linha:
//   1. Padrão binário dos sensores ativos (ex: 001100)
//   2. Barra visual da posição           (ex: [----------#----------])
//   3. Valores numéricos: posição, contagem e raw de cada sensor
//
// O '#' na barra representa a posição atual; '|' no centro representa 0.
// Essencial para calibração do THRESHOLD_LINE_SENSOR no Teste 6.
// ============================================================================
void LineSensor::printSensorValues() const {
    if (!DEBUG_MODE) return;

    Serial.print(F("[Line] "));
    for (int i = 0; i < 6; i++) {
        Serial.print(_state.active[i] ? '1' : '0');
    }

    // Constrói barra visual: mapeia -1.0..+1.0 para índice 0..20
    char bar[22];
    int  idx = (int)((_state.position + 1.0f) * 10.0f);
    if (idx < 0)  idx = 0;
    if (idx > 20) idx = 20;
    for (int i = 0; i < 21; i++) bar[i] = (i == 10) ? '|' : '-';
    bar[idx] = '#';
    bar[21]  = '\0';

    Serial.print(F(" ["));  Serial.print(bar);  Serial.print(F("] "));
    Serial.print(_state.position, 3);
    Serial.print(F(" cnt=")); Serial.print(_state.activeCount);

    // Valores brutos — permitem identificar sensor com defeito ou threshold incorreto
    Serial.print(F(" raw:"));
    for (int i = 0; i < 6; i++) {
        Serial.print(F(" ")); Serial.print(_state.raw[i]);
    }
    Serial.println();
}

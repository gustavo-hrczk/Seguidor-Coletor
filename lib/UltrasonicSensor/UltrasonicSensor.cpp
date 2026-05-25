#include "UltrasonicSensor.h"

// ============================================================================
// Construtor
// Inicializa todos os valores como inválidos (-1) para forçar
// que a primeira leitura real passe pelo processo de validação completo.
// ============================================================================
UltrasonicSensor::UltrasonicSensor()
    : currentDistance(-1), lastValidDistance(-1), previousDistance(-1),
      validationCounter(0), recentChange(false), readingStable(false),
      lastChangeTime(0) {}

// ============================================================================
// initialize()
// Configura pinos e garante TRIGGER em LOW como estado de repouso.
// Um TRIGGER em HIGH antes da medição causaria disparo indesejado.
// ============================================================================
void UltrasonicSensor::initialize() {
    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    digitalWrite(PIN_TRIGGER, LOW);   // estado de repouso — evita disparo acidental
}

// ============================================================================
// readDistance()
// Interface principal — realiza medição e validação em uma chamada.
// Sempre retorna lastValidDistance (última distância confirmada),
// não a medição bruta, para proteger o sistema de leituras espúrias.
// ============================================================================
int UltrasonicSensor::readDistance() {
    currentDistance = measureDistance();
    validateReading();
    return lastValidDistance;   // nunca retorna medição bruta diretamente
}

// ============================================================================
// measureDistance()
// Protocolo de disparo do HC-SR04:
//   1. Garante TRIGGER em LOW por 2µs (limpa pulso anterior)
//   2. Pulso HIGH de 10µs no TRIGGER — dispara o burst ultrassônico
//   3. pulseIn() mede o tempo até o ECHO retornar (timeout de 30ms = ~5m)
//   4. Converte duração para cm: distância = (tempo × velocidade_som) / 2
//      velocidade do som ≈ 343 m/s = 0.0343 cm/µs
//      divide por 2 pois o som percorre ida E volta
//
// Retorna -1 para leituras fora do alcance útil (< 2 cm ou > 400 cm)
// ou quando não há retorno de eco (objeto ausente ou muito distante).
// ============================================================================
int UltrasonicSensor::measureDistance() {
    digitalWrite(PIN_TRIGGER, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIGGER, LOW);

    // Timeout configurável via config.h — reduzir melhora responsividade
    // a custo de não detectar objetos muito distantes (acima de ~250cm a 15ms)
    long duration = pulseIn(PIN_ECHO, HIGH, ULTRASONIC_TIMEOUT_US);

    if (duration == 0) return -1;

    int distance = (int)(duration * 0.0343f / 2.0f);
    if (distance < 2 || distance > 400) return -1;

    return distance;
}

// ============================================================================
// validateReading()
// Implementa janela deslizante de validação:
//
//   Leitura inválida (-1):
//     Reduz contador gradualmente — degradação suave da confiança.
//     Só marca readingStable = false quando contador chega a zero,
//     evitando que um único eco perdido invalide uma detecção estável.
//
//   Primeira leitura válida:
//     Inicializa referência sem marcar como estável ainda.
//     É necessário acumular SENSOR_FILTER_CYCLES antes de confiar.
//
//   Leitura dentro da tolerância da referência:
//     Incrementa contador até SENSOR_FILTER_CYCLES.
//     Quando atingido: atualiza lastValidDistance e marca readingStable.
//
//   Leitura fora da tolerância:
//     Reduz contador. Se zerar: atualiza referência para nova distância
//     e reinicia o processo — permite rastrear objeto em movimento.
// ============================================================================
bool UltrasonicSensor::validateReading() {
    recentChange = false;

    // Leitura inválida: degrada confiança gradualmente
    if (currentDistance < 0) {
        if (validationCounter > 0) validationCounter--;
        if (validationCounter == 0) readingStable = false;
        return false;
    }

    // Primeira leitura válida: inicializa referência
    if (lastValidDistance < 0) {
        lastValidDistance = currentDistance;
        previousDistance  = currentDistance;
        validationCounter = 1;
        readingStable     = false;
        return false;   // ainda não é estável — precisa de mais leituras
    }

    if (isWithinTolerance(currentDistance, lastValidDistance)) {
        if (validationCounter < SENSOR_FILTER_CYCLES) {
            validationCounter++;
        }

        if (validationCounter >= SENSOR_FILTER_CYCLES) {
            // Detecta mudança real de valor (> 1 cm) ignorando ruído de ±1 cm
            if (abs(currentDistance - lastValidDistance) > 1) {
                recentChange   = true;
                lastChangeTime = millis();
            }
            lastValidDistance = currentDistance;
            readingStable     = true;
            return true;
        }

    } else {
        // Leitura divergiu — reduz confiança sem zerar abruptamente
        if (validationCounter > 0) validationCounter--;

        if (validationCounter == 0) {
            // Confiança esgotada: adota nova distância como referência
            readingStable     = false;
            recentChange      = true;
            lastChangeTime    = millis();
            lastValidDistance = currentDistance;   // nova referência
            validationCounter = 1;
        }
    }

    previousDistance = currentDistance;
    return false;
}

// ============================================================================
// isWithinTolerance()
// Tolerância adaptativa para acomodar o comportamento real do HC-SR04:
//
//   Distâncias curtas (≤ 20 cm):
//     Tolerância fixa de ±2 cm — o ruído absoluto do sensor nessa faixa
//     é constante, independente da distância.
//
//   Distâncias longas (> 20 cm):
//     Tolerância percentual (ULTRASONIC_NOISE_TOLERANCE %), limitada a ±4 cm.
//     O ruído cresce com a distância, mas o teto evita janelas muito largas
//     que aceitariam objetos diferentes como sendo o mesmo.
// ============================================================================
bool UltrasonicSensor::isWithinTolerance(int dist1, int dist2) const {
    if (dist1 < 0 || dist2 < 0) return false;

    int diff = abs(dist1 - dist2);

    if (dist2 <= 20) return diff <= 2;   // tolerância fixa para curta distância

    // Tolerância percentual com teto para evitar janela excessivamente larga
    int percentTol = (int)((float)dist2 * (ULTRASONIC_NOISE_TOLERANCE / 100.0f));
    int tol        = min(percentTol, 4);

    return diff <= tol;
}

// ============================================================================
// getApproachPhase()
// Classifica a distância válida em fases de aproximação.
// Usado pela máquina de estados principal para ajustar velocidade e
// decidir quando acionar a sequência de coleta.
// ============================================================================
UltrasonicSensor::ApproachPhase UltrasonicSensor::getApproachPhase() const {
    if (lastValidDistance < 0 || !isObjectDetected())
        return OBJECT_NOT_DETECTED;

    if      (lastValidDistance >= ULTRASONIC_DISTANCE_LONG)  return PHASE_1_DISTANT;
    else if (lastValidDistance >= ULTRASONIC_DISTANCE_SHORT) return PHASE_2_APPROACHING;
    else                                                      return PHASE_3_CONTACT;
}

// ============================================================================
// isObjectDetected()
// Combina três condições para garantir detecção confiável:
//   1. Distância válida (> 0)
//   2. Dentro do alcance útil (< 300 cm)
//   3. Leitura estável (passou pela validação)
// ============================================================================
bool UltrasonicSensor::isObjectDetected() const {
    return (lastValidDistance > 0 && lastValidDistance < 300 && readingStable);
}

// ============================================================================
// resetValidation()
// Limpa completamente o estado de validação.
// Chamar após coleta concluída — garante que a próxima detecção
// começa do zero sem herdar estado da detecção anterior.
// ============================================================================
void UltrasonicSensor::resetValidation() {
    validationCounter = 0;
    lastValidDistance = -1;
    previousDistance  = -1;
    recentChange      = false;
    readingStable     = false;
}

// ============================================================================
// printDistance()
// Saída de diagnóstico: distância atual (bruta), distância validada,
// flag de estabilidade e fase de aproximação.
// ============================================================================
void UltrasonicSensor::printDistance() const {
    if (!DEBUG_MODE) return;
    Serial.print(F("[Ultrasonic] Atual: "));
    Serial.print(currentDistance);
    Serial.print(F(" cm | Validado: "));
    Serial.print(lastValidDistance);
    Serial.print(F(" cm | Estavel: "));
    Serial.print(readingStable ? F("SIM") : F("NAO"));
    Serial.print(F(" | Fase: "));
    Serial.println(getApproachPhase());
}

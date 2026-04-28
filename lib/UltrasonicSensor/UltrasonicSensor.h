#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// CLASSE: UltrasonicSensor
// Responsável pela leitura do sensor ultrassônico HC-SR04 com validação
// de debounce e classificação em 3 fases de aproximação.
// ============================================================================

class UltrasonicSensor {
public:
    // Fases de distância para coleta
    enum ApproachPhase {
        OBJECT_NOT_DETECTED = 0,  // Sem objeto
        PHASE_1_DISTANT = 1,      // Distância longa (desacelerar)
        PHASE_2_APPROACHING = 2,  // Distância curta (velocidade lenta)
        PHASE_3_CONTACT = 3       // Distância de contato (parar e coletar)
    };

    // ===== CONSTRUTOR E INICIALIZAÇÃO =====
    UltrasonicSensor();
    
    /**
     * Inicializa pinos do sensor ultrassônico
     */
    void initialize();

    // ===== LEITURA E PROCESSAMENTO =====
    
    /**
     * Lê distância do sensor com debounce de 3 medições
     * @return Distância em cm, ou -1 se inválido
     */
    int readDistance();

    /**
     * Obtém fase de aproximação baseada na distância validada
     * @return ApproachPhase identificada
     */
    ApproachPhase getApproachPhase() const;

    /**
     * Verifica se objeto foi detectado e validado
     * @return true se passou no filtro de debounce
     */
    bool isObjectDetected() const;

    // ===== VALIDADORES =====
    
    /**
     * Verifica se a leitura passou no debounce de 5%
     * Requer 3 leituras consecutivas dentro da tolerância
     */
    bool validateReading();

    /**
     * Reseta o contador de validação (útil após mudança de estado)
     */
    void resetValidation();

    // ===== GETTERS =====
    int getLastValidDistance() const { return lastValidDistance; }
    int getCurrentDistance() const { return currentDistance; }
    uint8_t getValidationCounter() const { return validationCounter; }
    bool hasRecentChange() const { return recentChange; }

    // ===== DEBUG =====
    void printDistance() const;

private:
    // Estado interno
    int currentDistance;
    int lastValidDistance;
    int previousDistance;
    uint8_t validationCounter;
    bool recentChange;
    unsigned long lastChangeTime;

    // Métodos privados
    int measureDistance();
    bool isWithinTolerance(int dist1, int dist2) const;
    bool checkOutlier(int current, int previous) const;
};

#endif // ULTRASONIC_SENSOR_H

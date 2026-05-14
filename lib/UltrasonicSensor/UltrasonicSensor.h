#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// UltrasonicSensor — leitura e validação do sensor HC-SR04
//
// Responsabilidade: medir distância e garantir que apenas leituras estáveis
// (sem ruído ou reflexos espúrios) sejam reportadas ao sistema.
//
// Por que validar?
//   O HC-SR04 pode retornar valores espúrios por reflexão em ângulos
//   oblíquos, interferência ultrassônica entre sensores ou variação de
//   temperatura. Uma leitura isolada não é confiável para acionar a garra.
//
// Algoritmo de validação (janela deslizante):
//   - Aceita leitura se estiver dentro da tolerância da última válida
//   - Incrementa contador a cada leitura consistente
//   - Marca readingStable = true após SENSOR_FILTER_CYCLES consecutivas
//   - Reduz contador gradualmente em leituras inconsistentes (não zera)
//     → evita flicker quando o objeto se move levemente na borda da tolerância
//
// Fases de aproximação (usadas pela máquina de estados para controle de velocidade):
//   PHASE_1_DISTANT     : >= ULTRASONIC_DISTANCE_LONG  cm → desacelerar
//   PHASE_2_APPROACHING : >= ULTRASONIC_DISTANCE_SHORT cm → velocidade lenta
//   PHASE_3_CONTACT     : <  ULTRASONIC_DISTANCE_SHORT cm → parar e coletar
// ============================================================================

class UltrasonicSensor {
public:

    // Fases de aproximação — usadas em handleFollowing() para controle de velocidade
    enum ApproachPhase {
        OBJECT_NOT_DETECTED  = 0,
        PHASE_1_DISTANT      = 1,
        PHASE_2_APPROACHING  = 2,
        PHASE_3_CONTACT      = 3
    };

    // Construtor — inicializa todos os valores como inválidos (-1)
    UltrasonicSensor();

    // Configura pinos TRIGGER (OUTPUT) e ECHO (INPUT)
    void initialize();

    // Realiza medição física + validação em uma chamada.
    // Retorna lastValidDistance — use este valor para decisões.
    // Retorna -1 se ainda não há leitura estável.
    int readDistance();

    // Retorna a fase de aproximação baseada na última distância válida
    ApproachPhase getApproachPhase() const;

    // true se houver leitura estável dentro do alcance útil (2..300 cm)
    bool isObjectDetected() const;

    // Executa apenas a etapa de validação (sem nova medição).
    // Raramente necessário — prefira readDistance() que faz ambos.
    bool validateReading();

    // Reseta todo o estado de validação.
    // Usar após coleta concluída para garantir leitura limpa no próximo ciclo.
    void resetValidation();

    // Getters — acesso ao estado interno sem disparar nova leitura
    int     getLastValidDistance() const { return lastValidDistance;  }
    int     getCurrentDistance()   const { return currentDistance;    }
    uint8_t getValidationCounter() const { return validationCounter;  }
    bool    hasRecentChange()      const { return recentChange;       }
    bool    isReadingStable()      const { return readingStable;      }

    // Imprime estado atual no Serial (apenas se DEBUG_MODE = true)
    void printDistance() const;

private:
    int           currentDistance;    // Última medição bruta (pode ser espúria)
    int           lastValidDistance;  // Última distância confirmada pela validação
    int           previousDistance;   // Medição anterior (usado na tolerância)
    uint8_t       validationCounter;  // Contador de leituras consecutivas consistentes
    bool          recentChange;       // true se distância mudou na última validação
    bool          readingStable;      // true quando SENSOR_FILTER_CYCLES foi atingido
    unsigned long lastChangeTime;     // Timestamp da última mudança válida (ms)

    // Dispara pulso no TRIGGER e mede tempo de retorno no ECHO.
    // Retorna distância em cm, ou -1 se fora do alcance ou sem retorno.
    int measureDistance();

    // Tolerância adaptativa:
    //   dist <= 20 cm: tolerância fixa de ±2 cm (ruído absoluto do HC-SR04)
    //   dist >  20 cm: tolerância percentual limitada a ±4 cm
    bool isWithinTolerance(int dist1, int dist2) const;
};

#endif

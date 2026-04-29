#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>
#include "config.h"

class UltrasonicSensor {
public:
    enum ApproachPhase {
        OBJECT_NOT_DETECTED  = 0,
        PHASE_1_DISTANT      = 1,
        PHASE_2_APPROACHING  = 2,
        PHASE_3_CONTACT      = 3
    };

    UltrasonicSensor();
    void initialize();

    // Realiza medição + validação em uma chamada só (use no loop principal)
    int  readDistance();

    ApproachPhase getApproachPhase()  const;
    bool          isObjectDetected()  const;
    bool          validateReading();
    void          resetValidation();

    int      getLastValidDistance()   const { return lastValidDistance;  }
    int      getCurrentDistance()     const { return currentDistance;    }
    uint8_t  getValidationCounter()   const { return validationCounter;  }
    bool     hasRecentChange()        const { return recentChange;       }
    bool     isReadingStable()        const { return readingStable;      }  // novo

    void printDistance() const;

private:
    int           currentDistance;
    int           lastValidDistance;
    int           previousDistance;
    uint8_t       validationCounter;
    bool          recentChange;
    bool          readingStable;      // true enquanto leituras forem consistentes
    unsigned long lastChangeTime;

    int  measureDistance();
    bool isWithinTolerance(int dist1, int dist2) const;
};

#endif
#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

#include <Arduino.h>
#include "config.h"

class LineSensor {
public:
    struct SensorState {
        bool    sensors[6];
        uint8_t rawPattern;
        bool    isValid;
    };

    enum LinePattern {
        UNKNOWN      = 0,
        STRAIGHT     = 1,
        CURVE_LIGHT  = 2,
        CURVE_MEDIUM = 3,
        CURVE_SHARP  = 4,
        INTERSECTION = 5,
        LINE_LOST    = 6
    };

    LineSensor();
    void initialize();

    SensorState readSensors();
    void        resetFilter();

    SensorState getLastValidState() const { return lastValidState; }
    LinePattern getLinePattern()    const { return identifyPattern(lastValidState); }
    bool        isLineDetected()    const { return lastValidState.rawPattern != 0; }
    uint8_t     getRawPattern()     const { return lastValidState.rawPattern; }
    int         getActiveSensor()   const;

    void printSensorValues() const;

private:
    SensorState lastValidState;
    SensorState rawReadings;
    SensorState previousReading;
    uint8_t     filterCounter;
    uint8_t     stableCounter;     // conta leituras consecutivas idênticas

    SensorState performRawRead();
    LinePattern identifyPattern(const SensorState& state) const;
    bool        validateDebounce(const SensorState& current,
                                 const SensorState& previous) const;
};

#endif
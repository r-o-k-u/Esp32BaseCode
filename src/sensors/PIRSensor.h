/**
 * @file PIRSensor.h
 * @brief Passive Infrared (PIR) motion sensor control
 * @author Your Name
 * @version 2.0
 */

#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include "../config.h"
#include <Arduino.h>

class PIRSensor
{
private:
    uint8_t pin;
    bool motionDetected;
    unsigned long lastMotionTime;
    unsigned long debounceTime;

public:
    PIRSensor(uint8_t sensorPin, unsigned long debounceMs = 1000);

    bool begin();
    bool readMotion();
    bool isMotionDetected();
    unsigned long getLastMotionTime();
    void resetMotion();

    // Static callback for interrupt handling
    static void IRAM_ATTR motionDetectedISR();

private:
    static PIRSensor *instance; // For interrupt handling
};

#endif // PIR_SENSOR_H

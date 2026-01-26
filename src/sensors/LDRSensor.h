/**
 * @file LDRSensor.h
 * @brief Light Dependent Resistor (LDR) light sensor control
 * @author Your Name
 * @version 2.0
 */

#ifndef LDR_SENSOR_H
#define LDR_SENSOR_H

#include "../config.h"
#include <Arduino.h>

class LDRSensor
{
private:
    uint8_t pin;
    int rawValue;
    float voltage;
    float lux;
    int samples;
    int sampleIndex;
    int *sampleBuffer;

public:
    LDRSensor(uint8_t sensorPin, int sampleCount = LDR_SAMPLES);
    ~LDRSensor();

    bool begin();
    bool readLight();
    int getRawValue();
    float getVoltage();
    float getLux();
    String getLightLevel();

private:
    float calculateLux(int rawValue);
    float getAverageVoltage();
};

#endif // LDR_SENSOR_H

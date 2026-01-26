/**
 * @file SoilMoistureSensor.h
 * @brief Soil moisture sensor control
 * @author Your Name
 * @version 2.0
 */

#ifndef SOIL_MOISTURE_SENSOR_H
#define SOIL_MOISTURE_SENSOR_H

#include "../config.h"
#include <Arduino.h>

class SoilMoistureSensor
{
private:
    uint8_t pin;
    int rawValue;
    float voltage;
    float moisturePercentage;
    int dryValue;
    int wetValue;
    int samples;
    int sampleIndex;
    int *sampleBuffer;

public:
    SoilMoistureSensor(uint8_t sensorPin, int sampleCount = 10);
    ~SoilMoistureSensor();

    bool begin();
    bool readMoisture();
    int getRawValue();
    float getVoltage();
    float getMoisturePercentage();
    String getMoistureLevel();

    void calibrateDry(int dryReading);
    void calibrateWet(int wetReading);

private:
    float calculateMoisture(int rawValue);
    int getAverageRaw();
};

#endif // SOIL_MOISTURE_SENSOR_H

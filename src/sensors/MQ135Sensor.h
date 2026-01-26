/**
 * @file MQ135Sensor.h
 * @brief MQ135 Air Quality sensor control
 * @author Your Name
 * @version 2.0
 */

#ifndef MQ135_SENSOR_H
#define MQ135_SENSOR_H

#include "../config.h"
#include <Arduino.h>

class MQ135Sensor
{
private:
    uint8_t pin;
    int rawValue;
    float voltage;
    float resistance;
    float ppm;
    float r0; // Clean air resistance
    int samples;
    int sampleIndex;
    int *sampleBuffer;

    // Calibration values for different gases
    struct GasCalibration
    {
        float a; // Scale factor
        float b; // Exponent
        String name;
    };

    GasCalibration nh3;     // Ammonia
    GasCalibration co;      // Carbon Monoxide
    GasCalibration nox;     // Nitrogen Oxides
    GasCalibration alcohol; // Alcohol
    GasCalibration smoke;   // Smoke

public:
    MQ135Sensor(uint8_t sensorPin, int sampleCount = 10);
    ~MQ135Sensor();

    bool begin();
    bool readAirQuality();
    int getRawValue();
    float getVoltage();
    float getResistance();
    float getPPM();
    String getAirQualityLevel();

    void calibrateR0(float knownR0 = 0.0f);
    float calculatePPM(float ratio, GasCalibration gas);
    float getNH3PPM();
    float getCOPPM();
    float getNOxPPM();
    float getAlcoholPPM();
    float getSmokePPM();

private:
    float calculateResistance(int rawValue);
    int getAverageRaw();
    void initializeCalibration();
};

#endif // MQ135_SENSOR_H

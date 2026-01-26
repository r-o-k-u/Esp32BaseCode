/**
 * @file BMPSensor.h
 * @brief BMP280 pressure sensor interface
 * @author Your Name
 * @version 2.0
 */

#ifndef BMP_SENSOR_H
#define BMP_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

// BMP280 sampling modes
enum BMPOversampling
{
    BMP_OVERSAMPLING_SKIPPED = 0,
    BMP_OVERSAMPLING_X1 = 1,
    BMP_OVERSAMPLING_X2 = 2,
    BMP_OVERSAMPLING_X4 = 3,
    BMP_OVERSAMPLING_X8 = 4,
    BMP_OVERSAMPLING_X16 = 5
};

class BMPSensor
{
private:
    Adafruit_BMP280 *bmp;
    uint8_t i2cAddress;
    bool initialized;

    // Sensor data
    float lastTemperature;
    float lastPressure;
    float lastAltitude;
    float seaLevelPressure;

    // Statistics
    uint32_t lastReadTime;
    uint32_t readCount;
    uint32_t errorCount;

public:
    BMPSensor(uint8_t addr = 0x76);
    ~BMPSensor();

    bool begin(uint8_t sdaPin, uint8_t sclPin);
    bool read();

    // Data access
    float getTemperature();
    float getPressure();
    float getAltitude();
    float calculateAltitude(float pressure, float seaLevel);

    // Configuration
    void setSeaLevelPressure(float pressure);
    void configureSampling(BMPOversampling tempSampling,
                           BMPOversampling pressSampling,
                           uint8_t filter);

    // Status and statistics
    uint32_t getTimeSinceLastRead();
    float getSuccessRate();
    void resetStatistics();
    bool selfTest();

    // Information
    String getStatusString();
    void printInfo();

    // Getters
    bool isInitialized() { return initialized; }
    uint8_t getAddress() { return i2cAddress; }
    uint32_t getReadCount() { return readCount; }
    uint32_t getErrorCount() { return errorCount; }
};

#endif // BMP_SENSOR_H

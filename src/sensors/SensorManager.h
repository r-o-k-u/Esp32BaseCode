/**
 * @file SensorManager.h
 * @brief Sensor manager for ESP32
 *
 * Handles multiple sensors including DHT, BMP, PIR, etc.
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

class SensorManager
{
private:
    bool initialized;

public:
    SensorManager();

    // Initialization
    bool begin();

    // Sensor reading
    void getAllSensorData(JsonDocument &doc);
    float getTemperature();
    float getHumidity();
    float getPressure();
    float getAltitude();
    bool getMotion();
    int getLightLevel();
    int getSoilMoisture();

    // Utility
    void printStatus();
    uint8_t getSensorCount();
};

extern SensorManager sensorManager; // Global instance

#endif // SENSOR_MANAGER_H
